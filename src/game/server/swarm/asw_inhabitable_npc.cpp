#include "cbase.h"
#include "asw_inhabitable_npc.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "env_tonemap_controller.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar asw_controls;

LINK_ENTITY_TO_CLASS( funCASW_Inhabitable_NPC, CASW_Inhabitable_NPC );

IMPLEMENT_SERVERCLASS_ST( CASW_Inhabitable_NPC, DT_ASW_Inhabitable_NPC )
	SendPropEHandle( SENDINFO( m_Commander ) ),
	SendPropEHandle( SENDINFO( m_hUsingEntity ) ),
	SendPropVector( SENDINFO( m_vecFacingPointFromServer ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bInhabited ) ),
	SendPropBool( SENDINFO( m_bWalking ) ),
	SendPropIntWithMinusOneFlag( SENDINFO( m_iControlsOverride ) ),
	SendPropInt( SENDINFO( m_iHealth ), ASW_ALIEN_HEALTH_BITS ),
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
	DEFINE_FIELD( m_iControlsOverride, FIELD_INTEGER )
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Inhabitable_NPC, CBaseCombatCharacter, "Alien Swarm Inhabitable NPC" )
	DEFINE_SCRIPTFUNC( IsInhabited, "Returns true if a player is controlling this character, false if it is AI-controlled." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetCommander, "GetCommander", "Get the player entity that is \"owns\" this character, eg. the player that is playing this marine or added this marine bot to the lobby." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetControls, "SetControls", "Force this character to use a specific value for asw_controls." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetFogController, "SetFogController", "Force this character to use a specific env_fog_controller." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetPostProcessController, "SetPostProcessController", "Force this character to use a specific postprocess_controller." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetColorCorrection, "SetColorCorrection", "Force this character to use a specific color_correction." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetTonemapController, "SetTonemapController", "Force this character to use a specific env_tonemap_controller." )
END_SCRIPTDESC()

CASW_Inhabitable_NPC::CASW_Inhabitable_NPC()
{
	m_nOldButtons = 0;
	m_iControlsOverride = -1;
}

CASW_Inhabitable_NPC::~CASW_Inhabitable_NPC()
{
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

CASW_Player *CASW_Inhabitable_NPC::GetCommander() const
{
	return m_Commander.Get();
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

CASW_Weapon *CASW_Inhabitable_NPC::GetASWWeapon( int index ) const
{
	return assert_cast< CASW_Weapon * >( GetWeapon( index ) );
}

CASW_Weapon *CASW_Inhabitable_NPC::GetActiveASWWeapon( void ) const
{
	return assert_cast< CASW_Weapon * >( GetActiveWeapon() );
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
	return BaseClass::BaseClass::TranslateSchedule( scheduleType );
}

float CASW_Inhabitable_NPC::MaxSpeed()
{
	return 300;
}

ASW_Controls_t CASW_Inhabitable_NPC::GetASWControls()
{
	if ( m_iControlsOverride >= 0 )
	{
		return ( ASW_Controls_t )m_iControlsOverride.Get();
	}

	return ( ASW_Controls_t )asw_controls.GetInt();
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

void CASW_Inhabitable_NPC::DoImpactEffect( trace_t &tr, int nDamageType )
{
	// don't do impact effects, they're simulated clientside by the tracer usermessage
}

void CASW_Inhabitable_NPC::DoMuzzleFlash()
{
	// asw - muzzle flashes are triggered by tracer usermessages instead to save bandwidth
}

void CASW_Inhabitable_NPC::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
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

	UserMessageBegin( filter, tracer );
	WRITE_SHORT( entindex() );
	WRITE_FLOAT( tr.endpos.x );
	WRITE_FLOAT( tr.endpos.y );
	WRITE_FLOAT( tr.endpos.z );
	WRITE_SHORT( m_iDamageAttributeEffects );
	MessageEnd();
}

void CASW_Inhabitable_NPC::MakeUnattachedTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	const char *tracer = "ASWUTracerUnattached";

	CRecipientFilter filter;
	filter.AddAllPlayers();
	if ( gpGlobals->maxClients > 1 && IsInhabited() && GetCommander() )
	{
		filter.RemoveRecipient( GetCommander() );
	}

	UserMessageBegin( filter, tracer );
	WRITE_SHORT( entindex() );
	WRITE_FLOAT( tr.endpos.x );
	WRITE_FLOAT( tr.endpos.y );
	WRITE_FLOAT( tr.endpos.z );
	WRITE_FLOAT( vecTracerSrc.x );
	WRITE_FLOAT( vecTracerSrc.y );
	WRITE_FLOAT( vecTracerSrc.z );
	WRITE_SHORT( m_iDamageAttributeEffects );
	MessageEnd();
}
