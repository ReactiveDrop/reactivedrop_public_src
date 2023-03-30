#include "cbase.h"
#include "asw_fade_ent_shared.h"
#ifdef CLIENT_DLL
#include "dt_utlvector_recv.h"
#else
#include "dt_utlvector_send.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef RD_FADE_SINGLE_EDICT

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( asw_fade_system, CASW_Fade_Ent );
LINK_ENTITY_TO_CLASS( asw_fade_io_proxy, CASW_Fade_IO_Proxy );

extern const int SENDPROP_ANGROTATION_DEFAULT_BITS;
#endif

CASW_Fade_Ent *g_pASWFadeEnt = NULL;

BEGIN_NETWORK_TABLE_NOBASE( ASWFadeModel_t, DT_ASW_Fade_Model )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iType ) ),
	RecvPropInt( RECVINFO( m_iName ) ),
	RecvPropInt( RECVINFO( m_iModelIndex ) ),
	RecvPropEHandle( RECVINFO( m_hMoveParent ) ),
	RecvPropVector( RECVINFO( m_LocalOrigin ) ),
	RecvPropQAngles( RECVINFO( m_LocalAngles ) ),
	RecvPropFloat( SENDINFO( m_flFadeZOffset ) ),
	RecvPropInt( RECVINFO( m_iNormalAlpha ) ),
	RecvPropInt( RECVINFO( m_iFadeAlpha ) ),
#else
	SendPropInt( SENDINFO( m_iType ), NumBitsForCount( ASWFadeModel_t::_NUM_TYPES ), SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iName ), -1, SPROP_UNSIGNED ),
	SendPropModelIndex( SENDINFO( m_iModelIndex ) ),
	SendPropEHandle( SENDINFO( m_hMoveParent ) ),
	SendPropVector( SENDINFO( m_LocalOrigin ), 0, SPROP_COORD ),
	SendPropQAngles( SENDINFO( m_LocalAngles ), SENDPROP_ANGROTATION_DEFAULT_BITS ),
	SendPropFloat( SENDINFO( m_flFadeZOffset ), 0, SPROP_COORD ),
	SendPropInt( SENDINFO( m_iNormalAlpha ), -1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iFadeAlpha ), -1, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE();

void ASWFadeModel_t::NetworkStateChanged()
{
	Assert( g_pASWFadeEnt );
	if ( g_pASWFadeEnt )
	{
		g_pASWFadeEnt->m_Models.GetForModify();
	}
}

BEGIN_NETWORK_TABLE_NOBASE( ASWFadeModelIO_t, DT_ASW_Fade_Model_IO )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iFlags ) ),
#else
	SendPropInt( SENDINFO( m_iFlags ), ASWFadeModelIO_t::_NUM_BITS, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE();

void ASWFadeModelIO_t::NetworkStateChanged()
{
	Assert( g_pASWFadeEnt );
	if ( g_pASWFadeEnt )
	{
		g_pASWFadeEnt->m_ModelIO.GetForModify();
	}
}

BEGIN_NETWORK_TABLE_NOBASE( ASWFadeProxyPoint_t, DT_ASW_Fade_Proxy_Point )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE();

void ASWFadeProxyPoint_t::NetworkStateChanged()
{
	Assert( g_pASWFadeEnt );
	if ( g_pASWFadeEnt )
	{
		g_pASWFadeEnt->m_PointProxies.GetForModify();
	}
}

BEGIN_NETWORK_TABLE_NOBASE( ASWFadeProxyVolume_t, DT_ASW_Fade_Proxy_Volume )
#ifdef CLIENT_DLL
#else
#endif
END_NETWORK_TABLE();

void ASWFadeProxyVolume_t::NetworkStateChanged()
{
	Assert( g_pASWFadeEnt );
	if ( g_pASWFadeEnt )
	{
		g_pASWFadeEnt->m_VolumeProxies.GetForModify();
	}
}

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Fade_Ent, DT_ASW_Fade_Ent );

BEGIN_NETWORK_TABLE( CASW_Fade_Ent, DT_ASW_Fade_Ent )
#ifdef CLIENT_DLL
	RecvPropUtlVectorDataTable( m_Models, ASW_FADE_MAX_MODELS, DT_ASW_Fade_Model ),
	RecvPropUtlVectorDataTable( m_ModelIO, ASW_FADE_MAX_MODELS, DT_ASW_Fade_Model_IO ),
	RecvPropUtlVectorDataTable( m_PointProxies, ASW_FADE_MAX_PROXIES, DT_ASW_Fade_Proxy_Point ),
	RecvPropUtlVectorDataTable( m_VolumeProxies, ASW_FADE_MAX_PROXIES, DT_ASW_Fade_Proxy_Volume ),
#else
	SendPropUtlVectorDataTable( m_Models, ASW_FADE_MAX_MODELS, DT_ASW_Fade_Model ),
	SendPropUtlVectorDataTable( m_ModelIO, ASW_FADE_MAX_MODELS, DT_ASW_Fade_Model_IO ),
	SendPropUtlVectorDataTable( m_PointProxies, ASW_FADE_MAX_PROXIES, DT_ASW_Fade_Proxy_Point ),
	SendPropUtlVectorDataTable( m_VolumeProxies, ASW_FADE_MAX_PROXIES, DT_ASW_Fade_Proxy_Volume ),
#endif
END_NETWORK_TABLE();

CASW_Fade_Ent::CASW_Fade_Ent()
{
#ifndef CLIENT_DLL
	Assert( !g_pASWFadeEnt );
#endif

	g_pASWFadeEnt = this;
}

CASW_Fade_Ent::~CASW_Fade_Ent()
{
#ifndef CLIENT_DLL
	Assert( g_pASWFadeEnt == this );
#endif

	if ( g_pASWFadeEnt == this )
	{
		g_pASWFadeEnt = NULL;
	}
}

#ifdef CLIENT_DLL
void CASW_Fade_Ent::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}
void CASW_Fade_Ent::ClientThink()
{
	BaseClass::ClientThink();
}
#else
#endif

#ifdef GAME_DLL
BEGIN_DATADESC( CASW_Fade_IO_Proxy )
END_DATADESC();
#endif

#endif
