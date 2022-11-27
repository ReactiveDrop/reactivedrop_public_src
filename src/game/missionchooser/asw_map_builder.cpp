#include "convar.h"
#include "asw_map_builder.h"
#include "missionchooser/iasw_mission_chooser_source.h"
#include "missionchooser/iasw_random_missions.h"
#include "filesystem.h"
#include "threadtools.h"
#include "KeyValues.h"
#include "asw_mission_chooser.h"
#include "vstdlib/random.h"
#include "tilegen_core.h"
#include "MapLayout.h"
#include "layout_system/tilegen_layout_system.h"
#include "LevelTheme.h"
#include "VMFExporter.h"
#include "Room.h"
#include "cdll_int.h"

// includes needed for the creating of a new process and handling its output
// ASW TODO: Handle Linux/Xbox way of doing this
#pragma warning( disable : 4005 )
#include <windows.h>
#include <iostream>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// @TODO: Nuke all of the switches for VBSP1 vs VBSP2 and old mission system vs new mission system.  Move level generation to the background thread.

static ConVar tilegen_retry_count( "tilegen_retry_count", "20", FCVAR_CHEAT, "The number of level generation retries to attempt after which tilegen will give up." );

DEFINE_LOGGING_CHANNEL_NO_TAGS( LOG_TilegenGeneral, "TilegenGeneral" );

extern IVEngineClient *engine;

static const int g_ProgressAmounts[] = 
{
	0,		// Not yet started
	1,		// Initialized
	15,		// Map loaded
	30,		// Instances resolved
	70,		// BSP created
	100,	// BSP saved, all done.
};

static const char *g_ProgressLabels[] = 
{
	"Initializing...",
	"Loading map...",
	"Resolving Instances...",
	"Creating BSP...",
	"Saving BSP...",
	"Done!"
};

CASW_Map_Builder::CASW_Map_Builder() :
m_flStartProcessingTime( 0.0f ),
m_iBuildStage( STAGE_NONE ),
m_bStartedGeneration( false ),
m_flProgress( 0.0f ),
m_pGeneratedMapLayout( NULL ),
m_pBuildingMapLayout( NULL ),
m_pLayoutSystem( NULL ),
m_nLevelGenerationRetryCount( 0 ),
m_pMissionSettings( NULL ),
m_pMissionDefinition( NULL )
{
	m_szLayoutName[0] = '\0';
	m_iCurrentBuildSearch = 0;
	m_bRunningProcess = false;
	m_bFinishedExecution = false;	

	V_snprintf( m_szStatusMessage, sizeof( m_szStatusMessage ), "Generating map..." );

	m_pMapBuilderOptions = new KeyValues( "map_builder_options" );
	m_pMapBuilderOptions->LoadFromFile( g_pFullFileSystem, "resource/map_builder_options.txt", "GAME" );
}

CASW_Map_Builder::~CASW_Map_Builder()
{
	m_pMapBuilderOptions->deleteThis();

	delete m_pGeneratedMapLayout;
	delete m_pBuildingMapLayout;
	delete m_pLayoutSystem;
}

void CASW_Map_Builder::Update( float flEngineTime )
{
	if ( m_bRunningProcess )
	{
		ProcessExecution();
	}
	else if ( m_iBuildStage == STAGE_MAP_BUILD_SCHEDULED )
	{
		if ( m_flStartProcessingTime < flEngineTime )
		{
			BuildMap();
		}
	}
	else if ( m_iBuildStage == STAGE_GENERATE )
	{
		if ( m_flStartProcessingTime < flEngineTime )
		{
			if ( !m_bStartedGeneration )
			{
				delete m_pGeneratedMapLayout;
				delete m_pLayoutSystem;
				m_pLayoutSystem = new CLayoutSystem();
				AddListeners( m_pLayoutSystem );
				m_pGeneratedMapLayout = new CMapLayout( m_pMissionSettings->MakeCopy() );

				if ( !m_pLayoutSystem->LoadFromKeyValues( m_pMissionDefinition ) )
				{
					Log_Warning( LOG_TilegenLayoutSystem, "Failed to load mission from key values definition.\n" );
					m_iBuildStage = STAGE_NONE;
					return;
				}
				m_pLayoutSystem->BeginGeneration( m_pGeneratedMapLayout );
				m_bStartedGeneration = true;
			}
			else
			{
				if ( m_pLayoutSystem->IsGenerating() )
				{
					m_pLayoutSystem->ExecuteIteration();

					// If an error occurred and this map is randomly generated, try again and hope we get a successful layout this time.
					if ( m_pLayoutSystem->GenerationErrorOccurred() )
					{
						if ( m_nLevelGenerationRetryCount < tilegen_retry_count.GetInt() && m_pLayoutSystem->IsRandomlyGenerated() )
						{
							// Error generating layout
							Log_Msg( LOG_TilegenGeneral, "Retrying layout generation...\n" );
							m_pGeneratedMapLayout->Clear();
							m_pLayoutSystem->BeginGeneration( m_pGeneratedMapLayout );
							++ m_nLevelGenerationRetryCount;
						}
						else
						{
							Log_Warning( LOG_TilegenGeneral, "Failed to generate valid map layout after %d tries...\n", tilegen_retry_count.GetInt() );
							m_iBuildStage = STAGE_NONE;
						}
					}
				}
				else
				{
					Log_Msg( LOG_TilegenGeneral, "Map layout generated\n" );
					m_iBuildStage = STAGE_NONE;
					
					char layoutFilename[MAX_PATH];
					Q_snprintf( layoutFilename, MAX_PATH, "maps\\%s", m_szLayoutName );
					m_pGeneratedMapLayout->SaveMapLayout( layoutFilename );

					delete m_pGeneratedMapLayout;
					m_pGeneratedMapLayout = NULL;

					BuildMap();
				}
			}
		}
	}
}

bool CASW_Map_Builder::IsBuildingMission()
{
	return m_iBuildStage != STAGE_NONE;
}

void CASW_Map_Builder::Execute(const char* pszCmd, const char* pszCmdLine)
{
	m_bFinishedExecution = false;
	m_iProcessReturnValue = -1;
	SECURITY_ATTRIBUTES saAttr; 

	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child's STDOUT. 
	if(CreatePipe(&m_hChildStdoutRd, &m_hChildStdoutWr, &saAttr, 0))
	{
		if(CreatePipe(&m_hChildStdinRd, &m_hChildStdinWr, &saAttr, 0))
		{
			if (DuplicateHandle(GetCurrentProcess(),m_hChildStdoutWr, GetCurrentProcess(),&m_hChildStderrWr,0, TRUE,DUPLICATE_SAME_ACCESS))
			{
				/* Now create the child process. */ 
				STARTUPINFO si;
				memset(&si, 0, sizeof si);
				si.cb = sizeof(si);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdInput = m_hChildStdinRd;
				si.hStdError = m_hChildStderrWr;
				si.hStdOutput = m_hChildStdoutWr;
				PROCESS_INFORMATION pi;
				char cmdbuffer[512];
				Q_snprintf(cmdbuffer, sizeof(cmdbuffer), "%s %s", pszCmd, pszCmdLine);
				Msg("Sending command %s\n", cmdbuffer);
				// run the process from the current game's map directory				
				char dirbuffer[512];
				Q_snprintf(dirbuffer, sizeof(dirbuffer), "%s/maps", engine->GetGameDirectory() );
				Msg("  from directory %s\n", dirbuffer);

				if(CreateProcess(pszCmd, cmdbuffer, NULL, NULL, TRUE, 
					DETACHED_PROCESS | BELOW_NORMAL_PRIORITY_CLASS, NULL, dirbuffer, &si, &pi))
				{
					m_hProcess = pi.hProcess;
					m_bRunningProcess = true;
					m_bFinishedExecution = false;

					// do one process of the execution
					ProcessExecution();					
				}
				else
				{
					Msg("* Could not execute the command:\r\n   %s\r\n", cmdbuffer);
					m_bRunningProcess = false;
					FinishExecution();	// closes all handles
				}								
			}
			else
			{
				// close the 4 handles we've opened so far
				CloseHandle(m_hChildStdinRd);
				CloseHandle(m_hChildStdinWr);
				CloseHandle(m_hChildStdoutRd);
				CloseHandle(m_hChildStdoutWr);
			}
		}
		else
		{
			// close the 2 handles we've opened so far
			CloseHandle(m_hChildStdoutRd);
			CloseHandle(m_hChildStdoutWr);
		}
	}
}

void CASW_Map_Builder::ProcessExecution()
{
	DWORD dwCount = 0;
	DWORD dwRead = 0;

	// read from input handle
	PeekNamedPipe(m_hChildStdoutRd, NULL, NULL, NULL, &dwCount, NULL);
	if (dwCount)
	{
		dwCount = MIN (dwCount, 4096 - 1);
		ReadFile(m_hChildStdoutRd, m_szProcessBuffer, dwCount, &dwRead, NULL);
	}
	if(dwRead)
	{
		m_szProcessBuffer[dwRead] = 0;
		UpdateProgress();
		Msg(m_szProcessBuffer);
	}
	// check process termination
	else if ( WaitForSingleObject(m_hProcess, 1000) != WAIT_TIMEOUT )
	{
		if(m_bFinishedExecution)
		{
			m_iProcessReturnValue = 0;
			FinishExecution();
		}
		else
		{
			m_bFinishedExecution = true;
		}
	}	
}

// called when one of our processes finishes
void CASW_Map_Builder::FinishExecution()
{
	m_bRunningProcess = false;	// next time we get it

	CloseHandle(m_hChildStderrWr);

	CloseHandle(m_hChildStdinRd);
	CloseHandle(m_hChildStdinWr);

	CloseHandle(m_hChildStdoutRd);
	CloseHandle(m_hChildStdoutWr);

	if (m_iBuildStage == STAGE_VBSP && m_iProcessReturnValue == 0)
	{
		char buffer[512];
		Q_snprintf(buffer, sizeof(buffer), "-game ..\\ %s %s", m_pMapBuilderOptions->GetString( "vvis", "" ), m_szLayoutName);	// todo: add code to chop into 256 blocks here
		Execute("bin/vvis.exe", buffer);
		m_iBuildStage = STAGE_VVIS;
	}
	else if (m_iBuildStage == STAGE_VVIS && m_iProcessReturnValue == 0)
	{
		if ( !m_pMapBuilderOptions->FindKey( "vrad", false ) )		// if no vrad key is specified in the map builder options, then skip vrad
		{
			m_iBuildStage = STAGE_NONE;
			Msg("Map Build finished!\n");
			m_flProgress = 1.0f;
			Q_snprintf( m_szStatusMessage, sizeof( m_szStatusMessage ), "Build complete!" );
		}
		else
		{
			char buffer[512];
			Q_snprintf(buffer, sizeof(buffer), "-low -game ..\\ %s %s", m_pMapBuilderOptions->GetString( "vrad", "" ), m_szLayoutName);
			Execute("bin/vrad.exe", buffer);
			m_iBuildStage = STAGE_VRAD;
		}
	}
	else if (m_iBuildStage == STAGE_VRAD && m_iProcessReturnValue == 0)
	{
		m_iBuildStage = STAGE_NONE;
		Msg("Map Build finished!\n");
		m_flProgress = 1.0f;
		Q_snprintf( m_szStatusMessage, sizeof( m_szStatusMessage ), "Build complete!" );
	}
	if (m_iProcessReturnValue == -1)
	{
		//Msg("Map Build error\n");
	}
}

// search terms used to work out how far through the build we are
static char const *s_szProgressTerms[]={
	"Valve Software - vbsp.exe",
	"ProcessBlock_Thread:",
	"Processing areas",
	"WriteBSP...",	
	"Displacement Alpha",
	"Building Physics collision data",
	"Valve Software - vvis.exe",
	"BasePortalVis:",
	"PortalFlow:",
	"Valve Software - vrad.exe",
	"BuildFacelights:",
	"FinalLightFace:",
	"ThreadComputeLeafAmbient:",
	"Computing static prop lighting"
};

static char const *s_szStatusLabels[]={
	"Creating BSP...",
	"Creating BSP...",
	"Creating BSP...",
	"Creating BSP...",
	"Creating BSP...",
	"Creating BSP...",
	"Calculating visibility...",
	"Calculating visibility...",
	"Calculating visibility...",
	"Calculating lighting...",
	"Calculating lighting...",
	"Calculating lighting...",
	"Calculating lighting...",
	"Calculating prop lighting...",
};

// monitor output from our process to determine which part of the build we're in
void CASW_Map_Builder::UpdateProgress()
{
	// copy the new chars into our buffer
	int newcharslen = Q_strlen(m_szProcessBuffer);
	for (int i=0;i<newcharslen;i++)
	{
		m_szOutputBuffer[m_iOutputBufferPos++] = m_szProcessBuffer[i];
		// if we go over the end of our output buffer, then shift everything back by half the buffer and continue
		if (m_iOutputBufferPos >= MAP_BUILD_OUTPUT_BUFFER_SIZE)
		{
			for (int k=0;k<MAP_BUILD_OUTPUT_BUFFER_HALF_SIZE;k++)
			{
				m_szOutputBuffer[k] = m_szOutputBuffer[k + MAP_BUILD_OUTPUT_BUFFER_HALF_SIZE];
			}
			m_iOutputBufferPos = MAP_BUILD_OUTPUT_BUFFER_HALF_SIZE;
		}
	}

	// now scan our buffer for progress messages in reverse order
	int iNumSearch = NELEMS(s_szProgressTerms);
	for (int iSearch=iNumSearch-1;iSearch>m_iCurrentBuildSearch;iSearch--)
	{
		char *pos = Q_strstr(m_szOutputBuffer, s_szProgressTerms[iSearch]);
		if ( pos )
		{
			//Msg("Output (%s) matched (%s) result %s at %d\n", m_szOutputBuffer, s_szProgressTerms[iSearch], pos, pos - m_szOutputBuffer);
			m_iCurrentBuildSearch = iSearch;
			m_flProgress = float(iSearch) / float (iNumSearch);
			if (Q_strlen(s_szStatusLabels[iSearch]) > 0)
			{
				Q_snprintf( m_szStatusMessage, sizeof(m_szStatusMessage), "%s", s_szStatusLabels[iSearch] );

			}
			break;
		}
	}
}

// schedules a map to be compiled
void CASW_Map_Builder::ScheduleMapBuild(const char* pszMap, const float fTime)
{
	if ( m_iBuildStage != STAGE_NONE )
	{
		Log_Warning( LOG_TilegenGeneral, "Map builder is currently busy, ignoring request to schedule map build for map '%s'", pszMap );
		return;
	}

	Q_strncpy( m_szLayoutName, Q_UnqualifiedFileName( pszMap ), _countof( m_szLayoutName ) );
	Q_SetExtension( m_szLayoutName, "layout", _countof( m_szLayoutName ) );
	m_flStartProcessingTime = fTime;
	m_bStartedGeneration = false;
	m_iBuildStage = STAGE_MAP_BUILD_SCHEDULED;
	m_flProgress = 0.0f;

	Q_snprintf( m_szStatusMessage, sizeof( m_szStatusMessage ), "Generating map..." );
}

// schedules a map to be randomly generated
void CASW_Map_Builder::ScheduleMapGeneration( const char* pszMap, const float fTime, KeyValues *pMissionSettings, KeyValues *pMissionDefinition )
{
	if ( m_iBuildStage != STAGE_NONE )
	{
		Log_Warning( LOG_TilegenGeneral, "Map builder is currently busy, ignoring request to schedule map generation for map '%s'", pszMap );
		return;
	}

	Q_strncpy( m_szLayoutName, Q_UnqualifiedFileName( pszMap ), _countof( m_szLayoutName ) );
	Q_SetExtension( m_szLayoutName, "layout", _countof( m_szLayoutName ) );
	m_pMissionSettings = pMissionSettings;
	m_pMissionDefinition = pMissionDefinition;
	m_flStartProcessingTime = fTime;
	m_iBuildStage = STAGE_GENERATE;
	m_bStartedGeneration = false;
	m_nLevelGenerationRetryCount = 0;
	m_flProgress = 0.0f;

	Q_snprintf( m_szStatusMessage, sizeof( m_szStatusMessage ), "Generating map..." );
}

// Builds a map from a .layout file
void CASW_Map_Builder::BuildMap()
{
	char layoutFilename[MAX_PATH];
	char vmfFilename[MAX_PATH];
	
	Q_snprintf( layoutFilename, MAX_PATH, "maps\\%s", m_szLayoutName );
	Q_strncpy( vmfFilename, m_szLayoutName, MAX_PATH );
	Q_SetExtension( vmfFilename, "vmf", MAX_PATH );

	Log_Msg( LOG_TilegenGeneral, "Building map from layout: %s, emitting map file: %s\n", layoutFilename, vmfFilename );

	// Make sure our themes are loaded
	CLevelTheme::LoadLevelThemes();

	// Load the .layout from disk
	// @TODO: keep this in memory and avoid the round-trip
	delete m_pBuildingMapLayout;
	m_pBuildingMapLayout = new CMapLayout();
	if ( !m_pBuildingMapLayout->LoadMapLayout( layoutFilename ) )
	{
		delete m_pBuildingMapLayout;
		m_pBuildingMapLayout = NULL;
		return;
	}

	// Export it to VMF
	VMFExporter *pExporter = new VMFExporter();
	bool bSuccess = pExporter->ExportVMF( m_pBuildingMapLayout, m_szLayoutName );
	delete pExporter;

	if ( !bSuccess )
	{
		Log_Warning( LOG_TilegenGeneral, "Failed to create VMF from layout '%s'.\n", m_szLayoutName );
		delete m_pBuildingMapLayout;
		m_pBuildingMapLayout = NULL;
	}

	// Building map layout is ignored in VBSP1 codepath
	delete m_pBuildingMapLayout;
	m_pBuildingMapLayout = NULL;

	m_iBuildStage = STAGE_VBSP;

	char buffer[512];
	Q_snprintf( buffer, sizeof(buffer), "-game ..\\ %s %s", m_pMapBuilderOptions->GetString( "vbsp", "" ), vmfFilename );
	Execute( "bin/vbsp.exe", buffer );

	m_iCurrentBuildSearch = 0;
	m_iOutputBufferPos = 0;
	Q_memset( &m_szOutputBuffer, 0, sizeof( m_szOutputBuffer ) );
}
