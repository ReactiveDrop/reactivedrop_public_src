#include "cbase.h"
#include "rd_crafting_defs.h"
#ifdef GAME_DLL
#include "asw_marine_hint.h"
#include "asw_spawn_manager.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_marine_speech.h"
#include "asw_door_area.h"
#else
#include "c_asw_player.h"
#include "c_asw_inhabitable_npc.h"
#include "asw_gamerules.h"
#include "asw_input.h"
#include "vgui/ILocalize.h"
#include "vgui/IImage.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef RD_7A_DROPS
#ifdef GAME_DLL
ConVar rd_debug_material_spawns( "rd_debug_material_spawns", "0", FCVAR_CHEAT );
#else
ConVar glow_outline_color_crafting( "glow_outline_color_crafting", "204 102 0", FCVAR_NONE ); // #630 hex doubled
#endif

PRECACHE_REGISTER_BEGIN( GLOBAL, ReactiveDropCrafting )
	for ( int i = 1; i < NUM_RD_CRAFTING_MATERIAL_TYPES; i++ )
	{
		// precache crafting material models that might be found in a mission.
		if ( g_RD_Crafting_Material_Rarity_Info[g_RD_Crafting_Material_Info[i].m_iRarity].m_bCanFindInMission )
		{
			PRECACHE( MODEL, g_RD_Crafting_Material_Info[i].m_szModelName );
		}
	}
	for ( int i = 0; i < NUM_RD_CRAFTING_MATERIAL_RARITIES; i++ )
	{
		if ( g_RD_Crafting_Material_Rarity_Info[i].m_bCanFindInMission && g_RD_Crafting_Material_Rarity_Info[i].m_szPickupSound )
		{
			PRECACHE( GAMESOUND, g_RD_Crafting_Material_Rarity_Info[i].m_szPickupSound );
		}
	}
PRECACHE_REGISTER_END();

const RD_Crafting_Material_Info g_RD_Crafting_Material_Info[] =
{
	{}, // RD_CRAFTING_MATERIAL_NONE

	// common / "Industrial"
	{ "scrap_metal", 7004, 7028, 4002, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/scrap_metal.mdl" },
	{ "electrical_components", 7005, 7029, 4003, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/electrical_components.mdl" },
	{ "spare_pipe", 7006, 7030, 4004, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/spare_pipe.mdl" },
	{ "plastics", 7007, 7031, 4005, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/plastic_filament.mdl" },
	{ "coolant", 7008, 7032, 4006, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/coolant.mdl" },
	{ "mini_crate", 7009, 7033, 4007, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/mini_crate.mdl" },
	{ "battery_pack", 7010, 7034, 4008, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/battery_pack.mdl" },

	// ultra-common / "Bulk"
	{ "loose_wires", 7011, 7035, 4009, RD_CRAFTING_MATERIAL_RARITY_ULTRA_COMMON, "models/swarm/crafting/wires_loose.mdl" },
	{ "carbon", 7012, 7036, 4010, RD_CRAFTING_MATERIAL_RARITY_ULTRA_COMMON, "models/swarm/crafting/carbon_dust.mdl" },

	// uncommon / "Alien"
	{ "alien_chitin", 7013, 7037, 4011, RD_CRAFTING_MATERIAL_RARITY_UNCOMMON, "models/swarm/crafting/alien_chitin.mdl" },
	{ "biomass_sample", 7014, 7038, 4012, RD_CRAFTING_MATERIAL_RARITY_UNCOMMON, "models/swarm/crafting/biomass_sample.mdl" },
	{ "glowing_green_acid", 7015, 7039, 4013, RD_CRAFTING_MATERIAL_RARITY_UNCOMMON, "models/swarm/crafting/glowing_green_acid.mdl" },
	{ "claw_fragment", 7016, 7040, 4014, RD_CRAFTING_MATERIAL_RARITY_UNCOMMON, "models/swarm/crafting/claw_fragment.mdl" },

	// rare / "Tech"
	{ "memory_management_unit", 7017, 7041, 4015, RD_CRAFTING_MATERIAL_RARITY_RARE, "models/swarm/crafting/memory_management_unit.mdl" },
	{ "arithmetic_logic_unit", 7018, 7042, 4016, RD_CRAFTING_MATERIAL_RARITY_RARE, "models/swarm/crafting/arithmetic_logic_unit.mdl" },
	{ "data_storage_medium", 7019, 7043, 4017, RD_CRAFTING_MATERIAL_RARITY_RARE, "models/swarm/crafting/data_storage_medium.mdl" },

	// regional / "Salvaged"
	{ "pile_of_red_sand", 7020, 7044, 4018, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/pile_of_red_sand.mdl" },
	{ "antlion_carapace", 7021, 7045, 4019, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/antlion_carapace.mdl" },
	{ "corrosive_fluid_sample", 7022, 7046, 4020, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/corrosive_fluid_sample.mdl" },
	{ "cooled_volcanic_rock", 7023, 7047, 4021, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/cooled_volcanic_rock.mdl" },
	{ "retrieved_documents", 7024, 7048, 4022, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/retrieved_documents.mdl" },
	{ "unopened_synup_cola", 7025, 7049, 4023, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/unopened_synup_cola.mdl" },
	{ "roll_of_vent_tape", 7026, 7050, 4024, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/roll_of_vent_tape.mdl" },
	{ "isotopes", 7027, 7051, 4025, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/isotopes.mdl" },
	{ "cryotic", 7071, 7072, 4026, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/cryotic.mdl" },
	{ "argon_canister", 7073, 7074, 4027, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/argon_canister.mdl" },
	{ "probability_drive", 7075, 7076, 4028, RD_CRAFTING_MATERIAL_RARITY_REGIONAL, "models/swarm/crafting/probability_drive.mdl" },
};

const RD_Crafting_Material_Rarity_Info g_RD_Crafting_Material_Rarity_Info[] =
{
	{ "RD_Crafting_Material_Found.Industrial", true },
	{ "RD_Crafting_Material_Found.Bulk", true },
	{ "RD_Crafting_Material_Found.Alien", true },
	{ "RD_Crafting_Material_Found.Tech", true },
	{ "RD_Crafting_Material_Found.Salvaged", true },
};

// hard-coded so we don't have to search the entire item ID space to figure out which items can appear in a box.
// gets verified on startup in debug builds.
const CUtlVector<RD_Crafting_Contains_Any_List> g_RD_Crafting_Contains_Any_Lists
{{
	{"set_1_strange_weapon", {{2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026}}},
	{"set_1_strange_equipment", {{3000, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013, 3014, 3015, 3016, 3017}}},
	{"set_1_strange_device", {{5000, 5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008}}},
}};

#ifdef GAME_DLL
class CRD_CraftingMaterialUseAreaEnumerator : public IPartitionEnumerator
{
public:
	IterationRetval_t EnumElement( IHandleEntity *pHandleEntity ) override
	{
		CBaseEntity *pEnt = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
		CASW_Use_Area *pUseArea = dynamic_cast< CASW_Use_Area * >( pEnt );
		if ( !pUseArea )
			return ITERATION_CONTINUE;

		// exclude door areas with no associated door (used for counting marines sometimes)
		if ( pUseArea->Classify() != CLASS_ASW_DOOR_AREA || pUseArea->m_hUseTarget.Get() )
		{
			m_hEntity = pEnt;
			return ITERATION_STOP;
		}

		return ITERATION_CONTINUE;
	}

	EHANDLE m_hEntity;
};

void GenerateCraftingMaterialSpawnLocations( CUtlVector<Vector> &spawnLocations )
{
	CASW_Marine_Hint_Manager *pMarineHintManager = MarineHintManager();
	if ( !pMarineHintManager )
	{
		DevWarning( "Could not generate material spawn locations: missing marine hint manager\n" );
		return;
	}

	if ( pMarineHintManager->GetHintCount() == 0 )
	{
		DevWarning( "Could not generate material spawn locations: no marine hints in level\n" );
		return;
	}

	constexpr float flMinDistanceFromStart = 1536.0f;
	constexpr float flMinSpacing = 768.0f;
	constexpr int nDirections = 8;
	constexpr float flMaxOffset = 128.0f;
	constexpr float flSweepDist = 20.0f;

	CUtlVector<int> HintOrder;
	HintOrder.SetCount( pMarineHintManager->GetHintCount() );
	FOR_EACH_VEC( HintOrder, i )
	{
		HintOrder[i] = i;
	}

	// shuffle hint order
	FOR_EACH_VEC_BACK( HintOrder, i )
	{
		if ( i )
		{
			int j = RandomInt( 0, i );
			V_swap( HintOrder[i], HintOrder[j] );
		}
	}

	string_t iszInfoPlayerStart = AllocPooledString( "info_player_start" );

	spawnLocations.Purge();
	FOR_EACH_VEC( HintOrder, iHintOrder )
	{
		if ( spawnLocations.Count() >= RD_MAX_CRAFTING_MATERIAL_SPAWN_LOCATIONS )
			break;

		int iHint = HintOrder[iHintOrder];
		// ignore hints that can move or hints that no longer exist
		if ( pMarineHintManager->GetHintFlags( iHint ) & ( HintData_t::HINT_DELETED | HintData_t::HINT_DYNAMIC ) )
			continue;

		Vector vecHintOrigin = pMarineHintManager->GetHintPosition( iHint );

		// ignore hints that are too close to marine spawn locations
		if ( CBaseEntity *pEnt = gEntList.FindEntityByClassnameNearestFast( iszInfoPlayerStart, vecHintOrigin, flMinDistanceFromStart ) )
		{
			if ( rd_debug_material_spawns.GetBool() )
			{
				NDebugOverlay::Line( vecHintOrigin, pEnt->GetAbsOrigin(), 255, 127, 127, false, 120.0f );
				NDebugOverlay::Text( vecHintOrigin, "Too close to marine spawn location", false, 120.0f );
			}

			continue;
		}
		// check for an active escape trigger (some maps use disabled escape triggers covering the entire playable space)
		if ( CTriggerMultiple *pEnt = ASWSpawnManager()->EscapeTriggerAtPoint( vecHintOrigin, false ) )
		{
			if ( rd_debug_material_spawns.GetBool() )
			{
				NDebugOverlay::Line( vecHintOrigin, pEnt->GetAbsOrigin(), 255, 127, 127, false, 120.0f );
				NDebugOverlay::Text( vecHintOrigin, "Within bounds of escape trigger", false, 120.0f );
			}

			continue;
		}
		// check for a USE area that conflicts with our spawn location
		CRD_CraftingMaterialUseAreaEnumerator useAreaEnumerator;
		partition->EnumerateElementsAtPoint( PARTITION_ENGINE_TRIGGER_EDICTS, vecHintOrigin, false, &useAreaEnumerator );
		if ( useAreaEnumerator.m_hEntity )
		{
			if ( rd_debug_material_spawns.GetBool() )
			{
				NDebugOverlay::Line( vecHintOrigin, useAreaEnumerator.m_hEntity->GetAbsOrigin(), 255, 127, 127, false, 120.0f );
				NDebugOverlay::Text( vecHintOrigin, "Within bounds of use trigger", false, 120.0f );
			}

			continue;
		}
		// another material spawn point
		bool bTooClose = false;
		FOR_EACH_VEC( spawnLocations, i )
		{
			if ( spawnLocations[i].DistToSqr( vecHintOrigin ) < Square( flMinSpacing ) )
			{
				bTooClose = true;

				if ( rd_debug_material_spawns.GetBool() )
				{
					NDebugOverlay::Line( vecHintOrigin, spawnLocations[i], 255, 127, 127, false, 120.0f );
					NDebugOverlay::Text( vecHintOrigin, "Too close to other material spawn", false, 120.0f );
				}

				break;
			}
		}
		if ( bTooClose )
			continue;

		trace_t tr;
		Vector vecBestPos = vecHintOrigin;
		float flBestFraction = 1.0f;

		for ( int dir = 0; dir < nDirections; dir++ )
		{
			float flSin, flCos;
			SinCos( dir * M_PI * 2.0f / nDirections, &flSin, &flCos );

			// marine hull with padded sides and trimmed top and bottom
			Vector vecOffsetOrigin = vecHintOrigin + Vector( flCos * flMaxOffset, flSin * flMaxOffset, 0.0f );
			UTIL_TraceHull( vecHintOrigin, vecOffsetOrigin, Vector( -16, -16, 16 ), Vector( 16, 16, 56 ), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );

			if ( tr.startsolid || tr.allsolid || tr.fraction >= 1.0f )
				continue;

			if ( flBestFraction > tr.fraction )
			{
				vecBestPos = tr.endpos;
				flBestFraction = tr.fraction;
			}
		}

		if ( flBestFraction >= 1.0f )
		{
			if ( rd_debug_material_spawns.GetBool() )
			{
				NDebugOverlay::Text( vecHintOrigin, "No nearby walls", false, 120.0f );
			}
			continue;
		}

		// now we sweep towards the place we found, making sure we have relatively level floor the entire way
		int nIterations = Ceil2Int( flBestFraction * flMaxOffset / flSweepDist );
		bool bSweepFailed = false;
		for ( int i = 0; i < nIterations; i++ )
		{
			Vector vecStepStart = Lerp( ( i + 0.5f ) / nIterations, vecHintOrigin, vecBestPos ) + Vector( 0.0f, 0.0f, 48.0f );
			UTIL_TraceHull( vecStepStart, vecStepStart - Vector( 0.0f, 0.0f, 64.0f ), Vector( -12.0f, -12.0f, 0.0f ), Vector( 12.0f, 12.0f, 24.0f ), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );

			if ( tr.startsolid || tr.allsolid || tr.fraction >= 1.0f )
			{
				bSweepFailed = true;

				if ( rd_debug_material_spawns.GetBool() )
				{
					NDebugOverlay::Text( vecHintOrigin, "Floor check failed", false, 120.0f );
				}

				break;
			}
		}

		if ( bSweepFailed )
			continue;

		if ( rd_debug_material_spawns.GetBool() )
		{
			NDebugOverlay::Box( tr.endpos, Vector( -12, -12, 0 ), Vector( 12, 12, 24 ), 255, 255, 255, false, 120.0f );
		}

		spawnLocations.AddToTail( tr.endpos );
	}
}

CON_COMMAND( rd_debug_material_spawn_locations, "Generate and display crafting material spawn locations (requires developer and listen server)." )
{
	// doesn't work for non-listen server host anyway, so don't waste time generating positions
	if ( engine->IsDedicatedServer() || !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	CUtlVector<Vector> spawnLocations;

	GenerateCraftingMaterialSpawnLocations( spawnLocations );

	Msg( "Generated %d/%d locations:\n", spawnLocations.Count(), RD_MAX_CRAFTING_MATERIAL_SPAWN_LOCATIONS );
	FOR_EACH_VEC( spawnLocations, i )
	{
		Msg( "(%f, %f, %f)\n", VectorExpand( spawnLocations[i] ) );
	}
}
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( RD_Crafting_Material_Pickup, DT_RD_Crafting_Material_Pickup )

BEGIN_NETWORK_TABLE( CRD_Crafting_Material_Pickup, DT_RD_Crafting_Material_Pickup )
#ifdef CLIENT_DLL
	RecvPropIntWithMinusOneFlag( RECVINFO( m_iLocation ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_MaterialAtLocation ), RecvPropInt( RECVINFO( m_MaterialAtLocation[0] ) ) ),
	RecvPropBool( RECVINFO( m_bAnyoneFound ) ),
#else
	SendPropIntWithMinusOneFlag( SENDINFO( m_iLocation ), NumBitsForCount( RD_MAX_CRAFTING_MATERIAL_SPAWN_LOCATIONS + 1 ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_MaterialAtLocation ), SendPropInt( SENDINFO_ARRAY( m_MaterialAtLocation ), NumBitsForCount( NUM_RD_CRAFTING_MATERIAL_TYPES ), SPROP_UNSIGNED ) ),
	SendPropBool( SENDINFO( m_bAnyoneFound ) ),
#endif
END_NETWORK_TABLE()

Class_T CRD_Crafting_Material_Pickup::Classify() { return ( Class_T )CLASS_RD_CRAFTING_MATERIAL_PICKUP; }

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( rd_crafting_material_pickup, CRD_Crafting_Material_Pickup );

CRD_Crafting_Material_Pickup::CRD_Crafting_Material_Pickup()
{
	m_iLocation = -1;
	FOR_EACH_VEC( m_MaterialAtLocation, i )
	{
		m_MaterialAtLocation.Set( i, RD_CRAFTING_MATERIAL_NONE );
	}
	m_bAnyoneFound = false;

	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
}

void CRD_Crafting_Material_Pickup::Spawn()
{
	BaseClass::Spawn();

	if ( m_iLocation == -1 )
	{
		Warning( "%s not initialized properly; deleting\n", GetDebugName() );
		UTIL_Remove( this );
	}
}

int CRD_Crafting_Material_Pickup::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

void CRD_Crafting_Material_Pickup::ActivateUseIcon( CASW_Inhabitable_NPC *pUser, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( !pUser || !pUser->IsInhabited() )
		return;

	CASW_Player *pPlayer = pUser->GetCommander();
	if ( !pPlayer )
		return;

	if ( m_MaterialAtLocation[pPlayer->entindex() - 1] == RD_CRAFTING_MATERIAL_NONE )
		return;

	// only use longer supplies chatter line if we're the first one to find this location
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pUser );
	if ( pMarine )
	{
		pMarine->GetMarineSpeech()->Chatter( m_bAnyoneFound ? CHATTER_USE : CHATTER_SUPPLIES );
		pMarine->DoAnimationEventToAll( PLAYERANIMEVENT_PICKUP );
	}

	m_bAnyoneFound = true;
	m_MaterialAtLocation.Set( pPlayer->entindex() - 1, RD_CRAFTING_MATERIAL_NONE );
}
#else
CRD_Crafting_Material_Pickup::CRD_Crafting_Material_Pickup() :
	m_GlowObject{ this, glow_outline_color_crafting.GetColorAsVector(), 0.8f, true, true }
{
	m_iLastMaterialType = RD_CRAFTING_MATERIAL_NONE;
}

void CRD_Crafting_Material_Pickup::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
	else
	{
		CheckMaterialPickup();
	}
}

void CRD_Crafting_Material_Pickup::CheckMaterialPickup()
{
	// we picked up this material if:
	// - we are currently controlling a character
	HACK_GETLOCALPLAYER_GUARD( "picking up a crafting material" );
	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pLocalPlayer )
		return;

	C_ASW_Inhabitable_NPC *pNPC = pLocalPlayer->GetNPC();
	if ( !pNPC )
		return;

	// - m_iLastMaterialType is set from the previous frame
	if ( m_iLastMaterialType == RD_CRAFTING_MATERIAL_NONE )
		return;

	// - after the data update, our slot in m_MaterialAtLocation is empty
	if ( m_MaterialAtLocation[pLocalPlayer->entindex() - 1] != RD_CRAFTING_MATERIAL_NONE )
		return;

	// - our character is close enough to the pickup entity to interact with it (with a little bit of padding)
	if ( pNPC->GetAbsOrigin().DistTo( GetAbsOrigin() ) > ASW_MARINE_USE_RADIUS * 1.5f )
		return;

	// - we have a token and an exchange recipe prepared for that location and we haven't used it up yet
	ReactiveDropInventory::PickUpCraftingMaterialAtLocation( m_iLocation, m_iLastMaterialType );
}

void CRD_Crafting_Material_Pickup::ClientThink()
{
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	C_ASW_Inhabitable_NPC *pNPC = pPlayer ? pPlayer->GetViewNPC() : NULL;
	if ( pNPC && pNPC->IsInhabited() )
	{
		pPlayer = pNPC->GetCommander();
	}

	int iCurrentPlayerIndex = pPlayer ? pPlayer->entindex() : 0;
	RD_Crafting_Material_t iCurrentMaterial = iCurrentPlayerIndex ? m_MaterialAtLocation[iCurrentPlayerIndex - 1] : RD_CRAFTING_MATERIAL_NONE;
	if ( m_iLastMaterialType != iCurrentMaterial )
	{
		const char *szModelName = g_RD_Crafting_Material_Info[iCurrentMaterial].m_szModelName;
		if ( szModelName )
		{
			CAlienSwarm *pGameRules = ASWGameRules();

			VPhysicsDestroyObject();

			SetModel( szModelName );

			// init client-side collision so we can determine whether the cursor is over this item for use priority
			SetSolid( SOLID_VPHYSICS );
			AddSolidFlags( FSOLID_TRIGGER );
			VPhysicsInitStatic();

			if ( iCurrentMaterial == RD_CRAFTING_MATERIAL_LOOSE_WIRES || iCurrentMaterial == RD_CRAFTING_MATERIAL_UNOPENED_SYNUP_COLA )
			{
				int iRandomSeed = pGameRules ? pGameRules->m_iCosmeticRandomSeed : 0;
				SetSkin( unsigned( m_iLocation + iRandomSeed ) % 4u );
			}
			else if ( iCurrentMaterial == RD_CRAFTING_MATERIAL_BIOMASS_SAMPLE )
			{
				SetSkin( pGameRules && ( pGameRules->m_iCosmeticFlags & CAlienSwarm::COSMETIC_RED_BIOMASS ) ? 1 : 0 );
			}
			else
			{
				SetSkin( 0 );
			}
			
			RemoveEffects( EF_NODRAW );
		}
		else
		{
			AddEffects( EF_NODRAW );
		}

		m_iLastMaterialType = iCurrentMaterial;
	}

	if ( !IsEffectActive( EF_NODRAW ) )
	{
		bool bShouldGlow = false;
		float flDistanceToMarineSqr = 0.0f;
		float flWithinDistSqr = ( ASW_MARINE_USE_RADIUS * 1.5f ) * ( ASW_MARINE_USE_RADIUS * 1.5f );

		C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
		if ( pLocalPlayer && pLocalPlayer->GetViewNPC() && ASWInput()->GetUseGlowEntity() != this )
		{
			flDistanceToMarineSqr = ( pLocalPlayer->GetViewNPC()->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr();
			if ( flDistanceToMarineSqr < flWithinDistSqr )
				bShouldGlow = true;
		}

		m_GlowObject.SetRenderFlags( bShouldGlow, bShouldGlow );

		if ( m_GlowObject.IsRendering() )
		{
			m_GlowObject.SetAlpha( MIN( 0.7f, ( 1.0f - ( flDistanceToMarineSqr / flWithinDistSqr ) ) * 1.0f ) );
		}
	}
	else
	{
		m_GlowObject.SetRenderFlags( false, false );
	}
}

bool CRD_Crafting_Material_Pickup::GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser )
{
	if ( m_iLastMaterialType == RD_CRAFTING_MATERIAL_NONE )
		return false;

	wchar_t wszMaterialName[128]{};
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( g_RD_Crafting_Material_Info[m_iLastMaterialType].m_iItemDef );
	if ( pDef )
		V_UTF8ToUnicode( pDef->Name.Get(), wszMaterialName, sizeof( wszMaterialName ) );
	g_pVGuiLocalize->ConstructString( action.wszText, sizeof( action.wszText ), g_pVGuiLocalize->Find( "#rd_crafting_pickup_prompt" ), 1, wszMaterialName );

	if ( pDef && pDef->Icon && pDef->Icon->GetNumFrames() )
		action.iUseIconTexture = pDef->Icon->GetID();

	action.UseTarget = this;
	action.bShowUseKey = true;
	action.vecUseHighlightColor = glow_outline_color_crafting.GetColorAsVector();

	return true;
}
#endif

bool CRD_Crafting_Material_Pickup::IsUsable( CBaseEntity *pUser )
{
#ifdef CLIENT_DLL
	if ( m_iLastMaterialType == RD_CRAFTING_MATERIAL_NONE )
		return false;
#endif
	return ( pUser && pUser->GetAbsOrigin().DistTo( GetAbsOrigin() ) < ASW_MARINE_USE_RADIUS );	// near enough?
}
#endif

#ifdef DBGFLAG_ASSERT
static int __cdecl CompareItemIDs( const SteamItemDef_t *a, const SteamItemDef_t *b )
{
	if ( *a > *b )
		return 1;
	if ( *a < *b )
		return -1;
	return 0;
}

void CheckContainsAnyItemIDLists( const CUtlVector<SteamItemDef_t> &AllItemDefs )
{
	ISteamInventory *pInventory = SteamInventory();
#ifdef GAME_DLL
	if ( engine->IsDedicatedServer() )
		pInventory = SteamGameServerInventory();
#endif
	Assert( pInventory );
	if ( !pInventory )
		return;

	CUtlVector<CUtlVector<SteamItemDef_t>> ActualExchangeLists;
	ActualExchangeLists.SetCount( g_RD_Crafting_Contains_Any_Lists.Count() );

	char szExchange[4096];

	FOR_EACH_VEC( AllItemDefs, i )
	{
		uint32 nExchangeSize = sizeof( szExchange );
		if ( !pInventory->GetItemDefinitionProperty( AllItemDefs[i], "exchange", szExchange, &nExchangeSize ) )
			continue;

		// this code assumes that each item is in at most one contains_any list and isn't exchangable from anything else
		const char *szContainsAnyTag = StringAfterPrefixCaseSensitive( szExchange, "contains_any:" );
		if ( szContainsAnyTag )
		{
			bool bFound = false;
			FOR_EACH_VEC( g_RD_Crafting_Contains_Any_Lists, j )
			{
				if ( !V_strcmp( g_RD_Crafting_Contains_Any_Lists[j].m_szTag, szContainsAnyTag ) )
				{
					bFound = true;
					ActualExchangeLists[j].AddToTail( AllItemDefs[i] );
					break;
				}
			}

			Assert( bFound );
		}
		else
		{
			// for now, anything that isn't a contains_any either isn't exchangable or is exchangable for a specific item ID; check that it starts with a digit
			Assert( !szExchange[0] || V_isdigit( szExchange[0] ) );
		}
	}

	FOR_EACH_VEC( ActualExchangeLists, i )
	{
		ActualExchangeLists[i].Sort( CompareItemIDs );

		Assert( g_RD_Crafting_Contains_Any_Lists[i].m_ItemDefs.Count() == ActualExchangeLists[i].Count() );
		if ( g_RD_Crafting_Contains_Any_Lists[i].m_ItemDefs.Count() == ActualExchangeLists[i].Count() )
		{
			FOR_EACH_VEC( ActualExchangeLists[i], j )
			{
				Assert( g_RD_Crafting_Contains_Any_Lists[i].m_ItemDefs[j] == ActualExchangeLists[i][j] );
			}
		}
	}
}
#endif
