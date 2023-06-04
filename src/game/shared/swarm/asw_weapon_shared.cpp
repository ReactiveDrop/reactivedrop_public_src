#include "cbase.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#define CASW_Weapon C_ASW_Weapon
	#define CASW_Marine C_ASW_Marine
	#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
	#define CBasePlayer C_BasePlayer
	#include "c_asw_weapon.h"
	#include "c_asw_marine.h"
	#include "c_asw_player.h"
	#include "c_asw_marine_resource.h"
	#include "c_basecombatcharacter.h"
	#include "c_baseplayer.h"	
	#include "c_asw_fx.h"
	#include "prediction.h"
	#include "igameevents.h"
	#define CASW_Marine_Resource C_ASW_Marine_Resource
	#define CRecipientFilter C_RecipientFilter
#else
	#include "asw_lag_compensation.h"
	#include "asw_weapon.h"
	#include "asw_marine.h"
	#include "asw_marine_resource.h"
	#include "asw_player.h"
	#include "asw_marine_speech.h"
	#include "asw_gamestats.h"
	#include "ammodef.h"
	#include "asw_achievements.h"
	#include "asw_fx_shared.h"
#endif
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_marine_skills.h"
#include "asw_weapon_ammo_bag_shared.h"
#include "asw_weapon_fire_extinguisher_shared.h"
#include "asw_weapon_healgrenade_shared.h"
#include "asw_weapon_heal_gun_shared.h"
#include "asw_weapon_medrifle_shared.h"
#include "asw_gamerules.h"
#include "asw_melee_system.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "particle_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar asw_weapon_max_shooting_distance( "asw_weapon_max_shooting_distance", "1500", FCVAR_REPLICATED, "Maximum distance of the hitscan weapons." );
ConVar asw_weapon_force_scale("asw_weapon_force_scale", "1.0f", FCVAR_REPLICATED, "Force of weapon shots");
ConVar asw_fast_reload_enabled( "asw_fast_reload_enabled", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "Use fast reload mechanic?" );
#ifdef GAME_DLL
ConVar rd_fast_reload_explode_chance( "rd_fast_reload_explode_chance", "0.0f", FCVAR_CHEAT, "0-1 chance for marine to explode on failed fast reload" );
ConVar rd_fast_reload_explode_damage( "rd_fast_reload_explode_damage", "50.0f", FCVAR_CHEAT, "Damage of the fast reload fail explosion" );
ConVar rd_fast_reload_explode_radius( "rd_fast_reload_explode_radius", "50.0f", FCVAR_CHEAT, "Radius of the fast reload fail explosion" );
#endif

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
extern ConVar asw_DebugAutoAim;
extern ConVar rd_bot_strong;
#endif

BEGIN_DEFINE_LOGGING_CHANNEL( LOG_ASW_Weapons, "ASWWeapons", 0, LS_MESSAGE );
ADD_LOGGING_CHANNEL_TAG( "AlienSwarm" );
ADD_LOGGING_CHANNEL_TAG( "Weapons" );
END_DEFINE_LOGGING_CHANNEL();

#ifdef ASW_VERBOSE_MESSAGES
BEGIN_DEFINE_LOGGING_CHANNEL( LOG_ASW_WeaponDeveloper, "ASWVerboseWeapons", 0, LS_ERROR );
ADD_LOGGING_CHANNEL_TAG( "AlienSwarmVerbose" );
ADD_LOGGING_CHANNEL_TAG( "Weapons" );
END_DEFINE_LOGGING_CHANNEL();
#endif // #ifdef ASW_VERBOSE_MESSAGES

BEGIN_DEFINE_LOGGING_CHANNEL( LOG_ASW_Damage, "ASWDamage", 0, LS_MESSAGE );
ADD_LOGGING_CHANNEL_TAG( "AlienSwarm" );
ADD_LOGGING_CHANNEL_TAG( "Damage" );
END_DEFINE_LOGGING_CHANNEL();

void CASW_Weapon::Spawn()
{
	//InitializeAttributes();

	BaseClass::Spawn();

	SetModel( GetWorldModel() );

	m_nSkin					= GetWeaponInfo()->m_iPlayerModelSkin;
}

// get the player owner of this weapon (either the marine's commander if the weapon is
//  being held by marine, or the player directly if a player is holding this weapon)
CASW_Player* CASW_Weapon::GetCommander()
{
	CASW_Player *pOwner = NULL;
	CBaseCombatCharacter* pCombatCharOwner = GetOwner();
	if ( pCombatCharOwner && pCombatCharOwner->IsInhabitableNPC() )
	{
		CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pCombatCharOwner );
		pOwner = pNPC->GetCommander();
	}
	else
	{
		pOwner = ToASW_Player( ToBasePlayer( pCombatCharOwner ) );
	}

	return pOwner;
}

CASW_Marine *CASW_Weapon::GetMarine()
{
	CBaseEntity *pOwner = GetOwner();
	if ( pOwner && pOwner->Classify() == CLASS_ASW_MARINE )
		return assert_cast< CASW_Marine * >( pOwner );
	return NULL;
}

#if PREDICTION_ERROR_CHECK_LEVEL > 0
extern void DiffPrint( bool bServer, int nCommandNumber, char const *fmt, ... );
void CASW_Weapon::DiffPrint(  char const *fmt, ... )
{
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	va_list		argptr;
	char		string[1024];
	va_start (argptr,fmt);
	Q_vsnprintf(string, sizeof( string ), fmt,argptr);
	va_end (argptr);

	::DiffPrint( CBaseEntity::IsServer(), pPlayer->CurrentCommandNumber(), "%s", string );
}
#else
void CASW_Weapon::DiffPrint(char const *fmt, ...)
{
	// nothing
}
#endif

void CASW_Weapon::ItemBusyFrame( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return;
	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );

	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons(bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	// check for clearing our weapon switching bool
	if (m_bSwitchingWeapons && gpGlobals->curtime > m_flNextPrimaryAttack)
	{
		m_bSwitchingWeapons = false;
	}

	// check for clearing our firing bool from reloading
	if (m_bInReload && gpGlobals->curtime > m_fReloadClearFiringTime)
	{
		ClearIsFiring();
	}

	if ( ( bReload && !bOldReload ) && UsesClipsForAmmo1() && asw_fast_reload_enabled.GetBool() )
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
		if ( pMarine && m_bInReload )
		{
			// check for a fast reload
			//Msg("%f Check for fast reload while busy\n", gpGlobals->curtime);
			if ( gpGlobals->curtime >= m_fFastReloadStart && gpGlobals->curtime <= m_fFastReloadEnd )
			{
				// todo: reduce next attack time
				m_fFastReloadEnd = 0;
				m_fFastReloadStart = 0;

				float flSucceedDelay = gpGlobals->curtime + 0.5f;
				pOwner->SetNextAttack( flSucceedDelay );
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSucceedDelay;

				// TODO: hook up anim
				//pMarine->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_SUCCEED );

				DispatchParticleEffect( "fast_reload", PATTACH_POINT_FOLLOW, this, "muzzle" );
				pMarine->m_flPreventLaserSightTime = gpGlobals->curtime + 2.5f;

#ifdef GAME_DLL
				pMarine->m_nFastReloadsInARow++;

				IGameEvent *event = gameeventmanager->CreateEvent( "fast_reload" );
				if ( event )
				{
					event->SetInt( "marine", pMarine->entindex() );
					event->SetInt( "reloads", pMarine->m_nFastReloadsInARow );
					gameeventmanager->FireEvent( event );
				}

				if ( pMarine->m_nFastReloadsInARow >= 4 && pMarine->IsInhabited() )
				{
					if ( pMarine->GetMarineResource() )
					{
						pMarine->GetMarineResource()->m_bDidFastReloadsInARow = true;
					}

					if ( pMarine->GetCommander() )
					{
						pMarine->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_FAST_RELOADS_IN_A_ROW );
					}
				}
#endif
				CSoundParameters params;
				if ( !GetParametersForSound( "FastReload.Success", params, NULL ) )
					return;

				EmitSound_t playparams( params );
				playparams.m_nPitch = params.pitch;

				CBroadcastRecipientFilter filter;
				if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
				{
					filter.UsePredictionRules();
				}
				EmitSound( filter, entindex(), playparams );

				//Msg("%f RELOAD SUCCESS! - bAttack1 = %d, bOldAttack1 = %d\n", gpGlobals->curtime, bAttack1, bOldAttack1 );
				//Msg( "S: %f - %f - %f RELOAD SUCCESS! -- Progress = %f\n", gpGlobals->curtime, fFastStart, fFastEnd, flProgress );

#ifdef CLIENT_DLL
				if ( prediction->IsFirstTimePredicted() )
#endif
				{
					pMarine->DoAnimationEvent( PLAYERANIMEVENT_DROP_MAGAZINE_GIB );
				}

#ifdef GAME_DLL
				pMarine->GetMarineSpeech()->PersonalChatter( CHATTER_SELECTION );
#endif
				m_bFastReloadSuccess = true;
				m_bFastReloadFailure = false;
			}
			else if ( m_fFastReloadStart != 0 )
			{
				CSoundParameters params;
				if ( !GetParametersForSound( "FastReload.Miss", params, NULL ) )
					return;

				EmitSound_t playparams( params );
				playparams.m_nPitch = params.pitch;

				CBroadcastRecipientFilter filter;
				if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
				{
					filter.UsePredictionRules();
				}
				EmitSound( filter, entindex(), playparams );

				//Msg("%f RELOAD MISSED! - bAttack1 = %d, bOldAttack1 = %d\n", gpGlobals->curtime, bAttack1, bOldAttack1 );
				//Msg( "S: %f - %f - %f RELOAD MISSED! -- Progress = %f\n", gpGlobals->curtime, fFastStart, fFastEnd, flProgress );
				m_fFastReloadEnd = 0;
				m_fFastReloadStart = 0;

				if ( pOwner )
				{
					float flMissDelay = MAX( gpGlobals->curtime + 2.0f, m_flNextPrimaryAttack + 1.0f );
					pOwner->SetNextAttack( flMissDelay );
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = flMissDelay;
					m_flReloadFailTime = m_flNextPrimaryAttack - gpGlobals->curtime;
				}

				// TODO: hook up anim
				//pMarine->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_FAIL );

#ifdef GAME_DLL
				IGameEvent *event = gameeventmanager->CreateEvent( "fast_reload_fail" );
				if ( event )
				{
					event->SetInt( "marine", pMarine->entindex() );
					gameeventmanager->FireEvent( event, true );
				}

				pMarine->m_nFastReloadsInARow = 0;

				if ( rd_fast_reload_explode_chance.GetFloat() > 0 )
				{
					float flStartFraction = random->RandomFloat( 0.0f, 1.0f );
					if ( rd_fast_reload_explode_chance.GetFloat() > flStartFraction )
					{
						UTIL_ASW_GrenadeExplosion( GetAbsOrigin(), rd_fast_reload_explode_radius.GetFloat() );
						EmitSound( "ASWGrenade.Explode" );

						// damage to nearby things
						CTakeDamageInfo info( this, pMarine, rd_fast_reload_explode_damage.GetFloat(), DMG_BLAST );
						info.SetWeapon( this );
						ASWGameRules()->RadiusDamage( info, GetAbsOrigin(), rd_fast_reload_explode_radius.GetFloat(), CLASS_NONE, NULL );
					}
				}
#endif

				DispatchParticleEffect( "reload_fail", PATTACH_POINT_FOLLOW, this, "muzzle" );

#ifdef GAME_DLL	
				pMarine->GetMarineSpeech()->PersonalChatter( CHATTER_PAIN_SMALL );
#endif
				m_bFastReloadSuccess = false;
				m_bFastReloadFailure = true;

			}
		}
	}

#ifdef CLIENT_DLL
	if ( m_bInReload )
	{
		float fStart = m_fReloadStart;
		float fNext = MAX( m_flNextPrimaryAttack, pOwner->GetNextAttack() );
		float fTotalTime = fNext - fStart;
		if ( fTotalTime <= 0 )
			fTotalTime = 0.1f;

		m_fReloadProgress = ( gpGlobals->curtime - fStart ) / fTotalTime;
	}
	else
	{
		m_fReloadProgress = 0;
	}
	//Msg( "S: %f Reload Progress = %f\n", gpGlobals->curtime, m_fReloadProgress );
#endif //CLIENT_DLL
}

// check player or marine commander's buttons for firing/reload/etc
void CASW_Weapon::ItemPostFrame( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return;
	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );

	bool bThisActive = ( pNPC->GetActiveWeapon() == this );

	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons( bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( pMarine && ( pMarine->IsHacking() || pMarine->GetForcedActionRequest() ) )
	{
		bThisActive = bAttack1 = bAttack2 = bReload = false;
	}

	// check for clearing our weapon switching bool
	if ( m_bSwitchingWeapons && gpGlobals->curtime > m_flNextPrimaryAttack )
	{
		m_bSwitchingWeapons = false;
	}

	// check for clearing our firing bool from reloading
	if ( m_bInReload && gpGlobals->curtime > m_fReloadClearFiringTime )
	{
		ClearIsFiring();
	}

	if ( m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire )
	{
		DelayedAttack();
	}

	if ( UsesClipsForAmmo1() )
	{
		CheckReload();
	}

	bool bFired = false;
	if ( bThisActive )
	{
		//Track the duration of the fire
		//FIXME: Check for IN_ATTACK2 as well?
		//FIXME: What if we're calling ItemBusyFrame?
		m_fFireDuration = bAttack1 ? ( m_fFireDuration + gpGlobals->frametime ) : 0.0f;

		if ( bAttack2 && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) && ( !pMarine || gpGlobals->curtime > pMarine->m_fFFGuardTime ) )
		{
			if ( SecondaryAttackUsesPrimaryAmmo() )
			{
				if ( !IsMeleeWeapon() &&
					( ( UsesClipsForAmmo1() && !( this->PrimaryAmmoLoaded() ) ) || ( !UsesClipsForAmmo1() && pNPC->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) ) )
				{
					HandleFireOnEmpty();
				}
				else
				{
					bFired = true;
					SecondaryAttack();

#ifndef CLIENT_DLL
					if ( pNPC->IsInhabited() )
					{
						IGameEvent *event = gameeventmanager->CreateEvent( "player_alt_fire" );
						if ( event )
						{
							CASW_Player *pPlayer = pNPC->GetCommander();
							event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
							gameeventmanager->FireEvent( event );
						}
					}
#endif
				}
			}
			else if ( UsesSecondaryAmmo() &&
				( ( UsesClipsForAmmo2() && m_iClip2 <= 0 ) ||
					( !UsesClipsForAmmo2() && pNPC->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 ) ) )
			{
				if ( m_flNextEmptySoundTime < gpGlobals->curtime )
				{
					WeaponSound( EMPTY );
					m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
				}
			}
			else
			{
				bFired = true;
				SecondaryAttack();

#ifndef CLIENT_DLL
				if ( pNPC->IsInhabited() )
				{
					IGameEvent *event = gameeventmanager->CreateEvent( "player_alt_fire" );
					if ( event )
					{
						CASW_Player *pPlayer = pNPC->GetCommander();
						event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
						gameeventmanager->FireEvent( event );
					}
				}
#endif

				// Secondary ammo doesn't have a reload animation
				// this code makes secondary ammo come from nowhere!
				/*
				if ( UsesClipsForAmmo2() )
				{
					// reload clip2 if empty
					if (m_iClip2 < 1)
					{
						pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
						m_iClip2 = m_iClip2 + 1;
					}
				}*/
			}
		}

		if ( !bFired && bAttack1 && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( !pMarine || gpGlobals->curtime > pMarine->m_fFFGuardTime ) )
		{
			// Clip empty? Or out of ammo on a no-clip weapon?
			if ( !IsMeleeWeapon() &&
				( ( UsesClipsForAmmo1() && !( this->PrimaryAmmoLoaded() ) ) || ( !UsesClipsForAmmo1() && pNPC->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) ) )
			{
				HandleFireOnEmpty();
			}
			else if ( pNPC->GetWaterLevel() == 3 && m_bFiresUnderwater == false )
			{
				// This weapon doesn't fire underwater
				WeaponSound( EMPTY );
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
				return;
			}
			else
			{
				//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
				//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
				//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
				//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
				//			first shot.  Right now that's too much of an architecture change -- jdw

				// If the firing button was just pressed, reset the firing time
				if ( bAttack1 )
				{
#ifdef CLIENT_DLL
					//Msg("[Client] setting nextprimaryattack to now %f\n", gpGlobals->curtime);
#else
					//Msg("[Server] setting nextprimaryattack to now %f\n", gpGlobals->curtime);

					// Fire event when a player fires a weapon
					IGameEvent *event = gameeventmanager->CreateEvent( "weapon_fire" );
					if ( event )
					{
						CASW_Player *pPlayer = pNPC->GetCommander();

						event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
						event->SetInt( "marine", pNPC->entindex() );
						event->SetInt( "weapon", entindex() );

						gameeventmanager->FireEvent( event, true );
					}
#endif
					m_flNextPrimaryAttack = gpGlobals->curtime;
				}
				PrimaryAttack();
			}
		}
	}

	if ( !bAttack1 )	// clear our firing var if we don't have the attack button held down (not totally accurate since firing could continue for some time after pulling the trigger, but it's good enough for our purposes)
	{
		if ( SecondaryAttackEqualsPrimary() )
		{
			if ( !bAttack2 )
			{
				/* doesnt really important now
				bool bOldAttack2 = false;
				if ( pOwner->IsInhabited() && pOwner->GetCommander() )
				{
					bOldAttack2 = !!( pOwner->m_nOldButtons & IN_ATTACK2 );
				}
				*/
				m_bIsFiring = false;
				if ( bOldAttack1 /* || bOldAttack2 */ )
				{
					OnStoppedFiring();
				}
			}
		}
		else
		{
			m_bIsFiring = false;		// NOTE: Only want to clear primary fire IsFiring bool here (i.e. don't call ClearIsFiring as that'll clear secondary fire too, in subclasses that have it)
			if ( bOldAttack1 )
			{
				OnStoppedFiring();
			}
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( bReload && UsesClipsForAmmo1() )
	{
		if ( m_bInReload )
		{
			// todo: check for a fast reload
			//Msg("Check for fast reload\n");
		}
		else
		{
			// reload when reload is pressed, or if no buttons are down and weapon is empty.
			Reload();
			m_fFireDuration = 0.0f;
		}
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if ( !( bAttack1 || bAttack2 || bReload ) )
	{
		// no fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}

#if !defined( CLIENT_DLL )
	extern ConVar rd_infinite_ammo;

	CASW_Weapon *pWeapon = pNPC->GetActiveASWWeapon();
	if ( rd_infinite_ammo.GetBool() && pWeapon && pWeapon->Classify() != CLASS_ASW_HEAL_GUN && pWeapon->Classify() != CLASS_ASW_FIRE_EXTINGUISHER && pWeapon->Classify() != CLASS_ASW_HEALGRENADE )
	{
		pWeapon->m_iClip1 = pWeapon->GetMaxClip1();
		int iPrimaryAmmoType = pWeapon->GetPrimaryAmmoType();
		if ( iPrimaryAmmoType >= 0 )
			pNPC->SetAmmoCount( GetAmmoDef()->MaxCarry( iPrimaryAmmoType, pNPC ), iPrimaryAmmoType );

		if ( pWeapon->Classify() != CLASS_ASW_MEDRIFLE )
		{
			pWeapon->m_iClip2 = pWeapon->GetMaxClip2();
			int iSecondaryAmmoType = pWeapon->GetSecondaryAmmoType();
			if ( iSecondaryAmmoType >= 0 )
				pNPC->SetAmmoCount( GetAmmoDef()->MaxCarry( iSecondaryAmmoType, pNPC ), iSecondaryAmmoType );
		}
	}
#endif
}

// just dry fire by default
void CASW_Weapon::SecondaryAttack( void )
{
	SendWeaponAnim( ACT_VM_DRYFIRE );
	BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CASW_Weapon::ReloadOrSwitchWeapons( void )
{
	bool bAutoReload = true;
	CBaseCombatCharacter* pCombatCharOwner = GetOwner();
	if ( pCombatCharOwner && pCombatCharOwner->Classify() == CLASS_ASW_MARINE )
	{
		CASW_Marine* pMarine = assert_cast<CASW_Marine*>(pCombatCharOwner);
		if (pMarine->GetCommander() && pMarine->IsInhabited())
		{
			CASW_Player* pPlayer = pMarine->GetCommander();
			bAutoReload = pPlayer->ShouldAutoReload();
		}
	}

	// if (HasAnyAmmo())    // asw add later!
	m_bFireOnEmpty = false;
	if ( UsesClipsForAmmo1() && 
			 (m_iClip1 == 0) && 
			 (GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) == false && 
			 bAutoReload && 
			 m_flNextPrimaryAttack < gpGlobals->curtime && 			 
			 m_flNextSecondaryAttack < gpGlobals->curtime && !m_bInReload)
	{
		// if we're successfully reloading, we're done
		if ( Reload() )
		{
			return true;
		}
		else
		{
			ClearIsFiring();
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon has some ammo
//-----------------------------------------------------------------------------
bool CASW_Weapon::HasAmmo( void )
{
	// Weapons with no ammo types can always be selected
	if ( m_iPrimaryAmmoType == -1 && m_iSecondaryAmmoType == -1  )
		return true;
	if ( GetWeaponFlags() & ITEM_FLAG_SELECTONEMPTY )
		return true;

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return false;
	return ( m_iClip1 > 0 || pOwner->GetAmmoCount( m_iPrimaryAmmoType ) || m_iClip2 > 0 || pOwner->GetAmmoCount( m_iSecondaryAmmoType ) );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon has some ammo loaded in the primary clip
//
// Useful because some weapons (like the ammo bag) may have special conditions
// other than how many bullets are in the clip - They can specialize this fn
//-----------------------------------------------------------------------------
bool CASW_Weapon::PrimaryAmmoLoaded( void )
{
	return (m_iClip1 > 0);
}


void CASW_Weapon::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 )
	{
		Reload();
		return;
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return;
	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );
	CASW_Player *pPlayer = GetCommander();

	m_bIsFiring = true;
	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound( SINGLE );

	if ( m_iClip1 <= AmmoClickPoint() )
	{
		LowAmmoSound();
	}

	// tell the marine to tell its weapon to draw the muzzle flash
	pNPC->DoMuzzleFlash();

	// sets the animation on the weapon model itself
	SendWeaponAnim( GetPrimaryAttackActivity() );

	// sets the animation on the marine holding this weapon
	//pMarine->SetAnimation( PLAYER_ATTACK1 );

#ifdef GAME_DLL	// check for turning on lag compensation
	if ( pPlayer && pNPC->IsInhabited() )
	{
		CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetCurrentUserCommand() );
	}
#endif

	FireBulletsInfo_t info;
	info.m_vecSrc = pNPC->Weapon_ShootPosition();
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( pPlayer && pMarine && pMarine->IsInhabited() )
	{
		info.m_vecDirShooting = pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() );	// 45 degrees = 0.707106781187
	}
	else
	{
#ifdef CLIENT_DLL
		Msg( "Error, clientside firing of a weapon that's being controlled by an AI marine\n" );
#else
		info.m_vecDirShooting = pNPC->GetActualShootTrajectory( info.m_vecSrc );
#endif
	}

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();
	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		if ( !fireRate )
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = MIN( info.m_iShots, m_iClip1 );
		m_iClip1 -= info.m_iShots;

#ifdef GAME_DLL
		if ( m_iClip1 <= 0 && pMarine && pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		{
			// check he doesn't have ammo in an ammo bay
			CASW_Weapon_Ammo_Bag *pAmmoBag = NULL;
			CASW_Weapon *pWeapon = pMarine->GetASWWeapon( 0 );
			if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
				pAmmoBag = assert_cast< CASW_Weapon_Ammo_Bag * >( pWeapon );

			if ( !pAmmoBag )
			{
				pWeapon = pMarine->GetASWWeapon( 1 );
				if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
					pAmmoBag = assert_cast< CASW_Weapon_Ammo_Bag * >( pWeapon );
			}
			if ( !pAmmoBag || !pAmmoBag->CanGiveAmmoToWeapon( this ) )
				pMarine->OnWeaponOutOfAmmo( true );
		}
#endif
	}
	else
	{
		info.m_iShots = MIN( info.m_iShots, pNPC->GetAmmoCount( m_iPrimaryAmmoType ) );
		pNPC->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	info.m_flDistance = asw_weapon_max_shooting_distance.GetFloat();
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1; // asw tracer test everytime
	info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();

	info.m_vecSpread = GetBulletSpread();
	info.m_flDamage = GetWeaponDamage();
#ifndef CLIENT_DLL
	if ( asw_debug_marine_damage.GetBool() )
		Msg( "Weapon dmg = %f\n", info.m_flDamage );
	if ( pMarine && pMarine->GetMarineResource() )
		pMarine->GetMarineResource()->OnFired_ScaleDamage( info );
	if ( asw_DebugAutoAim.GetBool() )
	{
		NDebugOverlay::Line( info.m_vecSrc, info.m_vecSrc + info.m_vecDirShooting * info.m_flDistance, 64, 0, 64, true, 1.0 );
	}
#endif

	pNPC->FireBullets( info );

	// increment shooting stats
#ifndef CLIENT_DLL
	if ( pMarine && pMarine->GetMarineResource() )
	{
		pMarine->GetMarineResource()->UsedWeapon( this, info.m_iShots );
		pMarine->OnWeaponFired( this, info.m_iShots );
	}
#endif
}

bool CASW_Weapon::IsPredicted(void) const
{
	return true;
}

// ========================
// reloading
// ========================

float CASW_Weapon::GetReloadTime()
{
	// can adjust for marine's weapon skill here
	float fReloadTime = GetWeaponInfo()->flReloadTime;
	if (GetMarine())
	{
		float fSpeedScale = MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_RELOADING, ASW_MARINE_SUBSKILL_RELOADING_SPEED_SCALE);
		fReloadTime *= fSpeedScale;

#ifdef GAME_DLL
		// riflemod: bots reload very fast because they are stupid to die 
		// during long reloads
		if ( rd_bot_strong.GetBool() && !GetMarine()->IsInhabited() )
		{
			fReloadTime = 1.0f;
		}
#endif
	}

	//CALL_ATTRIB_HOOK_FLOAT( fReloadTime, mod_reload_time );

	return fReloadTime;
}

float CASW_Weapon::GetReloadFailTime()
{
	return m_flReloadFailTime;
}

bool CASW_Weapon::Reload( void )
{
	bool bReloaded = ASWReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );

	if ( bReloaded )
	{
#ifdef GAME_DLL
		CASW_Marine *pMarine = GetMarine();
		if ( pMarine )
		{
			if ( pMarine->IsAlienNear() )
			{
				pMarine->GetMarineSpeech()->Chatter(CHATTER_RELOADING);
			}

			CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
			if ( pMR )
			{
				pMR->m_TimelineAmmo.RecordValue( pMarine->GetAllAmmoCount() );
			}
		}
#endif
	}
	return bReloaded;
}

bool CASW_Weapon::ASWReload( int iClipSize1, int iClipSize2, int iActivity )
{
	if ( m_bInReload )	// we're already reloading!
	{
		Msg("ASWReload already reloading\n");
		Assert(false);
		return true;
	}

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || !ASWGameRules() )
		return false;

	bool bReload = false;
	if ( m_bIsFiring )
	{
		OnStoppedFiring();
	}

	// If you don't have clips, then don't try to reload them.
	if ( UsesClipsForAmmo1() )
	{
		// need to reload primary clip?
		int primary	= MIN( iClipSize1 - m_iClip1, pMarine->GetAmmoCount( m_iPrimaryAmmoType ) );
		if ( primary != 0 )
		{
			bReload = true;
		}
		else
		{
			// check if we have an ammo bag we can take a clip from instead
			CASW_Weapon_Ammo_Bag* pAmmoBag = NULL;
			CASW_Weapon* pWeapon = pMarine->GetASWWeapon(0);
			if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
				pAmmoBag = assert_cast<CASW_Weapon_Ammo_Bag*>(pWeapon);

			if (!pAmmoBag)
			{
				pWeapon = pMarine->GetASWWeapon(1);
				if (pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG)
					pAmmoBag = assert_cast<CASW_Weapon_Ammo_Bag*>(pWeapon);
			}

			if ( pAmmoBag && pAmmoBag->CanGiveAmmoToWeapon( this ) )
			{
#ifdef CLIENT_DLL
				bReload = true;
#else
				pAmmoBag->GiveClipTo(pMarine, m_iPrimaryAmmoType, true);

				// now we've given a clip, check if we can reload
				primary	= MIN(iClipSize1 - m_iClip1, pMarine->GetAmmoCount(m_iPrimaryAmmoType));
				if ( primary != 0 )
				{
					bReload = true;
				}
#endif
			}
		}
	}

	if ( UsesClipsForAmmo2() )
	{
		// need to reload secondary clip?
		int secondary = MIN( iClipSize2 - m_iClip2, pMarine->GetAmmoCount( m_iSecondaryAmmoType ) );
		if ( secondary != 0 )
		{
			bReload = true;
		}
	}

	if ( !bReload )
		return false;

	m_bFastReloadSuccess = false;
	m_bFastReloadFailure = false;

#ifndef CLIENT_DLL
	if ( GetMaxClip1() > 1 )
	{
		// Fire event when a player reloads a weapon with more than a bullet per clip
		IGameEvent * event = gameeventmanager->CreateEvent( "weapon_reload" );
		if ( event )
		{		
			CASW_Player *pPlayer = NULL;
			pPlayer = pMarine->GetCommander();

			int nClipSize = GetMaxClip1();

			int nClips = pMarine->GetAmmoCount( m_iPrimaryAmmoType ) / nClipSize;

			CASW_Weapon_Ammo_Bag* pAmmoBag = NULL;
			CASW_Weapon* pWeapon = pMarine->GetASWWeapon(0);
			if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
				pAmmoBag = assert_cast<CASW_Weapon_Ammo_Bag*>(pWeapon);

			if (!pAmmoBag)
			{
				pWeapon = pMarine->GetASWWeapon(1);
				if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AMMO_BAG )
					pAmmoBag = assert_cast<CASW_Weapon_Ammo_Bag*>(pWeapon);
			}
			if ( pAmmoBag && this != pAmmoBag )
			{
				nClips += pAmmoBag->NumClipsForWeapon( this );					
			}

			event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
			event->SetInt( "marine", pMarine->entindex() );
			event->SetInt( "lost", m_iClip1 );
			event->SetInt( "clipsize", nClipSize );
			event->SetInt( "clipsremaining", nClips - 1 );
			event->SetInt( "clipsmax", GetAmmoDef()->MaxCarry( m_iPrimaryAmmoType, pMarine ) / nClipSize );

			gameeventmanager->FireEvent( event );
		}
	}
#endif

	m_fReloadClearFiringTime = gpGlobals->curtime + GetFireRate();

	float fReloadTime = GetReloadTime();
	float flSequenceEndTime = gpGlobals->curtime + fReloadTime;
	pMarine->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;
	//Msg("  Setting nextprimary attack time to %f from aswreload\n", m_flNextPrimaryAttack);

	m_bInReload = true;	

	// set fast reload timings
	//  assuming 2.8 base reload time
	//    ~0.29 
	RandomSeed( CBaseEntity::GetPredictionRandomSeed() & 255 );	

	float flStartFraction = random->RandomFloat( 0.29f, 0.35f );

	// set width by difficulty
	float flFastReloadWidth = 0.12f;
	switch( ASWGameRules()->GetSkillLevel() )
	{
		default:
		case 1: 
		case 2: flFastReloadWidth = random->RandomFloat( 0.10f, 0.1f ); break;		// easy/normal
		case 3: flFastReloadWidth = random->RandomFloat( 0.08f, 0.12f ); break;		// hard
		case 4: flFastReloadWidth = random->RandomFloat( 0.06f, 0.10f ); break;		// insane
		case 5: flFastReloadWidth = random->RandomFloat( 0.055f, 0.09f ); break;		// imba
	}
	// scale by marine skills
	flFastReloadWidth *= MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_RELOADING, ASW_MARINE_SUBSKILL_RELOADING_FAST_WIDTH_SCALE );
	
	m_fReloadStart = gpGlobals->curtime;
	m_fFastReloadStart = gpGlobals->curtime + flStartFraction * fReloadTime;
	m_fFastReloadEnd = m_fFastReloadStart + flFastReloadWidth * fReloadTime;

	SendReloadEvents();

#ifdef GAME_DLL
	pMarine->RemoveWeaponPowerup( this );
#endif

	return true;
}


void CASW_Weapon::SendReloadEvents()
{
	CASW_Marine* marine = NULL;
	CBaseCombatCharacter* pCombatCharOwner = GetOwner();
	if ( pCombatCharOwner && pCombatCharOwner->Classify() == CLASS_ASW_MARINE )
		marine = assert_cast<CASW_Marine*>(pCombatCharOwner);
	else
		return;
	
#ifdef CLIENT_DLL
	if (marine->IsAnimatingReload())	// don't play the anim twice
		return;
#else
	CASW_GameStats.Event_MarineReloading( marine, this );
#endif

	// Make the player play his reload animation. (and send to clients)
	marine->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

// function unused (done by CASW_Marine::Weapon_Switch instead?)
void CASW_Weapon::SendWeaponSwitchEvents()
{
	CASW_Marine* marine = NULL;
	CBaseCombatCharacter* pCombatCharOwner = GetOwner();
	if ( pCombatCharOwner && pCombatCharOwner->Classify() == CLASS_ASW_MARINE )
		marine = assert_cast<CASW_Marine*>(pCombatCharOwner);
	else
		return;

	// Make the player play his reload animation. (and send to clients)
	marine->DoAnimationEvent( PLAYERANIMEVENT_WEAPON_SWITCH );
}

bool CASW_Weapon::IsReloading() const
{
	return m_bInReload;
}

// fixme: technically this returns if we're firing or anything else that stops firing (except reloading, which is checked for)
bool CASW_Weapon::IsFiring()// const
{
	return m_bIsFiring;
}

float CASW_Weapon::GetFireRate()
{
	float flRate = GetWeaponInfo()->m_flFireRate;

	//CALL_ATTRIB_HOOK_FLOAT( flRate, mod_fire_rate );

	return flRate;
}

void CASW_Weapon::GetButtons( bool &bAttack1, bool &bAttack2, bool &bReload, bool &bOldReload, bool &bOldAttack1 )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
	{
		bAttack1 = false;
		bAttack2 = false;
		bReload = false;
		bOldReload = false;
		bOldAttack1 = false;
		return;
	}

	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	// don't allow firing when frozen/stopped from a pickup/kick
	if ( ( pMarine && pMarine->IsControllingTurret() ) || ( pNPC->GetFlags() & FL_FROZEN )
		|| ( pMarine && gpGlobals->curtime < pMarine->GetStopTime() ) || ( pMarine && pMarine->GetCurrentMeleeAttack() ) )
	{
		bAttack1 = false;
		bAttack2 = false;
		bReload = false;
		bOldReload = false;
		bOldAttack1 = false;
		return;
	}

	if ( pNPC->IsInhabited() && pNPC->GetCommander() )
	{
		bAttack1 = !!( pNPC->GetCommander()->m_nButtons & IN_ATTACK );
		bAttack2 = !!( pNPC->GetCommander()->m_nButtons & IN_ATTACK2 );
		bReload = !!( pNPC->GetCommander()->m_nButtons & IN_RELOAD );
		bOldReload = !!( pNPC->m_nOldButtons & IN_RELOAD );
		bOldAttack1 = !!( pNPC->m_nOldButtons & IN_ATTACK );
		return;
	}

	// does our uninhabited marine want to fire?
#ifdef GAME_DLL
	bAttack1 = pNPC->AIWantsToFire();
	bAttack2 = pNPC->AIWantsToFire2();
	bReload = pNPC->AIWantsToReload();
	bOldReload = false;
	bOldAttack1 = false;
#else
	bAttack1 = false;
	bAttack2 = false;
	bReload = false;
	bOldReload = false;
	bOldAttack1 = false;
#endif
}

bool CASW_Weapon::ShouldMarineMoveSlow()
{
	return (IsReloading() || IsFiring());
}

void CASW_Weapon::ClearIsFiring()
{
	m_bIsFiring = false;
}


bool CASW_Weapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( m_bIsFiring )
	{
		OnStoppedFiring();
	}
	ClearIsFiring();

	/* UNDONE: This was causing trouble in the offhand welder. It seems like this was to enforce the grenade toss 
	animation completes, but it's probably better that pressing the offhand key guarantees item use 
	rather than having it misfire because of an rare condition.

	CASW_Marine* pMarine = dynamic_cast<CASW_Marine*>(GetOwner());

	// check if our offhand item is doing an offhand delayed attack, cancel it if so
	if (pMarine && pMarine->GetASWWeapon(2) && pMarine->GetASWWeapon(2)->m_bShotDelayed)	// cancel throwing if we're switching out
	{
		pMarine->GetASWWeapon(2)->m_bShotDelayed = false;
	}
	*/

	return BaseClass::Holster( pSwitchingTo );	

	m_bFastReloadSuccess = false;
	m_bFastReloadFailure = false;
}

void CASW_Weapon::FinishReload( void )
{
	CASW_Marine *pOwner = GetMarine();

	if (pOwner)
	{
		// If I use primary clips, reload primary
		if ( UsesClipsForAmmo1() )
		{
			// asw: throw away what's in the clip currently
			m_iClip1 = 0;
			int primary	= MIN( GetMaxClip1() - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));	
			m_iClip1 += primary;
			pOwner->RemoveAmmo( primary, m_iPrimaryAmmoType);
		}

		// If I use secondary clips, reload secondary
		if ( UsesClipsForAmmo2() )
		{
			int secondary = MIN( GetMaxClip2() - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
			m_iClip2 += secondary;
			pOwner->RemoveAmmo( secondary, m_iSecondaryAmmoType );
		}

		if ( m_bReloadsSingly )
		{
			m_bInReload = false;
		}

#ifdef GAME_DLL
		if ( !m_bFastReloadSuccess )
		{
			pOwner->m_nFastReloadsInARow = 0;
		}

		// Fire event when a player finishes reloading a weapon
		IGameEvent * event = gameeventmanager->CreateEvent( "weapon_reload_finish" );
		if ( event )
		{
			CASW_Player *pPlayer = pOwner->GetCommander();

			int nClipSize = GetMaxClip1();
			int nClips = pOwner->GetAmmoCount( m_iPrimaryAmmoType ) / nClipSize;

			event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
			event->SetInt( "marine", pOwner->entindex() );
			event->SetInt( "clipsize", nClipSize );
			event->SetInt( "clipsremaining", nClips );
			event->SetInt( "clipsmax", GetAmmoDef()->MaxCarry( m_iPrimaryAmmoType, pOwner ) / nClipSize );

			gameeventmanager->FireEvent( event );
		}
#endif

#ifdef CLIENT_DLL
		if ( prediction->IsFirstTimePredicted() )
#endif
		{
			if ( !m_bFastReloadSuccess )
			{
				pOwner->DoAnimationEvent( PLAYERANIMEVENT_DROP_MAGAZINE_GIB );
			}
		}

		m_bFastReloadSuccess = false;
		m_bFastReloadFailure = false;
	}
}

void CASW_Weapon::SetWeaponVisible( bool visible )
{
#ifdef CLIENT_DLL
//	Msg("[C] %s SetWeaponVisible %d\n", GetClassname(), visible);
#else
	//Msg("[S] %s SetWeaponVisible %d\n", GetClassname(), visible);
#endif
	if ( ViewModelIsMarineAttachment() && GetOwner() && GetOwner()->Classify() == CLASS_ASW_MARINE )
		visible = true;

	BaseClass::SetWeaponVisible(visible);
	/*
	if ( visible )
	{
		RemoveEffects( EF_NODRAW );
	}
	else
	{
		AddEffects( EF_NODRAW );
	}*/
}

void CASW_Weapon::ApplyWeaponSwitchTime(float fSwitchDelay)
{
	// play weaponswitch sound
	if (ASWGameRules() && ASWGameRules()->GetGameState() >= ASW_GS_INGAME)
	{
		WeaponSound(SPECIAL3);	
	}
	float flSequenceEndTime = gpGlobals->curtime + fSwitchDelay;
	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner)
		pOwner->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

	m_bSwitchingWeapons = true;
}

const float CASW_Weapon::GetAutoAimAmount()
{
	return 0.0f;
	//return AUTOAIM_2DEGREES;
}

// dot has to be lower than this to count
const float CASW_Weapon::GetVerticalAdjustOnlyAutoAimAmount()
{
	return 0.66f;
}

void CASW_Weapon::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/swarm/Bayonet/bayonet.mdl" );
	if ( const char *szMagazineGibName = GetMagazineGibModelName() )
		PrecacheModel( szMagazineGibName );
	PrecacheScriptSound( GetEquipSound() );
	PrecacheScriptSound( "ASW_Rifle.ReloadA" );
	PrecacheScriptSound( "ASW_Rifle.ReloadB" );
	PrecacheScriptSound( "ASW_Rifle.ReloadC" );
	PrecacheScriptSound( "FastReload.Success" );
	PrecacheScriptSound( "FastReload.Miss" );

	const CASW_WeaponInfo *pWeaponInfo = GetWeaponInfo();
	if ( pWeaponInfo )
	{
		// find equipment list index
		m_pEquipItem = g_ASWEquipmentList.GetEquipItemFor( GetClassname() );
		Assert( m_pEquipItem || Classify() == CLASS_RD_WEAPON_GENERIC_OBJECT );
		m_iEquipmentListIndex = m_pEquipItem ? m_pEquipItem->m_iItemIndex : -1;

		if ( m_pEquipItem )
		{
			m_iPrimaryAmmoType = m_pEquipItem->m_iAmmo1;
			m_iSecondaryAmmoType = m_pEquipItem->m_iAmmo2;
		}

		if ( pWeaponInfo->szDisplayModel && pWeaponInfo->szDisplayModel[0] )
		{
			PrecacheModel( pWeaponInfo->szDisplayModel );
		}
		if ( pWeaponInfo->szDisplayModel2 && pWeaponInfo->szDisplayModel2[0] )
		{
			PrecacheModel( pWeaponInfo->szDisplayModel2 );
		}
	}
}

const CASW_EquipItem *CASW_Weapon::GetEquipItem() const
{
	return m_pEquipItem;
}

const CASW_WeaponInfo *CASW_Weapon::GetWeaponInfo() const
{
	return assert_cast< const CASW_WeaponInfo * >( &GetWpnData() );
}

bool CASW_Weapon::SendWeaponAnim(int iActivity)
{
	// no animations in 3rd person unless subclasses decide to have them
	return false;
}

const char *CASW_Weapon::GetEquipSound()
{
	return "BaseCombatCharacter.AmmoPickup";
}

void CASW_Weapon::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );

	SetModel( GetViewModel() );

	if ( ViewModelIsMarineAttachment() )
	{
		m_nSkin = pOwner->GetSkin();
	}

	pOwner->EmitSound( GetEquipSound() );
}

float CASW_Weapon::GetTurnRateModifier()
{
	if (IsFiring())
		return 0.5f;

	return 1.0f;
}

int CASW_Weapon::ASW_SelectWeaponActivity(int idealActivity)
{
	switch( idealActivity )
	{
		//case ACT_IDLE:			idealActivity = ACT_DOD_STAND_IDLE; break;
		//case ACT_CROUCHIDLE:	idealActivity = ACT_DOD_CROUCH_IDLE; break;
		//case ACT_RUN_CROUCH:	idealActivity = ACT_DOD_CROUCHWALK_IDLE; break;
		case ACT_WALK:			idealActivity = ACT_WALK_AIM_RIFLE; break;
		case ACT_RUN:			idealActivity = ACT_RUN_AIM_RIFLE; break;
		case ACT_IDLE:			idealActivity = ACT_IDLE_RIFLE; break;
		default: break;
	}
	return idealActivity;
}

bool CASW_Weapon::SupportsBayonet()
{
	return false;
}

float CASW_Weapon::GetMovementScale()
{
	return ShouldMarineMoveSlow() ? 0.5f : 1.0f;
}

float CASW_Weapon::GetWeaponPvpDamageBase()
{
	extern ConVar rd_rifle_dmg_base;
	return rd_rifle_dmg_base.GetFloat();
}

int CASW_Weapon::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_ACCURACY;
}

int CASW_Weapon::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_ACCURACY_RIFLE_DMG;
}


float CASW_Weapon::GetWeaponDamage()
{
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	if ( ASWDeathmatchMode() )
	{
		flDamage = GetWeaponPvpDamageBase(); 
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine( GetMarine(), ASW_Skill(GetWeaponSkillId()), GetWeaponSubSkillId() );
	}

	//CALL_ATTRIB_HOOK_FLOAT( flDamage, mod_damage_done );

	return flDamage;
}

const char *CASW_Weapon::GetASWShootSound( int iIndex, int &iPitch )
{
	if ( iIndex == SINGLE || iIndex == SINGLE_NPC )
	{
		iIndex = IsCarriedByLocalPlayer() ? SINGLE : SINGLE_NPC;
	}

	if ( iIndex == WPN_DOUBLE || iIndex == DOUBLE_NPC )
	{
		iIndex = IsCarriedByLocalPlayer() ? WPN_DOUBLE : DOUBLE_NPC;
	}

	if ( iIndex == RELOAD || iIndex == RELOAD_NPC )
	{
		iIndex = IsCarriedByLocalPlayer() ? RELOAD : RELOAD_NPC;

		// play the weapon sound according to marine skill
		int iSkill = MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_RELOADING, ASW_MARINE_SUBSKILL_RELOADING_SOUND);
		switch (iSkill)
		{
		case 5: return GetWpnData().aShootSounds[ FAST_RELOAD ]; break;
		case 4: iPitch = 120; return GetWpnData().aShootSounds[ iIndex ]; break;
		case 3: iPitch = 115; return GetWpnData().aShootSounds[ iIndex ]; break;
		case 2: iPitch = 110; return GetWpnData().aShootSounds[ iIndex ]; break;
		case 1: iPitch = 105; return GetWpnData().aShootSounds[ iIndex ]; break;
		default: return GetWpnData().aShootSounds[ iIndex ]; break;
		};
	}
	return GetShootSound( iIndex );
}

void CASW_Weapon::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
	//asw hack - don't allow normal reloading sounds to be triggered, since we fire them from the marine's reloading animation anim event
	if (sound_type == RELOAD)
		return;

	if (sound_type == SPECIAL2)
		sound_type = RELOAD;
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	int iPitch = 100;
	const char *shootsound = GetASWShootSound( sound_type, iPitch );
	//Msg("%s:%f WeaponSound %d %s\n", IsServer() ? "S" : "C", gpGlobals->curtime, sound_type, shootsound);
	if ( !shootsound || !shootsound[0] )
		return;	

	CSoundParameters params;
	if ( !GetParametersForSound( shootsound, params, NULL ) )
		return;

	EmitSound_t playparams(params);
	if (soundtime != 0)
		playparams.m_flSoundTime = soundtime;
	playparams.m_nPitch = params.pitch;

	CASW_Player *pPlayer = GetCommander();

	if ( params.play_to_owner_only )
	{
		// Am I only to play to my owner?
		if ( GetOwner() && pPlayer && pPlayer->GetNPC() == GetOwner() )
		{
			CSingleUserRecipientFilter filter( pPlayer );
			if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
			{
				filter.UsePredictionRules();
			}
			EmitSound(filter, GetOwner()->entindex(), playparams);
			//EmitSound( filter, GetOwner()->entindex(), shootsound, NULL, soundtime );
		}
	}
	else
	{
		// Play weapon sound from the owner
		if ( GetOwner() )
		{
			CPASAttenuationFilter filter( GetOwner(), params.soundlevel );
			if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
			{
				filter.UsePredictionRules();
			}
			EmitSound(filter, GetOwner()->entindex(), playparams);

#if !defined( CLIENT_DLL )
			if( sound_type == EMPTY )
			{
				CSoundEnt::InsertSound( SOUND_COMBAT, GetOwner()->GetAbsOrigin(), SOUNDENT_VOLUME_EMPTY, 0.2, GetOwner() );
			}
#endif
		}
		// If no owner play from the weapon (this is used for thrown items)
		else
		{
			CPASAttenuationFilter filter( this, params.soundlevel );
			if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
			{
				filter.UsePredictionRules();
			}
			EmitSound( filter, entindex(), shootsound, NULL, soundtime ); 
		}
	}
}

void CASW_Weapon::PlaySoundDirectlyToOwner( const char *szSoundName )
{
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	if ( !assert_cast< CASW_Inhabitable_NPC * >( GetOwner() )->IsInhabited() )
		return;

	CSoundParameters params;
	if ( !GetParametersForSound( szSoundName, params, NULL ) )
		return;

	EmitSound_t playparams( params );

	CSingleUserRecipientFilter filter( pPlayer );
	if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
	{
		filter.UsePredictionRules();
	}
	EmitSound( filter, GetOwner()->entindex(), playparams );
}

void CASW_Weapon::PlaySoundToOthers( const char *szSoundName )
{
	CASW_Player *pPlayer = GetCommander();
	CSoundParameters params;
	if ( !GetParametersForSound( szSoundName, params, NULL ) )
		return;

	EmitSound_t playparams( params );

	// Play weapon sound from the owner
	if ( GetOwner() )
	{
		CPASAttenuationFilter filter( GetOwner(), params.soundlevel );
		if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
		{
			filter.UsePredictionRules();
		}
		if ( pPlayer && assert_cast< CASW_Inhabitable_NPC * >( GetOwner() )->IsInhabited() )
		{
			filter.RemoveRecipient( pPlayer );
		}
		EmitSound(filter, GetOwner()->entindex(), playparams);
	}
	// If no owner play from the weapon (this is used for thrown items)
	else
	{
		CPASAttenuationFilter filter( this, params.soundlevel );
		if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
		{
			filter.UsePredictionRules();
		}
		EmitSound(filter, entindex(), playparams);
	}
}


void CASW_Weapon::LowAmmoSound()
{
	CASW_Player *pPlayer = GetCommander();
	if ( GetOwner() && pPlayer && pPlayer->GetNPC() == GetOwner() )
	{
		CSingleUserRecipientFilter filter( pPlayer );
		if ( IsPredicted() && CBaseEntity::GetPredictionPlayer() )
		{
			filter.UsePredictionRules();
		}
		CSoundParameters params;
		if ( !GetParametersForSound( "ASW_Weapon.LowAmmoClick", params, NULL ) )
			return;

		EmitSound_t playparams(params);
		EmitSound( filter, GetOwner()->entindex(), playparams );
	}
}

// no autoswitching our weapons
bool CASW_Weapon::AllowsAutoSwitchFrom( void ) const
{
	return false;
}

int CASW_Weapon::AmmoClickPoint()
{
	return 6;
}

// user message based tracer type
const char* CASW_Weapon::GetUTracerType()
{
	return "ASWUTracer";
}

void CASW_Weapon::UpdateOnRemove( void )
{
#ifdef CLIENT_DLL
    RemoveLaserPointerEffect();
	if ( m_hLaserSight.Get() )
	{
		UTIL_Remove( m_hLaserSight );
	}
#endif
	//IHasAttributes *pOwnerAttribInterface = dynamic_cast<IHasAttributes *>( GetOwnerEntity() );
	//if ( pOwnerAttribInterface )
	//{
		//pOwnerAttribInterface->GetAttributeManager()->RemoveProvider( this );
	//}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Dropped weapon functions
//-----------------------------------------------------------------------------

bool CASW_Weapon::AllowedToPickup( CASW_Inhabitable_NPC *pNPC )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine || !ASWGameRules() || !pMarine->GetMarineResource() )
		return false;

	// check if we're swapping for an existing item
	int i = pMarine->GetWeaponPositionForPickup( GetClassname(), m_bIsTemporaryPickup );
	CASW_Weapon *pWeapon = pMarine->GetASWWeapon( i );
	const char *szSwappingClass = pWeapon ? pWeapon->GetClassname() : "";

	// first check if the gamerules will allow it
	bool bAllowed = ASWGameRules()->MarineCanPickup( pMarine->GetMarineResource(), GetClassname(), szSwappingClass );
#ifdef CLIENT_DLL
	m_bSwappingWeapon = ( pWeapon != NULL );
#endif

	return bAllowed;
}

bool CASW_Weapon::IsUsable(CBaseEntity *pUser)
{
	return (!IsBeingCarried() && pUser && pUser->GetAbsOrigin().DistTo(GetAbsOrigin()) < ASW_MARINE_USE_RADIUS);	// near enough?
}

int CASW_Weapon::LookupAttachment( const char *pAttachmentName )
{
	// skip over the special basecombatweapon lookup that always uses the world model instead of the current model
	return BaseClass::BaseClass::LookupAttachment( pAttachmentName );
}

void CASW_Weapon::OnStoppedFiring()
{
	// used by child classes
}

// marine has started doing a diving roll
void CASW_Weapon::OnStartedRoll()
{
	if ( m_bFastReloadSuccess )
		return;

	// cancel reloading
	m_bFastReloadSuccess = false;
	m_bFastReloadFailure = false;
	m_bInReload = false; 
}

const char *CASW_Weapon::GetPrintName() const
{
	Assert( GetEquipItem() );
	return GetEquipItem() ? GetEquipItem()->m_szShortName : "";
}

int CASW_Weapon::GetMaxClip1() const
{
	Assert( GetEquipItem() );
	return GetEquipItem() ? GetEquipItem()->MaxAmmo1() : WEAPON_NOCLIP;
}

int CASW_Weapon::GetMaxClip2() const
{
	Assert( GetEquipItem() );
	return GetEquipItem() ? GetEquipItem()->MaxAmmo2() : WEAPON_NOCLIP;
}

int CASW_Weapon::GetDefaultClip1() const
{
	Assert( GetEquipItem() );
	return GetEquipItem() ? GetEquipItem()->DefaultAmmo1() : WEAPON_NOCLIP;
}

int CASW_Weapon::GetDefaultClip2() const
{
	Assert( GetEquipItem() );
	return GetEquipItem() ? GetEquipItem()->DefaultAmmo2() : WEAPON_NOCLIP;
}

bool CASW_Weapon::IsInventoryEquipSlotValid() const
{
	if ( m_hOriginalOwnerPlayer == NULL || m_iInventoryEquipSlot == -1 )
		return false;

	Assert( m_pEquipItem );
	if ( !m_pEquipItem )
		return false;

	const CRD_ItemInstance &instance = m_hOriginalOwnerPlayer->m_EquippedItemDataDynamic[m_iInventoryEquipSlot];
	if ( !instance.IsSet() )
		return false;

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( instance.m_iItemDefID );
	Assert( pDef );
	if ( !pDef )
		return false;

	if ( pDef->EquipIndex != m_pEquipItem->m_iItemIndex )
		return false;

	return pDef->ItemSlotMatches( m_pEquipItem->m_bIsExtra ? "extra" : "weapon" );
}
