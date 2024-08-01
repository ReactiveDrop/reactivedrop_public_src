#include "cbase.h"
#include "asw_burning.h"
#include "asw_marine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_burning_interval( "rd_burning_interval", "0.4", FCVAR_CHEAT, "time between afterburn damage ticks" );
ConVar rd_burning_marine_damage( "rd_burning_marine_damage", "10", FCVAR_CHEAT, "damage per second caused by afterburn to marines" );
ConVar rd_burning_humanoid_damage( "rd_burning_humanoid_damage", "30", FCVAR_CHEAT, "damage per second caused by afterburn to non-marine humanoid targets" ); // was 10 multiplied by asw_fire_alien_damage_scale (3), collapsed into just a simple 30 with no scaling
ConVar rd_burning_alien_damage( "rd_burning_alien_damage", "15", FCVAR_CHEAT, "damage per second caused by afterburn to non-humanoid targets" ); // was 5 multiplied by asw_fire_alien_damage_scale (3), collapsed into just a simple 15 with no scaling
ConVar rd_burning_biomass_damage( "rd_burning_biomass_damage", "5", FCVAR_CHEAT, "damage per second caused by afterburn to biomass" ); // was asw_goo_burning_damage (2) divided by 0.4, collapsed into just a simple 5 per second rather than 2 per 0.4 second tick
ConVar rd_burning_rampup( "rd_burning_rampup", "1", FCVAR_CHEAT, "bias for damage ramp-up for non-friendly fire afterburn (0.5 = linear)", true, 0, true, 1 );
ConVar rd_burning_rampup_ff( "rd_burning_rampup_ff", "1", FCVAR_CHEAT, "bias for damage ramp-up for friendly fire afterburn (0.5 = linear)", true, 0, true, 1 );
ConVar rd_burning_rampup_hcff( "rd_burning_rampup_hcff", "1", FCVAR_CHEAT, "bias for damage ramp-up for hardcore friendly fire afterburn (0.5 = linear)", true, 0, true, 1 );
ConVar rd_burning_rampup_time( "rd_burning_rampup_time", "0", FCVAR_CHEAT, "number of seconds for ramp-up for non-friendly fire afterburn", true, 0, false, 0 );
ConVar rd_burning_rampup_time_ff( "rd_burning_rampup_time_ff", "0", FCVAR_CHEAT, "number of seconds for ramp-up for friendly fire afterburn", true, 0, false, 0 );
ConVar rd_burning_rampup_time_hcff( "rd_burning_rampup_time_hcff", "0", FCVAR_CHEAT, "number of seconds for ramp-up for hardcore friendly fire afterburn", true, 0, false, 0 );
extern ConVar asw_marine_ff_absorption;

CASW_BurnInfo::CASW_BurnInfo( CBaseEntity *pEntity, CBaseEntity *pAttacker, float fDieTime, float fBurnInterval, float fDamagePerInterval, CBaseEntity *pDamagingWeapon, bool bFriendlyFire ) :
	m_fDieTime( fDieTime ),
	m_fBurnInterval( fBurnInterval ),
	m_fDamagePerInterval( fDamagePerInterval )
{
	m_fStartTime = gpGlobals->curtime;
	m_fNextBurnTime = gpGlobals->curtime + fBurnInterval;
	m_hEntity = pEntity;
	m_hAttacker = pAttacker;
	m_hDamagingWeapon = pDamagingWeapon;
	m_bFriendlyFire = bFriendlyFire;
}

LINK_ENTITY_TO_CLASS( asw_burning, CASW_Burning );

BEGIN_DATADESC( CASW_Burning )
	DEFINE_THINKFUNC( FireThink ),
END_DATADESC()

CASW_Burning *g_pASW_Burning = NULL;
CASW_Burning *ASWBurning() { return g_pASW_Burning; }

CASW_Burning::CASW_Burning()
{
	Assert( !g_pASW_Burning );
	g_pASW_Burning = this;
	Reset();
}

CASW_Burning::~CASW_Burning()
{
	g_pASW_Burning = NULL;
}

void CASW_Burning::Reset()
{
	m_Burning.PurgeAndDeleteElements();
}

void CASW_Burning::Spawn()
{
	BaseClass::Spawn();
	Precache();
	SetThink( &CASW_Burning::FireThink );
	SetNextThink( gpGlobals->curtime + 0.01f );
}

void CASW_Burning::Precache()
{
	PrecacheScriptSound( "ASWFire.BurningFlesh" );
	PrecacheScriptSound( "ASWFire.BurningObject" );
	PrecacheScriptSound( "ASWFire.StopBurning" );
}

void CASW_Burning::FireThink()
{
	int c = m_Burning.Count();
	CBaseEntity *pEnt;
	for ( int i = c - 1; i >= 0; i-- )	// go backwards in case we remove one
	{
		pEnt = m_Burning[i]->m_hEntity.Get();
		if ( m_Burning[i]->m_fDieTime <= gpGlobals->curtime || !pEnt )
		{
			delete m_Burning[i];
			m_Burning.Remove( i );
			CBaseAnimating *pAnim = pEnt ? pEnt->GetBaseAnimating() : NULL;
			if ( pAnim )
			{
				if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pAnim ) )
					pMarine->Extinguish( NULL, NULL );
				else
					pAnim->Extinguish();	// this makes the entity remove its FL_ONFIRE flag, its m_bOnFire bool and then calls back to us to check its not in the list anymore
			}
		}
		else
		{
			while ( m_Burning[i]->m_fNextBurnTime <= gpGlobals->curtime + 0.01f )
			{
				// hurt the entity!
				CBaseEntity *pAttacker = m_Burning[i]->m_hAttacker.Get();
				if ( !pAttacker )
					pAttacker = this;
				CTakeDamageInfo info( this, pAttacker, m_Burning[i]->m_hDamagingWeapon.Get(), vec3_origin, pEnt->WorldSpaceCenter(), m_Burning[i]->m_fDamagePerInterval, DMG_BURN | DMG_DIRECT );

				float flRampDuration = rd_burning_rampup_time.GetFloat();
				float flRampBias = rd_burning_rampup.GetFloat();
				if ( m_Burning[i]->m_bFriendlyFire )
				{
					if ( asw_marine_ff_absorption.GetInt() > 0 )
					{
						flRampDuration = rd_burning_rampup_time_ff.GetFloat();
						flRampBias = rd_burning_rampup_ff.GetFloat();
					}
					else
					{
						flRampDuration = rd_burning_rampup_time_hcff.GetFloat();
						flRampBias = rd_burning_rampup_hcff.GetFloat();
					}
				}

				if ( flRampDuration > 0 )
				{
					info.ScaleDamage( Bias( RemapValClamped( m_Burning[i]->m_fNextBurnTime - m_Burning[i]->m_fStartTime, 0, flRampDuration, 0, 1 ), flRampBias ) );
				}

				pEnt->TakeDamage( info );

				m_Burning[i]->m_fNextBurnTime += m_Burning[i]->m_fBurnInterval;
			}
		}
	}

	SetNextThinkTime();
}

void CASW_Burning::BurnEntity( CBaseEntity *pEntity, CBaseEntity *pAttacker, float fFireDuration, float fBurnInterval, float fDamagePerInterval, CBaseEntity *pDamagingWeapon /*= NULL */, bool bFriendlyFire /*= false */ )
{
	if ( !pEntity )
		return;

	int c = m_Burning.Count();
	for ( int i = 0; i < c; i++ )
	{
		if ( m_Burning[i]->m_hEntity.Get() == pEntity )	// already burning this ent
		{
			m_Burning[i]->m_fDieTime = MAX( m_Burning[i]->m_fDieTime, gpGlobals->curtime + fFireDuration );

			// can disable friendly fire protection but not enable it
			if ( m_Burning[i]->m_bFriendlyFire )
				m_Burning[i]->m_bFriendlyFire = bFriendlyFire;

			return;
		}
	}

	// add a new one
	CASW_BurnInfo *pBurn = new CASW_BurnInfo( pEntity, pAttacker, gpGlobals->curtime + fFireDuration, fBurnInterval, fDamagePerInterval, pDamagingWeapon, bFriendlyFire );
	if ( pBurn )
	{
		m_Burning.AddToTail( pBurn );
		SetNextThinkTime();
	}
}

void CASW_Burning::SetNextThinkTime()
{
	int next = -1;
	float next_time = 0;
	int c = m_Burning.Count();
	for ( int i = 0; i < c; i++ )
	{
		if ( next_time == 0 || m_Burning[i]->m_fNextBurnTime < next_time )
		{
			next_time = m_Burning[i]->m_fNextBurnTime;
			next = i;
		}
	}

	if ( next != -1 )
	{
		SetThink( &CASW_Burning::FireThink );
		SetNextThink( next_time );
		return;
	}
	SetThink( &CASW_Burning::FireThink );
	SetNextThink( gpGlobals->curtime + 5.0f );
}

void CASW_Burning::Extinguish( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	int c = m_Burning.Count();
	for ( int i = c - 1; i >= 0; i-- )
	{
		if ( m_Burning[i]->m_hEntity.Get() == pEntity )
		{
			delete m_Burning[i];
			m_Burning.Remove( i );
		}
	}
}

void CASW_Burning::ExtendBurning( CBaseEntity *pEntity, float fFireDuration, bool bFriendlyFire /*= false */ )
{
	if ( !pEntity )
		return;

	int c = m_Burning.Count();
	for ( int i = c - 1; i >= 0; i-- )
	{
		if ( m_Burning[i]->m_hEntity.Get() == pEntity )
		{
			m_Burning[i]->m_fDieTime = gpGlobals->curtime + fFireDuration;

			// can disable friendly fire protection but not enable it
			if ( m_Burning[i]->m_bFriendlyFire )
				m_Burning[i]->m_bFriendlyFire = bFriendlyFire;
		}
	}
}
