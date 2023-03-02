#include "cbase.h"
#include "asw_button_area.h"
#include "asw_door.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_marine_profile.h"
#include "asw_hack_wire_tile.h"
#include "asw_marine_speech.h"
#include "asw_util_shared.h"
#include "asw_director.h"
#include "asw_gamerules.h"
#include "asw_achievements.h"
#include "cvisibilitymonitor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( trigger_asw_button_area, CASW_Button_Area );

#define ASW_MEDAL_WORTHY_WIRE_HACK 60

BEGIN_DATADESC( CASW_Button_Area )
	DEFINE_FIELD(m_bIsDoorButton, FIELD_BOOLEAN),
	DEFINE_FIELD(m_hDoorHack, FIELD_EHANDLE),
	DEFINE_FIELD(m_bIsInUse, FIELD_BOOLEAN),
	DEFINE_FIELD(m_fHackProgress, FIELD_FLOAT),
	DEFINE_FIELD(m_fStartedHackTime, FIELD_FLOAT),
	DEFINE_FIELD(m_fLastButtonUseTime, FIELD_TIME),
	DEFINE_FIELD(m_bWaitingForInput, FIELD_BOOLEAN ),
	DEFINE_FIELD(m_bWasLocked, FIELD_BOOLEAN ),

	//DEFINE_KEYFIELD(m_iHackLevel, FIELD_INTEGER, "hacklevel" ),
	DEFINE_KEYFIELD(m_bIsLocked, FIELD_BOOLEAN, "locked" ),
	DEFINE_KEYFIELD(m_bNoPower, FIELD_BOOLEAN, "nopower" ),
	DEFINE_KEYFIELD(m_bNeedsTech, FIELD_BOOLEAN, "needstech"),
	
	DEFINE_KEYFIELD(m_bChangePanelSkin, FIELD_BOOLEAN, "changepanelskin"),
	DEFINE_KEYFIELD(m_bUseAfterHack, FIELD_BOOLEAN, "useafterhack" ),
	DEFINE_KEYFIELD(m_bDisableAfterUse, FIELD_BOOLEAN, "disableafteruse" ),

	DEFINE_KEYFIELD( m_iWireColumns, FIELD_INTEGER, "wirecolumns"),
	DEFINE_KEYFIELD( m_iWireRows, FIELD_INTEGER, "wirerows"),
	DEFINE_KEYFIELD( m_iNumWires, FIELD_INTEGER, "numwires"),

	DEFINE_KEYFIELD( m_flHoldTime, FIELD_FLOAT, "HoldTime" ),
	DEFINE_KEYFIELD( m_bDestroyHeldObject, FIELD_BOOLEAN, "DestroyHeldObject" ),

	DEFINE_INPUTFUNC( FIELD_VOID,	"PowerOn",	InputPowerOn ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"PowerOff",	InputPowerOff ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ResetHack",	InputResetHack ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Unlock",	InputUnlock ),
	DEFINE_OUTPUT( m_OnFastHackFailed, "OnFastHackFailed" ),
	DEFINE_OUTPUT( m_OnButtonHackStarted, "OnButtonHackStarted" ),
	DEFINE_OUTPUT( m_OnButtonHackAt25Percent, "OnButtonHackAt25Percent" ),
	DEFINE_OUTPUT( m_OnButtonHackAt50Percent, "OnButtonHackAt50Percent" ),
	DEFINE_OUTPUT( m_OnButtonHackAt75Percent, "OnButtonHackAt75Percent" ),	
	DEFINE_OUTPUT( m_OnButtonHackCompleted, "OnButtonHackCompleted" ),
	DEFINE_OUTPUT( m_OnButtonActivated, "OnButtonActivated" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CASW_Button_Area, DT_ASW_Button_Area)
	SendPropInt			(SENDINFO(m_iHackLevel)),
	SendPropBool		(SENDINFO(m_bIsLocked)),
	SendPropBool		(SENDINFO(m_bIsDoorButton)),
	SendPropBool		(SENDINFO(m_bIsInUse)),
	SendPropFloat		(SENDINFO(m_fHackProgress)),
	SendPropBool		(SENDINFO(m_bNoPower)),	
	SendPropBool		(SENDINFO(m_bWaitingForInput)),	

	SendPropString		(SENDINFO( m_NoPowerMessage ) ),
	SendPropString		(SENDINFO(m_UsePanelMessage)),
	SendPropString		(SENDINFO(m_NeedTechMessage)),
	SendPropString		(SENDINFO(m_ExitPanelMessage)),
	SendPropString		(SENDINFO(m_HackPanelMessage)),

	SendPropBool		(SENDINFO(m_bNeedsTech)),
	SendPropFloat		(SENDINFO(m_flHoldTime)),
END_SEND_TABLE()

ConVar asw_ai_button_hacking_scale( "asw_ai_button_hacking_scale", "0.3", FCVAR_CHEAT, "Button panel hacking speed scale for AI marines" );
ConVar asw_tech_order_hack_range( "asw_tech_order_hack_range", "1200", FCVAR_CHEAT, "Max range when searching for a nearby AI tech to hack for you" );
ConVar asw_debug_button_skin( "asw_debug_button_skin", "0", FCVAR_CHEAT, "If set, button panels will output skin setting details" );
extern ConVar asw_simple_hacking;
extern ConVar asw_debug_medals;

CASW_Button_Area::CASW_Button_Area()
{
	//Msg("[S] CASW_Button_Area created\n");
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
	m_iAliensKilledBeforeHack = 0;

	m_bChangePanelSkin = true;
	m_bNeedsTech = true;

	m_iHackLevel = 6;

	m_flHoldTime = -1;
	m_bDestroyHeldObject = false;

	m_UsePanelMessage.GetForModify()[0] = '\0';
	m_NeedTechMessage.GetForModify()[0] = '\0';
	m_ExitPanelMessage.GetForModify()[0] = '\0';
	m_HackPanelMessage.GetForModify()[0] = '\0';
}

CASW_Button_Area::~CASW_Button_Area()
{
	if (m_hDoorHack.Get())
	{
		UTIL_Remove(m_hDoorHack.Get());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CASW_Button_Area::Spawn( void )
{
	if (m_flWait == -1)
		m_bDisableAfterUse = true;

	BaseClass::Spawn();
	Precache();

	// check if this button is linked to a door or not
	CBaseEntity* pUseTarget = m_hUseTarget.Get();
	if ( pUseTarget && pUseTarget->Classify() == CLASS_ASW_DOOR )
	{
		m_bIsDoorButton = true;
	}
	else
	{
		m_bIsDoorButton = false;
	}

	if ( m_bIsLocked )
	{
		m_bWasLocked = true;
	}

	UpdateWaitingForInput();
	if ( m_bChangePanelSkin )
	{
		UpdatePanelSkin();
	}
}

void CASW_Button_Area::Precache()
{
	PrecacheScriptSound("ASWComputer.HackComplete");
	PrecacheScriptSound("ASWComputer.AccessDenied");
	PrecacheScriptSound("ASWComputer.TimeOut");
	PrecacheScriptSound("ASWButtonPanel.TileLit");
}

void CASW_Button_Area::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( !HasPower() || !ASWGameResource() || !CheckHeldObject( pNPC ) )
	{
		return;
	}

	if ( m_bIsLocked )
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
		if ( !pMarine )
		{
			return;
		}

		if ( nHoldType == ASW_USE_HOLD_START )
		{
			return;
		}

		if ( pMarine->GetMarineProfile()->CanHack() || !m_bNeedsTech )
		{
			// can hack, get the player to launch his hacking window
			if ( !m_bIsInUse )
			{
				if ( pMarine->StartUsing( this ) )
				{
					if ( GetHackProgress() <= 0 && pMarine->GetMarineResource() )
					{
						pMarine->GetMarineResource()->m_iDamageTakenDuringHack = 0;
					}
					m_iAliensKilledBeforeHack = ASWGameResource()->GetAliensKilledInThisMission();

					m_OnButtonHackStarted.FireOutput( pMarine, this );
					if ( !asw_simple_hacking.GetBool() && pMarine->IsInhabited() )
					{
						if ( !GetCurrentHack() ) // if we haven't created a hack object for this computer yet, then create one
						{
							m_hDoorHack = CreateEntityByName( "asw_hack_wire_tile" );
						}

						if ( GetCurrentHack() )
						{
							GetCurrentHack()->InitHack( pMarine->GetCommander(), pMarine, this );
						}
					}

					if ( m_iHackLevel > 20 )
					{
						pMarine->GetMarineSpeech()->Chatter( CHATTER_HACK_LONG_STARTED );
					}
					else
					{
						pMarine->GetMarineSpeech()->Chatter( CHATTER_HACK_STARTED );
					}
				}
			}
			else
			{
				if ( pMarine->m_hUsingEntity.Get() == this )
				{
					pMarine->StopUsing();
				}
			}
		}
		else
		{
			EmitSound( "ASWComputer.AccessDenied" );

			// check for a nearby AI tech marine
			float flMarineDistance;
			CASW_Marine *pTech = UTIL_ASW_NearestMarine( WorldSpaceCenter(), flMarineDistance, MARINE_CLASS_TECH, true );
			if ( pTech && flMarineDistance < asw_tech_order_hack_range.GetFloat() )
			{
				pTech->OrderHackArea( this );
			}

			return;
		}
	}
	else if ( m_flHoldTime > 0 )
	{
		if ( nHoldType == ASW_USE_RELEASE_QUICK )
		{
			if ( pNPC )
			{
				pNPC->StopUsing();
			}

			return;
		}

		if ( nHoldType == ASW_USE_HOLD_START && pNPC )
		{
			CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
			if ( pMarine && !pMarine->m_hUsingEntity )
			{
				pMarine->GetMarineSpeech()->Chatter( CHATTER_USE );
			}

			pNPC->StartUsing( this );
		}

		if ( nHoldType == ASW_USE_HOLD_RELEASE_FULL )
		{
			if ( pNPC )
			{
				pNPC->StopUsing();

				CASW_Player *pPlayer = pNPC->GetCommander();
				if ( pPlayer && pNPC->IsInhabited() )
				{
					pPlayer->m_hUseKeyDownEnt = NULL;
					pPlayer->m_flUseKeyDownTime = 0;
				}
			}

			ActivateUnlockedButton( pNPC );
		}
	}
	else
	{
		if ( nHoldType == ASW_USE_HOLD_START )
			return;

		CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
		if ( pMarine )
		{
			pMarine->GetMarineSpeech()->Chatter( CHATTER_USE );
		}

		ActivateUnlockedButton( pNPC );
	}
}

void CASW_Button_Area::ActivateUnlockedButton( CASW_Inhabitable_NPC *pNPC )
{
	// don't use the button if we're in the delay between using
	if ( m_fLastButtonUseTime != 0 && gpGlobals->curtime < m_fLastButtonUseTime + m_flWait )
		return;
	if ( !pNPC )
		return;

	if ( !RequirementsMet( pNPC ) )
		return;

	if ( m_iHeldObjectName.Get() != NULL_STRING && *STRING( m_iHeldObjectName.Get() ) )
	{
		// We require a held object. If it's not there anymore, bail.
		if ( !CheckHeldObject( pNPC ) )
		{
			return;
		}

		// No more bailing after this point.

		if ( m_bDestroyHeldObject )
		{
			CBaseCombatWeapon *pWeapon = pNPC->GetActiveWeapon();
			pNPC->Weapon_Drop( pWeapon, NULL, NULL );
			UTIL_Remove( pWeapon );
		}
	}

	if ( m_bIsDoorButton )
	{
		// if the door isn't sealed (or greater than a certain amount of damage?)
		//  then make it open
		CASW_Door *pDoor = GetDoor();
		if ( pDoor )
		{
			if ( pDoor->GetSealAmount() > 0 )
			{
				//Msg("Door mechanism not responding.  Maintenance Division has been notified of the problem.\n");
			}
			else
			{
				//Msg("Toggling door...\n");
				variant_t emptyVariant;
				pDoor->AcceptInput( "Toggle", pNPC, this, emptyVariant, 0 );
			}
		}
	}

	// send our 'activated' output
	m_OnButtonActivated.FireOutput( pNPC, this );

	// Fire event
	IGameEvent *event = gameeventmanager->CreateEvent( "button_area_used" );
	if ( event )
	{
		CASW_Player *pPlayer = pNPC->GetCommander();

		event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
		event->SetInt( "entindex", entindex() );
		gameeventmanager->FireEvent( event );
	}

	m_fLastButtonUseTime = gpGlobals->curtime;

	UpdateWaitingForInput();

	if ( m_bDisableAfterUse )
	{
		m_bUseAreaEnabled = false;
		UTIL_Remove( this );
	}
}

CASW_Door* CASW_Button_Area::GetDoor()
{
	CBaseEntity* pUseTarget = m_hUseTarget.Get();
	if ( pUseTarget && pUseTarget->Classify() == CLASS_ASW_DOOR )
		return assert_cast<CASW_Door*>(pUseTarget);
	return NULL;
}

CASW_Hack* CASW_Button_Area::GetCurrentHack()
{
	return dynamic_cast<CASW_Hack*>(m_hDoorHack.Get());
}

// traditional Swarm hacking
void CASW_Button_Area::NPCUsing(CASW_Inhabitable_NPC *pNPC, float deltatime)
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
	{
		return;
	}

	if ( m_bIsInUse && m_bIsLocked && ( asw_simple_hacking.GetBool() || !pNPC->IsInhabited() ) )
	{
		float fTime = ( deltatime * ( 1.0f / ( ( float )m_iHackLevel ) ) );
		// boost fTime by the marine's hack skill
		fTime *= MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_HACKING, ASW_MARINE_SUBSKILL_HACKING_SPEED_SCALE );
		fTime *= asw_ai_button_hacking_scale.GetFloat();
		SetHackProgress( m_fHackProgress + fTime, pMarine );
	}
}

void CASW_Button_Area::NPCStartedUsing( CASW_Inhabitable_NPC *pNPC )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
	{
		return;
	}

	m_bIsInUse = true;
	UpdateWaitingForInput();

	if ( ASWDirector() && m_bIsLocked )
	{
		ASWDirector()->OnMarineStartedHack( pMarine, this );
	}
}

void CASW_Button_Area::NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
	{
		return;
	}

	//Msg( "Marine stopped using button area\n" );
	m_bIsInUse = false;
	UpdateWaitingForInput();

	if ( GetCurrentHack() )	// notify our current hack that we've stopped using the console
	{
		GetCurrentHack()->MarineStoppedUsing( pMarine );
	}
}

bool CASW_Button_Area::IsActive( void )
{
	return ( m_bIsInUse || !m_bIsLocked );
}

void CASW_Button_Area::InputPowerOn( inputdata_t &inputdata )
{
	m_bNoPower = false;
	UpdateWaitingForInput();
	if ( m_bChangePanelSkin )
	{
		UpdatePanelSkin();
	}
}

void CASW_Button_Area::InputPowerOff( inputdata_t &inputdata )
{
	m_bNoPower = true;
	UpdateWaitingForInput();
	if ( m_bChangePanelSkin )
	{
		UpdatePanelSkin();
	}
}

void CASW_Button_Area::InputUnlock( inputdata_t &inputdata )
{
	if (m_bIsLocked)
	{
		m_bIsLocked = false;
		m_fHackProgress = 1.0f;
		UpdateWaitingForInput();
		if ( m_bChangePanelSkin )
		{
			UpdatePanelSkin();
		}

		if ( inputdata.pActivator && inputdata.pActivator->Classify() == CLASS_ASW_MARINE )
		{
			CASW_Marine* pMarine = assert_cast<CASW_Marine*>(inputdata.pActivator);

			// if set to use on unlock, then do so
			if ( m_bUseAfterHack && pMarine && m_flHoldTime <= 0 )
			{
				ActivateUnlockedButton(pMarine);
			}
		}
	}
}

void CASW_Button_Area::InputResetHack( inputdata_t &inputdata )
{
	if ( !m_bIsInUse )
	{
		m_fStartedHackTime = false;
		SetHackProgress(0, NULL);
		m_bIsLocked = true;
		UpdateWaitingForInput();
		if ( m_bChangePanelSkin )
		{
			UpdatePanelSkin();
		}
		if (m_hDoorHack.Get())
		{
			// need to delete the hack
			UTIL_Remove(m_hDoorHack.Get());
			m_hDoorHack = NULL;
		}
	}
}

void CASW_Button_Area::SetHackProgress(float f, CASW_Marine *pMarine)
{
	CBaseEntity *pActor = pMarine;
	if ( !pActor )
	{
		pActor = this;
	}

	if (m_fHackProgress <= 0 && f > 0 && m_fStartedHackTime == 0)
	{
		m_fStartedHackTime = gpGlobals->curtime;
	}
	if (m_fHackProgress < 0.25f && f >= 0.25f)
	{
		m_OnButtonHackAt25Percent.FireOutput(pActor, this);
	}
	if (m_fHackProgress < 0.5f && f >= 0.5f)
	{
		m_OnButtonHackAt50Percent.FireOutput(pActor, this);
		if (pMarine)
			pMarine->GetMarineSpeech()->Chatter(CHATTER_HACK_HALFWAY);
	}
	if (m_fHackProgress < 0.75f && f >= 0.75f)
	{
		m_OnButtonHackAt75Percent.FireOutput(pActor, this);
	}
	if (m_fHackProgress < 1.0f && f >= 1.0f)
	{	
		if (asw_simple_hacking.GetBool() && pMarine)
		{
			pMarine->StopUsing();
		}

		float time_taken = gpGlobals->curtime - m_fStartedHackTime;
		// decide if this was a fast hack or not
		float fSpeedScale = 1.0f;
		if (pMarine)
			fSpeedScale *= MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_HACKING, ASW_MARINE_SUBSKILL_HACKING_SPEED_SCALE);
		float ideal_time = UTIL_ASW_CalcFastDoorHackTime(m_iWireRows, m_iWireColumns, m_iNumWires, m_iHackLevel, fSpeedScale);

		bool bFastHack = time_taken <= ideal_time;
		if (asw_debug_medals.GetBool())
		{
			Msg("Finished door hack, fast = %d.\n", bFastHack);
			Msg(" ideal time is %f you took %f\n", ideal_time, time_taken);
		}
		CASW_Marine_Resource* pMR = pMarine->GetMarineResource();
		if (bFastHack && pMarine && pMR)
		{
			pMR->m_iFastDoorHacks++;
			CASW_Player* pCommander = pMarine->GetCommander();
			if ( pMarine->IsInhabited() && pCommander)
			{
				pCommander->AwardAchievement( ACHIEVEMENT_ASW_FAST_WIRE_HACKS );
			}

			IGameEvent *pEvent = gameeventmanager->CreateEvent( "fast_hack_success" );
			if ( pEvent )
			{
				pEvent->SetInt( "entindex", entindex() );
				pEvent->SetInt( "marine", pMarine->entindex() );
				gameeventmanager->FireEvent( pEvent );
			}
		}

		CASW_Hack* pHack = GetCurrentHack();
		if ( pHack )
		{
			pHack->OnHackComplete();
		}

		if ( pMarine && pMR && pMR->m_iDamageTakenDuringHack <= 0 )
		{
			int nAliensKilled = ASWGameResource() ? ASWGameResource()->GetAliensKilledInThisMission() : 0;
			if ( ( nAliensKilled - m_iAliensKilledBeforeHack ) > 10 )
			{
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
				{
					CASW_Player* pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
					if ( !pPlayer || !pPlayer->IsConnected() )
						continue;
					
					CASW_Marine* pOtherMarine = CASW_Marine::AsMarine( pPlayer->GetNPC() );
					if ( !pOtherMarine || pOtherMarine == pMarine )
						continue;

					CASW_Marine_Resource* pMROther = pOtherMarine->GetMarineResource();
					if ( pMROther )
					{
						pMROther->m_bProtectedTech = true;
					}
					pPlayer->AwardAchievement( ACHIEVEMENT_ASW_PROTECT_TECH );
				}
			}
		}

		// unlock it
		m_OnButtonHackCompleted.FireOutput(pActor, this);
		m_bIsLocked = false;
		UpdateWaitingForInput();
		if ( m_bChangePanelSkin )
		{
			UpdatePanelSkin();
		}

		EmitSound("ASWComputer.HackComplete");

		if ( pMarine && bFastHack )
		{
			pMarine->GetMarineSpeech()->QueueChatter(CHATTER_HACK_BUTTON_FINISHED, gpGlobals->curtime + 2.0f, gpGlobals->curtime + 3.0f);
		}

		// if set to use on unlock, then do so
		if ( m_bUseAfterHack && pMarine && m_flHoldTime <= 0 )
		{
			ActivateUnlockedButton(pMarine);
		}

		if ( pMarine && !pMarine->IsInhabited() )
		{
			pMarine->StopUsing();
		}
	}

	m_fHackProgress = clamp( f, 0.0f, 1.0f );
}

bool CASW_Button_Area::KeyValue( const char *szKeyName, const char *szValue )
{	
	if ( FStrEq( szKeyName, "nopowermessage" ) )
	{
		Q_strncpy( m_NoPowerMessage.GetForModify(), szValue, 255 );
		return true;
	}	
	else if ( FStrEq( szKeyName, "usepanelmessage" ) )
	{
		Q_strncpy( m_UsePanelMessage.GetForModify(), szValue, 255 );
		return true;
	}	
	else if ( FStrEq( szKeyName, "needtechmessage" ) )
	{
		Q_strncpy( m_NeedTechMessage.GetForModify(), szValue, 255 );
		return true;
	}	
	else if ( FStrEq( szKeyName, "exitpanelmessage" ) )
	{
		Q_strncpy( m_ExitPanelMessage.GetForModify(), szValue, 255 );
		return true;
	}	
	else if ( FStrEq( szKeyName, "hackpanelmessage" ) )
	{
		Q_strncpy( m_HackPanelMessage.GetForModify(), szValue, 255 );
		return true;
	}	
	return BaseClass::KeyValue( szKeyName, szValue );
}

bool CASW_Button_Area::WaitingForInputVismonEvaluator( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer )
{
	CASW_Button_Area *pButtonArea = static_cast< CASW_Button_Area* >( pVisibleEntity );
	return pButtonArea->m_bWaitingForInput && pButtonArea->m_bUseAreaEnabled;
}

bool CASW_Button_Area::WaitingForInputVismonCallback( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer )
{
	CASW_Button_Area *pButtonArea = static_cast< CASW_Button_Area* >( pVisibleEntity );
	if ( !pButtonArea->m_bUseAreaEnabled )
		return false;

	IGameEvent * event = gameeventmanager->CreateEvent( "button_area_active" );
	if ( event )
	{
		event->SetInt( "userid", pViewingPlayer->GetUserID() );
		event->SetInt( "entindex", pButtonArea->entindex() );
		event->SetInt( "prop", pButtonArea->m_hPanelProp.Get() ? pButtonArea->m_hPanelProp.Get()->entindex() : pButtonArea->entindex() );
		event->SetBool( "locked", pButtonArea->IsLocked() );
		gameeventmanager->FireEvent( event );
	}

	return false;
}

void CASW_Button_Area::UpdateWaitingForInput()
{
	bool bOldWaitingForInput = m_bWaitingForInput;

	bool bSet = false;

	if ( m_bIsDoorButton )
	{
		CASW_Door *pDoor = GetDoor();
		if ( pDoor && pDoor->IsAutoOpen() )
		{
			m_bWaitingForInput = false;
			bSet = true;
		}
	}

	if ( !bSet )
	{
		m_bWaitingForInput = ASWGameRules()->GetGameState() == ASW_GS_INGAME && 
							 ( !m_bNoPower && !m_bIsInUse && ( m_bIsLocked || ( !m_bWasLocked && m_fLastButtonUseTime == 0 ) ) );
	}

	if ( !bOldWaitingForInput && m_bWaitingForInput )
	{
		VisibilityMonitor_AddEntity( this, asw_visrange_generic.GetFloat() * 0.9f, &CASW_Button_Area::WaitingForInputVismonCallback, &CASW_Button_Area::WaitingForInputVismonEvaluator );
	}
	else if ( bOldWaitingForInput && !m_bWaitingForInput )
	{
		VisibilityMonitor_RemoveEntity( this );

		IGameEvent * event = gameeventmanager->CreateEvent( "button_area_inactive" );
		if ( event )
		{
			event->SetInt( "entindex", entindex() );
			gameeventmanager->FireEvent( event );
		}
	}
}

// updates the panel prop (if any) with a skin to reflect the button's state
void CASW_Button_Area::UpdatePanelSkin()
{
	CBaseEntity* pPanel = m_hPanelProp.Get();

	if ( !pPanel )
		return;

	if ( asw_debug_button_skin.GetBool() )
	{
		Msg( "CASW_Button_Area:%d: UpdatePanelSkin\n", entindex() );
	}
	CBaseAnimating *pAnim = pPanel->GetBaseAnimating();
	CBaseEntity *pFindAnim = NULL;
	while (pAnim)
	{		
		if (HasPower())
		{
			if (m_bIsLocked)
			{
				pAnim->m_nSkin = 1;	// locked skin
				if ( asw_debug_button_skin.GetBool() )
				{
					Msg( "  Panel is locked, setting prop %d skin to %d\n", pAnim->entindex(), pAnim->m_nSkin.Get() );
				}
			}
			else
			{
				pAnim->m_nSkin = 2;	// unlocked skin
				if ( asw_debug_button_skin.GetBool() )
				{
					Msg( "  Panel is unlocked, setting prop %d skin to %d\n", pAnim->entindex(), pAnim->m_nSkin.Get() );
				}
			}
		}
		else
		{
			pAnim->m_nSkin = 0;	// no power skin
			if ( asw_debug_button_skin.GetBool() )
			{
				Msg( "  Panel is no power, setting prop %d skin to %d\n", pAnim->entindex(), pAnim->m_nSkin.Get() );
			}
		}

		if (m_bMultiplePanelProps)
		{
			pFindAnim = gEntList.FindEntityByName(pAnim, m_szPanelPropName);
			pAnim = pFindAnim ? pFindAnim->GetBaseAnimating() : NULL;
		}
		else
			pAnim = NULL;
	}
}
