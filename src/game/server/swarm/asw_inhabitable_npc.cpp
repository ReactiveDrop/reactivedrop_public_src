#include "cbase.h"
#include "asw_inhabitable_npc.h"
#include "asw_alien_classes.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "env_tonemap_controller.h"
#include "asw_burning.h"
#include "asw_gamerules.h"
#include "asw_marine.h"
#include "asw_gamestats.h"
#include "asw_director.h"
#include "asw_base_spawner.h"
#include "asw_physics_prop_statue.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define NPC_DEBUG_OVERLAY_FLAGS (OVERLAY_NPC_ROUTE_BIT | OVERLAY_BBOX_BIT | OVERLAY_PIVOT_BIT | OVERLAY_TASK_TEXT_BIT | OVERLAY_TEXT_BIT)

static void DebugNPCsChanged( IConVar *var, const char *pOldValue, float flOldValue );
ConVar asw_debug_npcs( "asw_debug_npcs", "0", FCVAR_CHEAT, "Enables debug overlays for various NPCs", DebugNPCsChanged );
ConVar asw_fire_alien_damage_scale( "asw_fire_alien_damage_scale", "3.0", FCVAR_CHEAT );
ConVar asw_alien_burn_duration( "asw_alien_burn_duration", "5.0f", FCVAR_CHEAT, "Alien burn time" );
extern ConVar asw_alien_stunned_speed;
extern ConVar asw_alien_hurt_speed;
extern ConVar asw_stun_grenade_time;
extern ConVar asw_controls;
extern ConVar asw_debug_alien_damage;

static void DebugNPCsChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
	// don't do anything if sv_cheats isn't on
	if ( !ConVarRef( "sv_cheats" ).GetBool() )
		return;

	bool bSetOverlays = asw_debug_npcs.GetBool();
	for ( CBaseEntity *pEnt = gEntList.FirstEnt(); pEnt; pEnt = gEntList.NextEnt( pEnt ) )
	{
		if ( !pEnt->IsInhabitableNPC() )
			continue;

		if ( bSetOverlays )
			pEnt->m_debugOverlays |= NPC_DEBUG_OVERLAY_FLAGS;
		else
			pEnt->m_debugOverlays &= ~NPC_DEBUG_OVERLAY_FLAGS;
	}
}


LINK_ENTITY_TO_CLASS( funCASW_Inhabitable_NPC, CASW_Inhabitable_NPC );

IMPLEMENT_SERVERCLASS_ST( CASW_Inhabitable_NPC, DT_ASW_Inhabitable_NPC )
	SendPropEHandle( SENDINFO( m_Commander ) ),
	SendPropEHandle( SENDINFO( m_hUsingEntity ) ),
	SendPropVector( SENDINFO( m_vecFacingPointFromServer ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bInhabited ) ),
	SendPropBool( SENDINFO( m_bWalking ) ),
	SendPropIntWithMinusOneFlag( SENDINFO( m_iControlsOverride ) ),
	SendPropInt( SENDINFO( m_iHealth ), ASW_ALIEN_HEALTH_BITS ),
#if PREDICTION_ERROR_CHECK_LEVEL > 1
	SendPropVector( SENDINFO( m_vecBaseVelocity ), -1, SPROP_COORD ),
#else
	SendPropVector( SENDINFO( m_vecBaseVelocity ), 20, 0, -1000, 1000 ),
#endif
	SendPropBool( SENDINFO( m_bElectroStunned ) ),
	SendPropBool( SENDINFO( m_bOnFire ) ),
	SendPropFloat( SENDINFO( m_fSpeedScale ) ),
	SendPropTime( SENDINFO( m_fHurtSlowMoveTime ) ),
	SendPropVector( SENDINFO( m_vecGlowColor ), 10, 0, 0, 1 ),
	SendPropFloat( SENDINFO( m_flGlowAlpha ), 8, 0, 0, 1 ),
	SendPropBool( SENDINFO( m_bGlowWhenOccluded ) ),
	SendPropBool( SENDINFO( m_bGlowWhenUnoccluded ) ),
	SendPropBool( SENDINFO( m_bGlowFullBloom ) ),
	SendPropInt( SENDINFO( m_iAlienClassIndex ), NumBitsForCount( MAX( NELEMS( g_Aliens ), NELEMS( g_NonSpawnableAliens ) + 2 ) ) + 1 ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Inhabitable_NPC )
	DEFINE_FIELD( m_Commander, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hUsingEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecFacingPointFromServer, FIELD_VECTOR ),
	DEFINE_FIELD( m_nOldButtons, FIELD_INTEGER ),
	DEFINE_FIELD( m_hFogController, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hPostProcessController, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hColorCorrection, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTonemapController, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iControlsOverride, FIELD_INTEGER ),
	DEFINE_FIELD( m_iAlienClassIndex, FIELD_INTEGER ),

	DEFINE_FIELD( m_hSpawner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bOnFire, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fHurtSlowMoveTime, FIELD_TIME ),
	DEFINE_FIELD( m_flElectroStunSlowMoveTime, FIELD_TIME ),
	DEFINE_FIELD( m_bElectroStunned, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fNextStunSound, FIELD_FLOAT ),
	DEFINE_FIELD( m_bIgnoreMarines, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_AlienOrders, FIELD_INTEGER ),
	DEFINE_FIELD( m_vecAlienOrderSpot, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_AlienOrderObject, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHoldoutAlien, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flFrozenTime, FIELD_TIME ),

	DEFINE_KEYFIELD( m_bFlammable, FIELD_BOOLEAN, "flammable" ),
	DEFINE_KEYFIELD( m_bTeslable, FIELD_BOOLEAN, "teslable" ),
	DEFINE_KEYFIELD( m_bFreezable, FIELD_BOOLEAN, "freezable" ),
	DEFINE_KEYFIELD( m_flFreezeResistance, FIELD_FLOAT, "freezeresistance" ),
	DEFINE_KEYFIELD( m_bFlinchable, FIELD_BOOLEAN, "flinchable" ),
	DEFINE_KEYFIELD( m_bGrenadeReflector, FIELD_BOOLEAN, "reflector" ),
	DEFINE_KEYFIELD( m_iHealthBonus, FIELD_INTEGER, "healthbonus" ),
	DEFINE_KEYFIELD( m_fSizeScale, FIELD_FLOAT, "sizescale" ),
	DEFINE_KEYFIELD( m_fSpeedScale, FIELD_FLOAT, "speedscale" ),
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Inhabitable_NPC, CBaseCombatCharacter, "Alien Swarm Inhabitable NPC" )
	DEFINE_SCRIPTFUNC( IsInhabited, "Returns true if a player is controlling this character, false if it is AI-controlled." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetCommander, "GetCommander", "Get the player entity that is \"owns\" this character, eg. the player that is playing this marine or added this marine bot to the lobby." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetControls, "SetControls", "Force this character to use a specific value for asw_controls." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetFogController, "SetFogController", "Force this character to use a specific env_fog_controller." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetPostProcessController, "SetPostProcessController", "Force this character to use a specific postprocess_controller." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetColorCorrection, "SetColorCorrection", "Force this character to use a specific color_correction." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetTonemapController, "SetTonemapController", "Force this character to use a specific env_tonemap_controller." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetGlow, "SetGlow", "Make this character glow when occluded or when unoccluded. Does not affect cases where the character would glow due to built-in game logic." )

	DEFINE_SCRIPTFUNC_NAMED( ClearAlienOrders, "ClearOrders", "clear the alien's orders" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptOrderMoveTo, "OrderMoveTo", "order the alien to move to an entity handle, second parameter ignore marines" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptChaseNearestMarine, "ChaseNearestMarine", "order the alien to chase the nearest marine" )
	DEFINE_SCRIPTFUNC( Extinguish, "Extinguish a burning alien." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptIgnite, "Ignite", "Ignites the alien into flames." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptFreeze, "Freeze", "Freezes the alien." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptElectroStun, "ElectroStun", "Stuns the alien." )
	DEFINE_SCRIPTFUNC( Wake, "Wake up the alien." )
	DEFINE_SCRIPTFUNC( SetSpawnZombineOnMarineKill, "Used to spawn a zombine in the place of a killed marine." )
END_SCRIPTDESC()

CASW_Inhabitable_NPC::CASW_Inhabitable_NPC()
{
	m_nOldButtons = 0;
	m_iControlsOverride = -1;

	m_bWasOnFireForStats = false;
	m_bFlammable = true;
	m_bTeslable = true;
	m_bFreezable = true;
	m_flFreezeResistance = 0.0f;
	m_bFlinchable = true;
	m_bGrenadeReflector = false;
	m_iHealthBonus = 0;
	m_fSizeScale = 1.0f;
	m_fSpeedScale = 1.0f;

	m_fHurtSlowMoveTime = 0;
	m_flElectroStunSlowMoveTime = 0;
	m_hSpawner = NULL;
	m_AlienOrders = AOT_None;
	m_vecAlienOrderSpot = vec3_origin;
	m_AlienOrderObject = NULL;
	m_bIgnoreMarines = false;

	m_flBaseThawRate = 0.5f;
	m_flFrozenTime = 0.0f;

	m_bSpawnZombineOnMarineKill = false;
	m_vecGlowColor.Init( 1, 1, 1 );
	m_flGlowAlpha = 1;
	m_bGlowWhenOccluded = false;
	m_bGlowWhenUnoccluded = false;
	m_bGlowFullBloom = false;
}

CASW_Inhabitable_NPC::~CASW_Inhabitable_NPC()
{
}

void CASW_Inhabitable_NPC::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "ASW_Tesla_Laser.Damage" );
}

void CASW_Inhabitable_NPC::Spawn()
{
	BaseClass::Spawn();

	if ( m_SquadName != NULL_STRING )
	{
		CapabilitiesAdd( bits_CAP_SQUAD );
	}

	SetModelScale( m_fSizeScale );
	SetHealthByDifficultyLevel();

	m_iAlienClassIndex = GetAlienClassIndex( this );
}

void CASW_Inhabitable_NPC::OnRestore()
{
	BaseClass::OnRestore();

	m_LagCompensation.Init( this );
}

void CASW_Inhabitable_NPC::NPCInit()
{
	BaseClass::NPCInit();

	if ( asw_debug_npcs.GetBool() )
	{
		m_debugOverlays |= NPC_DEBUG_OVERLAY_FLAGS;
	}

	m_LagCompensation.Init( this );
}

void CASW_Inhabitable_NPC::NPCThink()
{
	BaseClass::NPCThink();

	// stop electro stunning if we're slowed
	if ( m_bElectroStunned && m_lifeState != LIFE_DYING )
	{
		if ( m_flElectroStunSlowMoveTime < gpGlobals->curtime )
		{
			m_bElectroStunned = false;
		}
		else
		{
			if ( gpGlobals->curtime >= m_fNextStunSound )
			{
				m_fNextStunSound = gpGlobals->curtime + RandomFloat( 0.2f, 0.5f );

				EmitSound( "ASW_Tesla_Laser.Damage" );
			}
		}
	}

	if ( gpGlobals->maxClients > 1 )
		m_LagCompensation.StorePositionHistory();

	UpdateThawRate();
}

int	CASW_Inhabitable_NPC::DrawDebugTextOverlays()
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if ( m_debugOverlays & OVERLAY_TEXT_BIT )
	{
		NDebugOverlay::EntityText( entindex(), text_offset, CFmtStr( "Freeze amt.: %f", m_flFrozen.Get() ), 0 );
		text_offset++;
		NDebugOverlay::EntityText( entindex(), text_offset, CFmtStr( "Freeze time: %f", m_flFrozenTime - gpGlobals->curtime ), 0 );
		text_offset++;
	}
	return text_offset;
}

// sets which player commands this marine
void CASW_Inhabitable_NPC::SetCommander( CASW_Player *player )
{
	if ( m_Commander.Get() == player )
	{
		return;
	}

	m_Commander = player;

	if ( player )
	{
		player->OnNPCCommanded( this );
	}
}

HSCRIPT CASW_Inhabitable_NPC::ScriptGetCommander() const
{
	return ToHScript( GetCommander() );
}

// store ASWNetworkID of first commander
void CASW_Inhabitable_NPC::SetInitialCommander( CASW_Player *player )
{
	Q_snprintf( m_szInitialCommanderNetworkID, sizeof( m_szInitialCommanderNetworkID ), "%s", player ? player->GetASWNetworkID() : "None" );
	DevMsg( " %s %d:%s SetInitialCommander id to %s\n", GetClassname(), entindex(), GetEntityNameAsCStr(), m_szInitialCommanderNetworkID );
}

const char *CASW_Inhabitable_NPC::GetPlayerName() const
{
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
	{
		return BaseClass::GetPlayerName();
	}

	return pPlayer->GetPlayerName();
}

void CASW_Inhabitable_NPC::Suicide()
{
	if ( GetFlags() & FL_FROZEN )	// don't allow this if the marine is frozen
		return;

	m_iHealth = 1;

	CTakeDamageInfo info( this, this, Vector( 0, 0, 0 ), GetAbsOrigin(), 100, DMG_NEVERGIB );
	TakeDamage( info );
}

// using entities over time
bool CASW_Inhabitable_NPC::StartUsing( CBaseEntity *pEntity )
{
	if ( GetHealth() <= 0 )
		return false;

	IASW_Server_Usable_Entity *pUsable = dynamic_cast< IASW_Server_Usable_Entity * >( pEntity );
	if ( pUsable )
	{
		if ( !pUsable->IsUsable( this ) )
			return false;

		if ( !pUsable->RequirementsMet( this ) )
			return false;

		pUsable->NPCStartedUsing( this );
		m_hUsingEntity = pEntity;
		return true;
	}

	return false;
}

void CASW_Inhabitable_NPC::StopUsing()
{
	if ( !m_hUsingEntity )
		return;

	IASW_Server_Usable_Entity *pUsable = dynamic_cast< IASW_Server_Usable_Entity * >( m_hUsingEntity.Get() );
	if ( pUsable )
	{
		pUsable->NPCStoppedUsing( this );
	}

	m_hUsingEntity = NULL;
}

// forces marine to look towards a certain point
void CASW_Inhabitable_NPC::SetFacingPoint( const Vector &vec, float fDuration )
{
	m_vecFacingPointFromServer = vec;
	m_fStopFacingPointTime = gpGlobals->curtime + fDuration;
}

int CASW_Inhabitable_NPC::TranslateSchedule( int scheduleType )
{
	// skip CAI_PlayerAlly as it makes enemies back up when they hit an obstacle
	return CAI_BaseActor::TranslateSchedule( scheduleType );
}

float CASW_Inhabitable_NPC::GetIdealSpeed() const
{
	float flBaseSpeed = BaseClass::GetIdealSpeed() * m_fSpeedScale;

	// if the alien is hurt, move slower
	if ( ShouldMoveSlow() )
	{
		if ( m_bElectroStunned.Get() )
		{
			return flBaseSpeed * asw_alien_stunned_speed.GetFloat();
		}
		else
		{
			return flBaseSpeed * asw_alien_hurt_speed.GetFloat();
		}
	}

	return flBaseSpeed;
}

bool CASW_Inhabitable_NPC::ModifyAutoMovement( Vector &vecNewPos )
{
	float fFactor = 1.0f;
	if ( ShouldMoveSlow() )
	{
		if ( m_bElectroStunned.Get() )
		{
			fFactor *= asw_alien_stunned_speed.GetFloat() * 0.1f;
		}
		else
		{
			fFactor *= asw_alien_hurt_speed.GetFloat() * 0.1f;
		}
		Vector vecRelPos = vecNewPos - GetAbsOrigin();
		vecRelPos *= fFactor;
		vecNewPos = GetAbsOrigin() + vecRelPos;
		return true;
	}
	return false;
}

bool CASW_Inhabitable_NPC::OverrideMove( float flInterval )
{
	if ( IsMovementFrozen() )
	{
		SetAbsVelocity( vec3_origin );
		GetMotor()->SetMoveVel( vec3_origin );
		return true;
	}

	return BaseClass::OverrideMove( flInterval );
}

void CASW_Inhabitable_NPC::ScriptSetControls( int iControls )
{
	m_iControlsOverride = MAX( -1, iControls );

	// trigger control scheme update callback
	asw_controls.SetValue( asw_controls.GetString() );
}

#define IMPLEMENT_SCRIPT_SET_CONTROLLER_ENT_FUNC( FuncName, EntityClass, MemberVariable, HammerClassName ) \
void CASW_Inhabitable_NPC::FuncName( HSCRIPT hEnt ) \
{ \
	CBaseEntity *pEnt = ToEnt( hEnt ); \
	if ( !pEnt ) \
	{ \
		MemberVariable = NULL; \
		return; \
	} \
\
	EntityClass *pController = dynamic_cast< EntityClass * >( pEnt ); \
	if ( !pController ) \
	{ \
		Warning( "[%s] " #FuncName " called with an entity that was not " #HammerClassName ".\n", GetDebugName() ); \
		return; \
	} \
\
	MemberVariable = pController; \
}

IMPLEMENT_SCRIPT_SET_CONTROLLER_ENT_FUNC( ScriptSetFogController, CFogController, m_hFogController, env_fog_controller )
IMPLEMENT_SCRIPT_SET_CONTROLLER_ENT_FUNC( ScriptSetPostProcessController, CPostProcessController, m_hPostProcessController, postprocess_controller )
IMPLEMENT_SCRIPT_SET_CONTROLLER_ENT_FUNC( ScriptSetColorCorrection, CColorCorrection, m_hColorCorrection, color_correction )
IMPLEMENT_SCRIPT_SET_CONTROLLER_ENT_FUNC( ScriptSetTonemapController, CEnvTonemapController, m_hTonemapController, env_tonemap_controller )

void CASW_Inhabitable_NPC::OnTonemapTriggerStartTouch( CTonemapTrigger *pTonemapTrigger )
{
	m_hTriggerTonemapList.FindAndRemove( pTonemapTrigger );
	m_hTriggerTonemapList.AddToTail( pTonemapTrigger );
}

void CASW_Inhabitable_NPC::OnTonemapTriggerEndTouch( CTonemapTrigger *pTonemapTrigger )
{
	m_hTriggerTonemapList.FindAndRemove( pTonemapTrigger );
}

void CASW_Inhabitable_NPC::ScriptSetGlow( Vector vecColor, float flAlpha, bool bGlowWhenOccluded, bool bGlowWhenUnoccluded, bool bFullBloom )
{
	m_vecGlowColor = vecColor;
	m_flGlowAlpha = flAlpha;
	m_bGlowWhenOccluded = bGlowWhenOccluded;
	m_bGlowWhenUnoccluded = bGlowWhenUnoccluded;
	m_bGlowFullBloom = bFullBloom;
}

void CASW_Inhabitable_NPC::DoImpactEffect( trace_t &tr, int nDamageType )
{
	// don't do impact effects, they're simulated clientside by the tracer usermessage
}

void CASW_Inhabitable_NPC::DoMuzzleFlash()
{
	// asw - muzzle flashes are triggered by tracer usermessages instead to save bandwidth
}

void CASW_Inhabitable_NPC::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType, int iDamageType )
{
	const char *tracer = "ASWUTracer";
	if ( GetActiveASWWeapon() )
		tracer = GetActiveASWWeapon()->GetUTracerType();

	CRecipientFilter filter;
	filter.AddAllPlayers();
	if ( gpGlobals->maxClients > 1 && IsInhabited() && GetCommander() )
	{
		filter.RemoveRecipient( GetCommander() );
	}

	int iDamageAtt = ( iDamageType & DMG_BUCKSHOT ) ? BULLET_ATT_TRACER_BUCKSHOT : 0;

	UserMessageBegin( filter, tracer );
		WRITE_SHORT( entindex() );
		WRITE_FLOAT( tr.endpos.x );
		WRITE_FLOAT( tr.endpos.y );
		WRITE_FLOAT( tr.endpos.z );
		WRITE_SHORT( m_iDamageAttributeEffects | iDamageAtt );
	MessageEnd();
}

void CASW_Inhabitable_NPC::MakeUnattachedTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType, int iDamageType )
{
	const char *tracer = "ASWUTracerUnattached";

	CRecipientFilter filter;
	filter.AddAllPlayers();
	if ( gpGlobals->maxClients > 1 && IsInhabited() && GetCommander() )
	{
		filter.RemoveRecipient( GetCommander() );
	}

	int iDamageAtt = ( iDamageType & DMG_BUCKSHOT ) ? BULLET_ATT_TRACER_BUCKSHOT : 0;

	UserMessageBegin( filter, tracer );
		WRITE_SHORT( entindex() );
		WRITE_FLOAT( tr.endpos.x );
		WRITE_FLOAT( tr.endpos.y );
		WRITE_FLOAT( tr.endpos.z );
		WRITE_FLOAT( vecTracerSrc.x );
		WRITE_FLOAT( vecTracerSrc.y );
		WRITE_FLOAT( vecTracerSrc.z );
		WRITE_SHORT( m_iDamageAttributeEffects | iDamageAtt );
	MessageEnd();
}

// marines override this
void CASW_Inhabitable_NPC::InhabitedPhysicsSimulate()
{
	BaseClass::PhysicsSimulate();

	CASW_Weapon *pWeapon = GetActiveASWWeapon();
	if ( pWeapon )
		pWeapon->ItemPostFrame();

	// check if offhand weapon needs postframe
	CASW_Weapon *pExtra = GetASWWeapon( ASW_INVENTORY_SLOT_EXTRA );
	if ( pExtra && pExtra != pWeapon && pExtra->m_bShotDelayed )
	{
		pExtra->ItemPostFrame();
	}
}

void CASW_Inhabitable_NPC::PhysicsSimulate()
{
	if ( IsInhabited() )
	{
		InhabitedPhysicsSimulate();
		return;
	}

	BaseClass::PhysicsSimulate();

	CASW_Weapon *pWeapon = GetActiveASWWeapon();
	if ( pWeapon )
		pWeapon->ItemPostFrame();

	// check if offhand weapon needs postframe
	CASW_Weapon *pExtra = GetASWWeapon( ASW_INVENTORY_SLOT_EXTRA );
	if ( pExtra && pExtra != pWeapon && pExtra->m_bShotDelayed )
	{
		pExtra->ItemPostFrame();
	}
}

void CASW_Inhabitable_NPC::SetHealth( int amt )
{
	Assert( amt < ( 1 << ASW_ALIEN_HEALTH_BITS ) || Classify() == CLASS_ASW_QUEEN );
	BaseClass::SetHealth( amt );
}

void CASW_Inhabitable_NPC::SetHealthByDifficultyLevel()
{
	Assert( ASWGameRules() );
	int iHealth = GetBaseHealth();
	iHealth = MAX( 1, ASWGameRules()->ModifyAlienHealthBySkillLevel( iHealth ) );
	if ( asw_debug_alien_damage.GetBool() )
		Msg( "Setting %s's initial health to %d\n", GetClassname(), iHealth + m_iHealthBonus );
	SetHealth( iHealth + m_iHealthBonus );
	SetMaxHealth( iHealth + m_iHealthBonus );
}

int CASW_Inhabitable_NPC::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int iHealthBefore = GetHealth();
	CTakeDamageInfo newInfo = info;

	int result = 0;

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( Classify() == CLASS_ASW_MARINE )
	{
		// marine doesn't want any of this handling, but does want hit confirms
		result = BaseClass::OnTakeDamage_Alive( info );
	}
	else
	{
		CASW_Burning *pBurning = NULL;
		CBaseEntity *pInflictor = info.GetInflictor();
		if ( pInflictor && pInflictor->Classify() == CLASS_ASW_BURNING )
			pBurning = assert_cast< CASW_Burning * >( pInflictor );

		// scale burning damage up
		if ( pBurning )
		{
			newInfo.ScaleDamage( asw_fire_alien_damage_scale.GetFloat() );
			if ( asw_debug_alien_damage.GetBool() )
			{
				Msg( "%d %s hurt by %f dmg (scaled up by asw_fire_alien_damage_scale)\n", entindex(), GetClassname(), newInfo.GetDamage() );
			}
		}
		else
		{
			if ( asw_debug_alien_damage.GetBool() )
			{
				Msg( "%d %s hurt by %f dmg\n", entindex(), GetClassname(), newInfo.GetDamage() );
			}
		}

		result = BaseClass::OnTakeDamage_Alive( newInfo );

		// if we take fire damage, catch on fire
		if ( result > 0 && ( newInfo.GetDamageType() & DMG_BURN ) && m_bFlammable && newInfo.GetWeapon() && !pBurning )
		{
			ASW_Ignite( asw_alien_burn_duration.GetFloat(), 0, pAttacker, newInfo.GetWeapon() );
		}

		// make the alien move slower for 0.5 seconds
		if ( ( newInfo.GetDamageType() & DMG_SHOCK ) && m_bTeslable )
		{
			ElectroStun( asw_stun_grenade_time.GetFloat() );

			m_fNoDamageDecal = true;
		}
		else
		{
			if ( m_fHurtSlowMoveTime < gpGlobals->curtime + 0.5f )
				m_fHurtSlowMoveTime = gpGlobals->curtime + 0.5f;
		}

		if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pAttacker ) )
		{
			pMarine->HurtAlien( this, newInfo );
		}

		// Notify gamestats of the damage
		CASW_GameStats.Event_AlienTookDamage( this, newInfo );
	}

	UTIL_RD_HitConfirm( this, iHealthBefore, newInfo );

	return result;
}

void CASW_Inhabitable_NPC::Event_Killed( const CTakeDamageInfo &info )
{
	if ( Classify() != CLASS_ASW_MARINE )
	{
		if ( ASWGameRules() )
		{
			ASWGameRules()->AlienKilled( this, info );
		}

		CASW_GameStats.Event_AlienKilled( this, info );

		if ( ASWDirector() )
		{
			ASWDirector()->Event_AlienKilled( this, info );
		}

		if ( m_hSpawner.Get() )
		{
			m_hSpawner->AlienKilled( this );
		}
	}

	if ( m_flFrozen >= 0.1f )
	{
		bool bShatter = ( RandomFloat() >= IceStatueChance() );
		CreateASWServerStatue( this, COLLISION_GROUP_NONE, info, bShatter, StatueShatterDelay() );
		BaseClass::Event_Killed( CTakeDamageInfo( info.GetAttacker(), info.GetAttacker(), info.GetDamage(), DMG_GENERIC | DMG_REMOVENORAGDOLL | DMG_PREVENT_PHYSICS_FORCE ) );
		RemoveDeferred();
		return;
	}

	BaseClass::Event_Killed( info );
}

void CASW_Inhabitable_NPC::ASW_Ignite( float flFlameLifetime, float flSize, CBaseEntity *pAttacker, CBaseEntity *pDamagingWeapon )
{
	if ( AllowedToIgnite() )
	{
		if ( IsOnFire() )
		{
			// reactivedrop
			if ( ASWBurning() )
				ASWBurning()->ExtendBurning( this, flFlameLifetime );	// 2.5 dps, applied every 0.4 seconds
			//
			return;
		}

		AddFlag( FL_ONFIRE );
		m_bOnFire = true;
		if ( ASWBurning() )
			ASWBurning()->BurnEntity( this, pAttacker, flFlameLifetime, 0.4f, 2.5f * 0.4f, pDamagingWeapon );	// 2.5 dps, applied every 0.4 seconds

		m_OnIgnite.FireOutput( this, this );
	}
}

void CASW_Inhabitable_NPC::Ignite( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner )
{
	Assert( 0 ); // use ASW_Ignite instead
}

void CASW_Inhabitable_NPC::ScriptIgnite( float flFlameLifetime )
{
	ASW_Ignite( flFlameLifetime, 0, NULL, NULL );
}

void CASW_Inhabitable_NPC::Extinguish()
{
	m_bOnFire = false;
	if ( ASWBurning() )
		ASWBurning()->Extinguish( this );
	RemoveFlag( FL_ONFIRE );
}

void CASW_Inhabitable_NPC::ElectroStun( float flStunTime )
{
	if ( m_fHurtSlowMoveTime < gpGlobals->curtime + flStunTime )
		m_fHurtSlowMoveTime = gpGlobals->curtime + flStunTime;
	if ( m_flElectroStunSlowMoveTime < gpGlobals->curtime + flStunTime )
		m_flElectroStunSlowMoveTime = gpGlobals->curtime + flStunTime;

	m_bElectroStunned = true;

	if ( ASWGameResource() && Classify() != CLASS_ASW_MARINE )
	{
		ASWGameResource()->m_iElectroStunnedAliens++;
	}

	// can't jump after being elecrostunned
	CapabilitiesRemove( bits_CAP_MOVE_JUMP );
}

void CASW_Inhabitable_NPC::ScriptElectroStun( float flStunTime )
{
	ElectroStun( flStunTime );
}

//-----------------------------------------------------------------------------
// Freezes this NPC in place for a period of time.
//-----------------------------------------------------------------------------
void CASW_Inhabitable_NPC::Freeze( float flFreezeAmount, CBaseEntity *pFreezer, Ray_t *pFreezeRay )
{
	if ( !m_bFreezable )
		return;

	if ( flFreezeAmount <= 0.0f )
	{
		SetCondition( COND_NPC_FREEZE );
		SetMoveType( MOVETYPE_NONE );
		SetGravity( 0 );
		SetLocalAngularVelocity( vec3_angle );
		SetAbsVelocity( vec3_origin );
		return;
	}

	if ( !CanBeFullyFrozen() && ( flFreezeAmount > 1.0f || flFreezeAmount * ( 1.0f - m_flFreezeResistance ) + m_flFrozen >= 1.0f ) )
		return;

	if ( flFreezeAmount > 1.0f )
	{
		float flFreezeDuration = flFreezeAmount - 1.0f;

		// if freezing permanently, then reduce freeze duration by freeze resistance
		flFreezeDuration *= ( 1.0f - m_flFreezeResistance );

		BaseClass::Freeze( 1.0f, pFreezer, pFreezeRay );			// make alien fully frozen

		m_flFrozenTime = gpGlobals->curtime + flFreezeDuration;
	}
	else
	{
		// if doing a partial freeze, then freeze resistance reduces that
		flFreezeAmount *= ( 1.0f - m_flFreezeResistance );

		BaseClass::Freeze( flFreezeAmount, pFreezer, pFreezeRay );
	}

	UpdateThawRate();
}

void CASW_Inhabitable_NPC::Unfreeze()
{
	BaseClass::Unfreeze();

	// fix up movement type
	if ( IsInhabited() )
	{
		SetMoveType( MOVETYPE_WALK );
	}
}

//-----------------------------------------------------------------------------
// VScript: Freezes this NPC in place for a period of time.
//-----------------------------------------------------------------------------
void CASW_Inhabitable_NPC::ScriptFreeze( float flFreezeAmount )
{
	Freeze( flFreezeAmount, NULL, NULL );
}

void CASW_Inhabitable_NPC::UpdateThawRate()
{
	if ( m_flFrozenTime > gpGlobals->curtime )
	{
		m_flFrozenThawRate = 0.0f;
	}
	else
	{
		m_flFrozenThawRate = m_flBaseThawRate * ( 1.5f - m_flFrozen );
	}
}

// set orders for our alien
//   select schedule should activate the appropriate orders
void CASW_Inhabitable_NPC::SetAlienOrders( AlienOrder_t Orders, Vector vecOrderSpot, CBaseEntity *pOrderObject )
{
	m_AlienOrders = Orders;
	m_vecAlienOrderSpot = vecOrderSpot;	// unused currently
	m_AlienOrderObject = pOrderObject;

	Wake(); // Make sure we at least consider following the orders.

	if ( Orders == AOT_None )
	{
		ClearAlienOrders();
		return;
	}

	ForceDecisionThink();
}

void CASW_Inhabitable_NPC::ClearAlienOrders()
{
	m_AlienOrders = AOT_None;
	m_vecAlienOrderSpot = vec3_origin;
	m_AlienOrderObject = NULL;
	m_bIgnoreMarines = false;
	m_bFailedMoveTo = false;
}

// we're blocking a fellow alien from spawning, let's move a short distance
void CASW_Inhabitable_NPC::MoveAside()
{
	if ( !GetEnemy() && !IsMoving() )
	{
		// random nearby position
		if ( !GetNavigator()->SetWanderGoal( 90, 200 ) )
		{
			if ( !GetNavigator()->SetRandomGoal( 150.0f ) )
			{
				return;	// couldn't find a wander spot
			}
		}
	}
}

void CASW_Inhabitable_NPC::ScriptOrderMoveTo( HSCRIPT hOrderObject, bool bIgnoreMarines )
{
	SetAlienOrders( bIgnoreMarines ? AOT_MoveToIgnoringMarines : AOT_MoveTo, vec3_origin, ToEnt( hOrderObject ) );
}

void CASW_Inhabitable_NPC::ScriptChaseNearestMarine()
{
	SetAlienOrders( AOT_MoveToNearestMarine, vec3_origin, NULL );
}

void CASW_Inhabitable_NPC::SetSpawnZombineOnMarineKill( bool bSpawn )
{
	m_bSpawnZombineOnMarineKill = bSpawn;
}