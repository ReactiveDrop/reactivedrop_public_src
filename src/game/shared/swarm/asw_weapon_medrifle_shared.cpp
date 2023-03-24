#include "cbase.h"
#include "asw_weapon_medrifle_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "c_asw_game_resource.h"
#include "asw_hud_crosshair.h"
#include "asw_input.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_fail_advice.h"
#include "effect_dispatch_data.h"
#include "asw_marine_speech.h"
#include "asw_marine_resource.h"
#include "asw_triggers.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "particle_parse.h"
#include "asw_deathmatch_mode_light.h"
#include "soundenvelope.h"
#include "asw_gamerules.h"

#include "asw_weapon_heal_gun_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//static const float ASW_HG_HEAL_RATE = 0.25f; // Rate at which we heal entities we're latched on
static const float ASW_HG_SEARCH_DIAMETER = 48.0f; // How far past the range to search for enemies to latch on to
static const float SQRT3 = 1.732050807569; // for computing max extents inside a box

extern ConVar asw_laser_sight;
extern ConVar asw_marine_special_heal_chatter_chance;
extern ConVar rd_medgun_infinite_ammo;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_MedRifle, DT_ASW_Weapon_MedRifle )

BEGIN_NETWORK_TABLE( CASW_Weapon_MedRifle, DT_ASW_Weapon_MedRifle )
#ifdef CLIENT_DLL
	// recvprops
	RecvPropTime( RECVINFO( m_fSlowTime ) ),
	RecvPropInt( RECVINFO( m_FireState ) ),
	RecvPropEHandle( RECVINFO( m_hHealEntity ) ),
	RecvPropVector( RECVINFO( m_vecHealPos ) ),
#else
	// sendprops
	SendPropTime( SENDINFO( m_fSlowTime ) ),
	SendPropInt( SENDINFO( m_FireState ), Q_log2(ASW_HG_NUM_FIRE_STATES)+1, SPROP_UNSIGNED ),
	SendPropEHandle( SENDINFO( m_hHealEntity ) ),
	SendPropVector( SENDINFO( m_vecHealPos ) ),
#endif
END_NETWORK_TABLE()

#if defined( CLIENT_DLL )
BEGIN_PREDICTION_DATA( CASW_Weapon_MedRifle )
	DEFINE_PRED_FIELD_TOL( m_fSlowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD( m_vecHealPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_medrifle, CASW_Weapon_MedRifle );
PRECACHE_WEAPON_REGISTER(asw_weapon_medrifle);

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_MedRifle )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_MedRifle::CASW_Weapon_MedRifle()
{
	m_flLastHealTime = 0;
	m_flNextHealMessageTime = 0.0f;
	m_flTotalAmountHealed = 0;

	m_pSearchSound = NULL;
	m_pHealSound = NULL;
}


CASW_Weapon_MedRifle::~CASW_Weapon_MedRifle()
{

}

void CASW_Weapon_MedRifle::Precache()
{
	// precache the weapon model here?	
	PrecacheScriptSound( "ASW_Weapon_MedRifle.Single" );
	PrecacheScriptSound( "ASW_Weapon_MedRifle.SingleFP" );
	PrecacheScriptSound( "ASW_HealGun.SearchLoop" );
	PrecacheScriptSound( "ASW_HealGun.HealLoop" );
	PrecacheScriptSound( "ASW_HealGun.StartHeal" );
	PrecacheScriptSound( "ASW_HealGun.StartSearch" );
	PrecacheScriptSound( "ASW_HealGun.Off" );

	PrecacheParticleSystem( "heal_gun_noconnect" );
	PrecacheParticleSystem( "heal_gun_attach" );

	PrecacheScriptSound( "ASW_Weapon.Empty" );

	BaseClass::Precache();
}


float CASW_Weapon_MedRifle::GetWeaponDamage()
{
	//float flDamage = 7.0f;
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_medrifle_dmg_base;
	if ( rd_medrifle_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_medrifle_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine( GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_MEDRIFLE_DMG );
	}

	return flDamage;
}

void CASW_Weapon_MedRifle::SecondaryAttack()
{
	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || !pMarine->IsAlive() )
	{
		EndAttack();
		return;
	}

	// don't fire underwater
	if ( pMarine->GetWaterLevel() == 3 )
	{
		WeaponSound( EMPTY );

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
		return;
	}
#ifndef CLIENT_DLL
	Vector vecSrc = pMarine->Weapon_ShootPosition();
#endif
	Vector vecAiming = vec3_origin;

	if ( pPlayer && pMarine->IsInhabited() )
	{
		vecAiming = pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187
	}
	else
	{
#ifndef CLIENT_DLL
		vecAiming = pMarine->GetActualShootTrajectory( vecSrc );
#endif
	}

#ifdef GAME_DLL
	m_bIsFiring = true;

	switch ( m_FireState )
	{
		case ASW_HG_FIRE_OFF:
		{
			Fire( vecSrc, vecAiming );
			m_flNextPrimaryAttack = gpGlobals->curtime;
			break;
		}

		case ASW_HG_FIRE_HEALSELF:
		{
			EHANDLE hHealEntity = m_hHealEntity.Get();

			if ( !hHealEntity || !TargetCanBeHealed( hHealEntity.Get() ) )
			{
				HealDetach();
				SetFiringState( ASW_HG_FIRE_DISCHARGE );
				break;
			}

			if ( hHealEntity.Get() == GetMarine() )
			{
				HealSelf();
				break;
			}
		}

		case ASW_HG_FIRE_DISCHARGE:
		{
			Fire( vecSrc, vecAiming );

			EHANDLE hHealEntity = m_hHealEntity.Get();

			// Search for nearby entities to heal
			if ( !hHealEntity )
			{
				if ( pPlayer && pPlayer->GetHighlightEntity() && pPlayer->GetHighlightEntity()->Classify() == CLASS_ASW_MARINE )
				{
					CASW_Marine* pTargetMarine = static_cast< CASW_Marine* >( pPlayer->GetHighlightEntity() );
					if ( pTargetMarine )
					{
						// healing self
						if ( pTargetMarine == GetMarine() && TargetCanBeHealed( pTargetMarine ) )
						{
							HealSelf();
							break;
						}
						else if ( HealAttach( pTargetMarine ) )
						{
							SetFiringState( ASW_HG_FIRE_ATTACHED );
							m_flLastHealTime = gpGlobals->curtime;
							break;
						}
					}
				}
			}
			
			m_flNextPrimaryAttack = gpGlobals->curtime;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack;
			StartSearchSound();
			break;
		}

		case ASW_HG_FIRE_ATTACHED:
		{
			Fire( vecSrc, vecAiming );

			EHANDLE hHealEntity = m_hHealEntity.Get();
			// detach if...
			// full health, dead or not here
			if ( !hHealEntity || !TargetCanBeHealed( hHealEntity.Get() ) )
			{
				HealDetach();
				SetFiringState( ASW_HG_FIRE_DISCHARGE );
				break;
			}
			
			// too far
			float flHealDistance = (hHealEntity->GetAbsOrigin() - pMarine->GetAbsOrigin()).LengthSqr();
			if ( flHealDistance > (GetWeaponRange() + SQRT3*ASW_HG_SEARCH_DIAMETER)*(GetWeaponRange() + SQRT3*ASW_HG_SEARCH_DIAMETER) )
			{
				HealDetach();
				SetFiringState( ASW_HG_FIRE_DISCHARGE );
				break;
			}

			// facing another direction
			Vector vecAttach = hHealEntity->GetAbsOrigin() - pMarine->GetAbsOrigin();
			Vector vecForward;
			QAngle vecEyeAngles;
			if ( pMarine->IsInhabited() && pPlayer )
				vecEyeAngles = pPlayer->EyeAngles();
			else
				vecEyeAngles = pMarine->ASWEyeAngles();

			AngleVectors( vecEyeAngles, &vecForward );
			if ( DotProduct( vecForward, vecAttach ) < 0.0f )
			{
				HealDetach();
				SetFiringState( ASW_HG_FIRE_DISCHARGE );
				break;
			}

			if ( gpGlobals->curtime > m_flLastHealTime + GetSecondaryFireRate() )
			{
				HealEntity();
				m_flLastHealTime = gpGlobals->curtime;
			}

			m_flNextPrimaryAttack = gpGlobals->curtime + GetSecondaryFireRate();
			m_flNextSecondaryAttack = m_flNextPrimaryAttack;

			//StartHealSound();
			break;
		}
	}

	SendWeaponAnim( GetPrimaryAttackActivity() );			

	SetWeaponIdleTime( gpGlobals->curtime + GetSecondaryFireRate() );
#endif

	m_fSlowTime = gpGlobals->curtime + GetSecondaryFireRate();
}

float CASW_Weapon_MedRifle::GetSecondaryFireRate()
{
	float flRate = GetWeaponInfo()->m_flSecondaryFireRate;

	return flRate;
}

void CASW_Weapon_MedRifle::SetFiringState( ASW_Weapon_HealGunFireState_t state )
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
			EmitSound( "ASW_HealGun.StartSearch" );
		}
		else if ( state == ASW_HG_FIRE_OFF )
		{
			//StopSound( "ASW_HealGun.SearchLoop" );
			EmitSound( "ASW_Tesla_Laser.Stop" );

		}
	}

	m_FireState = state;
}

bool CASW_Weapon_MedRifle::Reload( void )
{
	EndAttack();

	return BaseClass::Reload();
}

void CASW_Weapon_MedRifle::WeaponIdle( void )
{
	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( m_FireState != ASW_HG_FIRE_OFF )
	{
		EndAttack();
	}

	float flIdleTime = gpGlobals->curtime + 5.0;

	SetWeaponIdleTime( flIdleTime );
}

const Vector& CASW_Weapon_MedRifle::GetBulletSpread( void )
{
	static Vector cone = Vector( 0.13053, 0.13053, 0.02 );	// VECTOR_CONE_15DEGREES with flattened Z

	return cone;
}

void CASW_Weapon_MedRifle::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CheckEndFiringState();
}

void CASW_Weapon_MedRifle::ItemBusyFrame( void )
{
	BaseClass::ItemBusyFrame();

	CheckEndFiringState();
}

void CASW_Weapon_MedRifle::Drop( const Vector &vecVelocity )
{	
	EndAttack();

	BaseClass::Drop( vecVelocity );
}

bool CASW_Weapon_MedRifle::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	EndAttack();

	return BaseClass::Holster( pSwitchingTo );
}

bool CASW_Weapon_MedRifle::ShouldMarineMoveSlow()
{
	return m_fSlowTime > gpGlobals->curtime || ( IsReloading() || IsFiring() );
}

#ifdef GAME_DLL
void CASW_Weapon_MedRifle::GetButtons(bool& bAttack1, bool& bAttack2, bool& bReload, bool& bOldReload, bool& bOldAttack1 )
{
	CASW_Marine *pMarine = GetMarine();

	// make AI fire this weapon whenever they have a heal target
	if ( pMarine && !pMarine->IsInhabited() && m_hHealEntity->Get() != NULL )
	{
		bAttack1 = false;
		bAttack2 = true;
		bReload = false;
		bOldReload = false;

		return;
	}

	BaseClass::GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );
}
#endif

void CASW_Weapon_MedRifle::CheckEndFiringState( void )
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons(bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	bool bNotAttacking = !bAttack2;
	if ( (m_bIsFiring || m_FireState != ASW_HG_FIRE_OFF) && bNotAttacking )
	{
		EndAttack();
	}
}

void CASW_Weapon_MedRifle::UpdateOnRemove()
{
	if ( m_pSearchSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pSearchSound );
		m_pSearchSound = NULL;
	}
	if ( m_pHealSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pHealSound );
		m_pHealSound = NULL;
	}
	BaseClass::UpdateOnRemove();
}

bool CASW_Weapon_MedRifle::TargetCanBeHealed( CBaseEntity* pTarget )
{	
	if ( pTarget->Classify() != CLASS_ASW_MARINE )
		return false;

	CASW_Marine *pMarine = static_cast<CASW_Marine*>( pTarget );
	if ( !pMarine )
		return false;

	if ( pMarine->GetHealth() <= 0 || ( pMarine->GetHealth() + pMarine->m_iSlowHealAmount ) >= pMarine->GetMaxHealth() )
		return false;

	return true;
}

void CASW_Weapon_MedRifle::HealSelf( void )
{
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return;

	if ( !TargetCanBeHealed( pMarine ) )
	{
		// TODO: have some better feedback here if the player is full on health?
		WeaponSound( EMPTY );
		m_flNextPrimaryAttack = gpGlobals->curtime + GetSecondaryFireRate();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack;
		return;
	}

	if ( pMarine != m_hHealEntity.Get() )
	{
		SetFiringState( ASW_HG_FIRE_OFF );
		HealDetach();
	}

	m_flLastHealTime = gpGlobals->curtime;
	m_hHealEntity.Set( pMarine );
	HealEntity();
	SetFiringState( ASW_HG_FIRE_HEALSELF );
	m_bIsFiring = true;

	m_flNextPrimaryAttack = gpGlobals->curtime + GetSecondaryFireRate();
	m_flNextSecondaryAttack = m_flNextPrimaryAttack;
	m_fSlowTime = m_flNextSecondaryAttack;

	SetWeaponIdleTime( m_flNextSecondaryAttack );
}

void CASW_Weapon_MedRifle::HealEntity( void )
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

	if ( m_iClip2 <= 0 )
		return;

	// apply heal
	int iHealAmount = MIN( GetHealAmount(), pTarget->GetMaxHealth() - ( pTarget->GetHealth() + pTarget->m_iSlowHealAmount ) );

	if ( iHealAmount == 0 )
		return;

#ifdef GAME_DLL
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

#endif

    if ( false == rd_medgun_infinite_ammo.GetBool() )
    {
	    // decrement ammo
	    m_iClip2 -= 1;
    }

	// emit heal sound
	StartHealSound();

#ifdef GAME_DLL
	bool bHealingSelf = (pMarine == pTarget);
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
	if ( m_iClip2 <= 0 )
	{
		// Out of ammo
		ASWFailAdvice()->OnMedSatchelEmpty();

		pMarine->GetMarineSpeech()->Chatter( CHATTER_MEDS_NONE );

		CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
		if ( pMR )
		{
			char szName[ 256 ];
			pMR->GetDisplayName( szName, sizeof( szName ) );
			UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_out_of_meds", szName );
		}

		bSkipChatter = true;
	}
	else if ( m_iClip2 == 10 )
	{
		if ( pMarine->GetMarineSpeech()->Chatter( CHATTER_MEDS_LOW ) )
		{
			bSkipChatter = true;
		}
	}
	else if ( (m_flLastHealTime + 4.0f) > gpGlobals->curtime )
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
		if ( pMarine->GetMarineSpeech()->AllowCalmConversations(CONV_HEALING_CRASH) )
		{
			if ( !pTarget->m_bDoneWoundedRebuke && pTarget->GetMarineResource() && pTarget->GetMarineResource()->m_bTakenWoundDamage )
			{
				// marine has been wounded and this is our first heal after the fact - good chance of the medic saying something
				if ( random->RandomFloat() < asw_marine_special_heal_chatter_chance.GetFloat() * 3 )
				{
					if ( CASW_MarineSpeech::StartConversation(CONV_SERIOUS_INJURY, pMarine, pTarget) )
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
						if ( CASW_MarineSpeech::StartConversation(CONV_HEALING_CRASH, pMarine, pTarget) )
							bSkipChatter = true;						
					}
					else
					{
						if ( CASW_MarineSpeech::StartConversation(CONV_MEDIC_COMPLAIN, pMarine, pTarget) )
							bSkipChatter = true;
					}
				}
				else
				{
					if ( CASW_MarineSpeech::StartConversation(CONV_MEDIC_COMPLAIN, pMarine, pTarget) )
						bSkipChatter = true;
				}
			}
		}
		if ( !bSkipChatter )
			pMarine->GetMarineSpeech()->Chatter( CHATTER_HEALING );
	}
#endif
}

bool CASW_Weapon_MedRifle::HealAttach( CBaseEntity *pEntity )
{
#ifdef CLIENT_DLL
	return false;
#else

	if ( !pEntity || (pEntity->Classify() != CLASS_ASW_MARINE) ||
		(pEntity == m_hHealEntity.Get()) || !TargetCanBeHealed( pEntity ) )  // we can attach to ourselves //|| pEntity == GetMarine() 
	{
		return false;
	}

	if ( m_hHealEntity.Get() )
	{
		HealDetach();
	}

	m_hHealEntity.Set( pEntity );

	// we need a trace_t struct representing the entity we're healing, but if we 
	//  attached based on proximity to the end of the beam, we might not have actually performed a trace - for now just update m_AttackTrace
	//  but it'd be nice to decouple attribute damage from requiring a trace
	if ( m_AttackTrace.m_pEnt != pEntity )
	{
		m_AttackTrace.endpos = pEntity->GetAbsOrigin();
		m_AttackTrace.m_pEnt = pEntity;
	}

	ASW_WPN_DEV_MSG( "Attached to %d:%s\n", m_hHealEntity.Get()->entindex(), m_hHealEntity.Get()->GetClassname() );
	return true;
#endif
}

void CASW_Weapon_MedRifle::HealDetach()
{
#ifdef GAME_DLL
	ASW_WPN_DEV_MSG( "Detaching from %d:%s\n", m_hHealEntity.Get() ? m_hHealEntity.Get()->entindex() : -1, m_hHealEntity.Get() ? m_hHealEntity.Get()->GetClassname() : "(null)" );
	m_hHealEntity.Set( NULL );
#endif
	StopHealSound();
}

void CASW_Weapon_MedRifle::Fire( const Vector &vecOrigSrc, const Vector &vecDir )
{
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
	{
		return;
	}

	Vector vecDest	= vecOrigSrc + (vecDir * GetWeaponRange());
	trace_t	tr;
	UTIL_TraceLine( vecOrigSrc, vecDest, MASK_SHOT, pMarine, COLLISION_GROUP_NONE, &tr );

	if ( tr.allsolid )
		return;

	m_AttackTrace = tr;

	if( pMarine->IsInhabited() )
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if ( ( !pEntity || !TargetCanBeHealed( pEntity ) ) && TargetCanBeHealed( pMarine ) )
		{
			pEntity = pMarine;
		}

		Vector vecUp, vecRight;
		QAngle angDir;

		VectorAngles( vecDir, angDir );
		AngleVectors( angDir, NULL, &vecRight, &vecUp );

		if ( HealAttach( pEntity ) )
		{
			SetFiringState( ASW_HG_FIRE_ATTACHED );
		}
	}
	else
	{
		SetFiringState( ASW_HG_FIRE_ATTACHED );
	}

	if ( !m_hHealEntity.Get() )
	{
		// If we fail to hit something, and we're not healing someone, we're just shooting out a beam
		SetFiringState( ASW_HG_FIRE_DISCHARGE );
	}

	vecDest = tr.endpos;
	m_vecHealPos = vecDest;

/*
#ifdef GAME_DLL
	if (pMarine && m_iClip2 <= 0 && pMarine->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		// check he doesn't have ammo in an ammo bay
		CASW_Weapon_Ammo_Bag* pAmmoBag = dynamic_cast<CASW_Weapon_Ammo_Bag*>(pMarine->GetASWWeapon(0));
		if (!pAmmoBag)
			pAmmoBag = dynamic_cast<CASW_Weapon_Ammo_Bag*>(pMarine->GetASWWeapon(1));
		if (!pAmmoBag || !pAmmoBag->CanGiveAmmoToWeapon(this))
			pMarine->OnWeaponOutOfAmmo(true);
	}
#endif
	*/
}


void CASW_Weapon_MedRifle::EndAttack( void )
{
	SetWeaponIdleTime( gpGlobals->curtime + 2.0 );

	if ( m_FireState != ASW_HG_FIRE_OFF )
	{
		EmitSound( "ASW_HealGun.Off" );
	}

	SetFiringState( ASW_HG_FIRE_OFF );
	
	HealDetach();

	//ClearIsFiring();

	//m_bIsFiring = false;
}

extern ConVar asw_heal_gun_start_heal_fade;
extern ConVar asw_heal_gun_heal_fade;

void CASW_Weapon_MedRifle::StartSearchSound()
{
	if ( !m_pSearchSound )
	{
		CPASAttenuationFilter filter( this );
		m_pSearchSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_HealGun.SearchLoop" );
	}
	CSoundEnvelopeController::GetController().Play( m_pSearchSound, 1.0, 100 );
}

void CASW_Weapon_MedRifle::StartHealSound()
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
		m_pHealSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_HealGun.HealLoop" );
		EmitSound( "ASW_HealGun.StartHeal" );
	}
	CSoundEnvelopeController::GetController().Play( m_pHealSound, 0.0, 100 );
	CSoundEnvelopeController::GetController().SoundChangeVolume( m_pHealSound, 1.0, asw_heal_gun_start_heal_fade.GetFloat() );
}

void CASW_Weapon_MedRifle::StopHealSound()
{
	if ( m_pSearchSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pSearchSound );
		m_pSearchSound = NULL;
	}
	if ( m_pHealSound )
	{
		CSoundEnvelopeController::GetController().SoundFadeOut( m_pHealSound, asw_heal_gun_heal_fade.GetFloat(), true );
		m_pHealSound = NULL;
	}
}


int CASW_Weapon_MedRifle::GetHealAmount()
{
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
	{
		return 0;
	}

	float flHealAmount = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_HEALING, ASW_MARINE_SUBSKILL_HEAL_GUN_HEAL_AMOUNT);

	return flHealAmount;
}

float CASW_Weapon_MedRifle::GetInfestationCureAmount()
{
	CASW_Marine *pMarine = GetMarine();
	if (!pMarine)
		return 0.0f;		

	float flCureAmount = MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_XENOWOUNDS) / 100.0f;

	return flCureAmount;
}


#ifdef CLIENT_DLL
bool CASW_Weapon_MedRifle::ShouldShowLaserPointer()
{
	if ( !asw_laser_sight.GetBool() || IsDormant() || !GetOwner() )
	{
		return false;
	}

	C_ASW_Marine *pMarine = GetMarine();

	return ( pMarine && pMarine->GetActiveWeapon() == this );
}

// if the player has his mouse over another marine, highlight it, cos he's the one we can give health to
void CASW_Weapon_MedRifle::MouseOverEntity(C_BaseEntity *pEnt, Vector vecWorldCursor)
{
	C_ASW_Marine *pOtherMarine = C_ASW_Marine::AsMarine( pEnt );
	CASW_Player *pOwner = GetCommander();
	CASW_Marine *pMarine = GetMarine();
	if ( !pOwner || !pMarine )
		return;

	if ( !pOtherMarine )
	{
		C_ASW_Game_Resource *pGameResource = ASWGameResource();
		if ( pGameResource )
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

	if ( !pOtherMarine || !TargetCanBeHealed( pOtherMarine ) )
	{
		if ( TargetCanBeHealed( pMarine ) )
		{
			pOtherMarine = pMarine;
		}
		else
		{
			return;
		}
	}
	else
	{
		float dist = ( pMarine->GetAbsOrigin() - pOtherMarine->GetAbsOrigin() ).Length2D();
		if ( dist >= GetWeaponRange() )
		{
			if ( TargetCanBeHealed( pMarine ) )
			{
				pOtherMarine = pMarine;
			}
			else
			{
				return;
			}
		}
	}

	// if the marine our cursor is over is near enough, highlight them
	bool bCanGiveHealth = m_iClip1 > 0;
	ASWInput()->SetHighlightEntity( pOtherMarine, bCanGiveHealth );

	CASWHudCrosshair *pCrosshair = GET_HUDELEMENT( CASWHudCrosshair );
	if ( pCrosshair )
	{
		pCrosshair->SetShowGiveHealth( bCanGiveHealth );
	}
}

void CASW_Weapon_MedRifle::ClientThink()
{
	BaseClass::ClientThink();

	UpdateEffects();
}

/*const char* CASW_Weapon_MedRifle::GetPartialReloadSound(int iPart)
{
	switch (iPart)
	{
		case 1: return "ASW_Rifle.ReloadB"; break;
		case 2: return "ASW_Rifle.ReloadC"; break;
		default: break;
	};
	return "ASW_Rifle.ReloadA";
}*/

void CASW_Weapon_MedRifle::UpdateEffects()
{
	if ( !m_hHealEntity.Get() || m_hHealEntity.Get()->Classify() != CLASS_ASW_MARINE || !GetMarine() || m_iClip2 <= 0 )
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
				if ( m_pDischargeEffect->GetControlPointEntity( 1 ) != m_hHealEntity.Get() )
				{
					m_pDischargeEffect->StopEmission();
					m_pDischargeEffect = NULL;
				}
			}

			// don't create the effect if you are healing yourself
			if ( bHealingSelf )
				break;

			if ( !m_pDischargeEffect )
			{				
				m_pDischargeEffect = ParticleProp()->Create( "heal_gun_attach", PATTACH_POINT_FOLLOW, "muzzle" ); 
			}

			Assert( m_pDischargeEffect );
	
			if ( m_pDischargeEffect->GetControlPointEntity( 1 ) == NULL )
			{
				float flHeight = pMarine->BoundingRadius();
				Vector vOffset( 0.0f, 0.0f, flHeight * 0.25 );

				ParticleProp()->AddControlPoint( m_pDischargeEffect, 1, pMarine, PATTACH_ABSORIGIN_FOLLOW, NULL, vOffset );
				m_pDischargeEffect->SetControlPointOrientation( 0, GetMarine()->Forward(), -GetMarine()->Left(), GetMarine()->Up() );
			}

			break;
		}
	}
}
#endif
