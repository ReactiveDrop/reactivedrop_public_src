#pragma once

#ifdef CLIENT_DLL
#define CRD_HUD_VScript C_RD_HUD_VScript
class CRD_Infection_Deathmatch_Stats;
#endif

class CRD_HUD_VScript : public CBaseEntity
{
	DECLARE_CLASS( CRD_HUD_VScript, CBaseEntity );
public:
	DECLARE_NETWORKCLASS();
	DECLARE_ENT_SCRIPTDESC();

	CRD_HUD_VScript();
	~CRD_HUD_VScript();

#ifdef CLIENT_DLL
	static CUtlVector<CRD_HUD_VScript *> s_HUDEntities;

	void OnDataChanged( DataUpdateType_t type ) override;
	void OnSetDormant( bool bDormant ) override;
	virtual void Paint();

	int Script_LookupTexture( const char *name );
	int Script_LookupFont( const char *name );
	int Script_GetFontTall( int font );
	int Script_GetTextWide( int font, const char *text );
	void Script_PaintText( int x, int y, int r, int g, int b, int a, int font, const char *text );
	float Script_GetZbalermornaTextWide( int font, const char *text );
	void Script_PaintZbalermornaText( float x, float y, int r, int g, int b, int a, int font, const char *text );
	void Script_PaintRectangle( int x0, int y0, int x1, int y1, int r, int g, int b, int a );
	void Script_PaintRectangleFade( int x0, int y0, int x1, int y1, int r, int g, int b, int a0, int a1, int fadeStart, int fadeEnd, bool horizontal );
	void Script_PaintTexturedRectangle( int x0, int y0, int x1, int y1, int r, int g, int b, int a, int texture );
	void Script_PaintTexturedRectangleAdvanced( HSCRIPT table );
	void Script_PaintPolygon( HSCRIPT vertices, int r, int g, int b, int a, int texture );

	CNetworkString( m_szClientVScript, 64 );
	CScriptScope m_ScriptScope;
	HSCRIPT m_hUpdateFunc{ INVALID_HSCRIPT };
	HSCRIPT m_hPaintFunc{ INVALID_HSCRIPT };
	CUtlDict<int> m_Textures;
	bool m_bIsPainting{ false };
	CRD_Infection_Deathmatch_Stats *m_pInfectionDeathmatchStats{};
#else
	DECLARE_DATADESC();

	int ShouldTransmit( const CCheckTransmitInfo *pInfo ) override;
	int UpdateTransmitState() override;

	void SetEntity( int i, HSCRIPT entity );
	void SetInt( int i, int value );
	void SetFloat( int i, float value );
	void SetString( int i, const char *string );

	CNetworkVar( string_t, m_szClientVScript );
#endif

	HSCRIPT GetEntity( int i ) const;
	int GetInt( int i ) const;
	float GetFloat( int i ) const;
	const char *GetString( int i ) const;

	virtual const char *GetDebugClassname() const { return "rd_hud_vscript"; }

	CNetworkHandle( CBaseEntity, m_hDataEntity );
	CNetworkString( m_szDataString, 256 );
	CNetworkArray( float, m_flDataFloat, 32 );
	CNetworkArray( int, m_iDataInt, 64 );
};
