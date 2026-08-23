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
	{ "#rd_crafting_material_rarity_industrial", "RD_Crafting_Material_Found.Industrial", true, true },
	{ "#rd_crafting_material_rarity_bulk", "RD_Crafting_Material_Found.Bulk", true, true },
	{ "#rd_crafting_material_rarity_alien", "RD_Crafting_Material_Found.Alien", true, true },
	{ "#rd_crafting_material_rarity_tech", "RD_Crafting_Material_Found.Tech", true, true },
	{ "#rd_crafting_material_rarity_salvaged", "RD_Crafting_Material_Found.Salvaged", true, true },
};

// hard-coded so we don't have to search the entire item ID space to figure out which items can appear in a box.
// gets verified on startup in debug builds.
const CUtlVector<RD_Crafting_Contains_Any_List> g_RD_Crafting_Contains_Any_Lists
{{
	{ "set_1_strange_weapon", {{2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026}} },
	{ "set_1_strange_equipment", {{3000, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013, 3014, 3015, 3016, 3017}} },
	{ "set_1_strange_device", {{5000, 5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008}} },
}};

const CUtlVector<RD_Crafting_Recipe> g_RD_Crafting_Recipes
{{
	{ "#rd_crafting_recipe_hoiaf_create_stack_participant", "#rd_crafting_recipe_hoiaf_stack_flavor", "#rd_crafting_recipe_hoiaf_stack_warning", {{
		{48, {{{{{1, 30}}}, {{{1, 30}}}}}},
	}}, {{47}}, {} },
	{ "#rd_crafting_recipe_hoiaf_add_to_stack_participant", "#rd_crafting_recipe_hoiaf_stack_flavor", "#rd_crafting_recipe_hoiaf_stack_warning", {{
		{48, {{{{{1, 30}}}, {{{47}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}},
	}}, {}, {{47}} },
	{ "#rd_crafting_recipe_hoiaf_create_stack_elite", "#rd_crafting_recipe_hoiaf_stack_flavor", "#rd_crafting_recipe_hoiaf_stack_warning", {{
		{50, {{{{{2, 31}}}, {{{2, 31}}}}}}, // green+green
		{58, {{{{{2, 31}}}, {{{4, 32}}}}}}, // green+red
		{60, {{{{{2, 31}}}, {{{5, 33}}}}}}, // green+yellow
		{62, {{{{{2, 31}}}, {{{6, 34}}}}}}, // green+blue
		{58, {{{{{4, 32}}}, {{{2, 31}}}}}}, // red+green
		{52, {{{{{4, 32}}}, {{{4, 32}}}}}}, // red+red
		{64, {{{{{4, 32}}}, {{{5, 33}}}}}}, // red+yellow
		{66, {{{{{4, 32}}}, {{{6, 34}}}}}}, // red+blue
		{60, {{{{{5, 33}}}, {{{2, 31}}}}}}, // yellow+green
		{64, {{{{{5, 33}}}, {{{4, 32}}}}}}, // yellow+red
		{54, {{{{{5, 33}}}, {{{5, 33}}}}}}, // yellow+yellow
		{68, {{{{{5, 33}}}, {{{6, 34}}}}}}, // yellow+blue
		{62, {{{{{6, 34}}}, {{{2, 31}}}}}}, // blue+green
		{66, {{{{{6, 34}}}, {{{4, 32}}}}}}, // blue+red
		{68, {{{{{6, 34}}}, {{{5, 33}}}}}}, // blue+yellow
		{56, {{{{{6, 34}}}, {{{6, 34}}}}}}, // blue+blue
	}}, {{49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77}}, {} },
	{ "#rd_crafting_recipe_hoiaf_add_to_stack_elite", "#rd_crafting_recipe_hoiaf_stack_flavor", "#rd_crafting_recipe_hoiaf_stack_warning", {{
		{50, {{{{{2, 31}}}, {{{49}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+green
		{58, {{{{{2, 31}}}, {{{51, 57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+red
		{60, {{{{{2, 31}}}, {{{53, 59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+yellow
		{62, {{{{{2, 31}}}, {{{55, 61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+blue
		{58, {{{{{4, 32}}}, {{{49, 57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+green
		{52, {{{{{4, 32}}}, {{{51}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+red
		{64, {{{{{4, 32}}}, {{{53, 63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+yellow
		{66, {{{{{4, 32}}}, {{{55, 65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+blue
		{60, {{{{{5, 33}}}, {{{49, 59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+green
		{64, {{{{{5, 33}}}, {{{51, 63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+red
		{54, {{{{{5, 33}}}, {{{53}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+yellow
		{68, {{{{{5, 33}}}, {{{55, 67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+blue
		{62, {{{{{6, 34}}}, {{{49, 61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+green
		{66, {{{{{6, 34}}}, {{{51, 65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+red
		{68, {{{{{6, 34}}}, {{{53, 67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+yellow
		{56, {{{{{6, 34}}}, {{{55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+blue
		{70, {{{{{2, 31}}}, {{{63, 69}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+red/yellow
		{72, {{{{{2, 31}}}, {{{65, 71}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+red/blue
		{74, {{{{{2, 31}}}, {{{67, 73}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+yellow/blue
		{70, {{{{{4, 32}}}, {{{59, 69}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+green/yellow
		{72, {{{{{4, 32}}}, {{{61, 71}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+green/blue
		{76, {{{{{4, 32}}}, {{{67, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+yellow/blue
		{70, {{{{{5, 33}}}, {{{57, 69}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+green/red
		{74, {{{{{5, 33}}}, {{{61, 73}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+green/blue
		{76, {{{{{5, 33}}}, {{{65, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+red/blue
		{72, {{{{{6, 34}}}, {{{57, 71}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+green/red
		{74, {{{{{6, 34}}}, {{{59, 73}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+green/yellow
		{76, {{{{{6, 34}}}, {{{63, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+red/yellow
		{78, {{{{{2, 31}}}, {{{75, 77}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green+red/yellow/blue
		{78, {{{{{4, 32}}}, {{{73, 77}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red+green/yellow/blue
		{78, {{{{{5, 33}}}, {{{71, 77}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow+green/red/blue
		{78, {{{{{6, 34}}}, {{{69, 77}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue+green/red/yellow
	}}, {}, {{49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77}} },
	{ "#rd_crafting_recipe_hoiaf_create_stack_top20", "#rd_crafting_recipe_hoiaf_stack_flavor", "#rd_crafting_recipe_hoiaf_stack_warning", {{
		{80, {{{{{35}}}, {{{35}}}}}}, // emerald+emerald
		{88, {{{{{35}}}, {{{36}}}}}}, // emerald+ruby
		{90, {{{{{35}}}, {{{37}}}}}}, // emerald+topaz
		{92, {{{{{35}}}, {{{38}}}}}}, // emerald+sapphire
		{88, {{{{{36}}}, {{{35}}}}}}, // ruby+emerald
		{82, {{{{{36}}}, {{{36}}}}}}, // ruby+ruby
		{94, {{{{{36}}}, {{{37}}}}}}, // ruby+topaz
		{96, {{{{{36}}}, {{{38}}}}}}, // ruby+sapphire
		{90, {{{{{37}}}, {{{35}}}}}}, // topaz+emerald
		{94, {{{{{37}}}, {{{36}}}}}}, // topaz+ruby
		{84, {{{{{37}}}, {{{37}}}}}}, // topaz+topaz
		{98, {{{{{37}}}, {{{38}}}}}}, // topaz+sapphire
		{92, {{{{{38}}}, {{{35}}}}}}, // sapphire+emerald
		{96, {{{{{38}}}, {{{36}}}}}}, // sapphire+ruby
		{98, {{{{{38}}}, {{{37}}}}}}, // sapphire+topaz
		{86, {{{{{38}}}, {{{38}}}}}}, // sapphire+sapphire
		{108, {{{{{15}}}, {{{15, 35, 36, 37, 38}}}}}}, // original+...
		{108, {{{{{35, 36, 37, 38}}}, {{{15}}}}}}, // ...+original
	}}, {{79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107}}, {} },
	{ "#rd_crafting_recipe_hoiaf_add_to_stack_top20", "#rd_crafting_recipe_hoiaf_stack_flavor", "#rd_crafting_recipe_hoiaf_stack_warning", {{
		{80, {{{{{35}}}, {{{79}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+emerald
		{88, {{{{{35}}}, {{{81, 87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+ruby
		{90, {{{{{35}}}, {{{83, 89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+topaz
		{92, {{{{{35}}}, {{{85, 91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+sapphire
		{88, {{{{{36}}}, {{{79, 87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+emerald
		{82, {{{{{36}}}, {{{81}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+ruby
		{94, {{{{{36}}}, {{{83, 93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+topaz
		{96, {{{{{36}}}, {{{85, 95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+sapphire
		{90, {{{{{37}}}, {{{79, 89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+emerald
		{94, {{{{{37}}}, {{{81, 93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+ruby
		{84, {{{{{37}}}, {{{83}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+topaz
		{98, {{{{{37}}}, {{{85, 97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+sapphire
		{92, {{{{{38}}}, {{{79, 91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+emerald
		{96, {{{{{38}}}, {{{81, 95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+ruby
		{98, {{{{{38}}}, {{{83, 97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+topaz
		{86, {{{{{38}}}, {{{85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+sapphire
		{100, {{{{{35}}}, {{{93, 99}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+ruby/topaz
		{102, {{{{{35}}}, {{{95, 101}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+ruby/sapphire
		{104, {{{{{35}}}, {{{97, 103}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+topaz/sapphire
		{100, {{{{{36}}}, {{{89, 99}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+emerald/topaz
		{102, {{{{{36}}}, {{{91, 101}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+emerald/sapphire
		{106, {{{{{36}}}, {{{97, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+topaz/sapphire
		{100, {{{{{37}}}, {{{87, 99}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+emerald/ruby
		{104, {{{{{37}}}, {{{91, 103}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+emerald/sapphire
		{106, {{{{{37}}}, {{{95, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+ruby/sapphire
		{102, {{{{{38}}}, {{{87, 101}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+emerald/ruby
		{104, {{{{{38}}}, {{{89, 103}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+emerald/topaz
		{106, {{{{{38}}}, {{{93, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+ruby/topaz
		{108, {{{{{35}}}, {{{105, 107}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald+ruby/topaz/sapphire
		{108, {{{{{36}}}, {{{103, 107}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby+emerald/topaz/sapphire
		{108, {{{{{37}}}, {{{101, 107}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz+emerald/ruby/sapphire
		{108, {{{{{38}}}, {{{99, 107}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire+emerald/ruby/topaz
		{108, {{{{{15}}}, {{{79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // original+...
	}}, {}, {{79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107}} },
	{ "#rd_crafting_recipe_refinement_wires", "#rd_crafting_recipe_refinement_wires_flavor", nullptr, {{
		{4031, {{{{{4009}}, RD_CRAFTING_RECIPE_AUTO_SELECT, 300}}}},
	}}, {}, {}, RD_CRAFTING_RECIPE_QUICK_CONFIRM },
	{ "#rd_crafting_recipe_refinement_carbon", "#rd_crafting_recipe_refinement_carbon_flavor", nullptr, {{
		{4032, {{{{{4010}}, RD_CRAFTING_RECIPE_AUTO_SELECT, 2500}}}},
	}}, {}, {}, RD_CRAFTING_RECIPE_QUICK_CONFIRM },
}};

const CUtlVector<RD_Crafting_Recipe_Variant> g_RD_Crafting_Recipes_Auto
{{
	{58, {{{{{49}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{51}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green + red = green/red
	{58, {{{{{49, 51}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green or red + green/red = green/red
	{60, {{{{{49}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{53}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green + yellow = green/yellow
	{60, {{{{{49, 53}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green or yellow + green/yellow = green/yellow
	{62, {{{{{49}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green + blue = green/blue
	{62, {{{{{49, 55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green or blue + green/blue = green/blue
	{64, {{{{{51}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{53}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red + yellow = red/yellow
	{64, {{{{{51, 53}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red or yellow + red/yellow = red/yellow
	{66, {{{{{51}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red + blue = red/blue
	{66, {{{{{51, 55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red or blue + red/blue = red/blue
	{68, {{{{{53}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow + blue = yellow/blue
	{68, {{{{{53, 55}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow or blue + yellow/blue = yellow/blue
	{70, {{{{{53, 59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow or green/yellow + green/red = green/red/yellow
	{70, {{{{{51, 63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red or red/yellow + green/yellow = green/red/yellow
	{70, {{{{{49, 57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green or green/red + red/yellow = green/red/yellow
	{70, {{{{{49, 51, 53, 57, 59, 63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{69}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + green/red/yellow = green/red/yellow
	{72, {{{{{55, 61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue or green/blue + green/red = green/red/blue
	{72, {{{{{51, 65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red or red/blue + green/blue = green/red/blue
	{72, {{{{{49, 57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green or green/red + red/blue = green/red/blue
	{72, {{{{{49, 51, 55, 57, 61, 65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{71}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + green/red/blue = green/red/blue
	{74, {{{{{55, 61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue or green/blue + green/yellow = green/yellow/blue
	{74, {{{{{53, 67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow or yellow/blue + green/blue = green/yellow/blue
	{74, {{{{{49, 59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green or green/yellow + yellow/blue = green/yellow/blue
	{74, {{{{{49, 53, 55, 59, 61, 67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{73}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + green/yellow/blue = green/yellow/blue
	{76, {{{{{55, 65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // blue or red/blue + red/yellow = red/yellow/blue
	{76, {{{{{53, 67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // yellow or yellow/blue + red/blue = red/yellow/blue
	{76, {{{{{51, 63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // red or red/yellow + yellow/blue = red/yellow/blue
	{76, {{{{{51, 53, 55, 63, 65, 67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + red/yellow/blue = red/yellow/blue
	{78, {{{{{55, 61, 65, 67, 71, 73, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{69}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../blue + green/red/yellow = green/red/yellow/blue
	{78, {{{{{53, 59, 63, 67, 73, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{71}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../yellow + green/red/blue = green/red/yellow/blue
	{78, {{{{{51, 57, 63, 65, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{73}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../red + green/yellow/blue = green/red/yellow/blue
	{78, {{{{{49, 57, 59, 61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{75}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../green + red/yellow/blue = green/red/yellow/blue
	{78, {{{{{57}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{67}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green/red + yellow/blue = green/red/yellow/blue
	{78, {{{{{59}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{65}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green/yellow + red/blue = green/red/yellow/blue
	{78, {{{{{61}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{63}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // green/blue + red/yellow = green/red/yellow/blue
	{78, {{{{{49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75}}, RD_CRAFTING_RECIPE_AUTO_SELECT},  {{{77}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + green/red/yellow/blue = green/red/yellow/blue
	{88, {{{{{79}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{81}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald + ruby = emerald/ruby
	{88, {{{{{79, 81}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald or ruby + emerald/ruby = emerald/ruby
	{90, {{{{{79}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{83}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald + topaz = emerald/topaz
	{90, {{{{{79, 83}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald or topaz + emerald/topaz = emerald/topaz
	{92, {{{{{79}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald + sapphire = emerald/sapphire
	{92, {{{{{79, 85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald or sapphire + emerald/sapphire = emerald/sapphire
	{94, {{{{{81}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{83}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby + topaz = ruby/topaz
	{94, {{{{{81, 83}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby or topaz + ruby/topaz = ruby/topaz
	{96, {{{{{81}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby + sapphire = ruby/sapphire
	{96, {{{{{81, 85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby or sapphire + ruby/sapphire = ruby/sapphire
	{98, {{{{{83}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz + sapphire = topaz/sapphire
	{98, {{{{{83, 85}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz or sapphire + topaz/sapphire = topaz/sapphire
	{100, {{{{{83, 89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz or emerald/topaz + emerald/ruby = emerald/ruby/topaz
	{100, {{{{{81, 93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby or ruby/topaz + emerald/topaz = emerald/ruby/topaz
	{100, {{{{{79, 87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald or emerald/ruby + ruby/topaz = emerald/ruby/topaz
	{100, {{{{{79, 81, 83, 87, 89, 93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{99}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + emerald/ruby/topaz = emerald/ruby/topaz
	{102, {{{{{85, 91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire or emerald/sapphire + emerald/ruby = emerald/ruby/sapphire
	{102, {{{{{81, 95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby or ruby/sapphire + emerald/sapphire = emerald/ruby/sapphire
	{102, {{{{{79, 87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald or emerald/ruby + ruby/sapphire = emerald/ruby/sapphire
	{102, {{{{{79, 81, 85, 87, 91, 95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{101}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + emerald/ruby/sapphire = emerald/ruby/sapphire
	{104, {{{{{85, 91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire or emerald/sapphire + emerald/topaz = emerald/topaz/sapphire
	{104, {{{{{83, 97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz or topaz/sapphire + emerald/sapphire = emerald/topaz/sapphire
	{104, {{{{{79, 89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald or emerald/topaz + topaz/sapphire = emerald/topaz/sapphire
	{104, {{{{{79, 83, 85, 89, 91, 97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{103}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + emerald/topaz/sapphire = emerald/topaz/sapphire
	{106, {{{{{85, 95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // sapphire or ruby/sapphire + ruby/topaz = ruby/topaz/sapphire
	{106, {{{{{83, 97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // topaz or topaz/sapphire + ruby/sapphire = ruby/topaz/sapphire
	{106, {{{{{81, 93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ruby or ruby/topaz + topaz/sapphire = ruby/topaz/sapphire
	{106, {{{{{81, 83, 85, 93, 95, 97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + ruby/topaz/sapphire = ruby/topaz/sapphire
	{108, {{{{{85, 91, 95, 97, 101, 103, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{99}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../sapphire + emerald/ruby/topaz = emerald/ruby/topaz/sapphire
	{108, {{{{{83, 89, 93, 97, 103, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{101}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../topaz + emerald/ruby/sapphire = emerald/ruby/topaz/sapphire
	{108, {{{{{81, 87, 93, 95, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{103}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../ruby + emerald/topaz/sapphire = emerald/ruby/topaz/sapphire
	{108, {{{{{79, 87, 89, 91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{105}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // .../emerald + ruby/topaz/sapphire = emerald/ruby/topaz/sapphire
	{108, {{{{{87}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{97}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald/ruby + topaz/sapphire = emerald/ruby/topaz/sapphire
	{108, {{{{{89}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{95}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald/topaz + ruby/sapphire = emerald/ruby/topaz/sapphire
	{108, {{{{{91}}, RD_CRAFTING_RECIPE_AUTO_SELECT}, {{{93}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // emerald/sapphire + ruby/topaz = emerald/ruby/topaz/sapphire
	{108, {{{{{79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105}}, RD_CRAFTING_RECIPE_AUTO_SELECT},  {{{107}}, RD_CRAFTING_RECIPE_AUTO_SELECT}}}}, // ... + emerald/ruby/topaz/sapphire = emerald/ruby/topaz/sapphire
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
			UTIL_TraceHull( vecStepStart, vecStepStart - Vector( 0.0f, 0.0f, 80.0f ), Vector( -12.0f, -12.0f, 0.0f ), Vector( 12.0f, 12.0f, 24.0f ), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );

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
PRECACHE_REGISTER( rd_crafting_material_pickup );

CRD_Crafting_Material_Pickup::CRD_Crafting_Material_Pickup()
{
	m_iLocation = -1;
	FOR_EACH_VEC( m_MaterialAtLocation, i )
	{
		m_MaterialAtLocation.Set( i, RD_CRAFTING_MATERIAL_NONE );
	}
	m_bAnyoneFound = false;
}

void CRD_Crafting_Material_Pickup::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/swarm/crafting/placeholder.mdl" );
}

void CRD_Crafting_Material_Pickup::Spawn()
{
	BaseClass::Spawn();

	if ( m_iLocation == -1 )
	{
		Warning( "%s not initialized properly; deleting\n", GetDebugName() );
		UTIL_Remove( this );
	}

	SetModel( "models/swarm/crafting/placeholder.mdl" );
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
}

int	CRD_Crafting_Material_Pickup::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
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
	m_flLastWorkaroundReset = 0.0f;
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
	if ( m_iLastMaterialType != iCurrentMaterial || m_flLastWorkaroundReset < gpGlobals->curtime - 5.0f )
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
		m_flLastWorkaroundReset = gpGlobals->curtime;
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
	if ( pUser && pUser->IsInhabitableNPC() )
	{
		CASW_Player *pPlayer = assert_cast< CASW_Inhabitable_NPC * >( pUser )->GetCommander();
		if ( !pPlayer || m_MaterialAtLocation[pPlayer->entindex() - 1] == RD_CRAFTING_MATERIAL_NONE )
		{
			return false;
		}
	}

	return ( pUser && pUser->GetAbsOrigin().DistTo( GetAbsOrigin() ) < ASW_MARINE_USE_RADIUS );	// near enough?
}

#ifdef DBGFLAG_ASSERT
static int __cdecl CompareItemIDs( const SteamItemDef_t *a, const SteamItemDef_t *b )
{
	if ( *a > *b )
		return 1;
	if ( *a < *b )
		return -1;
	return 0;
}

void ValidateCraftingDefs( const CUtlVector<SteamItemDef_t> &AllItemDefs )
{
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	ISteamInventory *pInventory = SteamInventory();
#else
	ISteamInventory *pInventory = SteamInventory();
#endif
#ifdef GAME_DLL
	if ( engine->IsDedicatedServer() )
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pInventory = SteamGameServerInventory();
#else
		pInventory = SteamGameServerInventory();
#endif
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
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		if ( !pInventory->GetItemDefinitionProperty( AllItemDefs[i], "exchange", szExchange, &nExchangeSize ) )
#else
		if ( !pInventory->GetItemDefinitionProperty( AllItemDefs[i], "exchange", szExchange, &nExchangeSize ) )
#endif
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
#endif
