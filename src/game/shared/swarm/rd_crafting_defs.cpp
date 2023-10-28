#include "cbase.h"
#include "rd_crafting_defs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef RD_7A_DROPS
PRECACHE_REGISTER_BEGIN( GLOBAL, ReactiveDropCrafting )
	for ( int i = 0; i < NUM_RD_CRAFTING_MATERIAL_TYPES; i++ )
	{
		PRECACHE( MODEL, g_RD_Crafting_Material_Info[i].m_szModelName );
	}
PRECACHE_REGISTER_END();

const RD_Crafting_Material_Info g_RD_Crafting_Material_Info[] =
{
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
};
#endif
