#pragma once

#ifdef CLIENT_DLL
#define CPoint_ASW_Fade_Proxy C_Point_ASW_Fade_Proxy
#define CTrigger_ASW_Fade_Proxy C_Trigger_ASW_Fade_Proxy
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#endif

class CASW_Inhabitable_NPC;

class CPoint_ASW_Fade_Proxy : public CBaseEntity
{
public:
	DECLARE_CLASS( CPoint_ASW_Fade_Proxy, CBaseEntity );
	DECLARE_NETWORKCLASS();

	CPoint_ASW_Fade_Proxy();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
	void Activate() override;
	int	UpdateTransmitState() override;
	int ShouldTransmit( const CCheckTransmitInfo *pInfo ) override;

	void InputEnable( inputdata_t &data );
	void InputDisable( inputdata_t &data );
	void InputToggle( inputdata_t &data );
#endif

	virtual bool ShouldFade( const Vector & vecEyePosition );

	CNetworkVar( bool, m_bDisabled );
	CNetworkVar( bool, m_bBrushOnly );
	CNetworkVar( float, m_flFreeRadius );
	CNetworkVar( float, m_flMaxRadius );
};

class CTrigger_ASW_Fade_Proxy : public CPoint_ASW_Fade_Proxy
{
public:
	DECLARE_CLASS( CTrigger_ASW_Fade_Proxy, CPoint_ASW_Fade_Proxy );
	DECLARE_NETWORKCLASS();

#ifdef CLIENT_DLL
	bool ShouldDraw() override { return true; }
	int DrawModel( int flags, const RenderableInstance_t &instance ) override;

	bool m_bLastTouchingTrigger;
#else
	DECLARE_DATADESC();
	void Spawn() override;
#endif

	bool ShouldFade( const Vector &vecEyePosition ) override;
};
