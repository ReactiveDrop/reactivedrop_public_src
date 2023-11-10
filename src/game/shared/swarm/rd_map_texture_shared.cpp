#include "cbase.h"
#include "rd_map_texture_shared.h"
#ifdef CLIENT_DLL
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef CLIENT_DLL
IMPLEMENT_AUTO_LIST( IRDMapTextures );
#else
LINK_ENTITY_TO_CLASS( rd_map_texture, CRD_Map_Texture );

BEGIN_DATADESC( CRD_Map_Texture )
	DEFINE_KEYFIELD( m_szMaterialName, FIELD_STRING, "material" ),
	DEFINE_KEYFIELD( m_MapMins, FIELD_VECTOR, "mins" ),
	DEFINE_KEYFIELD( m_MapMaxs, FIELD_VECTOR, "maxs" ),
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC();

extern void SendProxy_String_tToString( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( RD_Map_Texture, DT_RD_Map_Texture );

BEGIN_NETWORK_TABLE( CRD_Map_Texture, DT_RD_Map_Texture )
#ifdef CLIENT_DLL
	RecvPropString( RECVINFO( m_szMaterialName ) ),
	RecvPropVectorXY( RECVINFO( m_MapMins ) ),
	RecvPropVectorXY( RECVINFO( m_MapMaxs ) ),
	RecvPropBool( RECVINFO( m_bDisabled ) ),
#else
	SendPropString( SENDINFO( m_szMaterialName ), 0, SendProxy_String_tToString ),
	SendPropVectorXY( SENDINFO( m_MapMins ) ),
	SendPropVectorXY( SENDINFO( m_MapMaxs ) ),
	SendPropBool( SENDINFO( m_bDisabled ) ),
#endif
END_NETWORK_TABLE();

CRD_Map_Texture::CRD_Map_Texture()
{
#ifdef CLIENT_DLL
	m_szMaterialName.GetForModify()[0] = '\0';

	m_hVGuiTexture = NULL;
#else
	m_szMaterialName.Set( NULL_STRING );
#endif

	m_MapMins.Init();
	m_MapMaxs.Init();
	m_bDisabled = false;
}

CRD_Map_Texture::~CRD_Map_Texture()
{
#ifdef CLIENT_DLL
	if ( m_hVGuiTexture && vgui::surface() )
	{
		vgui::surface()->DestroyTextureID( m_hVGuiTexture );
		m_hVGuiTexture = NULL;
	}
#endif
}

void CRD_Map_Texture::Precache()
{
	BaseClass::Precache();

	if ( const char *szMaterialName = STRING( m_szMaterialName.Get() ) )
	{
		PrecacheMaterial( szMaterialName );
	}
}

Vector CRD_Map_Texture::GetTopLeftCorner() const
{
	Vector pos;
	EntityToWorldSpace( Vector{ m_MapMins.GetX(), m_MapMins.GetY(), 0.0f }, &pos );
	return pos;
}
Vector CRD_Map_Texture::GetTopRightCorner() const
{
	Vector pos;
	EntityToWorldSpace( Vector{ m_MapMaxs.GetX(), m_MapMins.GetY(), 0.0f }, &pos );
	return pos;
}
Vector CRD_Map_Texture::GetBottomLeftCorner() const
{
	Vector pos;
	EntityToWorldSpace( Vector{ m_MapMins.GetX(), m_MapMaxs.GetY(), 0.0f }, &pos );
	return pos;
}
Vector CRD_Map_Texture::GetBottomRightCorner() const
{
	Vector pos;
	EntityToWorldSpace( Vector{ m_MapMaxs.GetX(), m_MapMaxs.GetY(), 0.0f }, &pos );
	return pos;
}

#ifndef CLIENT_DLL
void CRD_Map_Texture::Spawn()
{
	Precache();

	if ( m_szMaterialName.Get() == NULL_STRING )
	{
		Warning( "rd_map_texture entity (%d:%s) @ %f %f %f has no material assigned. Deleting.\n", GetHammerID(), GetDebugName(), VectorExpand( GetAbsOrigin() ) );
		UTIL_Remove( this );
		return;
	}

	BaseClass::Spawn();
}

int CRD_Map_Texture::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CRD_Map_Texture::InputEnable( inputdata_t &inputdata )
{
	m_bDisabled = false;
}

void CRD_Map_Texture::InputDisable( inputdata_t &inputdata )
{
	m_bDisabled = true;
}

void CRD_Map_Texture::InputToggle( inputdata_t &inputdata )
{
	m_bDisabled = !m_bDisabled;
}
#endif
