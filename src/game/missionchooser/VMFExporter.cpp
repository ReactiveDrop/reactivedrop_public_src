#include "VMFExporter.h"
#include "KeyValues.h"
#include "fgdlib/fgdlib.h"
#include "tier2/fileutils.h"
#include "mathlib/mathlib.h"

#include "TileSource/RoomTemplate.h"
#include "TileSource/Room.h"
#include "TileSource/LevelTheme.h"
#include "TileSource/MapLayout.h"
#include "TileGenDialog.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

ConVar rd_tilegen_seal_material( "rd_tilegen_seal_material", "TOOLS/TOOLSSKYBOX", FCVAR_NONE );
ConVar rd_tilegen_seal_texscale( "rd_tilegen_seal_texscale", "4", FCVAR_NONE );
ConVar rd_tilegen_seal_lightmap( "rd_tilegen_seal_lightmap", "256", FCVAR_NONE );
ConVar rd_tilegen_seal_height( "rd_tilegen_seal_height", "2048", FCVAR_NONE );
ConVar rd_tilegen_seal_padding( "rd_tilegen_seal_padding", "32", FCVAR_NONE );

VMFExporter::VMFExporter()
{
	m_pMapLayout = NULL;

	Init();
}

VMFExporter::~VMFExporter()
{
	ClearExportErrors();
}

void VMFExporter::Init()
{
	m_szLastExporterError[0] = '\0';
	m_vecLastPlaneOffset = vec3_origin;
	m_iEntityCount = 1;	// 1 already, from the worldspawn
	m_iSolidCount = 0;
	m_iSideCount = 0;
	m_iNodeCount = 0;
	m_iInstanceCount = 0;

	m_pRoom = NULL;
	m_iCurrentRoom = 0;

	m_iMapExtents_XMin = 0;
	m_iMapExtents_YMin = 0;
	m_iMapExtents_XMax = 0;
	m_iMapExtents_YMax = 0;
	m_pExportKeys = NULL;
	m_vecStartRoomOrigin = vec3_origin;

	ClearExportErrors();

	m_GD.ClearData();
	m_RemoveEntities.Purge();
}

void VMFExporter::ExportError( const char *pMsg, ... )
{
	char msg[4096];
	va_list marker;
	va_start( marker, pMsg );
	V_vsnprintf( msg, sizeof( msg ), pMsg, marker );
	va_end( marker );

	m_ExportErrors.AddToTail( TileGenCopyString( msg ) );
}

void VMFExporter::ClearExportErrors()
{
	m_ExportErrors.PurgeAndDeleteElements();
}

bool VMFExporter::ShowExportErrors()
{
	if ( m_ExportErrors.Count() <= 0 )
		return false;

	char msg[4096]{};
	for ( int i = 0; i < m_ExportErrors.Count(); i++ )
	{
		V_snprintf( msg, sizeof( msg ), "%s\n%s", msg, m_ExportErrors[i] );
	}

	VGUIMessageBox( g_pTileGenDialog, "Export problems:", msg );

	return true;
}

bool VMFExporter::ExportVMF( CMapLayout *pLayout, const char *mapname, bool bPopupWarnings )
{
	m_bPopupWarnings = bPopupWarnings;

	Init();

	m_pMapLayout = pLayout;

	if ( pLayout->m_PlacedRooms.Count() <= 0 )
	{
		V_snprintf( m_szLastExporterError, sizeof( m_szLastExporterError ), "Failed to export: No rooms placed in the map layout!\n" );
		return false;
	}

	// see if we have a start room
	bool bHasStartRoom = false;
	for ( int i = 0; i < pLayout->m_PlacedRooms.Count(); i++ )
	{
		if ( pLayout->m_PlacedRooms[i]->m_pRoomTemplate->IsStartRoom() )
		{
			int half_map_size = MAP_LAYOUT_TILES_WIDE * 0.5f;		// shift back so the middle of our grid is the origin
			m_vecStartRoomOrigin.x = ( pLayout->m_PlacedRooms[i]->m_iPosX - half_map_size ) * ASW_TILE_SIZE;
			m_vecStartRoomOrigin.y = ( pLayout->m_PlacedRooms[i]->m_iPosY - half_map_size ) * ASW_TILE_SIZE;
			bHasStartRoom = true;
			break;
		}
	}

	m_pExportKeys = new KeyValues( "ExportKeys" );

	m_pExportKeys->AddSubKey( GetVersionInfo() );
	m_pExportKeys->AddSubKey( GetDefaultVisGroups() );
	m_pExportKeys->AddSubKey( GetViewSettings() );
	m_pExportWorldKeys = GetDefaultWorldChunk();
	if ( !m_pExportWorldKeys )
	{
		V_snprintf( m_szLastExporterError, sizeof( m_szLastExporterError ), "Failed to save world chunk start\n" );
		return false;
	}
	m_pExportKeys->AddSubKey( m_pExportWorldKeys );


	// save out the big cube the whole level sits in	
	if ( !AddLevelContainer() )
	{
		V_snprintf( m_szLastExporterError, sizeof( m_szLastExporterError ), "Failed to save level container\n" );
		return false;
	}

	// add each room as an instance
	int nLogicalRooms = m_pMapLayout->m_LogicalRooms.Count();
	int nPlacedRooms = m_pMapLayout->m_PlacedRooms.Count();

	m_pRoom = NULL;
	for ( int i = 0; i < nLogicalRooms; ++i )
	{
		AddRoomInstance( m_pMapLayout->m_LogicalRooms[i] );
	}

	for ( int i = 0; i < nPlacedRooms; ++i )
	{
		m_pRoom = m_pMapLayout->m_PlacedRooms[i];
		AddRoomInstance( m_pRoom->m_pRoomTemplate, i );
	}

	// add some player starts to the map in the tile the user selected
	if ( !bHasStartRoom )
	{
		m_pExportKeys->AddSubKey( GetPlayerStarts() );
	}

	// now for the magic: flatten all of the instances
	if ( !FlattenInstances() )
	{
		Warning( "Failed while processing TileGen map!\n" );
		return false;
	}

	m_pExportKeys->AddSubKey( GetDefaultCamera() );

	// save out the export keys
	char filename[512];
	V_snprintf( filename, sizeof( filename ), "maps\\%s", mapname );
	V_SetExtension( filename, "vmf", sizeof( filename ) );
	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	for ( KeyValues *pKey = m_pExportKeys->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
	{
		pKey->RecursiveSaveToFile( buf, 0 );
	}
	if ( !g_pFullFileSystem->WriteFile( filename, "MOD", buf ) )
	{
		Warning( "Failed to SaveToFile %s\n", filename );
		return false;
	}

	// save the map layout there (so the game can get information about rooms during play)
	V_snprintf( filename, sizeof( filename ), "maps\\%s", mapname );
	V_SetExtension( filename, "layout", sizeof( filename ) );
	if ( !m_pMapLayout->SaveMapLayout( filename ) )
	{
		V_snprintf( m_szLastExporterError, sizeof( m_szLastExporterError ), "Failed to save .layout file\n" );
		return false;
	}

	return true;
}

const Vector &VMFExporter::GetCurrentRoomOffset()
{
	static Vector s_vecCurrentRoomOffset = vec3_origin;

	if ( m_pRoom )
	{
		int half_map_size = MAP_LAYOUT_TILES_WIDE * 0.5f;
		s_vecCurrentRoomOffset.x = ( m_pRoom->m_iPosX - half_map_size ) * ASW_TILE_SIZE;
		s_vecCurrentRoomOffset.y = ( m_pRoom->m_iPosY - half_map_size ) * ASW_TILE_SIZE;
		s_vecCurrentRoomOffset.z = 0;
	}
	else
	{
		// place logical rooms at the origin and up above the camera
		s_vecCurrentRoomOffset.x = 0;
		s_vecCurrentRoomOffset.y = 0;
		s_vecCurrentRoomOffset.z = 1024;
	}
	return s_vecCurrentRoomOffset;
}

void VMFExporter::AddRoomInstance( const CRoomTemplate *pRoomTemplate, int nPlacedRoomIndex )
{
	KeyValues *pFuncInstance = new KeyValues( "entity" );
	pFuncInstance->SetInt( "id", ++m_iEntityCount );
	pFuncInstance->SetString( "classname", "func_instance" );
	pFuncInstance->SetString( "angles", "0 0 0" );

	// Used to identify rooms for later fixup (e.g. adding/swapping instances and models)
	if ( nPlacedRoomIndex != -1 )
	{
		pFuncInstance->SetInt( "PlacedRoomIndex", nPlacedRoomIndex );

		char szRoomPrefix[128];
		V_snprintf( szRoomPrefix, sizeof( szRoomPrefix ), "Room%d", nPlacedRoomIndex );
		pFuncInstance->SetString( "targetname", szRoomPrefix );
	}

	if ( m_pRoom )
	{
		pFuncInstance->SetInt( "InstanceSeed", m_pRoom->m_nInstanceSeed );
	}

	char vmfName[MAX_PATH];
	V_snprintf( vmfName, sizeof( vmfName ), "tilegen/roomtemplates/%s/%s.vmf", pRoomTemplate->m_pLevelTheme->m_szName, pRoomTemplate->GetFullName() );
	// Convert backslashes to forward slashes to please the VMF parser
	int nStrLen = V_strlen( vmfName );
	for ( int i = 0; i < nStrLen; ++i )
	{
		if ( vmfName[i] == '\\' ) vmfName[i] = '/';
	}

	pFuncInstance->SetString( "file", vmfName );

	Vector vOrigin = GetCurrentRoomOffset();
	char buf[128];
	V_snprintf( buf, sizeof( buf ), "%f %f %f", vOrigin.x, vOrigin.y, vOrigin.z );
	pFuncInstance->SetString( "origin", buf );
	m_pExportKeys->AddSubKey( pFuncInstance );
}

//-----------------------------------------------------------------------------
// Purpose: Saves the version information chunk.
// Input  : *pFile - 
// Output : Returns ChunkFile_Ok on success, an error code on failure.
//-----------------------------------------------------------------------------
KeyValues *VMFExporter::GetVersionInfo()
{
	KeyValues *pKeys = new KeyValues( "versioninfo" );
	pKeys->SetInt( "editorversion", 400 );
	pKeys->SetInt( "editorbuild", 3900 );
	pKeys->SetInt( "mapversion", 1 );
	pKeys->SetInt( "formatversion", 100 );
	pKeys->SetBool( "prefab", false );
	return pKeys;
}

// just save out an empty visgroups section
KeyValues *VMFExporter::GetDefaultVisGroups()
{
	KeyValues *pKeys = new KeyValues( "visgroups" );
	return pKeys;
}

KeyValues *VMFExporter::GetViewSettings()
{
	KeyValues *pKeys = new KeyValues( "viewsettings" );
	pKeys->SetBool( "bSnapToGrid", true );
	pKeys->SetBool( "bShowGrid", true );
	pKeys->SetBool( "bShowLogicalGrid", false );
	pKeys->SetInt( "nGridSpacing", ASW_TILE_SIZE );
	pKeys->SetBool( "bShow3DGrid", false );
	return pKeys;
}

KeyValues *VMFExporter::GetDefaultCamera()
{
	KeyValues *pKeys = new KeyValues( "cameras" );
	pKeys->SetInt( "activecamera", 0 );

	KeyValues *pSubKeys = new KeyValues( "camera" );
	pSubKeys->SetString( "position", "[135.491 -60.314 364.02]" );
	pSubKeys->SetString( "look", "[141.195 98.7571 151.25]" );
	pKeys->AddSubKey( pSubKeys );

	return pKeys;
}

KeyValues *VMFExporter::GetPlayerStarts()
{
	KeyValues *pKeys = new KeyValues( "entity" );
	pKeys->SetInt( "id", ++m_iEntityCount );
	pKeys->SetString( "classname", "info_player_start" );
	pKeys->SetString( "angles", "0 0 0" );

	// put a single player start in the centre of the player start tile selected
	float player_start_x = 128.0f;
	float player_start_y = 128.0f;
	int half_map_size = MAP_LAYOUT_TILES_WIDE * 0.5f;
	player_start_x += ( m_pMapLayout->m_iPlayerStartTileX - half_map_size ) * ASW_TILE_SIZE;
	player_start_y += ( m_pMapLayout->m_iPlayerStartTileY - half_map_size ) * ASW_TILE_SIZE;

	char buffer[128];
	V_snprintf( buffer, sizeof( buffer ), "%f %f 1.0", player_start_x, player_start_y );
	pKeys->SetString( "origin", buffer );

	return pKeys;
}

KeyValues *VMFExporter::GetDefaultWorldChunk()
{
	KeyValues *pKeys = new KeyValues( "world" );
	pKeys->SetInt( "id", 1 );
	pKeys->SetInt( "mapversion", 1 );
	pKeys->SetString( "classname", "worldspawn" );
	pKeys->SetString( "skyname", "blacksky" );
	pKeys->SetInt( "maxpropscreenwidth", -1 );
	pKeys->SetString( "detailvbsp", "detail.vbsp" );
	pKeys->SetString( "detailmaterial", "detail/detailsprites" );
	pKeys->SetInt( "speedruntime", 180 );
	return pKeys;
}

KeyValues *VMFExporter::CreateBasicSolid( const char *szMaterial, float textureScale, int lightmapScale, float xMin, float yMin, float zMin, float xMax, float yMax, float zMax )
{
	KeyValues *pSolid = new KeyValues( "solid" );
	pSolid->SetInt( "id", ++m_iSolidCount );

	KeyValues::AutoDelete pSide( "side" );
	pSide->SetString( "material", szMaterial );
	pSide->SetInt( "rotation", 0 );
	pSide->SetInt( "lightmapscale", lightmapScale );
	pSide->SetInt( "smoothing_groups", 0 );

	char temp[256];

	// top
	pSide->SetInt( "id", ++m_iSideCount );
	V_snprintf( temp, sizeof( temp ), "[1 0 0 0] %f", textureScale );
	pSide->SetString( "uaxis", temp );
	V_snprintf( temp, sizeof( temp ), "[0 -1 0 0] %f", textureScale );
	pSide->SetString( "vaxis", temp );
	V_snprintf( temp, sizeof( temp ), "(%f %f %f) (%f %f %f) (%f %f %f)", xMin, yMax, zMax, xMax, yMax, zMax, xMax, yMin, zMax );
	pSide->SetString( "plane", temp );
	pSolid->AddSubKey( pSide->MakeCopy() );

	// bottom
	pSide->SetInt( "id", ++m_iSideCount );
	V_snprintf( temp, sizeof( temp ), "[1 0 0 0] %f", textureScale );
	pSide->SetString( "uaxis", temp );
	V_snprintf( temp, sizeof( temp ), "[0 -1 0 0] %f", textureScale );
	pSide->SetString( "vaxis", temp );
	V_snprintf( temp, sizeof( temp ), "(%f %f %f) (%f %f %f) (%f %f %f)", xMin, yMin, zMin, xMax, yMin, zMin, xMax, yMax, zMin );
	pSide->SetString( "plane", temp );
	pSolid->AddSubKey( pSide->MakeCopy() );

	// west
	pSide->SetInt( "id", ++m_iSideCount );
	V_snprintf( temp, sizeof( temp ), "[0 1 0 0] %f", textureScale );
	pSide->SetString( "uaxis", temp );
	V_snprintf( temp, sizeof( temp ), "[0 0 -1 0] %f", textureScale );
	pSide->SetString( "vaxis", temp );
	V_snprintf( temp, sizeof( temp ), "(%f %f %f) (%f %f %f) (%f %f %f)", xMin, yMax, zMax, xMin, yMin, zMax, xMin, yMin, zMin );
	pSide->SetString( "plane", temp );
	pSolid->AddSubKey( pSide->MakeCopy() );

	// east
	pSide->SetInt( "id", ++m_iSideCount );
	V_snprintf( temp, sizeof( temp ), "[0 1 0 0] %f", textureScale );
	pSide->SetString( "uaxis", temp );
	V_snprintf( temp, sizeof( temp ), "[0 0 -1 0] %f", textureScale );
	pSide->SetString( "vaxis", temp );
	V_snprintf( temp, sizeof( temp ), "(%f %f %f) (%f %f %f) (%f %f %f)", xMax, yMax, zMin, xMax, yMin, zMin, xMax, yMin, zMax );
	pSide->SetString( "plane", temp );
	pSolid->AddSubKey( pSide->MakeCopy() );

	// north
	pSide->SetInt( "id", ++m_iSideCount );
	V_snprintf( temp, sizeof( temp ), "[1 0 0 0] %f", textureScale );
	pSide->SetString( "uaxis", temp );
	V_snprintf( temp, sizeof( temp ), "[0 0 -1 0] %f", textureScale );
	pSide->SetString( "vaxis", temp );
	V_snprintf( temp, sizeof( temp ), "(%f %f %f) (%f %f %f) (%f %f %f)", xMax, yMax, zMax, xMin, yMax, zMax, xMin, yMax, zMin );
	pSide->SetString( "plane", temp );
	pSolid->AddSubKey( pSide->MakeCopy() );

	// south
	pSide->SetInt( "id", ++m_iSideCount );
	V_snprintf( temp, sizeof( temp ), "[1 0 0 0] %f", textureScale );
	pSide->SetString( "uaxis", temp );
	V_snprintf( temp, sizeof( temp ), "[0 0 -1 0] %f", textureScale );
	pSide->SetString( "vaxis", temp );
	V_snprintf( temp, sizeof( temp ), "(%f %f %f) (%f %f %f) (%f %f %f)", xMax, yMin, zMin, xMin, yMin, zMin, xMin, yMin, zMax );
	pSide->SetString( "plane", temp );
	pSolid->AddSubKey( pSide->MakeCopy() );

	return pSolid;
}

bool VMFExporter::AddLevelContainer()
{
	// set the extents of the map
	m_pMapLayout->GetExtents( m_iMapExtents_XMin, m_iMapExtents_XMax, m_iMapExtents_YMin, m_iMapExtents_YMax );
	DevMsg( "Layout extents: Topleft: %d %d - Lower right: %d %d\n", m_iMapExtents_XMin, m_iMapExtents_YMin, m_iMapExtents_XMax, m_iMapExtents_YMax );
	// adjust to be relative to the centre of the map
	const int half_map_size = MAP_LAYOUT_TILES_WIDE * 0.5f;
	m_iMapExtents_XMin -= half_map_size;
	m_iMapExtents_XMax -= half_map_size;
	m_iMapExtents_YMin -= half_map_size;
	m_iMapExtents_YMax -= half_map_size;
	// pad them by 3 blocks, so the player doesn't move his camera into the wall when at an edge block
	m_iMapExtents_XMin -= 3;
	m_iMapExtents_XMax += 3;
	m_iMapExtents_YMin -= 3;
	m_iMapExtents_YMax += 3;

	DevMsg( "   Adjusted to: Topleft: %d %d - Lower right: %d %d\n", m_iMapExtents_XMin, m_iMapExtents_YMin, m_iMapExtents_XMax, m_iMapExtents_YMax );

	KeyValues *pEntity = new KeyValues( "entity" );
	m_pExportKeys->AddSubKey( pEntity );

	pEntity->SetInt( "id", ++m_iEntityCount );
	pEntity->SetString( "classname", "func_brush" );
	pEntity->SetString( "origin", "0 0 0" );
	pEntity->SetString( "targetname", "structure_seal" );

	const char *const szMaterial = rd_tilegen_seal_material.GetString();
	const float flTexScale = rd_tilegen_seal_texscale.GetFloat();
	const int nLightmapScale = rd_tilegen_seal_lightmap.GetInt();
	const float flHeight = rd_tilegen_seal_height.GetFloat();
	const float flPadding = rd_tilegen_seal_padding.GetFloat();

	// top
	pEntity->AddSubKey( CreateBasicSolid( szMaterial, flTexScale, nLightmapScale,
		m_iMapExtents_XMin * ASW_TILE_SIZE, m_iMapExtents_YMin * ASW_TILE_SIZE, flHeight,
		m_iMapExtents_XMax * ASW_TILE_SIZE, m_iMapExtents_YMax * ASW_TILE_SIZE, flHeight + flPadding
	) );
	// bottom
	pEntity->AddSubKey( CreateBasicSolid( szMaterial, flTexScale, nLightmapScale,
		m_iMapExtents_XMin * ASW_TILE_SIZE, m_iMapExtents_YMin * ASW_TILE_SIZE, -flHeight - flPadding,
		m_iMapExtents_XMax * ASW_TILE_SIZE, m_iMapExtents_YMax * ASW_TILE_SIZE, -flHeight
	) );
	// west
	pEntity->AddSubKey( CreateBasicSolid( szMaterial, flTexScale, nLightmapScale,
		m_iMapExtents_XMin * ASW_TILE_SIZE - flPadding, m_iMapExtents_YMin * ASW_TILE_SIZE, -flHeight,
		m_iMapExtents_XMin * ASW_TILE_SIZE, m_iMapExtents_YMax * ASW_TILE_SIZE, flHeight
	) );
	// east
	pEntity->AddSubKey( CreateBasicSolid( szMaterial, flTexScale, nLightmapScale,
		m_iMapExtents_XMax * ASW_TILE_SIZE, m_iMapExtents_YMin * ASW_TILE_SIZE, -flHeight,
		m_iMapExtents_XMax * ASW_TILE_SIZE + flPadding, m_iMapExtents_YMax * ASW_TILE_SIZE, flHeight
	) );
	// north
	pEntity->AddSubKey( CreateBasicSolid( szMaterial, flTexScale, nLightmapScale,
		m_iMapExtents_XMin * ASW_TILE_SIZE, m_iMapExtents_YMax * ASW_TILE_SIZE, -flHeight,
		m_iMapExtents_XMax * ASW_TILE_SIZE, m_iMapExtents_YMax * ASW_TILE_SIZE + flPadding, flHeight
	) );
	// south
	pEntity->AddSubKey( CreateBasicSolid( szMaterial, flTexScale, nLightmapScale,
		m_iMapExtents_XMin * ASW_TILE_SIZE, m_iMapExtents_YMin * ASW_TILE_SIZE - flPadding, -flHeight,
		m_iMapExtents_XMax * ASW_TILE_SIZE, m_iMapExtents_YMin * ASW_TILE_SIZE, flHeight
	) );

	return true;
}

bool VMFExporter::FlattenInstances()
{
	// This function is largely based on src/utils/vbsp/map.cpp from Source SDK 2013.
	// TODO: split this function out as a standalone tool; see https://github.com/ReactiveDrop/reactivedrop_public_src/issues/416
	// problem is, the standalone tool won't be able to load workshop addons without pretending to be AS:RD in Steam.

	if ( !LoadGameInfo() )
	{
		return false;
	}

	for ( KeyValues *pEntity = m_pExportKeys->GetFirstSubKey(); pEntity; pEntity = pEntity->GetNextKey() )
	{
		if ( V_strcmp( pEntity->GetName(), "entity" ) )
		{
			continue;
		}

		if ( !ProcessEntity( pEntity ) )
		{
			return false;
		}
	}

	m_FuncASWFade.Purge();

	FOR_EACH_VEC( m_RemoveEntities, i )
	{
		m_pExportKeys->RemoveSubKey( m_RemoveEntities[i] );
		m_RemoveEntities[i]->deleteThis();
	}

	m_RemoveEntities.Purge();

	return true;
}

static VMFExporter *m_pExporterLoadingGameData = NULL;
static void GameDataErrorHelper( int level, const char *format, ... )
{
	Assert( m_pExporterLoadingGameData );
	if ( !m_pExporterLoadingGameData )
	{
		return;
	}

	char msg[4096];
	va_list marker;
	va_start( marker, format );
	V_vsnprintf( msg, sizeof( msg ), format, marker );
	va_end( marker );

	m_pExporterLoadingGameData->m_ExportErrors.AddToTail( TileGenCopyString( msg ) );
}

bool VMFExporter::LoadGameInfo()
{
	m_szInstancePath[0] = '\0';

	KeyValues::AutoDelete pGameInfo( "GameInfo" );
	if ( !pGameInfo->LoadFromFile( g_pFullFileSystem, "GameInfo.txt", "MOD" ) )
	{
		ExportError( "Could not locate GameInfo.txt for Instance Remapping\n" );
		return false;
	}

	const char *szInstancePath = pGameInfo->GetString( "InstancePath", NULL );
	if ( szInstancePath )
	{
		V_strncpy( m_szInstancePath, szInstancePath, sizeof( m_szInstancePath ) );
		V_strlower( m_szInstancePath );
		V_FixSlashes( m_szInstancePath );
	}

	const char *szGameDataFile = pGameInfo->GetString( "GameData", NULL );
	if ( !szGameDataFile )
	{
		ExportError( "Could not locate 'GameData' key in GameInfo.txt\n" );
		return false;
	}

	CUtlVector<CUtlString> GameBinPath;
	GetSearchPath( GameBinPath, "EXECUTABLE_PATH" );

	// The filesystem interface can't make a relative path absolute without truncating the result, so we're doing it manually.
	Assert( GameBinPath.Count() == 1 );
	CUtlString szFGDPath = CUtlString::PathJoin( GameBinPath[0], szGameDataFile );
	if ( !g_pFullFileSystem->FileExists( szFGDPath ) )
	{
		ExportError( "Could not locate GameData file %s\n", szGameDataFile );
		return false;
	}

	GDSetMessageFunc( &GameDataErrorHelper );
	m_pExporterLoadingGameData = this;
	bool bSuccess = m_GD.Load( szFGDPath );
	m_pExporterLoadingGameData = NULL;
	if ( !bSuccess )
	{
		ExportError( "Failed to load game data from %s\n", szFGDPath.Get() );
		return false;
	}

	return true;
}

bool VMFExporter::ProcessEntity( KeyValues *pEntity )
{
	const char *szClassName = pEntity->GetString( "classname" );
	if ( !V_stricmp( szClassName, "func_instance" ) )
	{
		return ProcessInstance( pEntity );
	}

	if ( !V_stricmp( szClassName, "rd_tilegen_instance" ) || !V_stricmp( szClassName, "func_instance_random" ) )
	{
		return ProcessRandomInstance( pEntity );
	}

	return true;
}

struct RandomInstanceChoice_t
{
	char szInstanceFile[MAX_PATH];
	float flMaxRandom;
};

bool VMFExporter::ProcessRandomInstance( KeyValues *pEntity )
{
	CUtlVector<RandomInstanceChoice_t> choices;
	float flMaxRandom = 0.0f;

	for ( int i = 1; i <= 9; i++ )
	{
		char szKey[16];
		V_snprintf( szKey, sizeof( szKey ), "weight%d", i );

		float flWeight = pEntity->GetFloat( szKey );
		if ( flWeight <= 0 )
		{
			continue;
		}

		V_snprintf( szKey, sizeof( szKey ), "glob%d", i );

		const char *szGlob = pEntity->GetString( szKey );
		if ( szGlob[0] == '\0' )
		{
			choices.AddToTail( RandomInstanceChoice_t
				{
					{'\0'},
					( flMaxRandom += flWeight )
				} );
			continue;
		}

		char szSearchPath[MAX_PATH];
		V_snprintf( szSearchPath, sizeof( szSearchPath ), "%s%s", m_szInstancePath, szGlob );
		V_SetExtension( szSearchPath, ".vmf", sizeof( szSearchPath ) );
		V_FixSlashes( szSearchPath );

		CUtlStringList names;
		FileFindHandle_t ffh;
		for ( const char *szName = g_pFullFileSystem->FindFirstEx( szSearchPath, "GAME", &ffh ); szName; szName = g_pFullFileSystem->FindNext( ffh ) )
		{
			names.CopyAndAddToTail( szName );
		}

		g_pFullFileSystem->FindClose( ffh );

		if ( !names.Count() )
		{
			Warning( "[TileGen] no results for instance pattern '%s'\n", szSearchPath );
			continue;
		}

		V_StripFilename( szSearchPath );

		flWeight /= names.Count();

		FOR_EACH_VEC( names, j )
		{
			RandomInstanceChoice_t choice{};
			V_snprintf( choice.szInstanceFile, sizeof( choice.szInstanceFile ), "%s%s", szSearchPath, names[j] );
			choice.flMaxRandom = ( flMaxRandom += flWeight );
			choices.AddToTail( choice );
		}
	}

	if ( flMaxRandom <= 0 || !choices.Count() )
	{
		return MergeInstance( pEntity, "" );
	}

	CUniformRandomStream random;
	random.SetSeed( pEntity->GetInt( "InstanceSeed" ) + pEntity->GetInt( "id" ) );
	float flChoice = random.RandomFloat( 0.0f, flMaxRandom );

	FOR_EACH_VEC( choices, i )
	{
		if ( choices[i].flMaxRandom <= flChoice )
		{
			continue;
		}

		return MergeInstance( pEntity, choices[i].szInstanceFile );
	}

	return MergeInstance( pEntity, choices[choices.Count() - 1].szInstanceFile );
}

bool VMFExporter::ProcessInstance( KeyValues *pEntity )
{
	const char *szFileName = pEntity->GetString( "file" );
	if ( szFileName[0] == '\0' )
	{
		return MergeInstance( pEntity, "" );
	}

	char szFixedFileName[MAX_PATH];
	V_strncpy( szFixedFileName, szFileName, sizeof( szFixedFileName ) );
	V_SetExtension( szFixedFileName, ".vmf", sizeof( szFixedFileName ) );
	V_FixSlashes( szFixedFileName );

	if ( g_pFullFileSystem->FileExists( szFixedFileName, "GAME" ) )
	{
		return MergeInstance( pEntity, szFixedFileName );
	}

	char szInstanceFileName[MAX_PATH];
	V_snprintf( szInstanceFileName, sizeof( szInstanceFileName ), "%s%s", m_szInstancePath, szFixedFileName );
	if ( g_pFullFileSystem->FileExists( szInstanceFileName, "GAME" ) )
	{
		return MergeInstance( pEntity, szInstanceFileName );
	}

	ExportError( "Cannot find instance '%s'\n", szFixedFileName );
	return false;
}

bool VMFExporter::MergeInstance( KeyValues *pEntity, const char *szFile )
{
	m_RemoveEntities.AddToTail( pEntity );

	if ( szFile[0] == '\0' )
	{
		// no filename; just delete instance entity
		return true;
	}

	m_iInstanceCount++;
	m_pInstanceEntity = pEntity;

	int iRoomID = pEntity->GetInt( "PlacedRoomIndex", -1 );
	if ( iRoomID >= 0 )
	{
		Assert( iRoomID < m_pMapLayout->m_PlacedRooms.Count() );
		m_iCurrentRoom = iRoomID;
		m_pRoom = m_pMapLayout->m_PlacedRooms[iRoomID];
	}

	m_vecInstanceOrigin = vec3_origin;
	( void )sscanf( pEntity->GetString( "origin" ), "%f %f %f", &m_vecInstanceOrigin.x, &m_vecInstanceOrigin.y, &m_vecInstanceOrigin.z );
	m_angInstanceAngles = vec3_angle;
	( void )sscanf( pEntity->GetString( "angles" ), "%f %f %f", &m_angInstanceAngles.x, &m_angInstanceAngles.y, &m_angInstanceAngles.z );

	AngleMatrix( m_angInstanceAngles, m_vecInstanceOrigin, m_matInstanceTransform );

	if ( const char *szTargetName = pEntity->GetString( "targetname", NULL ) )
	{
		V_strncpy( m_szFixupName, szTargetName, sizeof( m_szFixupName ) );
	}
	else if ( const char *szName = pEntity->GetString( "name", NULL ) )
	{
		V_strncpy( m_szFixupName, szName, sizeof( m_szFixupName ) );
	}
	else
	{
		V_snprintf( m_szFixupName, sizeof( m_szFixupName ), "InstanceAuto%d", m_iInstanceCount );
	}

	m_iFixupStyle = ( GameData::TNameFixup )pEntity->GetInt( "fixup_style" );

	m_iMaxEntity = 0;
	m_iMaxSolid = 0;
	m_iMaxSide = 0;
	m_iMaxNode = 0;

	KeyValues::AutoDelete pInstanceMap( "" );
	if ( !pInstanceMap->LoadFromFile( g_pFullFileSystem, szFile, "GAME" ) )
	{
		ExportError( "Failed to load instance '%s'\n", szFile );
		return false;
	}

	for ( KeyValues *pKey = pInstanceMap; pKey; pKey = pKey->GetNextKey() )
	{
		const char *szName = pKey->GetName();
		if ( !V_strcmp( szName, "versioninfo" ) || !V_strcmp( szName, "visgroups" ) || !V_strcmp( szName, "viewsettings" ) || !V_strcmp( szName, "cameras" ) || !V_strcmp( szName, "cordons" ) )
		{
			// don't need these
			continue;
		}

		KeyValues *pEditor = pKey->FindKey( "editor" );
		if ( pEditor )
		{
			pKey->RemoveSubKey( pEditor );
			pEditor->deleteThis();
		}

		if ( !V_strcmp( szName, "world" ) )
		{
			MergeWorld( pKey );
			continue;
		}

		Assert( !V_strcmp( szName, "entity" ) );
		if ( !V_strcmp( szName, "entity" ) )
		{
			MergeEntity( pKey );
			continue;
		}

		KeyValuesDumpAsDevMsg( pKey );
		Warning( "Unhandled top-level vmf data type: %s\n", szName );
	}

	m_iEntityCount += m_iMaxEntity;
	m_iSolidCount += m_iMaxSolid;
	m_iSideCount += m_iMaxSide;
	m_iNodeCount += m_iMaxNode;

	return true;
}

void VMFExporter::MergeWorld( KeyValues *pWorld )
{
	MergeSolids( pWorld, true );
}

void VMFExporter::MergeEntity( KeyValues *pEntity )
{
	int id = pEntity->GetInt( "id" );
	m_iMaxEntity = MAX( m_iMaxEntity, id );
	pEntity->SetInt( "id", id + m_iEntityCount );

	MergeSolids( pEntity, false );

	const char *szClassName = pEntity->GetString( "classname" );

	FOR_EACH_VALUE( pEntity, pValue )
	{
		ReplaceVariables( pValue );
	}

	GameData::TNameFixup iFixup = m_iFixupStyle;
	if ( !V_strcmp( szClassName, "func_brush" ) && !V_strncmp( pEntity->GetString( "targetname" ), "structure_", strlen( "structure_" ) ) )
	{
		// Special case: don't rename func_brush that will be moved to worldspawn by vbsp
		iFixup = GameData::NAME_FIXUP_NONE;
	}

	char temp[2048];

	// Special case: overlays (it's not handled by the library because these are already no longer entities by the time vbsp gets here)
	if ( !V_strcmp( szClassName, "info_overlay" ) || !V_strcmp( szClassName, "info_overlay_transition" ) )
	{
		Vector vecOriginal, vecTransformed;
		( void )sscanf( pEntity->GetString( "BasisOrigin" ), "%f %f %f", &vecOriginal.x, &vecOriginal.y, &vecOriginal.z );
		VectorTransform( vecOriginal, m_matInstanceTransform, vecTransformed );
		V_snprintf( temp, sizeof( temp ), "%g %g %g", vecTransformed.x, vecTransformed.y, vecTransformed.z );
		pEntity->SetString( "BasisOrigin", temp );

		// the remainder of this block is based on Overlay_Translate from Source SDK 2013 VBSP overlay.cpp
		VMatrix matOverlay{ m_matInstanceTransform };
		matOverlay.SetTranslation( vec3_origin );

		if ( !matOverlay.IsIdentity() )
		{
			Vector vecU, vecOrigU, vecV, vecOrigV, vecNormal;

			( void )sscanf( pEntity->GetString( "BasisU" ), "%f %f %f", &vecOrigU.x, &vecOrigU.y, &vecOrigU.z );
			vecOrigU.NormalizeInPlace();
			matOverlay.V3Mul( vecOrigU, vecU );
			( void )sscanf( pEntity->GetString( "BasisV" ), "%f %f %f", &vecOrigV.x, &vecOrigV.y, &vecOrigV.z );
			vecOrigV.NormalizeInPlace();
			matOverlay.V3Mul( vecOrigV, vecV );
			( void )sscanf( pEntity->GetString( "BasisNormal" ), "%f %f %f", &vecOriginal.x, &vecOriginal.y, &vecOriginal.z );
			vecOriginal.NormalizeInPlace();
			matOverlay.V3Mul( vecOriginal, vecNormal );

			float fScaleU = vecU.Length();
			float fScaleV = vecV.Length();
			float flScaleNormal = vecNormal.Length();

			bool bIsUnit = ( CloseEnough( fScaleU, 1.0f, 0.0001 ) && CloseEnough( fScaleV, 1.0f, 0.0001 ) && CloseEnough( flScaleNormal, 1.0f, 0.0001 ) );
			bool bIsPerp = ( CloseEnough( DotProduct( vecU, vecV ), 0.0f, 0.0025 ) && CloseEnough( DotProduct( vecU, vecNormal ), 0.0f, 0.0025 ) && CloseEnough( DotProduct( vecV, vecNormal ), 0.0f, 0.0025 ) );
			if ( bIsUnit && bIsPerp )
			{
				V_snprintf( temp, sizeof( temp ), "%g %g %g", vecU.x, vecU.y, vecU.z );
				pEntity->SetString( "BasisU", temp );
				V_snprintf( temp, sizeof( temp ), "%g %g %g", vecV.x, vecV.y, vecV.z );
				pEntity->SetString( "BasisV", temp );
				V_snprintf( temp, sizeof( temp ), "%g %g %g", vecNormal.x, vecNormal.y, vecNormal.z );
				pEntity->SetString( "BasisNormal", temp );
			}
			else
			{
				// more complex transformation, move UV coordinates, but leave base axes 
				for ( int i = 0; i < 4; i++ )
				{
					char szKey[8];
					V_snprintf( szKey, sizeof( szKey ), "uv%d", i );
					( void )sscanf( pEntity->GetString( szKey ), "%f %f %f", &vecOriginal.x, &vecOriginal.y, &vecOriginal.z );
					vecOriginal = vecOriginal.x * vecOrigU + vecOriginal.y * vecOrigV;
					matOverlay.V3Mul( vecOriginal, vecTransformed );
					V_snprintf( temp, sizeof( temp ), "%g %g %g", vecOrigU.Dot( vecTransformed ), vecOrigV.Dot( vecTransformed ), 0.0f );
					pEntity->SetString( szKey, temp );
				}
			}
		}
	}

	GDclass *pEntClass = m_GD.BeginInstanceRemap( szClassName, m_szFixupName, m_vecInstanceOrigin, m_angInstanceAngles );
	if ( pEntClass )
	{
		for ( int i = 0; i < pEntClass->GetVariableCount(); i++ )
		{
			GDinputvariable *pEntVar = pEntClass->GetVariableAt( i );
			const char *pValue = pEntity->GetString( pEntVar->GetName() );
			if ( pValue[0] == '\0' )
			{
				continue;
			}

			if ( m_GD.RemapKeyValue( pEntVar->GetName(), pValue, temp, iFixup ) )
			{
				Assert( pEntVar->GetType() != ivSide && pEntVar->GetType() != ivSideList && pEntVar->GetType() != ivNodeID && pEntVar->GetType() != ivNodeDest );

				pEntity->SetString( pEntVar->GetName(), temp );
			}
			else if ( pEntVar->GetType() == ivSide || pEntVar->GetType() == ivSideList )
			{
				CSplitString sides( pValue, " " );
				temp[0] = '\0';
				FOR_EACH_VEC( sides, j )
				{
					int iSide = atoi( sides[j] );
					m_iMaxSide = MAX( m_iMaxSide, iSide );
					V_snprintf( temp, sizeof( temp ), "%s %d", temp, iSide + m_iSideCount );
				}

				pEntity->SetString( pEntVar->GetName(), temp + 1 );
			}
			else if ( pEntVar->GetType() == ivNodeID || pEntVar->GetType() == ivNodeDest )
			{
				int iNode = pEntity->GetInt( pEntVar->GetName() );
				m_iMaxNode = MAX( m_iMaxNode, iNode );
				pEntity->SetInt( pEntVar->GetName(), iNode + m_iNodeCount );
			}
		}
	}
	else
	{
		Warning( "No entity definition found for %s\n", szClassName );
	}


	if ( KeyValues *pConnections = pEntity->FindKey( "connections" ) )
	{
		FOR_EACH_VALUE( pConnections, pConnection )
		{
			ReplaceVariables( pConnection );

			char szOrigValue[4096];
			V_strncpy( szOrigValue, pConnection->GetString(), sizeof( szOrigValue ) );

			char *pComma = strchr( szOrigValue, ',' );
			if ( pComma )
			{
				*pComma = '\0';
				pComma++;
			}

			if ( m_GD.RemapNameField( szOrigValue, temp, iFixup ) )
			{
				if ( pComma )
				{
					V_snprintf( temp, sizeof( temp ), "%s,%s", temp, pComma );
				}

				pConnection->SetStringValue( temp );
			}
		}
	}

	// Propagate room information through instances.
	if ( !V_strcmp( szClassName, "func_instance" ) || !V_strcmp( szClassName, "rd_tilegen_instance" ) || !V_strcmp( szClassName, "func_instance_random" ) )
	{
		if ( m_pRoom )
		{
			pEntity->SetInt( "PlacedRoomIndex", m_pRoom->m_nPlacementIndex );
			pEntity->SetInt( "InstanceSeed", m_pRoom->m_nInstanceSeed );
		}
	}

	// Merge func_asw_fade entities that have the same settings and origin Z coordinates to save on edicts.
	if ( !V_strcmp( szClassName, "func_asw_fade" ) && pEntity->GetString( "targetname" )[0] == '\0' && pEntity->GetString( "parentname" )[0] == '\0' && pEntity->GetString( "angles" )[0] == '\0' )
	{
		static const char *const s_szFadeSettingNames[] =
		{
			"classname",
			"vrad_brush_cast_shadows",
			"AllowFade",
			"disablereceiveshadows",
			"disableshadows",
			"fade_opacity",
			"StartDisabled",
			"Solidity",
			"CollideWithGrenades",
			"renderamt",
			"solidbsp",
			"rendercolor",
			"rendermode",
			"renderfx",
		};

		Vector vecTargetOrigin{ 0, 0, 0 };
		( void )sscanf( pEntity->GetString( "origin" ), "%f %f %f", &vecTargetOrigin.x, &vecTargetOrigin.y, &vecTargetOrigin.z );

		FOR_EACH_VEC( m_FuncASWFade, i )
		{
			Vector vecCurOrigin{ 0, 0, 0 };
			( void )sscanf( m_FuncASWFade[i]->GetString( "origin" ), "%f %f %f", &vecCurOrigin.x, &vecCurOrigin.y, &vecCurOrigin.z );
			if ( !CloseEnough( vecTargetOrigin.z, vecCurOrigin.z ) )
			{
				continue;
			}

			bool bAnyDiffer = false;
			for ( int j = 0; j < NELEMS( s_szFadeSettingNames ); j++ )
			{
				if ( V_strcmp( pEntity->GetString( s_szFadeSettingNames[j] ), m_FuncASWFade[i]->GetString( s_szFadeSettingNames[j] ) ) )
				{
					bAnyDiffer = true;
					break;
				}
			}

			if ( !bAnyDiffer )
			{
				FOR_EACH_TRUE_SUBKEY( pEntity, pSolid )
				{
					if ( !V_strcmp( pSolid->GetName(), "solid" ) )
					{
						m_FuncASWFade[i]->AddSubKey( pSolid->MakeCopy() );
					}
				}

				return;
			}
		}

		KeyValues *pCopy = pEntity->MakeCopy();
		m_FuncASWFade.AddToTail( pCopy );
		m_pExportKeys->AddSubKey( pCopy );

		return;
	}

	m_pExportKeys->AddSubKey( pEntity->MakeCopy() );
}

void VMFExporter::ReplaceVariables( KeyValues *pValue )
{
	char szValue[MAX_KEYVALUE_LEN], szNewValue[MAX_KEYVALUE_LEN];
	bool bOverwritten = false;

	V_strncpy( szNewValue, pValue->GetString(), sizeof( szNewValue ) );
	FOR_EACH_VALUE( m_pInstanceEntity, pProperty )
	{
		if ( V_strnicmp( pProperty->GetName(), "replace", strlen( "replace" ) ) )
		{
			continue;
		}

		char szInstanceVariable[MAX_KEYVALUE_LEN];

		V_strncpy( szInstanceVariable, pProperty->GetString(), sizeof( szInstanceVariable ) );

		char *pReplaceValue = strchr( szInstanceVariable, ' ' );
		if ( !pReplaceValue )
		{
			continue;
		}

		*pReplaceValue = '\0';
		pReplaceValue++;

		V_strncpy( szValue, szNewValue, sizeof( szValue ) );
		if ( !V_StrSubst( szValue, szInstanceVariable, pReplaceValue, szNewValue, sizeof( szNewValue ), false ) )
		{
			bOverwritten = true;
		}
	}

	if ( !bOverwritten && V_strcmp( pValue->GetString(), szNewValue ) )
	{
		pValue->SetStringValue( szNewValue );
	}
}

void VMFExporter::MergeSolids( KeyValues *pContainer, bool bAddToWorld )
{
	char temp[2048];

	FOR_EACH_TRUE_SUBKEY( pContainer, pSolid )
	{
		if ( V_strcmp( pSolid->GetName(), "solid" ) )
		{
			continue;
		}

		int id = pSolid->GetInt( "id" );
		m_iMaxSolid = MAX( m_iMaxSolid, id );
		pSolid->SetInt( "id", id + m_iSolidCount );

		if ( KeyValues *pEditor = pSolid->FindKey( "editor" ) )
		{
			pSolid->RemoveSubKey( pEditor );
			pEditor->deleteThis();
		}

		FOR_EACH_TRUE_SUBKEY( pSolid, pSide )
		{
			if ( V_strcmp( pSide->GetName(), "side" ) )
			{
				continue;
			}

			id = pSide->GetInt( "id" );
			m_iMaxSide = MAX( m_iMaxSide, id );
			pSide->SetInt( "id", id + m_iSideCount );

			// adjust plane
			Vector a1, b1, c1;
			( void )sscanf( pSide->GetString( "plane" ), "(%f %f %f) (%f %f %f) (%f %f %f)", &a1.x, &a1.y, &a1.z, &b1.x, &b1.y, &b1.z, &c1.x, &c1.y, &c1.z );

			Vector a2, b2, c2;
			VectorTransform( a1, m_matInstanceTransform, a2 );
			VectorTransform( b1, m_matInstanceTransform, b2 );
			VectorTransform( c1, m_matInstanceTransform, c2 );

			V_snprintf( temp, sizeof( temp ), "(%g %g %g) (%g %g %g) (%g %g %g)", a2.x, a2.y, a2.z, b2.x, b2.y, b2.z, c2.x, c2.y, c2.z );
			pSide->SetString( "plane", temp );

			// adjust UV
			Vector u1, v1;
			float shiftU, shiftV, texelSizeU, texelSizeV;
			( void )sscanf( pSide->GetString( "uaxis" ), "[%f %f %f %f] %f", &u1.x, &u1.y, &u1.z, &shiftU, &texelSizeU );
			( void )sscanf( pSide->GetString( "vaxis" ), "[%f %f %f %f] %f", &v1.x, &v1.y, &v1.z, &shiftV, &texelSizeV );

			Vector u2, v2;
			VectorRotate( u1, m_matInstanceTransform, u2 );
			VectorRotate( v1, m_matInstanceTransform, v2 );
			shiftU -= m_vecInstanceOrigin.Dot( u1 ) / texelSizeU;
			shiftV -= m_vecInstanceOrigin.Dot( v1 ) / texelSizeV;

			V_snprintf( temp, sizeof( temp ), "[%g %g %g %g] %g", u2.x, u2.y, u2.z, shiftU, texelSizeU );
			pSide->SetString( "uaxis", temp );
			V_snprintf( temp, sizeof( temp ), "[%g %g %g %g] %g", v2.x, v2.y, v2.z, shiftV, texelSizeV );
			pSide->SetString( "vaxis", temp );

			if ( KeyValues *pDispInfo = pSide->FindKey( "dispinfo" ) )
			{
				// adjust displacement start position
				Vector vecStartPosition;
				( void )sscanf( "startposition", "[%f %f %f]", &vecStartPosition.x, &vecStartPosition.y, &vecStartPosition.z );

				Vector vecTransformedStartPosition;
				VectorTransform( vecStartPosition, m_matInstanceTransform, vecTransformedStartPosition );

				V_snprintf( temp, sizeof( temp ), "[%g %g %g]", vecTransformedStartPosition.x, vecTransformedStartPosition.y, vecTransformedStartPosition.z );
				pDispInfo->SetString( "startposition", temp );
			}
		}

		if ( bAddToWorld )
		{
			m_pExportWorldKeys->AddSubKey( pSolid->MakeCopy() );
		}
	}
}
