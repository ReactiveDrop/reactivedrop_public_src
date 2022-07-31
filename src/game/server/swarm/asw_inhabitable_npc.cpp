#include "cbase.h"
#include "asw_inhabitable_npc.h"
#include "asw_player.h"
#include "asw_weapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS( funCASW_Inhabitable_NPC, CASW_Inhabitable_NPC );

IMPLEMENT_SERVERCLASS_ST(CASW_Inhabitable_NPC, DT_ASW_Inhabitable_NPC)
	SendPropEHandle( SENDINFO( m_Commander ) ),
	SendPropEHandle( SENDINFO( m_hUsingEntity ) ),
	SendPropVector( SENDINFO( m_vecFacingPointFromServer ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bInhabited ) ),
	SendPropBool( SENDINFO( m_bWalking ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Inhabitable_NPC )
	DEFINE_FIELD( m_Commander, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hUsingEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecFacingPointFromServer, FIELD_VECTOR ),
	DEFINE_FIELD( m_nOldButtons, FIELD_INTEGER ),
END_DATADESC()

CASW_Inhabitable_NPC::CASW_Inhabitable_NPC()
{
	m_nOldButtons = 0;
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

float CASW_Inhabitable_NPC::MaxSpeed()
{
	return 300;
}
