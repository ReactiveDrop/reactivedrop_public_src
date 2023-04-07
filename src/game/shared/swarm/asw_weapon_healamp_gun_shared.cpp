#include "cbase.h"

#include "asw_weapon_healamp_gun_shared.h"
#ifdef CLIENT_DLL
	#include "c_asw_player.h"
	#include "c_asw_marine.h"
	#include "c_asw_marine_resource.h"
	#include "c_asw_game_resource.h"
	#include "asw_hud_crosshair.h"
	#include "asw_input.h"
#else
	#include "asw_player.h"
	#include "asw_marine.h"
	#include "asw_marine_speech.h"
	#include "asw_triggers.h"
	#include "asw_fail_advice.h"
	#include "asw_marine_resource.h"
#endif
#include "asw_gamerules.h"
#include "in_buttons.h"
#include "soundenvelope.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar asw_marine_special_heal_chatter_chance;
extern ConVar asw_heal_gun_start_heal_fade;
extern ConVar rd_medgun_infinite_ammo;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_HealAmp_Gun, DT_ASW_Weapon_HealAmp_Gun );

BEGIN_NETWORK_TABLE( CASW_Weapon_HealAmp_Gun, DT_ASW_Weapon_HealAmp_Gun )

#ifdef CLIENT_DLL
// recvprops
RecvPropTime( RECVINFO( m_fSlowTime ) ),
RecvPropInt( RECVINFO( m_FireState ) ),
RecvPropEHandle( RECVINFO( m_hHealEntity ) ),
RecvPropVector( RECVINFO( m_vecHealPos ) ),
RecvPropBool( RECVINFO( m_bIsBuffing ) ),
#else
// sendprops
SendPropTime( SENDINFO( m_fSlowTime ) ),
SendPropInt( SENDINFO( m_FireState ), Q_log2(ASW_HG_NUM_FIRE_STATES)+1, SPROP_UNSIGNED ),
SendPropEHandle( SENDINFO( m_hHealEntity ) ),
SendPropVector( SENDINFO( m_vecHealPos ) ),
SendPropBool( SENDINFO( m_bIsBuffing ) ),
#endif
END_NETWORK_TABLE()    

LINK_ENTITY_TO_CLASS( asw_weapon_healamp_gun, CASW_Weapon_HealAmp_Gun );
PRECACHE_WEAPON_REGISTER( asw_weapon_healamp_gun );

#if defined( CLIENT_DLL )
BEGIN_PREDICTION_DATA( CASW_Weapon_HealAmp_Gun )
	DEFINE_PRED_FIELD_TOL( m_fSlowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD( m_vecHealPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

CASW_Weapon_HealAmp_Gun::CASW_Weapon_HealAmp_Gun(void)
{
	m_bIsBuffing = false; 
}

void CASW_Weapon_HealAmp_Gun::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound( "ASW_BuffGrenade.ActiveLoop" );
	PrecacheScriptSound( "ASW_BuffGrenade.BuffLoop" );
	PrecacheScriptSound( "ASW_BuffGrenade.GrenadeActivate" );
	PrecacheScriptSound( "ASW_BuffGrenade.StartBuff" );

	PrecacheScriptSound( "ASW_Weapon.Empty" );

}

void CASW_Weapon_HealAmp_Gun::SetFiringState(ASW_Weapon_HealGunFireState_t state)
{
#ifdef CLIENT_DLL
	//Msg("[C] SetFiringState %d\n", state);
#else
	//Msg("[C] SetFiringState %d\n", state);
#endif

	// Check for transitions
	if ( m_FireState != state )
	{	
		if ( state == ASW_HG_FIRE_DISCHARGE || state == ASW_HG_FIRE_ATTACHED )
		{
			if ( m_bIsBuffing )
			{
				EmitSound ( "ASW_BuffGrenade.GrenadeActivate" );
			}
			else
			{
				EmitSound( "ASW_HealGun.StartSearch" );
			}
		}
		else if ( state == ASW_HG_FIRE_OFF )
		{
			//StopSound( "ASW_HealGun.SearchLoop" );
			EmitSound( "ASW_Tesla_Laser.Stop" );

		}
	}

	m_FireState = state;
}

void CASW_Weapon_HealAmp_Gun::PrimaryAttack()
{
	m_bIsBuffing = false;
	BaseClass::PrimaryAttack();
}

void CASW_Weapon_HealAmp_Gun::SecondaryAttack()
{
	/*// HealSelf();
	m_bIsBuffing = !m_bIsBuffing;

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.4f;

	EmitSound( "ASW_Weapon.Empty" );*/

	m_bIsBuffing = true;
	BaseClass::PrimaryAttack();
}

bool CASW_Weapon_HealAmp_Gun::TargetCanBeHealed( CBaseEntity* pTarget )
{	
	if ( pTarget->Classify() != CLASS_ASW_MARINE )
		return false;

	CASW_Marine *pMarine = assert_cast<CASW_Marine *>( pTarget );
	if ( !pMarine || pMarine->m_bKnockedOut )
		return false;

	if ( m_bIsBuffing )
	{
		if ( pMarine->GetHealth() <= 0 )
			return false;
	}
	else 
	{
		if ( pMarine->GetHealth() <= 0 || ( pMarine->GetHealth() + pMarine->m_iSlowHealAmount ) >= pMarine->GetMaxHealth() )
			return false;
	}

	return true;
}

void CASW_Weapon_HealAmp_Gun::HealSelf( void )
{
	if ( m_bIsBuffing )						// don't buff self
		return;

	BaseClass::HealSelf();
}

void CASW_Weapon_HealAmp_Gun::HealEntity( void )
{
	// verify target
	CASW_Marine *pMarine = GetMarine();
	EHANDLE hHealEntity = m_hHealEntity.Get();
	Assert( hHealEntity && hHealEntity->m_takedamage != DAMAGE_NO && pMarine );
	if ( !pMarine )
		return;

	if ( hHealEntity.Get()->Classify() != CLASS_ASW_MARINE )
		return;

	CASW_Marine *pTarget = static_cast<CASW_Marine*>( static_cast<CBaseEntity*>( hHealEntity.Get() ) );
	if ( !pTarget )
		return;

#ifdef GAME_DLL
	if ( m_bIsBuffing )
	{
		if ( m_iClip2 <= 0 )
			return;

		pTarget->AddDamageBuff( NULL, 1, (Class_T) CLASS_ASW_HEALAMP_GUN, pMarine );	// buff target for 1 second
	}
	else
	{
		if ( m_iClip1 <= 0 )
			return;

		// apply heal
		int iHealAmount = MIN( GetHealAmount(), pTarget->GetMaxHealth() - ( pTarget->GetHealth() + pTarget->m_iSlowHealAmount ) );

		if ( iHealAmount == 0 )
			return;

		pTarget->AddSlowHeal( iHealAmount, 2, pMarine, this );

		m_flTotalAmountHealed += iHealAmount;
		if ( m_flTotalAmountHealed > 50 )
		{
			// TODO: only fire this off if we've healed enough
			ASWFailAdvice()->OnMarineHealed();
		}

		if ( gpGlobals->curtime > m_flNextHealMessageTime )
		{
			// Fire event
			IGameEvent * event = gameeventmanager->CreateEvent( "player_heal" );
			if ( event )
			{
				CASW_Player *pPlayer = GetCommander();
				event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
				event->SetInt( "entindex", pTarget->entindex() );
				gameeventmanager->FireEvent( event );
			}

			m_flNextHealMessageTime = gpGlobals->curtime + 2.0f;
		}

		if ( ASWGameRules()->GetInfoHeal() )
		{
			ASWGameRules()->GetInfoHeal()->OnMarineHealed( GetMarine(), pTarget, this );
		}
	}

#endif

	if ( !rd_medgun_infinite_ammo.GetBool() )
	{
		// decrement ammo
		if ( m_bIsBuffing )
			m_iClip2--;
		else
			m_iClip1--;

#ifdef GAME_DLL
		DestroyIfEmpty( true, true );
#endif
	}


	// emit heal sound
	StartHealSound();

#ifdef GAME_DLL
	if ( !m_bIsBuffing )
	{
		bool bHealingSelf = ( pMarine == pTarget );
		bool bInfested = pTarget->IsInfested();

		if ( bInfested )
		{
			float fCurePercent = GetInfestationCureAmount();

			// cure infestation
			if ( fCurePercent > 0.0f )
			{
				// Cure infestation on a per bullet basis (the full clip does cure relative to 9 heal grenades)
				pTarget->CureInfestation( pMarine, this, 1.0f - ( ( 1.0f - fCurePercent ) / ( GetMaxClip1() / 9.0f ) ) );
			}
		}

		bool bSkipChatter = bInfested;
		if ( m_iClip1 <= 0 )
		{
			// Out of ammo
			ASWFailAdvice()->OnMedSatchelEmpty();

			pMarine->GetMarineSpeech()->Chatter( CHATTER_MEDS_NONE );
			CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
			if ( pMR )
			{
				char szName[256];
				pMR->GetDisplayName( szName, sizeof( szName ) );
				UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_out_of_meds", szName );
			}

			bSkipChatter = true;
		}
		else if ( m_iClip1 == 10 )
		{
			if ( pMarine->GetMarineSpeech()->Chatter( CHATTER_MEDS_LOW ) )
			{
				bSkipChatter = true;
			}
		}
		else if ( ( m_flLastHealTime + 4.0f ) > gpGlobals->curtime )
		{
			bSkipChatter = true;
		}
		else if ( bHealingSelf )
		{
			bSkipChatter = true;
		}

		if ( !bSkipChatter )
		{
			// try and do a special chatter?
			if ( pMarine->GetMarineSpeech()->AllowCalmConversations( CONV_HEALING_CRASH ) )
			{
				if ( !pTarget->m_bDoneWoundedRebuke && pTarget->GetMarineResource() && pTarget->GetMarineResource()->m_bTakenWoundDamage )
				{
					// marine has been wounded and this is our first heal after the fact - good chance of the medic saying something
					if ( random->RandomFloat() < asw_marine_special_heal_chatter_chance.GetFloat() * 3 )
					{
						if ( CASW_MarineSpeech::StartConversation( CONV_SERIOUS_INJURY, pMarine, pTarget ) )
						{
							bSkipChatter = true;
							pTarget->m_bDoneWoundedRebuke = true;
						}
					}
				}

				// if we didn't complaint about a serious injury, check for doing a different kind of conv
				float fRand = random->RandomFloat();
				if ( !bSkipChatter && fRand < asw_marine_special_heal_chatter_chance.GetFloat() )
				{
					if ( pTarget->GetMarineProfile() && pTarget->GetMarineProfile()->m_VoiceType == ASW_VOICE_CRASH
						&& pMarine->GetMarineProfile() && pMarine->GetMarineProfile()->m_VoiceType == ASW_VOICE_BASTILLE )	// bastille healing crash
					{
						if ( random->RandomFloat() < 0.5f )
						{
							if ( CASW_MarineSpeech::StartConversation( CONV_HEALING_CRASH, pMarine, pTarget ) )
								bSkipChatter = true;
						}
						else
						{
							if ( CASW_MarineSpeech::StartConversation( CONV_MEDIC_COMPLAIN, pMarine, pTarget ) )
								bSkipChatter = true;
						}
					}
					else
					{
						if ( CASW_MarineSpeech::StartConversation( CONV_MEDIC_COMPLAIN, pMarine, pTarget ) )
							bSkipChatter = true;
					}
				}
			}
			if ( !bSkipChatter )
				pMarine->GetMarineSpeech()->Chatter( CHATTER_HEALING );
		}
	}
#endif
}

void CASW_Weapon_HealAmp_Gun::StartSearchSound()
{
	if ( !m_pSearchSound )
	{
		CPASAttenuationFilter filter( this );
		if ( m_bIsBuffing )
		{
			m_pSearchSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_BuffGrenade.BuffLoop" );
		}
		else 
		{
			m_pSearchSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_HealGun.SearchLoop" );
		}
		
	}
	CSoundEnvelopeController::GetController().Play( m_pSearchSound, 1.0, 100 );
}

void CASW_Weapon_HealAmp_Gun::StartHealSound()
{
	if ( !m_pSearchSound )
	{
		StartSearchSound();
	}

	//if ( m_pSearchSound )
	//{
	//	CSoundEnvelopeController::GetController().SoundFadeOut( m_pSearchSound, asw_heal_gun_start_heal_fade.GetFloat(), true );
	//	m_pSearchSound = NULL;
	//}

	if ( !m_pHealSound )
	{
		CPASAttenuationFilter filter( this );
		if ( m_bIsBuffing )
		{
			m_pHealSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_BuffGrenade.BuffLoop" );
			EmitSound( "ASW_BuffGrenade.StartBuff" );
		}
		else
		{
			m_pHealSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_HealGun.HealLoop" );
			EmitSound( "ASW_HealGun.StartHeal" );
		}

	}
	CSoundEnvelopeController::GetController().Play( m_pHealSound, 0.0, 100 );
	CSoundEnvelopeController::GetController().SoundChangeVolume( m_pHealSound, 1.0, asw_heal_gun_start_heal_fade.GetFloat() );
}

#ifdef CLIENT_DLL
// if the player has his mouse over another marine, highlight it, cos he's the one we can give health to
void CASW_Weapon_HealAmp_Gun::MouseOverEntity(C_BaseEntity *pEnt, Vector vecWorldCursor)
{
	C_ASW_Marine* pOtherMarine = C_ASW_Marine::AsMarine( pEnt );
	CASW_Player *pOwner = GetCommander();
	CASW_Marine *pMarine = GetMarine();
	if (!pOwner || !pMarine)
		return;

	if (!pOtherMarine)
	{
		C_ASW_Game_Resource *pGameResource = ASWGameResource();
		if (pGameResource)
		{
			// find marine closest to world cursor
			const float fMustBeThisClose = 70;
			const C_ASW_Game_Resource::CMarineToCrosshairInfo::tuple_t &info = pGameResource->GetMarineCrosshairCache()->GetClosestMarine();
			if ( info.m_fDistToCursor <= fMustBeThisClose )
			{
				pOtherMarine = info.m_hMarine.Get();
			}
		}
	}

	if ( ( !pOtherMarine || !TargetCanBeHealed( pOtherMarine ) ) && TargetCanBeHealed( pMarine ) && ShouldHealSelfOnInvalidTarget( pOtherMarine ) )
	{
		pOtherMarine = pMarine;
	}

	// if the marine our cursor is over is near enough, highlight him
	if (pOtherMarine)
	{
		float dist = (pMarine->GetAbsOrigin() - pOtherMarine->GetAbsOrigin()).Length2D();
		if (dist < GetWeaponRange() )
		{
			bool bCanGiveHealth = ( pOtherMarine->GetHealth() > 1 && ( pOtherMarine->GetHealth() + pOtherMarine->m_iSlowHealAmount ) < pOtherMarine->GetMaxHealth() && m_iClip1 > 0 );
			bool bCanHighlight = ( pOtherMarine->GetHealth() > 1 && ( pOtherMarine != pMarine || bCanGiveHealth ) && m_iClip1 > 0 );
			ASWInput()->SetHighlightEntity( pOtherMarine, bCanHighlight );
			if ( bCanGiveHealth )		// if he needs healing, show the give health cursor
			{
				CASWHudCrosshair *pCrosshair = GET_HUDELEMENT( CASWHudCrosshair );
				if ( pCrosshair )
				{
					pCrosshair->SetShowGiveHealth(true);
				}
			}
		}
	}
}

void CASW_Weapon_HealAmp_Gun::UpdateEffects()
{
	if ( !m_hHealEntity.Get() || m_hHealEntity.Get()->Classify() != CLASS_ASW_MARINE || !GetMarine() )
	{
		if ( m_pDischargeEffect )
		{
			m_pDischargeEffect->StopEmission();
			m_pDischargeEffect = NULL;
		}
		return;
	}

	C_ASW_Marine* pMarine = static_cast<C_ASW_Marine*>( static_cast<C_BaseEntity*>( m_hHealEntity.Get() ) );
	bool bHealingSelf = pMarine ? (pMarine == GetMarine()) : false;

	if ( bHealingSelf && m_pDischargeEffect )
	{
		m_pDischargeEffect->StopEmission();
		m_pDischargeEffect = NULL;
	}

	switch( m_FireState )
	{
		case ASW_HG_FIRE_OFF:
		{
			if ( m_pDischargeEffect )
			{
				m_pDischargeEffect->StopEmission();
				m_pDischargeEffect = NULL;
			}
			break;
		}
		case ASW_HG_FIRE_DISCHARGE:
		{
			if ( m_pDischargeEffect && m_pDischargeEffect->GetControlPointEntity( 1 ) != NULL )
			{
				// Still attach, detach us
				m_pDischargeEffect->StopEmission();
				m_pDischargeEffect = NULL;
			}

			// don't create the effect if you are healing yourself
			if ( bHealingSelf )
				break;

			if ( !m_pDischargeEffect )
			{
				m_pDischargeEffect = ParticleProp()->Create( "heal_gun_noconnect", PATTACH_POINT_FOLLOW, "muzzle" ); 
			}

			m_pDischargeEffect->SetControlPoint( 1, m_vecHealPos );
			m_pDischargeEffect->SetControlPointOrientation( 0, GetMarine()->Forward(), -GetMarine()->Left(), GetMarine()->Up() );
			
			break;
		}
		case ASW_HG_FIRE_ATTACHED:
		{
			if ( !pMarine )
			{
				break;
			}

			if ( m_pDischargeEffect )
			{
				// stop effect if we have wrong entity effect is attached to
				if ( m_pDischargeEffect->GetControlPointEntity( 1 ) != m_hHealEntity.Get() )
				{
					EHANDLE hHealEntity = m_hHealEntity.Get();
					CASW_Marine *pTarget = static_cast<CASW_Marine*>( static_cast<CBaseEntity*>( hHealEntity.Get() ) );
					bool control_point_is_weapon = false;
					if ( pTarget)
					{
						if ( m_pDischargeEffect->GetControlPointEntity( 1 ) == pTarget->GetActiveASWWeapon() )
						{
							control_point_is_weapon = true;
						}
					}
					
					if ( !control_point_is_weapon )
					{
						m_pDischargeEffect->StopEmission();
						m_pDischargeEffect = NULL;
					}					
				}
			}

			// don't create the effect if you are healing yourself
			if ( bHealingSelf )
				break;

			if ( !m_pDischargeEffect )
			{				
				if ( m_bIsBuffing )
				{
					m_pDischargeEffect = ParticleProp()->Create( "buffgrenade_attach_arc", PATTACH_POINT_FOLLOW, "muzzle" ); 
				}
				else
				{
					m_pDischargeEffect = ParticleProp()->Create( "heal_gun_attach", PATTACH_POINT_FOLLOW, "muzzle" ); 
				}
			}

			Assert( m_pDischargeEffect );
	
			if ( m_pDischargeEffect->GetControlPointEntity( 1 ) == NULL )
			{
				if ( m_bIsBuffing && pMarine && pMarine->GetActiveASWWeapon() )
				{
					C_ASW_Weapon *pWeapon = pMarine->GetActiveASWWeapon();
					int iAttachment = pWeapon->LookupAttachment( "muzzle" );
					if ( pWeapon->IsOffensiveWeapon() && iAttachment > 0 )
					{
						ParticleProp()->AddControlPoint( m_pDischargeEffect, 1, pWeapon, PATTACH_POINT_FOLLOW, "muzzle" );
					}
					//m_pDischargeEffect->SetControlPointOrientation( 0, GetMarine()->Forward(), -GetMarine()->Left(), GetMarine()->Up() );
				}
				else 
				{
					float flHeight = pMarine->BoundingRadius();
					Vector vOffset( 0.0f, 0.0f, flHeight * 0.25 );

					ParticleProp()->AddControlPoint( m_pDischargeEffect, 1, pMarine, PATTACH_ABSORIGIN_FOLLOW, NULL, vOffset );
					m_pDischargeEffect->SetControlPointOrientation( 0, GetMarine()->Forward(), -GetMarine()->Left(), GetMarine()->Up() );
				}
			}

			break;
		}
	}
}
#endif // CLIENT_DLL

bool CASW_Weapon_HealAmp_Gun::ShouldHealSelfOnInvalidTarget( CBaseEntity *pTarget )
{
	Assert( GetMarine() && GetMarine()->IsInhabited() && GetCommander() );
	if ( !GetMarine() || !GetMarine()->IsInhabited() || !GetCommander() )
		return false;

	// we can't aim at ourself in first or third person
	if ( GetCommander()->GetASWControls() != ASWC_TOPDOWN )
		return true;

	// if we're in controller aiming mode or the player is holding shift, self-heal
	if ( GetCommander()->m_nButtons & IN_WALK )
		return true;

	return false;
}
