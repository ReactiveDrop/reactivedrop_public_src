#include "cbase.h"
#include "c_asw_button_area.h"
#include "c_asw_marine.h"
#include "c_asw_player.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "c_asw_door.h"
#include "asw_marine_profile.h"
#include "asw_util_shared.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Button_Area, DT_ASW_Button_Area, CASW_Button_Area )
	RecvPropInt			(RECVINFO(m_iHackLevel)),
	RecvPropBool		(RECVINFO(m_bIsLocked)),
	RecvPropBool		(RECVINFO(m_bIsDoorButton)),
	RecvPropBool(RECVINFO(m_bIsInUse)),
	RecvPropFloat(RECVINFO(m_fHackProgress)),	
	RecvPropBool(RECVINFO(m_bNoPower)),
	RecvPropBool(RECVINFO(m_bWaitingForInput)),

	RecvPropString( RECVINFO( m_NoPowerMessage ) ),
	RecvPropString		(RECVINFO(m_UsePanelMessage)),
	RecvPropString		(RECVINFO(m_NeedTechMessage)),
	RecvPropString		(RECVINFO(m_ExitPanelMessage)),
	RecvPropString		(RECVINFO(m_HackPanelMessage)),

	RecvPropBool		(RECVINFO(m_bNeedsTech)),
	RecvPropFloat		(RECVINFO(m_flHoldTime)),
END_RECV_TABLE()

bool C_ASW_Button_Area::s_bLoadedLockedIconTexture = false;
int  C_ASW_Button_Area::s_nLockedIconTextureID = -1;
bool C_ASW_Button_Area::s_bLoadedOpenIconTexture = false;
int  C_ASW_Button_Area::s_nOpenIconTextureID = -1;
bool C_ASW_Button_Area::s_bLoadedCloseIconTexture = false;
int  C_ASW_Button_Area::s_nCloseIconTextureID = -1;
bool C_ASW_Button_Area::s_bLoadedUseIconTexture = false;
int  C_ASW_Button_Area::s_nUseIconTextureID = -1;
bool C_ASW_Button_Area::s_bLoadedHackIconTexture = false;
int  C_ASW_Button_Area::s_nHackIconTextureID = -1;
bool C_ASW_Button_Area::s_bLoadedNoPowerIconTexture = false;
int  C_ASW_Button_Area::s_nNoPowerIconTextureID = -1;

C_ASW_Button_Area::C_ASW_Button_Area()
{
	m_bOldWaitingForInput = false;
}

C_ASW_Door* C_ASW_Button_Area::GetDoor()
{
	CBaseEntity* pUseTargetH = GetUseTargetHandle().Get();
	if ( pUseTargetH && pUseTargetH->Classify() == CLASS_ASW_DOOR )
		return assert_cast<C_ASW_Door*>(pUseTargetH);
	return NULL;
}

// use icon textures

int C_ASW_Button_Area::GetLockedIconTextureID()
{
	if (!s_bLoadedLockedIconTexture)
	{
		// load the portrait textures
		s_nLockedIconTextureID = vgui::surface()->CreateNewTextureID();		
		vgui::surface()->DrawSetTextureFile( s_nLockedIconTextureID, "vgui/swarm/UseIcons/PanelLocked", true, false);
		s_bLoadedLockedIconTexture = true;
	}

	return s_nLockedIconTextureID;
}
int C_ASW_Button_Area::GetOpenIconTextureID()
{
	if (!s_bLoadedOpenIconTexture)
	{
		// load the portrait textures
		s_nOpenIconTextureID = vgui::surface()->CreateNewTextureID();		
		vgui::surface()->DrawSetTextureFile( s_nOpenIconTextureID, "vgui/swarm/UseIcons/PanelUnlocked", true, false);
		s_bLoadedOpenIconTexture = true;
	}

	return s_nOpenIconTextureID;
}
int C_ASW_Button_Area::GetCloseIconTextureID()
{
	if (!s_bLoadedCloseIconTexture)
	{
		// load the portrait textures
		s_nCloseIconTextureID = vgui::surface()->CreateNewTextureID();		
		vgui::surface()->DrawSetTextureFile( s_nCloseIconTextureID, "vgui/swarm/UseIcons/PanelUnlocked", true, false);
		s_bLoadedCloseIconTexture = true;
	}

	return s_nCloseIconTextureID;
}
int C_ASW_Button_Area::GetUseIconTextureID()
{
	if (!s_bLoadedUseIconTexture)
	{
		// load the portrait textures
		s_nUseIconTextureID = vgui::surface()->CreateNewTextureID();		
		vgui::surface()->DrawSetTextureFile( s_nUseIconTextureID, "vgui/swarm/UseIcons/PanelUnlocked", true, false);
		s_bLoadedUseIconTexture = true;
	}

	return s_nUseIconTextureID;
}
int C_ASW_Button_Area::GetHackIconTextureID()
{
	if (!s_bLoadedHackIconTexture)
	{
		// load the portrait textures
		s_nHackIconTextureID = vgui::surface()->CreateNewTextureID();		
		vgui::surface()->DrawSetTextureFile( s_nHackIconTextureID, "vgui/swarm/UseIcons/PanelLocked", true, false);
		s_bLoadedHackIconTexture = true;
	}

	return s_nHackIconTextureID;
}

int C_ASW_Button_Area::GetNoPowerIconTextureID()
{
	if (!s_bLoadedNoPowerIconTexture)
	{
		// load the portrait textures
		s_nNoPowerIconTextureID = vgui::surface()->CreateNewTextureID();		
		vgui::surface()->DrawSetTextureFile( s_nNoPowerIconTextureID, "vgui/swarm/UseIcons/PanelNoPower", true, false);
		s_bLoadedNoPowerIconTexture = true;
	}

	return s_nNoPowerIconTextureID;
}

bool C_ASW_Button_Area::GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser)
{
	action.UseIconRed = 255;
	action.UseIconGreen = 255;
	action.UseIconBlue = 255;
	action.bShowUseKey = true;
	action.iInventorySlot = -1;
	if ( !HasPower() || !CheckHeldObject( pUser ) )
	{
		action.iUseIconTexture = GetNoPowerIconTextureID();
		TryLocalize( GetNoPowerText(), action.wszText, sizeof( action.wszText ) );
		action.UseTarget = this;
		action.fProgress = GetHackProgress();
		action.bShowUseKey = false;
		return true;
	}
	if (IsLocked())
	{
		C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pUser );
		CASW_Marine_Profile *pProfile = pMarine ? pMarine->GetMarineProfile() : NULL;

		if ( pProfile && ( pProfile->CanHack() || !NeedsTech() ) )
		{
			action.iUseIconTexture = GetHackIconTextureID();
			TryLocalize( GetHackIconText( pUser ), action.wszText, sizeof( action.wszText ) );
			action.UseTarget = this;
			action.fProgress = GetHackProgress();
		}
		else
		{
			action.iUseIconTexture = GetLockedIconTextureID();
			TryLocalize( GetLockedIconText(), action.wszText, sizeof( action.wszText ) );
			action.UseTarget = this;
			action.fProgress = GetHackProgress();
		}
	}

	else
	{
		action.iUseIconTexture = GetUseIconTextureID();
		TryLocalize( GetUseIconText(), action.wszText, sizeof( action.wszText ) );
		action.UseTarget = this;
		action.fProgress = -1;

		if ( m_flHoldTime > 0 )
		{
			C_ASW_Player *pPlayer = pUser->GetCommander();
			if ( pPlayer && pUser->IsInhabited() && pPlayer->m_flUseKeyDownTime != 0.0f && (gpGlobals->curtime - pPlayer->m_flUseKeyDownTime) > 0.2f )
			{
				action.fProgress = ( ( gpGlobals->curtime - pPlayer->m_flUseKeyDownTime ) - 0.2f ) / ( m_flHoldTime - 0.2f );
				action.fProgress = clamp<float>( action.fProgress, 0.0f, 1.0f );
				action.bNoFadeIfSameUseTarget = true;
				action.bShowUseKey = false;
			}
			else
			{
				action.bShowHoldButtonUseKey = true;
			}
		}
	}
	return true;
}

const char *C_ASW_Button_Area::GetNoPowerText()
{
	const char *szCustom = m_NoPowerMessage;
	if ( !szCustom || Q_strlen( szCustom ) <= 0 )
		return "#asw_no_power";

	return szCustom;
}

const char *C_ASW_Button_Area::GetUseIconText()
{
	const char *szCustom = m_UsePanelMessage;
	if ( !szCustom || Q_strlen( szCustom ) <= 0 )
		return "#asw_use_panel";

	return szCustom;
}

const char *C_ASW_Button_Area::GetLockedIconText()
{
	const char *szCustom = m_NeedTechMessage;
	if ( !szCustom || Q_strlen( szCustom ) <= 0 )
		return "#asw_requires_tech";

	return szCustom;
}

const char *C_ASW_Button_Area::GetHackIconText( C_ASW_Inhabitable_NPC *pUser )
{
	if ( m_bIsInUse && pUser && pUser->m_hUsingEntity.Get() == this )
	{
		const char *szCustomExitPanelMessage = m_ExitPanelMessage;
		if ( !szCustomExitPanelMessage || Q_strlen( szCustomExitPanelMessage ) <= 0 )
			return "#asw_exit_panel";

		return szCustomExitPanelMessage;
	}

	const char *szCustomHackPanelMessage = m_HackPanelMessage;
	if ( !szCustomHackPanelMessage || Q_strlen( szCustomHackPanelMessage ) <= 0 )
		return "#asw_hack_panel";

	return szCustomHackPanelMessage;
}
