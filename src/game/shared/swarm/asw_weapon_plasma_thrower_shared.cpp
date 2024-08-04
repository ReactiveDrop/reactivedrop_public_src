#include "cbase.h"
#include "asw_weapon_plasma_thrower_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
ConVar rd_plasma_thrower_airblast_push( "rd_plasma_thrower_airblast_push", "10000", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of force (mass times acceleration) that the airblast applies to each target" );
ConVar rd_plasma_thrower_airblast_push_ally( "rd_plasma_thrower_airblast_push_ally", "1000", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of force (mass times acceleration) that the airblast applies to each allied target" );
ConVar rd_plasma_thrower_airblast_dot( "rd_plasma_thrower_airlast_dot", "0.866025403784", FCVAR_CHEAT | FCVAR_REPLICATED, "Minimum dot product for an object's position if that object gets hit.\n1.0 would be a zero degree arc, and 0.0 would be a 180 degree (90 to both sides) arc.\nDefault value is about 30 degrees to each side (60 degrees total).", true, -1, true, 1 );
ConVar rd_plasma_thrower_airblast_range( "rd_plasma_thrower_airblast_range", "400", FCVAR_CHEAT | FCVAR_REPLICATED, "Distance from the center of the marine that the airblast can affect targets.", true, 0, false, 0 );
ConVar rd_plasma_thrower_airblast_falloff( "rd_plasma_thrower_airblast_falloff", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED, "Adjustment for airblast force near the maximum distance for the airblast. 0 would mean the airblast doesn't work. 0.5 means the airblast falls off linearly.", true, 0, true, 1 );
ConVar rd_plasma_thrower_airblast_ammo( "rd_plasma_thrower_airblast_ammo", "20", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of ammo required for plasma thrower alt fire", true, 0, false, 0 );
ConVar rd_plasma_thrower_cooldown_primary( "rd_plasma_thrower_cooldown_primary", "1.0", FCVAR_CHEAT | FCVAR_REPLICATED, "How long (in seconds) the plasma thrower must cool down after stopping firing before it can fire again.", true, 0, false, 0 );
ConVar rd_plasma_thrower_cooldown_secondary( "rd_plasma_thrower_cooldown_secondary", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED, "How long (in seconds) the plasma thrower must wait after an airblast to fire again (overrides primary cooldown).", true, 0, false, 0 );
ConVar rd_plasma_thrower_cooldown_airblast( "rd_plasma_thrower_cooldown_airblast", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "How long (in seconds) the plasma thrower must wait after an airblast to perform another airblast.", true, 0, false, 0 );

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Plasma_Thrower, DT_ASW_Weapon_Plasma_Thrower );

BEGIN_NETWORK_TABLE( CASW_Weapon_Plasma_Thrower, DT_ASW_Weapon_Plasma_Thrower )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Plasma_Thrower )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_plasma_thrower, CASW_Weapon_Plasma_Thrower );
PRECACHE_WEAPON_REGISTER( asw_weapon_plasma_thrower );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Plasma_Thrower )
END_DATADESC()
#endif

// TODO: m_iPlasmaThrowerExtinguishMarine
#endif
