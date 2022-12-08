#ifndef TILEGEN_VMFEXPORTER_H
#define TILEGEN_VMFEXPORTER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChunkFile.h"
#include "utlvector.h"
#include "utlstring.h"
#include "fgdlib/fgdlib.h"

class CRoom;
class CMapLayout;
class CRoomTemplate;

// this class uses the placed rooms and room templates to build up a vmf file of the put together map

class VMFExporter
{
public:
	VMFExporter();
	virtual ~VMFExporter();

	void Init();
	void ExportError( const char *pMsg, ... );
	void ClearExportErrors();
	bool ShowExportErrors();

	bool ExportVMF( CMapLayout *pLayout, const char *mapname, bool bPopupWarnings = false );
	const Vector &GetCurrentRoomOffset();
	void AddRoomInstance( const CRoomTemplate *pRoomTemplate, int nPlacedRoomIndex = -1 );

	KeyValues *GetVersionInfo();
	KeyValues *GetDefaultVisGroups();
	KeyValues *GetViewSettings();
	KeyValues *GetDefaultCamera();
	KeyValues *GetPlayerStarts();
	KeyValues *GetDefaultWorldChunk();
	KeyValues *CreateBasicSolid( const char *szMaterial, float textureScale, int lightmapScale, float xMin, float yMin, float zMin, float xMax, float yMax, float zMax );
	bool AddLevelContainer();

	bool FlattenInstances();
	bool LoadGameInfo();
	bool ProcessEntity( KeyValues *pEntity );
	bool ProcessRandomInstance( KeyValues *pEntity );
	bool ProcessInstance( KeyValues *pEntity );
	bool MergeInstance( KeyValues *pEntity, const char *szFile );
	void MergeWorld( KeyValues *pWorld );
	void MergeEntity( KeyValues *pEntity );
	void ReplaceVariables( KeyValues *pValue );
	void MergeSolids( KeyValues *pContainer, bool bAddToWorld );

	CRoom *m_pRoom;	// the current CRoom we're writing out
	int m_iCurrentRoom;	// index of the current room we're writing out
	int m_iEntityCount;
	int m_iSolidCount;
	int m_iSideCount;
	int m_iNodeCount;
	int m_iInstanceCount;
	int m_iMaxEntity;
	int m_iMaxSolid;
	int m_iMaxSide;
	int m_iMaxNode;

	KeyValues *m_pExportKeys;			// the keys we're going to export as a vmf file
	KeyValues *m_pExportWorldKeys;		// world section of m_pExportKeys

	// the level extents, in tiles, relative to the centre of the map
	int m_iMapExtents_XMin, m_iMapExtents_YMin;
	int m_iMapExtents_XMax, m_iMapExtents_YMax;

	char m_szLastExporterError[256];

	Vector m_vecLastPlaneOffset;
	Vector m_vecStartRoomOrigin;

	CMapLayout *m_pMapLayout;		// the layout we're currently exporting

	CUtlVector<const char *> m_ExportErrors;
	bool m_bPopupWarnings;

	GameData m_GD;
	char m_szInstancePath[MAX_PATH];
	CUtlVector<KeyValues *> m_RemoveEntities;
	CUtlVector<KeyValues *> m_FuncASWFade;

	KeyValues *m_pInstanceEntity;
	Vector m_vecInstanceOrigin;
	QAngle m_angInstanceAngles;
	matrix3x4_t m_matInstanceTransform;
	char m_szFixupName[128];
	GameData::TNameFixup m_iFixupStyle;
};

#endif // TILEGEN_VMFEXPORTER_H