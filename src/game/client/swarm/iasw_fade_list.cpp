#include "cbase.h"
#include "iasw_fade_list.h"
#include "c_func_asw_fade.h"
#include "c_prop_asw_fade.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_AUTO_LIST( IASW_Fade_List_ );

ConVar asw_fade_duration( "asw_fade_duration", "0.5", FCVAR_ARCHIVE, "", true, 0.01f, false, 0 );

static int s_iFadeReflectionDepth = 0;

IASW_Fade_List::IASW_Fade_List() : IASW_Fade_List_( true )
{
	m_iLastControls = 1;
	m_hLastMarine = NULL;
	m_flInterpStart = 0;
	m_bFaded = false;
}

void IASW_Fade_List::DisableFading()
{
	s_iFadeReflectionDepth++;

	if ( s_iFadeReflectionDepth != 1 )
		return;

	FOR_EACH_VEC( IASW_Fade_List_::AutoList(), i )
	{
		IASW_Fade_List *pItem = assert_cast<IASW_Fade_List *>( IASW_Fade_List_::AutoList()[i] );
		C_BaseEntity *pEnt = pItem->GetEntity();

		pItem->m_nSavedAlpha = pEnt->GetRenderAlpha();
		pEnt->SetRenderAlpha( pItem->m_nNormalOpacity );
	}
}

void IASW_Fade_List::EnableFading()
{
	s_iFadeReflectionDepth--;

	if ( s_iFadeReflectionDepth != 0 )
		return;

	FOR_EACH_VEC( IASW_Fade_List_::AutoList(), i )
	{
		IASW_Fade_List *pItem = assert_cast<IASW_Fade_List *>( IASW_Fade_List_::AutoList()[i] );
		C_BaseEntity *pEnt = pItem->GetEntity();

		pEnt->SetRenderAlpha( pItem->m_nSavedAlpha );
	}
}

void IASW_Fade_List::OnDataChangedImpl( DataUpdateType_t updateType )
{
	C_BaseEntity *pEnt = GetEntity();

	if ( updateType == DATA_UPDATE_CREATED )
	{
		pEnt->SetNextClientThink( CLIENT_THINK_ALWAYS );
		m_bFaded = false;
		m_nNormalOpacity = pEnt->GetRenderAlpha();
	}

	if ( pEnt->GetRenderMode() == kRenderNormal )
	{
		pEnt->SetRenderMode( kRenderTransTexture );
	}
}

void IASW_Fade_List::ClientThinkImpl( const Vector & vecFadeOrigin )
{
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer )
	{
		return;
	}

	C_ASW_Marine *pMarine = pPlayer->GetViewMarine();

	C_BaseEntity *pEnt = GetEntity();

	bool bFade = pPlayer->GetASWControls() == 1 && pMarine && pMarine->GetAbsOrigin().z <= vecFadeOrigin.z && m_bAllowFade;
	byte target = bFade ? m_nFadeOpacity : m_nNormalOpacity;
	byte prev = bFade ? m_nNormalOpacity : m_nFadeOpacity;
	if ( bFade != m_bFaded )
	{
		m_bFaded = bFade;
		m_flInterpStart = gpGlobals->curtime - fabs( ( m_nFadeOpacity != m_nNormalOpacity ) ? asw_fade_duration.GetFloat() * ( pEnt->GetRenderAlpha() - prev ) / ( m_nFadeOpacity - m_nNormalOpacity ) : asw_fade_duration.GetFloat() );
		m_flInterpStart = MAX( 0, m_flInterpStart );
	}

	if ( pPlayer->GetASWControls() != m_iLastControls || pMarine != m_hLastMarine.Get() || !m_bAllowFade )
	{
		m_iLastControls = pPlayer->GetASWControls();
		m_hLastMarine = pMarine;
		m_flInterpStart = 0;
		pEnt->SetRenderAlpha( target );
		return;
	}

	if ( m_flInterpStart + asw_fade_duration.GetFloat() <= gpGlobals->curtime )
	{
		pEnt->SetRenderAlpha( target );
	}
	else if ( m_flInterpStart > 0 )
	{
		pEnt->SetRenderAlpha( Lerp( ( gpGlobals->curtime - m_flInterpStart ) / asw_fade_duration.GetFloat(), prev, target ) );
	}
}
