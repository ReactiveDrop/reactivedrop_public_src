#include "cbase.h"
#include "rd_crafting_defs.h"
#ifdef GAME_DLL
#include "asw_marine_hint.h"
#include "asw_spawn_manager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef RD_7A_DROPS
#ifdef GAME_DLL
ConVar rd_debug_material_spawns( "rd_debug_material_spawns", "0", FCVAR_CHEAT );
#endif

PRECACHE_REGISTER_BEGIN( GLOBAL, ReactiveDropCrafting )
	for ( int i = 1; i < NUM_RD_CRAFTING_MATERIAL_TYPES; i++ )
	{
		// precache crafting material models that might be found in a mission.
		if ( g_RD_Crafting_Material_Info[i].m_iRarity <= RD_CRAFTING_MATERIAL_RARITY_REGIONAL )
		{
			PRECACHE( MODEL, g_RD_Crafting_Material_Info[i].m_szModelName );
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
	{ "plastics", 7007, 7031, 4005, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/plastics.mdl" },
	{ "coolant", 7008, 7032, 4006, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/coolant.mdl" },
	{ "mini_crate", 7009, 7033, 4007, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/mini_crate.mdl" },
	{ "battery_pack", 7010, 7034, 4008, RD_CRAFTING_MATERIAL_RARITY_COMMON, "models/swarm/crafting/battery_pack.mdl" },

	// ultra-common / "Bulk"
	{ "loose_wires", 7011, 7035, 4009, RD_CRAFTING_MATERIAL_RARITY_ULTRA_COMMON, "models/swarm/crafting/loose_wires.mdl" },
	{ "carbon", 7012, 7036, 4010, RD_CRAFTING_MATERIAL_RARITY_ULTRA_COMMON, "models/swarm/crafting/carbon.mdl" },

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

#ifdef GAME_DLL
static void GenerateCraftingMaterialSpawnLocations( CUtlVector<Vector> &spawnLocations )
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
			NDebugOverlay::Text( vecHintOrigin, "No nearby walls", false, 120.0f );
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
#endif
