#include "cbase.h"
#include "rd_demo_utils.h"
#include "filesystem.h"
#include "matchmaking/imatchframework.h"
#include "demofile/demoformat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// this is FCVAR_USERINFO so that rd_force_all_marines_in_pvs can check its value server-side
ConVar rd_auto_record_lobbies( "rd_auto_record_lobbies", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Automatically keep this many lobby recordings. Executes the record and stop commands automatically." );
ConVar rd_auto_record_stop_on_retry( "rd_auto_record_stop_on_retry", "1", FCVAR_ARCHIVE, "Treat an instant restart the same way as a loading screen for auto recordings." );
ConVar rd_auto_record_debug( "rd_auto_record_debug", "0", FCVAR_NONE );
ConVar rd_auto_record_auto_fix_times( "rd_auto_record_auto_fix_times", "1", FCVAR_NONE, "If a demo has a negative duration, automatically fix the file's header." );

CRD_Auto_Record_System g_RD_Auto_Record_System;

CRD_Auto_Record_System::CRD_Auto_Record_System() : CAutoGameSystemPerFrame( "CRD_Auto_Record_System" )
{
	m_bStartedRecording = false;
	m_bJustConnected = false;
}

void CRD_Auto_Record_System::LevelInitPostEntity()
{
	Assert( !m_bStartedRecording );

	m_bJustConnected = true;
}

void CRD_Auto_Record_System::LevelShutdownPreEntity()
{
	if ( m_bStartedRecording )
	{
		if ( engine->IsRecordingDemo() )
		{
			if ( rd_auto_record_debug.GetBool() )
				Msg( "[Auto Record] Executing command: stop\n" );

			engine->ClientCmd_Unrestricted( "stop\n" );
		}
		else
		{
			// if we're disconnecting, the demo has already been stopped by the engine
			if ( rd_auto_record_debug.GetBool() )
				Msg( "[Auto Record] Demo recording was stopped by engine.\n" );
		}

		m_bStartedRecording = false;
	}
}

void CRD_Auto_Record_System::Update( float frametime )
{
	if ( m_bJustConnected )
	{
		m_bJustConnected = false;

		if ( engine->IsPlayingDemo() )
		{
			if ( rd_auto_record_debug.GetBool() )
				Msg( "[Auto Record] Playing a demo; ignoring level load.\n" );

			return;
		}

		if ( !g_pMatchFramework || !g_pMatchFramework->GetMatchSession() || !g_pMatchFramework->GetMatchSession()->GetSessionSettings() )
		{
			if ( rd_auto_record_debug.GetBool() )
				Msg( "[Auto Record] Not in a match framework session; ignoring level load.\n" );

			return;
		}

		if ( rd_auto_record_lobbies.GetInt() == 0 )
		{
			if ( rd_auto_record_debug.GetBool() )
				Msg( "[Auto Record] Number of requested auto-recordings is zero. Not touching anything.\n" );

			return;
		}

		if ( engine->IsRecordingDemo() )
		{
			if ( rd_auto_record_debug.GetBool() )
				Msg( "[Auto Record] Already recording a demo. Assuming manual demo and not touching anything.\n" );

			return;
		}

		CleanRecordingsFolder( true );

		struct tm time;
		Plat_GetLocalTime( &time );

		char szCmd[255];
		V_snprintf( szCmd, sizeof( szCmd ), "record \"recordings/rd-auto-%04d%02d%02d-%02d%02d%02d-%s\"\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, MapName() );
		V_strlower( szCmd );

		if ( rd_auto_record_debug.GetBool() )
			Msg( "[Auto Record] Executing command: %s", szCmd );

		engine->ClientCmd_Unrestricted( szCmd );

		m_bStartedRecording = true;

		return;
	}

	if ( m_bStartedRecording && !engine->IsRecordingDemo() )
	{
		Msg( "[Auto Record] Demo recording stopped manually or unexpectedly.\n" );

		m_bStartedRecording = false;
	}
}

struct AutoRecording_t
{
	int year, month, day;
	int hour, minute, second;
	char map[MAX_MAP_NAME];

	static int __cdecl Compare( const AutoRecording_t *a, const AutoRecording_t *b )
	{
		if ( a->year != b->year )
			return a->year - b->year;

		if ( a->month != b->month )
			return a->month - b->month;

		if ( a->day != b->day )
			return a->day - b->day;

		if ( a->hour != b->hour )
			return a->hour - b->hour;

		if ( a->minute != b->minute )
			return a->minute - b->minute;

		if ( a->second != b->second )
			return a->second - b->second;

		return 0;
	}
};

void CRD_Auto_Record_System::CleanRecordingsFolder( bool bLeaveEmptySlot )
{
	g_pFullFileSystem->CreateDirHierarchy( "recordings", "MOD" );

	if ( rd_auto_record_lobbies.GetInt() <= 0 )
	{
		if ( rd_auto_record_debug.GetBool() )
			Msg( "[Auto Record] Desired recording count is not positive. Not cleaning folder.\n" );

		return;
	}

	if ( rd_auto_record_debug.GetBool() )
		Msg( "[Auto Record] Cleaning recordings folder...\n" );

	CUtlVector<AutoRecording_t> recordings;

	FileFindHandle_t hFind;
	for ( const char *szName = g_pFullFileSystem->FindFirstEx( "recordings/rd-auto-*.dem", "MOD", &hFind ); szName; szName = g_pFullFileSystem->FindNext( hFind ) )
	{
		if ( V_strlen( szName ) <= 28 )
		{
			// Too short to be an auto-generated demo name.
			continue;
		}

		char num[5];
		AutoRecording_t rec;

		if ( szName[16] != '-' )
		{
#ifdef DBGFLAG_ASSERT
			// FIXME: this code stops working after the year 9999. you know, just in case we're still playing AS:RD then.
			struct tm time;
			Plat_GetLocalTime( &time );
			Assert( time.tm_year < 8100 );
#endif
			continue;
		}

		if ( szName[23] != '-' )
		{
			continue;
		}

		num[0] = szName[8];
		num[1] = szName[9];
		num[2] = szName[10];
		num[3] = szName[11];
		num[4] = '\0';
		rec.year = V_atoi( num );

		num[2] = '\0';

#define TWO_DIGITS( var, offset ) \
		num[0] = szName[offset]; \
		num[1] = szName[offset + 1]; \
		rec.var = V_atoi( num )

		TWO_DIGITS( month, 12 );
		TWO_DIGITS( day, 14 );
		TWO_DIGITS( hour, 17 );
		TWO_DIGITS( minute, 19 );
		TWO_DIGITS( second, 21 );

		char szMap[MAX_MAP_NAME + 4];
		V_strncpy( szMap, szName + 24, sizeof( szMap ) );
		V_StripExtension( szMap, rec.map, sizeof( rec.map ) );

		recordings.AddToTail( rec );
	}

	g_pFullFileSystem->FindClose( hFind );

	int nToRemove = recordings.Count() - rd_auto_record_lobbies.GetInt() + ( bLeaveEmptySlot ? 1 : 0 );

	if ( nToRemove <= 0 )
	{
		if ( rd_auto_record_debug.GetBool() )
			Msg( "[Auto Record] %d recordings is below limit. Don't need to delete any.\n", recordings.Count() );

		return;
	}

	recordings.Sort( &AutoRecording_t::Compare );

	if ( rd_auto_record_debug.GetBool() )
		Msg( "[Auto Record] Deleting %d oldest auto-recordings.\n", nToRemove );

	for ( int i = 0; i < nToRemove; i++ )
	{
		const AutoRecording_t &rec = recordings[i];

		char szName[MAX_PATH];
		V_snprintf( szName, sizeof( szName ), "recordings/rd-auto-%04d%02d%02d-%02d%02d%02d-%s.dem", rec.year, rec.month, rec.day, rec.hour, rec.minute, rec.second, rec.map );

		if ( rd_auto_record_debug.GetBool() )
			Msg( "[Auto Record] Deleting recording %s.\n", szName );

		g_pFullFileSystem->RemoveFile( szName, "MOD" );
	}
}

void CRD_Auto_Record_System::RecomputeDemoDuration( const char *szName, bool bForce )
{
	FileHandle_t hFile = g_pFullFileSystem->Open( szName, "r+b", "MOD" );
	if ( !hFile )
	{
		Warning( "[Auto Record] Failed to open file %s\n", szName );
		return;
	}

#define READ( var ) \
	if ( g_pFullFileSystem->Read( &var, sizeof( var ), hFile ) != sizeof( var ) ) \
	{ \
		g_pFullFileSystem->Close( hFile ); \
		Warning( "[Auto Record] Failed to read file %s\n", szName ); \
		return; \
	}

	demoheader_t header;
	READ( header );

	ByteSwap_demoheader_t( header );

	if ( memcmp( header.demofilestamp, DEMO_HEADER_ID, 8 ) )
	{
		g_pFullFileSystem->Close( hFile );
		Warning( "[Auto Record] File %s is not a demo.\n", szName );
		return;
	}

	if ( header.demoprotocol != DEMO_PROTOCOL )
	{
		g_pFullFileSystem->Close( hFile );
		Warning( "[Auto Record] Unsupported demo version %d in file %s.\n", header.demoprotocol, szName );
		return;
	}

	if ( header.playback_time >= 0 && !bForce )
	{
		g_pFullFileSystem->Close( hFile );
		Msg( "[Auto Record] File %s already has positive duration %f.\n", szName, header.playback_time );
		return;
	}

	int nTickRate = RoundFloatToInt( header.playback_ticks / header.playback_time );

	g_pFullFileSystem->Seek( hFile, header.signonlength, FILESYSTEM_SEEK_CURRENT );

	bool bFoundStopCmd = false;
	int nMaxTick = 0;
	while ( !bFoundStopCmd )
	{
#pragma pack(push, 1)
		struct
		{
			uint8_t cmd;
			int tick;
			int8_t playerslot;
		} cmdheader;
#pragma pack(pop)

		int nRead = g_pFullFileSystem->Read( &cmdheader, sizeof( cmdheader ), hFile );
		if ( nRead == 0 )
		{
			Msg( "[Auto Record] Fixing missing dem_stop command at end of %s.\n", nMaxTick, szName );
			cmdheader.cmd = dem_stop;
			cmdheader.tick = nMaxTick;
			cmdheader.playerslot = 0;
			g_pFullFileSystem->Write( &cmdheader, 6, hFile );
			bFoundStopCmd = true;
			break;
		}

		if ( nRead != sizeof( cmdheader ) )
		{
			g_pFullFileSystem->Close( hFile );
			Warning( "[Auto Record] Failed to read file %s\n", szName );
			return;
		}

		nMaxTick = MAX( nMaxTick, cmdheader.tick );

		switch ( cmdheader.cmd )
		{
		case dem_synctick:
		{
			break;
		}
		case dem_signon:
		case dem_packet:
		{
			democmdinfo_t cmdinfo;

			READ( cmdinfo );

			int sequence[2];

			READ( sequence );

			int size;

			READ( size );

			g_pFullFileSystem->Seek( hFile, size, FILESYSTEM_SEEK_CURRENT );

			break;
		}
		case dem_consolecmd:
		case dem_datatables:
		case dem_customdata:
		case dem_stringtables:
		{
			int size;

			READ( size );

			g_pFullFileSystem->Seek( hFile, size, FILESYSTEM_SEEK_CURRENT );
			break;
		}
		case dem_usercmd:
		{
			int cmdnum, size;

			READ( cmdnum );
			READ( size );

			g_pFullFileSystem->Seek( hFile, size, FILESYSTEM_SEEK_CURRENT );
			break;
		}
		case dem_stop:
		{
			bFoundStopCmd = true;
			break;
		}
		default:
		{
			g_pFullFileSystem->Close( hFile );
			Warning( "[Auto Record] Unhandled demo command number %d in %s\n", cmdheader.cmd, szName );
			return;
		}
		}
	}

	header.playback_ticks = nMaxTick;
	header.playback_time = ( float )nMaxTick / ( float )nTickRate;
	Msg( "[Auto Record] Recomputed duration to %f for demo %s.\n", header.playback_time, szName );

	g_pFullFileSystem->Seek( hFile, 0, FILESYSTEM_SEEK_HEAD );
	g_pFullFileSystem->Write( &header, sizeof( header ), hFile );

	g_pFullFileSystem->Close( hFile );
}

CON_COMMAND( rd_auto_record_force_clean, "Force a cleanup of the recordings directory" )
{
	g_RD_Auto_Record_System.CleanRecordingsFolder( false );
}

CON_COMMAND( rd_auto_record_fix_time, "Recompute the duration of a demo file." )
{
	if ( args.ArgC() != 2 )
	{
		Msg( "Usage: rd_auto_record_fix_time [file]\n" );
		return;
	}

	g_RD_Auto_Record_System.RecomputeDemoDuration( args.Arg( 1 ), false );
}
