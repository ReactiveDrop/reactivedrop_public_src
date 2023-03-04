#include "cbase.h"
#include "props.h"
#include "asw_sentry_base.h"
#include "asw_sentry_top.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_marine_skills.h"
#include "asw_marine_speech.h"
#include "asw_marine_resource.h"
#include "world.h"
#include "asw_util_shared.h"
#include "asw_fx_shared.h"
#include "asw_gamerules.h"
#include "asw_weapon_sentry_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SENTRY_BASE_MODEL "models/sentry_gun/sentry_base.mdl"
//#define SENTRY_BASE_MODEL "models/swarm/droneprops/DronePropIdle.mdl"

extern int	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud


ConVar asw_sentry_gun_type("asw_sentry_gun_type", "-1", FCVAR_CHEAT, "Force the type of sentry guns built to this. -1, the default, reads from the marine attributes.");
ConVar asw_sentry_infinite_ammo( "asw_sentry_infinite_ammo", "0", FCVAR_CHEAT );
ConVar asw_sentry_health_base( "asw_sentry_health_base", "300", FCVAR_CHEAT );
ConVar asw_sentry_health_step( "asw_sentry_health_step", "0", FCVAR_CHEAT );
ConVar rd_sentry_take_damage_from_marine( "rd_sentry_take_damage_from_marine", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "If set to 1, players can destroy sentry by shooting at it." );
ConVar rd_sentry_invincible( "rd_sentry_invincible", "0", FCVAR_CHEAT, "If set to 1 sentries will not take damage from anything" );
ConVar rd_sentry_refilled_by_dismantling( "rd_sentry_refilled_by_dismantling", "0", FCVAR_CHEAT, "If set to 1 marine will refill sentry ammo by dismantling it." );

static void *SendProxy_SentryItemDataForOwningPlayer( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	static CRD_ItemInstance s_BlankInstance;

	const CASW_Sentry_Base *pSentry = static_cast< const CASW_Sentry_Base * >( pStructBase );
	CASW_Player *pPlayer = pSentry->m_hOriginalOwnerPlayer;
	if ( !pPlayer || pSentry->m_iInventoryEquipSlotIndex == -1 )
		return &s_BlankInstance;

	return &pPlayer->m_EquippedItemData[pSentry->m_iInventoryEquipSlotIndex];
}

LINK_ENTITY_TO_CLASS( asw_sentry_base, CASW_Sentry_Base );
PRECACHE_REGISTER( asw_sentry_base );

IMPLEMENT_SERVERCLASS_ST(CASW_Sentry_Base, DT_ASW_Sentry_Base)
	SendPropBool(SENDINFO(m_bAssembled)),
	SendPropBool(SENDINFO(m_bIsInUse)),
	SendPropFloat(SENDINFO(m_fAssembleProgress)),
	SendPropFloat(SENDINFO(m_fAssembleCompleteTime)),
	SendPropInt(SENDINFO(m_iAmmo)),	
	SendPropInt(SENDINFO(m_iMaxAmmo)),	
	SendPropBool(SENDINFO(m_bSkillMarineHelping)),
	SendPropInt(SENDINFO(m_nGunType)),
	SendPropInt( SENDINFO( m_iOriginalOwnerSteamAccount ), -1, SPROP_UNSIGNED ),
	SendPropDataTable( "m_InventoryItemData", 0, &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ), SendProxy_SentryItemDataForOwningPlayer ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Sentry_Base )
	DEFINE_THINKFUNC( AnimThink ),
	DEFINE_FIELD( m_hSentryTop, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bAssembled, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsInUse, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fAssembleProgress, FIELD_FLOAT ),
	DEFINE_FIELD( m_fAssembleCompleteTime, FIELD_TIME ),
	DEFINE_FIELD( m_hDeployer, FIELD_EHANDLE ),
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Sentry_Base, CBaseAnimating, "sentry" )
	DEFINE_SCRIPTFUNC( GetAmmo, "" )
	DEFINE_SCRIPTFUNC( SetAmmo, "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetMaxAmmo, "GetMaxAmmo", "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetSentryTop, "GetSentryTop", "" )
END_SCRIPTDESC()

CASW_Sentry_Base::CASW_Sentry_Base()
	: bait1_( NULL ),
	bait2_( NULL ),
	bait3_( NULL ),
	bait4_( NULL )
{
	m_iAmmo = -1;
	m_fSkillMarineHelping = 0;
	m_bSkillMarineHelping = false;
	m_fDamageScale = 1.0f;
	m_nGunType = kAUTOGUN;
	m_bAlreadyTaken = false;
	m_iOriginalOwnerSteamAccount = 0;
	m_hOriginalOwnerPlayer = NULL;
}


CASW_Sentry_Base::~CASW_Sentry_Base()
{
	if ( bait1_ )
	{
		UTIL_Remove( bait1_ );
	}
	if ( bait2_ )
	{
		UTIL_Remove( bait2_ );
	}
	if ( bait3_ )
	{
		UTIL_Remove( bait3_ );
	}
	if ( bait4_ )
	{
		UTIL_Remove( bait4_ );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Sentry_Base::Spawn( void )
{
	SetMoveType( MOVETYPE_NONE );

	SetSolid( SOLID_BBOX );
	SetCollisionGroup( ASW_COLLISION_GROUP_SENTRY );

	Precache();
	SetModel(SENTRY_BASE_MODEL);

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );	

	SetCollisionBounds( Vector(-26,-26,0), Vector(26,26,60));

	int iHealth = asw_sentry_health_base.GetInt() + ( ASWGameRules() ? ASWGameRules()->GetMissionDifficulty() - 5 : 0 ) * asw_sentry_health_step.GetInt();
	SetMaxHealth( iHealth );
	SetHealth( iHealth );
	m_takedamage = DAMAGE_YES;

	SetThink( &CASW_Sentry_Base::AnimThink );	
	SetNextThink( gpGlobals->curtime + 0.1f );

	// check for attaching to elevators
	trace_t	tr;
	UTIL_TraceLine( GetAbsOrigin() + Vector(0, 0, 2),
					GetAbsOrigin() - Vector(0, 0, 32), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction < 1.0f && tr.m_pEnt && !tr.m_pEnt->IsWorld() && !tr.m_pEnt->IsNPC() )
	{
		// reactivedrop: prevent sentry sticking to weapons 
		// only allow func_movelinear and func_tracktrain(possible elevators)
		// 
		if (tr.m_pEnt->Classify() == CLASS_FUNC_MOVELINEAR ||
			tr.m_pEnt->Classify() == CLASS_FUNC_TRACKTRAIN)
		{
			SetParent(tr.m_pEnt);
		}
	}

	m_iMaxAmmo = GetBaseAmmoForGunType( GetGunType() );
	if ( m_iAmmo == -1 )
	{
		m_iAmmo = m_iMaxAmmo;
	}
}

void CASW_Sentry_Base::PlayDeploySound()
{
	EmitSound("ASW_Sentry.Deploy");
}

void CASW_Sentry_Base::Precache()
{
	PrecacheModel( SENTRY_BASE_MODEL );
	PrecacheScriptSound( "ASW_Sentry.SetupLoop" );
	PrecacheScriptSound( "ASW_Sentry.SetupInterrupt" );
	PrecacheScriptSound( "ASW_Sentry.SetupComplete" );
	PrecacheScriptSound( "ASW_Sentry.Dismantled" );

	BaseClass::Precache();
}


int CASW_Sentry_Base::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

int CASW_Sentry_Base::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CASW_Sentry_Base::AnimThink( void )
{		
	if (!m_bAssembled)
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
	
		StudioFrameAdvance();

		m_bSkillMarineHelping = (m_fSkillMarineHelping >= gpGlobals->curtime - 0.2f);
	}
	else
	{
		m_bSkillMarineHelping = false;
	}
}

// player has used this item
void CASW_Sentry_Base::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
		return;

	if ( !m_bIsInUse && !m_bAssembled && nHoldType != ASW_USE_HOLD_RELEASE_FULL )
	{
		pMarine->StartUsing( this );
		pMarine->GetMarineSpeech()->Chatter( CHATTER_USE );
	}
	else if ( m_bAssembled && GetSentryTop() )
	{
		if ( nHoldType == ASW_USE_HOLD_START )
		{
			pMarine->StartUsing( this );
			pMarine->GetMarineSpeech()->Chatter( CHATTER_USE );
		}
		else if ( nHoldType == ASW_USE_HOLD_RELEASE_FULL )
		{
			pMarine->StopUsing();

			if ( !m_bAlreadyTaken )
			{
				//Msg( "Disassembling sentry gun!\n" );
				IGameEvent *event = gameeventmanager->CreateEvent( "sentry_dismantled" );
				if ( event )
				{
					CBasePlayer *pPlayer = pMarine->GetCommander();
					event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
					event->SetInt( "entindex", entindex() );

					gameeventmanager->FireEvent( event );
				}

				CASW_Weapon_Sentry *pWeapon = assert_cast< CASW_Weapon_Sentry * >( Create( GetWeaponNameForGunType( GetGunType() ), WorldSpaceCenter(), GetAbsAngles(), NULL ) );
				if ( !rd_sentry_refilled_by_dismantling.GetBool() )
				{
					pWeapon->SetSentryAmmo( m_iAmmo );
				}
				pWeapon->m_iOriginalOwnerSteamAccount = m_iOriginalOwnerSteamAccount;
				pWeapon->m_hOriginalOwnerPlayer = m_hOriginalOwnerPlayer;
				pWeapon->m_iInventoryEquipSlotIndex = m_iInventoryEquipSlotIndex;

				pMarine->TakeWeaponPickup( pWeapon );
				EmitSound( "ASW_Sentry.Dismantled" );
				UTIL_Remove( this );
				m_bAlreadyTaken = true;
			}

			// TODO: just have the marine pick it up now and let that logic deal with the slot?

			// TODO: Find an empty inv slot. Or default to 2nd.
			//       Drop whatever's in that slot currently
			//		 Create a new sentry gun weapon with our ammo amount and give it to the marine
			//		 Destroy ourselves
		}
		else if ( nHoldType == ASW_USE_RELEASE_QUICK )
		{
			pMarine->StopUsing();

			pMarine->GetMarineSpeech()->Chatter( CHATTER_USE );

			IGameEvent *event = gameeventmanager->CreateEvent( "sentry_rotated" );
			if ( event )
			{
				CBasePlayer *pPlayer = pMarine->GetCommander();
				event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
				event->SetInt( "entindex", entindex() );

				gameeventmanager->FireEvent( event );
			}

			// tell the top piece to turn to face the same way as pMarine is facing
			GetSentryTop()->SetDeployYaw( pMarine->ASWEyeAngles().y );
			GetSentryTop()->PlayTurnSound();
		}
	}
}

#define SENTRY_ASSEMBLE_TIME 7.0f			// was 14.0
void CASW_Sentry_Base::NPCUsing( CASW_Inhabitable_NPC *pNPC, float deltatime )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( m_bIsInUse && !m_bAssembled && pMarine )
	{
		// check if any techs nearby have engineering skill to speed this up
		float fSkillScale = MarineSkills()->GetHighestSkillValueNearby( pMarine->GetAbsOrigin(), ENGINEERING_AURA_RADIUS,
			ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SENTRY );
		CASW_Marine *pSkillMarine = MarineSkills()->GetLastSkillMarine();
		if ( fSkillScale > 0.0f && pSkillMarine && pSkillMarine->GetMarineResource() )
		{
			pSkillMarine->m_fUsingEngineeringAura = gpGlobals->curtime;
			m_fSkillMarineHelping = gpGlobals->curtime;
		}
		else
		{
			m_fSkillMarineHelping = 0;
		}
		if ( fSkillScale < 1.0 )
			fSkillScale = 1.0f;
		float fSetupAmount = ( deltatime * ( 1.0f / SENTRY_ASSEMBLE_TIME ) ) * fSkillScale;
		m_fAssembleProgress += fSetupAmount;
		if ( m_fAssembleProgress >= 1.0f )
		{
			m_fAssembleProgress = 1.0f;

			pMarine->StopUsing();
			m_bAssembled = true;
			m_fAssembleCompleteTime = gpGlobals->curtime;
			pMarine->GetMarineSpeech()->Chatter( CHATTER_SENTRY );
			// spawn top half and activate it
			CASW_Sentry_Top *RESTRICT  pSentryTop = dynamic_cast< CASW_Sentry_Top * >( CreateEntityByName( GetEntityNameForGunType( GetGunType() ) ) );
			m_hSentryTop = pSentryTop;
			if ( pSentryTop )
			{
				pSentryTop->SetSentryBase( this );

				const QAngle &angles = GetAbsAngles();
				pSentryTop->SetAbsAngles( angles );
				DispatchSpawn( pSentryTop );
			}
		}
	}
}

void CASW_Sentry_Base::NPCStartedUsing( CASW_Inhabitable_NPC *pNPC )
{
	EmitSound( "ASW_Sentry.SetupLoop" );

	if ( GetModelPtr() && GetModelPtr()->numskinfamilies() >= kGUNTYPE_MAX + 2 ) // modeller guy says 2 first textures are a must
	{
		switch ( GetGunType() )
		{
		case kAUTOGUN:
			this->m_nSkin = 2;
			break;
		case kCANNON:
			this->m_nSkin = 5;
			break;
		case kFLAME:
			this->m_nSkin = 4;
			break;
		case kICE:
			this->m_nSkin = 3;
			break;
		}
	}

	if ( !m_bIsInUse && m_fAssembleProgress < 1.0f )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "sentry_start_building" );
		if ( event )
		{
			CBasePlayer *pPlayer = pNPC->GetCommander();
			event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
			event->SetInt( "entindex", entindex() );

			gameeventmanager->FireEvent( event );
		}
	}

	m_bIsInUse = true;
}

void CASW_Sentry_Base::NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC )
{
	if ( m_fAssembleProgress >= 1.0f )
	{
		EmitSound( "ASW_Sentry.SetupComplete" );

		IGameEvent *event = gameeventmanager->CreateEvent( "sentry_complete" );
		if ( event )
		{
			CBasePlayer *pPlayer = pNPC->GetCommander();
			event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
			event->SetInt( "entindex", entindex() );
			event->SetInt( "marine", pNPC->entindex() );

			gameeventmanager->FireEvent( event );
		}
	}
	else
	{
		EmitSound( "ASW_Sentry.SetupInterrupt" );

		IGameEvent *event = gameeventmanager->CreateEvent( "sentry_stop_building" );
		if ( event )
		{
			CBasePlayer *pPlayer = pNPC->GetCommander();
			event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
			event->SetInt( "entindex", entindex() );

			gameeventmanager->FireEvent( event );
		}
	}
	m_bIsInUse = false;
}

CASW_Sentry_Top* CASW_Sentry_Base::GetSentryTop()
{	
	return assert_cast<CASW_Sentry_Top*>(m_hSentryTop.Get());
}

HSCRIPT CASW_Sentry_Base::ScriptGetSentryTop()
{
	return ToHScript( GetSentryTop() );
}

bool CASW_Sentry_Base::IsUsable(CBaseEntity *pUser)
{
	return (pUser && pUser->GetAbsOrigin().DistTo(GetAbsOrigin()) < ASW_MARINE_USE_RADIUS);	// near enough?
}

void CASW_Sentry_Base::OnFiredShots( int nNumShots )
{
	if ( !asw_sentry_infinite_ammo.GetBool() )
		m_iAmmo -= nNumShots;

	if ( GetSentryTop() )
	{
		int nThreeQuarterAmmo = GetBaseAmmoForGunType( GetGunType() ) * 0.75f;
		int nLowAmmo = GetBaseAmmoForGunType( GetGunType() ) / 5;

		if ( m_iAmmo <= 0 )
		{
			GetSentryTop()->OnOutOfAmmo();
		}
		else if ( m_iAmmo <= nLowAmmo )
		{
			GetSentryTop()->OnLowAmmo();
		}
		else if ( m_iAmmo <= nThreeQuarterAmmo )
		{
			GetSentryTop()->OnUsedQuarterAmmo();
		}

		if( m_hDeployer )
			m_hDeployer->OnWeaponFired( GetSentryTop(), nNumShots );
	}
}

int CASW_Sentry_Base::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( rd_sentry_invincible.GetBool() )
		return 0;

	if ( !rd_sentry_take_damage_from_marine.GetBool() )
	{
		// no friendly fire damage 
		CBaseEntity *pAttacker = info.GetAttacker();
		if ( pAttacker && pAttacker->Classify() == CLASS_ASW_MARINE )
		{
			return 0;
		}
	}

	return BaseClass::OnTakeDamage(info);
}

// explode if we die
void CASW_Sentry_Base::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage = DAMAGE_NO;

	// explosion effect
	Vector vecPos = GetAbsOrigin() + Vector( 0, 0, 30 );

	trace_t		tr;
	UTIL_TraceLine( vecPos, vecPos - Vector( 0, 0, 60 ), MASK_SHOT,
		this, COLLISION_GROUP_NONE, &tr );

	if ( ( tr.m_pEnt != GetWorldEntity() ) || ( tr.hitbox != 0 ) )
	{
		// non-world needs smaller decals
		if ( tr.m_pEnt && !tr.m_pEnt->IsNPC() )
		{
			UTIL_DecalTrace( &tr, "SmallScorch" );
		}
	}
	else
	{
		UTIL_DecalTrace( &tr, "Scorch" );
	}

	UTIL_ASW_ScreenShake( vecPos, 25.0, 150.0, 1.0, 750, SHAKE_START );

	UTIL_ASW_GrenadeExplosion( vecPos, 400.0f );

	EmitSound( "ASWGrenade.Explode" );

	// damage to nearby things
	ASWGameRules()->RadiusDamage( CTakeDamageInfo( this, info.GetAttacker(), 150.0f, DMG_BLAST ), vecPos, 400.0f, CLASS_NONE, NULL );

	if ( GetSentryTop() )
	{
		UTIL_Remove( GetSentryTop() );
	}

	BaseClass::Event_Killed( info );
}

const char *CASW_Sentry_Base::GetEntityNameForGunType( GunType_t guntype )
{
	AssertMsg1( static_cast<int>(guntype) >= 0, "Faulty guntype %d passed to CASW_Sentry_Base::GetEntityNameForGunType()\n", guntype );
	if ( guntype >= 0 && guntype < kGUNTYPE_MAX )
	{
		return sm_gunTypeToInfo[guntype].m_entityName;
	}
	else
	{
		Warning( "GetEntityNameForGunType called with unsupported GunType_t %d .. defaulting to machine gun.\n", guntype );
		return sm_gunTypeToInfo[kAUTOGUN].m_entityName;
	}
}

const char *CASW_Sentry_Base::GetWeaponNameForGunType( GunType_t guntype )
{
	AssertMsg1( static_cast<int>(guntype) >= 0, "Faulty guntype %d passed to CASW_Sentry_Base::GetWeaponNameForGunType()\n", guntype );
	if ( guntype >= 0 && guntype < kGUNTYPE_MAX )
	{
		return sm_gunTypeToInfo[guntype].m_weaponName;
	}
	else
	{
		Warning( "GetWeaponNameForGunType called with unsupported GunType_t %d .. defaulting to machine gun.\n", guntype );
		return sm_gunTypeToInfo[kAUTOGUN].m_weaponName;
	}
}

int CASW_Sentry_Base::GetBaseAmmoForGunType( GunType_t guntype )
{
	AssertMsg1( static_cast<int>(guntype) >= 0, "Faulty guntype %d passed to CASW_Sentry_Base::GetBaseAmmoForGunType()\n", guntype );
	if ( guntype >= 0 && guntype < kGUNTYPE_MAX )
	{
		return sm_gunTypeToInfo[guntype].m_nBaseAmmo;
	}
	else
	{
		Warning( "GetBaseAmmoForGunType called with unsupported GunType_t %d .. defaulting to machine gun.\n", guntype );
		return sm_gunTypeToInfo[kAUTOGUN].m_nBaseAmmo;
	}
}

CASW_Sentry_Base::GunType_t CASW_Sentry_Base::GetGunType( void ) const
{
	// read the cvar
	int nCvarGunType = asw_sentry_gun_type.GetInt();
	if ( nCvarGunType >= 0 )
	{
		return static_cast<CASW_Sentry_Base::GunType_t>(nCvarGunType);
	}
	else
	{
		return (GunType_t) m_nGunType.Get();
	}
}

int CASW_Sentry_Base::ScriptGetMaxAmmo()
{
	return GetBaseAmmoForGunType( GetGunType() );
}


/// This must exactly match the enum CASW_Sentry_Base::GunType_t
const CASW_Sentry_Base::SentryGunTypeInfo_t CASW_Sentry_Base::sm_gunTypeToInfo[CASW_Sentry_Base::kGUNTYPE_MAX] =
{
	SentryGunTypeInfo_t("asw_sentry_top_machinegun", "asw_weapon_sentry", 450), // kAUTOGUN
	SentryGunTypeInfo_t("asw_sentry_top_cannon", "asw_weapon_sentry_cannon",	  40), // kCANNON
	SentryGunTypeInfo_t("asw_sentry_top_flamer", "asw_weapon_sentry_flamer",  1200), // kFLAME
	SentryGunTypeInfo_t("asw_sentry_top_icer", "asw_weapon_sentry_freeze",    800), // kICE
};
