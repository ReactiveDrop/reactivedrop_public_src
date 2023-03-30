#pragma once

#ifdef RD_FADE_SINGLE_EDICT
#ifdef CLIENT_DLL
#define CASW_Fade_Ent C_ASW_Fade_Ent
#endif

#define ASW_FADE_MAX_MODELS 2048
#define ASW_FADE_MAX_PROXIES 2048

class ASWFadeModel_t : public CMemZeroOnNew
{
public:
	DECLARE_CLASS_NOBASE( ASWFadeModel_t );

	enum Type_t
	{
		BRUSH,
		PROP,
		SPRITE,

		_NUM_TYPES,
	};

	CNetworkVar( Type_t, m_iType );
	CNetworkVar( UtlSymId_t, m_iName ); // subtract 1 to get real value, so that 0 can be our default instead of -1
	CNetworkVar( int, m_iModelIndex );
	model_t *m_pModel;
	CNetworkHandle( CBaseEntity, m_hMoveParent );
	CNetworkVector( m_LocalOrigin );
	CNetworkQAngle( m_LocalAngles );
	CNetworkVar( float, m_flFadeZOffset );
	CNetworkVar( unsigned char, m_iNormalAlpha );
	CNetworkVar( unsigned char, m_iFadeAlpha );

	float GetFadeZ() const;

	// For CNetworkVars.
	void NetworkStateChanged();
	void NetworkStateChanged( void *pVar ) { NetworkStateChanged(); }
};

class ASWFadeModelIO_t : public CMemZeroOnNew
{
public:
	DECLARE_CLASS_NOBASE( ASWFadeModelIO_t );

	enum
	{
		bits_ALLOW_FADE		= 0x0001,
		bits_HAS_PROXY		= 0x0002,

		_NUM_BITS = 2,
	};

	CNetworkVar( unsigned short, m_iFlags );

	// For CNetworkVars.
	void NetworkStateChanged();
	void NetworkStateChanged( void *pVar ) { NetworkStateChanged(); }
};

class ASWFadeProxyPoint_t : public CMemZeroOnNew
{
public:
	DECLARE_CLASS_NOBASE( ASWFadeProxyPoint_t );

	// For CNetworkVars.
	void NetworkStateChanged();
	void NetworkStateChanged( void *pVar ) { NetworkStateChanged(); }
};

class ASWFadeProxyVolume_t : public CMemZeroOnNew
{
public:
	DECLARE_CLASS_NOBASE( ASWFadeProxyVolume_t );

	// For CNetworkVars.
	void NetworkStateChanged();
	void NetworkStateChanged( void *pVar ) { NetworkStateChanged(); }
};

class CASW_Fade_Ent : public CBaseEntity
{
public:
	DECLARE_CLASS( CASW_Fade_Ent, CBaseEntity );
	DECLARE_NETWORKCLASS();

	CASW_Fade_Ent();
	virtual ~CASW_Fade_Ent();

#ifdef CLIENT_DLL
	void OnDataChanged( DataUpdateType_t updateType ) override;
	void ClientThink() override;
#else
	CUtlSymbolTable m_Names;
#endif
	CNetworkVar( CUtlVector<ASWFadeModel_t>, m_Models );
	CNetworkVar( CUtlVector<ASWFadeModelIO_t>, m_ModelIO );
	CNetworkVar( CUtlVector<ASWFadeProxyPoint_t>, m_PointProxies );
	CNetworkVar( CUtlVector<ASWFadeProxyVolume_t>, m_VolumeProxies );
};

extern CASW_Fade_Ent *g_pASWFadeEnt;

#ifdef GAME_DLL
class CASW_Fade_IO_Proxy : public CServerOnlyEntity
{
public:
	DECLARE_CLASS( CASW_Fade_IO_Proxy, CServerOnlyEntity );
	DECLARE_DATADESC();
};
#endif
#endif
