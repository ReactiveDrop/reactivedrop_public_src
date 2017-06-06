#include "cbase.h"
#include "asw_health_regen.h"
#include "asw_gamerules.h"
#include "asw_game_resource.h"
#include "asw_marine_resource.h"
#include "asw_marine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rm_health_regen_interval("rm_health_regen_interval", "5", FCVAR_NONE, "Interval between health regeneration", true, 0.01f, false, 0.0f);
ConVar rm_health_regen_amount("rm_health_regen_amount", "5", FCVAR_NONE, "Amount of health regenerating");
ConVar rm_health_decrease_amount("rm_health_decrease_amount", "7", FCVAR_NONE, "Amount of health decreasing during incapacitated");
extern ConVar rd_hp_regen;

LINK_ENTITY_TO_CLASS( asw_health_regen, CASW_Health_Regen );

CASW_Health_Regen::CASW_Health_Regen()
{
}

CASW_Health_Regen::~CASW_Health_Regen()
{
}

void CASW_Health_Regen::Spawn()
{
	BaseClass::Spawn();
	SetNextThink( gpGlobals->curtime ); // Think now
}

void CASW_Health_Regen::Think()
{
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		if ( !pMR )
		{
			continue;
		}

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( !pMarine || !pMarine->IsAlive() )
		{
			continue;
		}

		if ( pMarine->m_bKnockedOut )
		{
			if ( pMarine->GetHealth() == 1 )
			{
				pMarine->SetKnockedOut( false );
				pMarine->SetHealth( 1 );
				pMarine->m_bPreventKnockedOut = true;
				pMarine->TakeDamage( CTakeDamageInfo( this, this, 1000.0f, DMG_NEVERGIB | DMG_DIRECT | DMG_PREVENT_PHYSICS_FORCE ) );
			}
			else
			{
				pMarine->SetHealth( MAX( pMarine->GetHealth() - rm_health_decrease_amount.GetInt(), 1 ) );
			}
		}
		else if ( rd_hp_regen.GetBool() && pMarine->GetHealth() < pMarine->GetMaxHealth() )
		{
			pMarine->SetHealth( MIN( pMarine->GetHealth() + rm_health_regen_amount.GetInt(), pMarine->GetMaxHealth() ) );
		}
	}

	SetNextThink( gpGlobals->curtime + rm_health_regen_interval.GetFloat() );
}
