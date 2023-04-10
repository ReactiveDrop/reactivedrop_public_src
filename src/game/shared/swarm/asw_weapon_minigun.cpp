#include "cbase.h"
#include "asw_weapon_minigun.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_te_legacytempents.h"
#include "c_asw_gun_smoke_emitter.h"
#include "soundenvelope.h"
#define CASW_Marine_Resource C_ASW_Marine_Resource
#define CASW_Marine C_ASW_Marine
#else
#include "asw_lag_compensation.h"
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_weapon_ammo_bag_shared.h"
#include "asw_marine_resource.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "asw_deathmatch_mode_light.h"
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar asw_minigun_spin_rate_threshold( "asw_minigun_spin_rate_threshold", "0.75f", FCVAR_CHEAT | FCVAR_REPLICATED, "Minimum barrel spin rate before minigun will fire" );
ConVar asw_minigun_spin_up_rate( "asw_minigun_spin_up_rate", "1.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Spin up speed of minigun" );
ConVar asw_minigun_spin_down_rate( "asw_minigun_spin_down_rate", "0.4", FCVAR_CHEAT | FCVAR_REPLICATED, "Spin down speed of minigun" );
ConVar rd_minigun_additional_piercing_chance("rd_minigun_additional_piercing_chance", "0.2", FCVAR_CHEAT | FCVAR_REPLICATED, "Additional piercing chance after rd_minigun_additional_piercing_delay of continuous shooting");
ConVar rd_minigun_additional_piercing_delay("rd_minigun_additional_piercing_delay", "3.0", FCVAR_CHEAT | FCVAR_REPLICATED, "After this time piercing chance increased");
extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;
extern ConVar asw_DebugAutoAim;
#ifdef CLIENT_DLL
extern ConVar rd_tracer_tint_self;
extern ConVar rd_tracer_tint_other;
#endif


IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Minigun, DT_ASW_Weapon_Minigun )

BEGIN_NETWORK_TABLE( CASW_Weapon_Minigun, DT_ASW_Weapon_Minigun )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flSpinRate ) ),
	RecvPropBool( RECVINFO( m_bHalfShot ) ),
#else
	SendPropFloat( SENDINFO( m_flSpinRate ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bHalfShot ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Minigun )
	DEFINE_PRED_FIELD_TOL( m_flSpinRate, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD( m_bHalfShot, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bShouldUpdateActivityClient, FIELD_BOOLEAN, FTYPEDESC_PRIVATE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_minigun, CASW_Weapon_Minigun );
PRECACHE_WEAPON_REGISTER( asw_weapon_minigun );

BEGIN_ENT_SCRIPTDESC( CASW_Weapon_Minigun, CASW_Weapon, "IAF Minigun" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptClip1, "Clip1", "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetMaxClip1, "GetMaxClip1", "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetDefaultClip1, "GetDefaultClip1", "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetMaxAmmo1, "GetMaxAmmo1", "" )
#ifdef GAME_DLL
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetClip1, "SetClip1", "" )
#endif
END_SCRIPTDESC()

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Minigun )
	
END_DATADESC()

void CASW_Weapon_Minigun::Spawn()
{
	BaseClass::Spawn();

	UseClientSideAnimation();
}

void CASW_Weapon_Minigun::SecondaryAttack( void )
{
	CASW_Player *pPlayer = GetCommander();
	if (!pPlayer)
		return;

	CASW_Marine *pMarine = GetMarine();
	if (!pMarine)
		return;

	// dry fire
	//SendWeaponAnim( ACT_VM_DRYFIRE );
	//BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

#endif /* not client */

CASW_Weapon_Minigun::CASW_Weapon_Minigun()
{
#ifdef CLIENT_DLL
	m_pBarrelSpinSound = NULL;
	m_flLastMuzzleFlashTime = 0;
	m_bShouldUpdateActivityClient = false;
#endif
	m_flTimeFireStarted = 0;
	m_bHalfShot = false;
}

bool CASW_Weapon_Minigun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// stop the barrel spinning
	m_flSpinRate = 0.0f;

	return BaseClass::Holster( pSwitchingTo );
}


CASW_Weapon_Minigun::~CASW_Weapon_Minigun()
{

}

void CASW_Weapon_Minigun::Precache()
{
	PrecacheScriptSound("ASW_Autogun.ReloadA");
	PrecacheScriptSound("ASW_Autogun.ReloadB");
	PrecacheScriptSound("ASW_Autogun.ReloadC");
	PrecacheScriptSound( "ASW_Minigun.Spin" );

	BaseClass::Precache();
}

void CASW_Weapon_Minigun::PrimaryAttack()
{
	// can't attack until the minigun barrel has spun up
	if ( GetSpinRate() < asw_minigun_spin_rate_threshold.GetFloat() )
		return;
	
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{		
		Reload();
		return;
	}

	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return;

	m_bIsFiring = true;
	// MUST call sound before removing a round from the clip of a CMachineGun
	//WeaponSound(SINGLE);

	if (m_iClip1 <= AmmoClickPoint())
	{
		LowAmmoSound();
	}

	// tell the marine to tell its weapon to draw the muzzle flash
	pMarine->DoMuzzleFlash();

	// sets the animation on the weapon model itself
	SendWeaponAnim( GetPrimaryAttackActivity() );

	// sets the animation on the marine holding this weapon
	//pMarine->SetAnimation( PLAYER_ATTACK1 );

#ifdef GAME_DLL	// check for turning on lag compensation
	if (pPlayer && pMarine->IsInhabited())
	{
		CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetCurrentUserCommand() );
	}
#endif

	FireBulletsInfo_t info;
	info.m_vecSrc	 = pMarine->Weapon_ShootPosition( );
	if ( pPlayer && pMarine->IsInhabited() )
	{
		info.m_vecDirShooting = pPlayer->GetAutoaimVectorForMarine(pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount());	// 45 degrees = 0.707106781187
	}
	else
	{
#ifdef CLIENT_DLL
		Msg("Error, clientside firing of a weapon that's being controlled by an AI marine\n");
#else
		info.m_vecDirShooting = pMarine->GetActualShootTrajectory( info.m_vecSrc );
#endif
	}

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate() * ( 1.0f / MAX( GetSpinRate(), asw_minigun_spin_rate_threshold.GetFloat() ) );
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
		int iEffectiveClip = m_iClip1 * 2;
		if ( m_bHalfShot && iEffectiveClip )
		{
			iEffectiveClip--;
		}
		info.m_iShots = MIN( info.m_iShots, iEffectiveClip );
		iEffectiveClip -= info.m_iShots;
		m_iClip1 = ( iEffectiveClip + 1 ) / 2;
		m_bHalfShot = ( iEffectiveClip & 1 ) != 0;

#ifdef GAME_DLL
		if ( m_iClip1 <= 0 && pMarine->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
		{
			// check he doesn't have ammo in an ammo bay
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
			if ( !pAmmoBag || !pAmmoBag->CanGiveAmmoToWeapon(this) )
				pMarine->OnWeaponOutOfAmmo(true);
		}
#endif
	}
	else
	{
		info.m_iShots = MIN( info.m_iShots, pMarine->GetAmmoCount( m_iPrimaryAmmoType ) );
		pMarine->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	info.m_flDistance = asw_weapon_max_shooting_distance.GetFloat();
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;  // asw tracer test everytime
	info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();

	info.m_vecSpread = GetBulletSpread();
	info.m_flDamage = GetWeaponDamage();
#ifndef CLIENT_DLL
	if (asw_debug_marine_damage.GetBool())
		Msg("Weapon dmg = %f\n", info.m_flDamage);
	CASW_Marine_Resource* pPMR = pMarine->GetMarineResource();
	if ( pPMR )
		pPMR->OnFired_ScaleDamage( info );
	if (asw_DebugAutoAim.GetBool())
	{
		NDebugOverlay::Line(info.m_vecSrc, info.m_vecSrc + info.m_vecDirShooting * info.m_flDistance, 64, 0, 64, true, 1.0);
	}
#endif

	if ( !m_flTimeFireStarted )
		m_flTimeFireStarted = gpGlobals->curtime;

	FireBullets(pMarine, info);

	// increment shooting stats
#ifndef CLIENT_DLL
	if (pPMR)
	{
		pPMR->UsedWeapon(this, info.m_iShots);
		pMarine->OnWeaponFired( this, info.m_iShots );
	}
#endif
}

void CASW_Weapon_Minigun::FireBullets( CASW_Marine* pMarine, const FireBulletsInfo_t& info )
{
	float fPiercingChance = 0;
	CASW_Marine_Profile* pProfile = pMarine->GetMarineProfile();
	if ( pProfile && pProfile->GetMarineClass() == MARINE_CLASS_SPECIAL_WEAPONS )
		fPiercingChance = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_PIERCING );

	if ( gpGlobals->curtime - m_flTimeFireStarted > rd_minigun_additional_piercing_delay.GetFloat() )
	{
		float addChance = rd_minigun_additional_piercing_chance.GetFloat();
		addChance = MAX(0, addChance);
		fPiercingChance += addChance;
	}

	if ( fPiercingChance > 0 )
	{
		pMarine->FirePenetratingBullets( info, 1, fPiercingChance, 0 );
	}
	else
	{
		pMarine->FireRegularBullets( info );
	}
}

void CASW_Weapon_Minigun::OnStoppedFiring()
{
	BaseClass::OnStoppedFiring();

	m_flTimeFireStarted = 0.0f;
}

void CASW_Weapon_Minigun::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	UpdateSpinRate();
}

void CASW_Weapon_Minigun::ItemBusyFrame( void )
{
	BaseClass::ItemBusyFrame();

	UpdateSpinRate();
}

void CASW_Weapon_Minigun::UpdateSpinRate()
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons(bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	CASW_Marine *pMarine = GetMarine();
	bool bMeleeing = pMarine && ( pMarine->GetCurrentMeleeAttack() != NULL );
	bool bWalking = pMarine && ( pMarine->m_bWalking.Get() || pMarine->m_bForceWalking.Get() );

	bool bSpinUp = !m_bInReload && !bMeleeing && ( bAttack1 || bAttack2 || bWalking );
	if ( bSpinUp )
	{
		m_flSpinRate = MIN( 1.0f, GetSpinRate() + gpGlobals->frametime * asw_minigun_spin_up_rate.GetFloat() );
	}
	else
	{
		m_flSpinRate = MAX( 0.0f, GetSpinRate() - gpGlobals->frametime * asw_minigun_spin_down_rate.GetFloat() * ( ( m_bInReload || bMeleeing ) ? 3.0f : 1.0f ) );
	}
}

#ifdef CLIENT_DLL

void CASW_Weapon_Minigun::OnMuzzleFlashed()
{
	BaseClass::OnMuzzleFlashed();
	
	//m_flPlaybackRate = 1.0f;
	m_flLastMuzzleFlashTime = gpGlobals->curtime;

	//Vector attachOrigin;
	//QAngle attachAngles;
	
	int iAttachment = LookupAttachment( "eject1" );
	if( iAttachment != -1 )
	{				
		//		tempents->EjectBrass( attachOrigin, attachAngles, GetAbsAngles(), 1 );		// 0 = brass shells, 2 = grey shells, 3 = plastic shotgun shell casing
		EjectParticleBrass( "weapon_shell_casing_rifle", iAttachment );
	}
	if ( m_hGunSmoke.IsValid() )
	{
		m_hGunSmoke->OnFired();
	}
}


void CASW_Weapon_Minigun::ReachedEndOfSequence()
{
	BaseClass::ReachedEndOfSequence();
	//if (gpGlobals->curtime - m_flLastMuzzleFlashTime > 1.0)	// 0.3 is the real firing time, but spin for a bit longer
	//{
		//if (GetSequenceActivity(GetSequence()) != ACT_VM_IDLE)
			//SetActivity(ACT_VM_IDLE, 0);
	//}
}

float CASW_Weapon_Minigun::GetMuzzleFlashScale( void )
{
	// if we haven't calculated the muzzle scale based on the carrying marine's skill yet, then do so
	if (m_fMuzzleFlashScale == -1)
	{
		C_ASW_Marine *pMarine = GetMarine();
		if (pMarine)
			m_fMuzzleFlashScale = 2.0f * MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_MUZZLE);
		else
			return 2.0f;
	}
	return m_fMuzzleFlashScale;
}

Vector CASW_Weapon_Minigun::GetMuzzleFlashTint()
{
	HACK_GETLOCALPLAYER_GUARD( "need local player to see what color the muzzle flash should be" );
	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	C_ASW_Inhabitable_NPC *pViewNPC = pLocalPlayer ? pLocalPlayer->GetViewNPC() : NULL;
	Vector vecColor = pViewNPC && GetOwner() == pViewNPC ? rd_tracer_tint_self.GetColorAsVector() : rd_tracer_tint_other.GetColorAsVector();

	if ( ( GetMuzzleFlashScale() / 2.0f ) >= 1.15f ) // red if our muzzle flash is the biggest size based on our skill
	{
		vecColor.y *= 0.65f;
		vecColor.z *= 0.65f;
	}

	return vecColor;
}

#endif

float CASW_Weapon_Minigun::GetMovementScale()
{
	return ShouldMarineMoveSlow() ? 0.4f : 0.95f;
}

bool CASW_Weapon_Minigun::ShouldMarineMoveSlow()
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons(bAttack1, bAttack2, bReload, bOldReload, bOldAttack1 );

	return ( BaseClass::ShouldMarineMoveSlow() || bAttack2 || bAttack1 || GetSpinRate() >= 0.99f );
}

float CASW_Weapon_Minigun::GetWeaponDamage()
{
	//float flDamage = 7.0f;
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_minigun_dmg_base;
	if ( rd_minigun_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_minigun_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_AUTOGUN, ASW_MARINE_SUBSKILL_AUTOGUN_DMG);
	}

	//CALL_ATTRIB_HOOK_FLOAT( flDamage, mod_damage_done );

	return flDamage;
}

const Vector &CASW_Weapon_Minigun::GetBulletSpread( void )
{
	static Vector cone = Vector( 0.13053, 0.13053, 0.02 );	// VECTOR_CONE_15DEGREES with flattened Z
	static Vector cone_duck = Vector( 0.05234, 0.05234, 0.01 ); // VECTOR_CONE_6DEGREES with flattened Z

	CASW_Marine *marine = GetMarine();

	if ( marine )
	{
		if ( marine->GetAbsVelocity() == Vector( 0, 0, 0 ) && marine->m_bWalking )
			return cone_duck;
	}
	return cone;
}

#ifdef CLIENT_DLL
const char* CASW_Weapon_Minigun::GetPartialReloadSound(int iPart)
{
	switch (iPart)
	{
		case 1: return "ASW_Autogun.ReloadB"; break;
		case 2: return "ASW_Autogun.ReloadC"; break;
		default: break;
	};
	return "ASW_Autogun.ReloadA";
}

void CASW_Weapon_Minigun::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateGunSmoke();

		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void CASW_Weapon_Minigun::ClientThink()
{
	BaseClass::ClientThink();

	UpdateSpinningBarrel();
}

ConVar asw_minigun_pitch_min( "asw_minigun_pitch_min", "50", FCVAR_CHEAT | FCVAR_REPLICATED, "Pitch of barrel spin sound" );
ConVar asw_minigun_pitch_max( "asw_minigun_pitch_max", "150", FCVAR_CHEAT | FCVAR_REPLICATED, "Pitch of barrel spin sound" );

void CASW_Weapon_Minigun::UpdateSpinningBarrel()
{
	if ( GetSpinRate() > 0.1 && GetSequenceActivity( GetSequence() ) != ACT_VM_PRIMARYATTACK )
	{
		SetActivity( ACT_VM_PRIMARYATTACK, 0 );
		m_bShouldUpdateActivityClient = true;
	}

	if ( GetSpinRate() < 0.1 && m_bShouldUpdateActivityClient )
	{
		SetActivity( ACT_VM_IDLE, 0 );
		m_bShouldUpdateActivityClient = false;
	}

	if ( GetSpinRate() > 0.01f ) //seems always > 0 due to computations, lets increase it a little.
	{
		if( !m_pBarrelSpinSound )
		{
			CPASAttenuationFilter filter( this );
			m_pBarrelSpinSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_Minigun.Spin" );
			CSoundEnvelopeController::GetController().Play( m_pBarrelSpinSound, 0.0, 100 );
		}
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pBarrelSpinSound, MIN( 1.0f, GetSpinRate() * 3.0f ), 0.0f );
		CSoundEnvelopeController::GetController().SoundChangePitch( m_pBarrelSpinSound, asw_minigun_pitch_min.GetFloat() + ( GetSpinRate() * ( asw_minigun_pitch_max.GetFloat() - asw_minigun_pitch_min.GetFloat() ) ), 0.0f );
	}
	else
	{
		if ( m_pBarrelSpinSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pBarrelSpinSound );
			m_pBarrelSpinSound = NULL;
		}
	}
}

void CASW_Weapon_Minigun::CreateGunSmoke()
{	
	int iAttachment = LookupAttachment( "muzzle" );
	if ( iAttachment <= 0 )
	{
		Msg("error, couldn't find muzzle attachment for flamer tip\n");
		return;
	}

	C_ASW_Gun_Smoke_Emitter *pEnt = new C_ASW_Gun_Smoke_Emitter;
	if (!pEnt)
	{
		Msg("Error, couldn't create new C_ASW_Gun_Smoke_Emitter for autogun smoke\n");
		return;
	}
	if ( !pEnt->InitializeAsClientEntity( NULL, false ) )
	{
		Msg("Error, couldn't InitializeAsClientEntity for autogun smoke\n");
		UTIL_Remove( pEnt );
		return;
	}
	
	Vector vecMuzzle;
	QAngle angMuzzle;
	GetAttachment( iAttachment, vecMuzzle, angMuzzle );

	Q_snprintf(pEnt->m_szTemplateName, sizeof(pEnt->m_szTemplateName), "autogunsmoke");
	pEnt->m_fScale = 1.0f;
	pEnt->m_bEmit = false;
	pEnt->SetAbsOrigin(vecMuzzle);
	pEnt->CreateEmitter();
	pEnt->SetAbsOrigin(vecMuzzle);
	pEnt->SetAbsAngles(angMuzzle);

	pEnt->ClientAttach(this, "muzzle");

	m_hGunSmoke = pEnt;
}

void CASW_Weapon_Minigun::SetDormant( bool bDormant )
{
	if ( bDormant )
	{
		if ( m_pBarrelSpinSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy(m_pBarrelSpinSound);
			m_pBarrelSpinSound = NULL;
		}
	}
	BaseClass::SetDormant( bDormant );
}

void CASW_Weapon_Minigun::UpdateOnRemove()
{
	if ( m_hGunSmoke.IsValid() && m_hGunSmoke.Get())
	{
		UTIL_Remove( m_hGunSmoke.Get() );
	}

	if ( m_pBarrelSpinSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pBarrelSpinSound );
		m_pBarrelSpinSound = NULL;
	}

	BaseClass::UpdateOnRemove();
}
#else

void CASW_Weapon_Minigun::Drop( const Vector &vecVelocity )
{
	// stop the barrel spinning
	m_flSpinRate = 0.0f;

	BaseClass::Drop( vecVelocity );
}
#endif

#ifdef CLIENT_DLL

// if true, the marine shoots from minigun
bool CASW_Weapon_Minigun::ShouldMarineMinigunShoot()
{
	return IsFiring();
}

#endif

int CASW_Weapon_Minigun::DisplayClip1()
{
	int iClip1 = Clip1() * 2;
	if ( m_bHalfShot )
	{
		iClip1--;
	}

	return iClip1;
}

int CASW_Weapon_Minigun::DisplayMaxClip1()
{
	return GetMaxClip1() * 2;
}

int CASW_Weapon_Minigun::ScriptClip1()
{
	return DisplayClip1();
}

int CASW_Weapon_Minigun::ScriptGetMaxClip1()
{
	return DisplayMaxClip1();
}

int CASW_Weapon_Minigun::ScriptGetDefaultClip1()
{
	return GetDefaultClip1() * 2;
}

int CASW_Weapon_Minigun::ScriptGetMaxAmmo1()
{
	return BaseClass::ScriptGetMaxAmmo1() * 2;
}

#ifdef GAME_DLL
void CASW_Weapon_Minigun::ScriptSetClip1( int iAmmo )
{
	BaseClass::ScriptSetClip1( ( iAmmo + 1 ) / 2 );
	m_bHalfShot = ( iAmmo & 1 ) != 0;
}
#endif

void CASW_Weapon_Minigun::FinishReload()
{
	BaseClass::FinishReload();

	m_bHalfShot = false;
}
