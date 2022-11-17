#include "cbase.h"
#include "asw_weapon_normal_armor.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "asw_marine_resource.h"
#endif
#include "asw_gamerules.h"
#include "asw_marine_skills.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Normal_Armor, DT_ASW_Weapon_Normal_Armor )

BEGIN_NETWORK_TABLE( CASW_Weapon_Normal_Armor, DT_ASW_Weapon_Normal_Armor )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iLayersMissing ) ),
#else
	SendPropInt( SENDINFO( m_iLayersMissing ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Normal_Armor )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_normal_armor, CASW_Weapon_Normal_Armor );
PRECACHE_WEAPON_REGISTER( asw_weapon_normal_armor );

ConVar rd_marine_passive_armor_layers_enabled( "rd_marine_passive_armor_layers_enabled", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "heavy armor can restore protection level" );
ConVar rd_marine_passive_armor_layers_amount( "rd_marine_passive_armor_layers_amount", "5", FCVAR_CHEAT | FCVAR_REPLICATED, "heavy armor has this many exta layers of nanites" );
ConVar asw_marine_passive_armor_scale( "asw_marine_passive_armor_scale", "0.85", FCVAR_CHEAT | FCVAR_REPLICATED, "heavy armor will scale damage taken by this much" );
ConVar rd_marine_passive_armor_layer_protection_value( "rd_marine_passive_armor_layer_protection_value", "0.04", FCVAR_CHEAT | FCVAR_REPLICATED, "proportion of damage one layer of heavy armor absorbs" );
ConVar rd_marine_passive_armor_layer_update_interval( "rd_marine_passive_armor_layer_update_interval", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "how much time it takes each subsequent layer of heavy armor to restore" );
ConVar rd_marine_passive_armor_layer_update_interval_initial( "rd_marine_passive_armor_layer_update_interval_initial", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "how much time it takes one layer of heavy armor to restore after damage" );


#ifndef CLIENT_DLL

#define LAYER_RESTORE_CONTEXT "LayerRestoreContext"

//---------------------------------------------------------
// Save/Restore (really?)
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Normal_Armor )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Normal_Armor::CASW_Weapon_Normal_Armor()
{
#ifndef CLIENT_DLL
	m_iLayersMissing = 0;
#endif
}


CASW_Weapon_Normal_Armor::~CASW_Weapon_Normal_Armor()
{

}

void CASW_Weapon_Normal_Armor::PrimaryAttack( void )
{
	// passive, do nothing
}

#ifndef CLIENT_DLL
void CASW_Weapon_Normal_Armor::LayerRestoreThink()
{
	if ( m_iLayersMissing > 0 )
	{
		m_iLayersMissing--;
		SetNextThink( gpGlobals->curtime + rd_marine_passive_armor_layer_update_interval.GetFloat(), LAYER_RESTORE_CONTEXT );
	}
}

float CASW_Weapon_Normal_Armor::GetDamageScaleFactor()
{
	float flScaleFactor = asw_marine_passive_armor_scale.GetFloat();

	if ( rd_marine_passive_armor_layers_enabled.GetBool() )
	{
		flScaleFactor -= ( rd_marine_passive_armor_layers_amount.GetFloat() - m_iLayersMissing ) * rd_marine_passive_armor_layer_protection_value.GetFloat();
	}

	return MAX( flScaleFactor, 0 );
}

void CASW_Weapon_Normal_Armor::LayerRemoveOnDamage()
{
	if ( rd_marine_passive_armor_layers_enabled.GetBool() )
	{
		if ( m_iLayersMissing < rd_marine_passive_armor_layers_amount.GetInt() )
		{
			m_iLayersMissing++;
		}

		SetContextThink( &CASW_Weapon_Normal_Armor::LayerRestoreThink, gpGlobals->curtime + rd_marine_passive_armor_layer_update_interval_initial.GetFloat(), LAYER_RESTORE_CONTEXT );
	}
}
#else
float CASW_Weapon_Normal_Armor::GetBatteryCharge()
{
	if ( !rd_marine_passive_armor_layers_enabled.GetBool() )
	{
		return -1.0f;
	}

	return 1.0f - m_iLayersMissing / rd_marine_passive_armor_layers_amount.GetFloat();
}
#endif
