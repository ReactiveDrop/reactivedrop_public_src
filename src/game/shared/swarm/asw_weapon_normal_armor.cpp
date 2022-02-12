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
#else
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Normal_Armor )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_normal_armor, CASW_Weapon_Normal_Armor );
PRECACHE_WEAPON_REGISTER( asw_weapon_normal_armor );

#ifndef CLIENT_DLL

ConVar asw_marine_passive_armor_scale( "asw_marine_passive_armor_scale", "0.85", FCVAR_CHEAT, "heavy armor will scale damage taken by this much" );
ConVar rd_marine_passive_armor_layers_amount("rd_marine_passive_armor_layers_amount", "10", FCVAR_CHEAT, "heavy armor has this many exta layers of nanites");
ConVar rd_marine_passive_armor_layers_enabled("rd_marine_passive_armor_layers_enabled", "1", FCVAR_CHEAT, "heavy armor can restore protection level");
ConVar rd_marine_passive_armor_layer_protection_value("rd_marine_passive_armor_layer_protection_value", "2", FCVAR_CHEAT, "how much damage one layer of heavy armor adsorbs in percentage of total damage");
ConVar rd_marine_passive_armor_layer_update_interval("rd_marine_passive_armor_layer_update_interval", "2", FCVAR_CHEAT, "how much time takes one layer of heavy armor to restore");

//---------------------------------------------------------
// Save/Restore (really?)
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Normal_Armor )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Normal_Armor::CASW_Weapon_Normal_Armor()
{
#ifndef CLIENT_DLL
	m_flDamageScaleFactor = asw_marine_passive_armor_scale.GetFloat();
	m_iLayers = rd_marine_passive_armor_layers_amount.GetInt();
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
	if (m_iLayersMissing > 0)
	{
		m_iLayersMissing--;
		SetNextThink(gpGlobals->curtime + rd_marine_passive_armor_layer_update_interval.GetFloat());
		//Msg("Armor layer has been restored\n");
	}
}

float CASW_Weapon_Normal_Armor::GetDamageScaleFactor()
{
	if ( rd_marine_passive_armor_layers_enabled.GetBool() )
	{
		m_flDamageScaleFactor = asw_marine_passive_armor_scale.GetFloat() - (m_iLayers - m_iLayersMissing) * (rd_marine_passive_armor_layer_protection_value.GetFloat() / 100.0f);

		if (m_flDamageScaleFactor < 0)
			m_flDamageScaleFactor = 0;
	}
	else
	{
		m_flDamageScaleFactor = asw_marine_passive_armor_scale.GetFloat();
	}
	return m_flDamageScaleFactor;
}

void CASW_Weapon_Normal_Armor::LayerRemoveOnDamage()
{
	if (rd_marine_passive_armor_layers_enabled.GetBool())
	{
		if (m_iLayersMissing < m_iLayers)
		{
			m_iLayersMissing++;
			//Msg("Armor layer has been removed\n");
		}

		SetThink(NULL);
		SetThink(&CASW_Weapon_Normal_Armor::LayerRestoreThink);
		SetNextThink(gpGlobals->curtime + rd_marine_passive_armor_layer_update_interval.GetFloat());
	}
}
#endif

/*
void CASW_Weapon_Normal_Armor::Precache()
{
	BaseClass::Precache();
}
*/
