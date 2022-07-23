#ifndef _INCLUDED_IASW_FADE_LIST_H
#define _INCLUDED_IASW_FADE_LIST_H
#ifdef _WIN32
#pragma once
#endif

#include "util_shared.h"
#include "asw_player_shared.h"

DECLARE_AUTO_LIST( IASW_Fade_List_ );

class C_ASW_Inhabitable_NPC;

class IASW_Fade_List : public IASW_Fade_List_
{
public:
	static int s_iFadeReflectionDepth;

	IASW_Fade_List();

	byte m_nSavedAlpha;

	static void DisableFading();
	static void EnableFading();

protected:
	ASW_Controls_t m_iLastControls;
	CHandle<C_ASW_Inhabitable_NPC> m_hLastNPC;
	float m_flInterpStart;
	byte m_nNormalOpacity;
	byte m_nFadeOpacity;
	bool m_bFaded;
	bool m_bAllowFade;

	void OnDataChangedImpl( DataUpdateType_t updateType );
	void ClientThinkImpl( const Vector & vecFadeOrigin );
};

#define IMPLEMENT_ASW_FADE_LIST( vecFadeOrigin ) \
	IMPLEMENT_AUTO_LIST_GET(); \
	virtual void OnDataChanged( DataUpdateType_t updateType ) { BaseClass::OnDataChanged( updateType ); OnDataChangedImpl( updateType ); } \
	virtual void ClientThink() { BaseClass::ClientThink(); ClientThinkImpl( vecFadeOrigin ); }

#endif	// _INCLUDED_IASW_FADE_LIST_H
