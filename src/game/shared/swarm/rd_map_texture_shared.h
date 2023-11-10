#pragma once

#ifdef CLIENT_DLL
#define CRD_Map_Texture C_RD_Map_Texture

DECLARE_AUTO_LIST( IRDMapTextures );
#endif

class CRD_Map_Texture : public CBaseEntity
#ifdef CLIENT_DLL
	, public IRDMapTextures
#endif
{
	DECLARE_CLASS( CRD_Map_Texture, CBaseEntity );
public:
	DECLARE_NETWORKCLASS();

	CRD_Map_Texture();
	virtual ~CRD_Map_Texture();

	void Precache() override;

	Vector GetTopLeftCorner() const;
	Vector GetTopRightCorner() const;
	Vector GetBottomLeftCorner() const;
	Vector GetBottomRightCorner() const;

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	void Spawn() override;
	int UpdateTransmitState() override;

	CNetworkVar( string_t, m_szMaterialName );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
#else
	IMPLEMENT_AUTO_LIST_GET();

	CNetworkString( m_szMaterialName, 255 );
	vgui::HTexture m_hVGuiTexture;
#endif

	CNetworkVector( m_MapMins );
	CNetworkVector( m_MapMaxs );
	CNetworkVar( bool, m_bDisabled );
};
