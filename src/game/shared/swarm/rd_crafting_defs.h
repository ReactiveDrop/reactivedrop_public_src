#pragma once

#include "steam/isteaminventory.h"
#ifdef CLIENT_DLL
#include "iasw_client_usable_entity.h"
#include "glow_outline_effect.h"
#define CRD_Crafting_Material_Pickup C_RD_Crafting_Material_Pickup
#else
#include "iasw_server_usable_entity.h"
#endif

#define RD_MAX_CRAFTING_MATERIAL_SPAWN_LOCATIONS 5

#ifdef RD_7A_DROPS
enum RD_Crafting_Material_t
{
	RD_CRAFTING_MATERIAL_NONE,

	// common
	RD_CRAFTING_MATERIAL_SCRAP_METAL,
	RD_CRAFTING_MATERIAL_ELECTRICAL_COMPONENTS,
	RD_CRAFTING_MATERIAL_SPARE_PIPE,
	RD_CRAFTING_MATERIAL_PLASTICS,
	RD_CRAFTING_MATERIAL_COOLANT,
	RD_CRAFTING_MATERIAL_MINI_CRATE,
	RD_CRAFTING_MATERIAL_BATTERY_PACK,

	// ultra-common
	RD_CRAFTING_MATERIAL_LOOSE_WIRES,
	RD_CRAFTING_MATERIAL_CARBON,

	// uncommon
	RD_CRAFTING_MATERIAL_ALIEN_CHITIN,
	RD_CRAFTING_MATERIAL_BIOMASS_SAMPLE,
	RD_CRAFTING_MATERIAL_GLOWING_GREEN_ACID,
	RD_CRAFTING_MATERIAL_CLAW_FRAGMENT,

	// rare
	RD_CRAFTING_MATERIAL_MEMORY_MANAGEMENT_UNIT,
	RD_CRAFTING_MATERIAL_ARITHMETIC_LOGIC_UNIT,
	RD_CRAFTING_MATERIAL_DATA_STORAGE_MEDIUM,

	// regional
	RD_CRAFTING_MATERIAL_PILE_OF_RED_SAND,
	RD_CRAFTING_MATERIAL_ANTLION_CARAPACE,
	RD_CRAFTING_MATERIAL_CORROSIVE_FLUID_SAMPLE,
	RD_CRAFTING_MATERIAL_COOLED_VOLCANIC_ROCK,
	RD_CRAFTING_MATERIAL_RETRIEVED_DOCUMENTS,
	RD_CRAFTING_MATERIAL_UNOPENED_SYNUP_COLA,
	RD_CRAFTING_MATERIAL_ROLL_OF_VENT_TAPE,
	RD_CRAFTING_MATERIAL_ISOTOPES,
	RD_CRAFTING_MATERIAL_CRYOTIC,
	RD_CRAFTING_MATERIAL_ARGON_CANISTER,
	RD_CRAFTING_MATERIAL_PROBABILITY_DRIVE,

	NUM_RD_CRAFTING_MATERIAL_TYPES,
};

enum RD_Crafting_Material_Rarity_t
{
	RD_CRAFTING_MATERIAL_RARITY_COMMON,
	RD_CRAFTING_MATERIAL_RARITY_ULTRA_COMMON,
	RD_CRAFTING_MATERIAL_RARITY_UNCOMMON,
	RD_CRAFTING_MATERIAL_RARITY_RARE,
	RD_CRAFTING_MATERIAL_RARITY_REGIONAL,
};

struct RD_Crafting_Material_Info
{
	const char *m_szName;
	SteamItemDef_t m_iTokenDef;
	SteamItemDef_t m_iRedeemDef;
	SteamItemDef_t m_iItemDef;
	RD_Crafting_Material_Rarity_t m_iRarity;

	const char *m_szModelName;
};

extern const RD_Crafting_Material_Info g_RD_Crafting_Material_Info[NUM_RD_CRAFTING_MATERIAL_TYPES];

#ifdef GAME_DLL
void GenerateCraftingMaterialSpawnLocations( CUtlVector<Vector> &spawnLocations )
#endif

class CRD_Crafting_Material_Pickup :
	public CBaseAnimating,
#ifdef CLIENT_DLL
	public IASW_Client_Usable_Entity
#else
	public IASW_Server_Usable_Entity
#endif
{
	DECLARE_CLASS( CRD_Crafting_Material_Pickup, CBaseAnimating );
public:
	DECLARE_NETWORKCLASS();

	CRD_Crafting_Material_Pickup();

	IMPLEMENT_AUTO_LIST_GET();

	CNetworkVar( int, m_iLocation );
	CNetworkArray( RD_Crafting_Material_t, m_MaterialAtLocation, MAX_PLAYERS );

#ifdef CLIENT_DLL
	void OnDataChanged( DataUpdateType_t updateType ) override;
	void ClientThink() override;

	// IASW_Client_Usable_Entity implementation
	bool IsUsable( C_BaseEntity *pUser ) override;
	bool GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser ) override;
	void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) override {}
	bool ShouldPaintBoxAround() override { return false; }
	bool NeedsLOSCheck() override { return false; }

	RD_Crafting_Material_t m_iLastMaterialType;
	CGlowObject m_GlowObject;
#else
	void Spawn() override;

	// IASW_Server_Usable_Entity implementation
	bool IsUsable( CBaseEntity *pUser ) override;
	bool RequirementsMet( CBaseEntity *pUser ) override { return true; }
	void ActivateUseIcon( CASW_Inhabitable_NPC *pUser, int nHoldType ) override;
	void NPCStartedUsing( CASW_Inhabitable_NPC *pUser ) override {}
	void NPCStoppedUsing( CASW_Inhabitable_NPC *pUser ) override {}
	void NPCUsing( CASW_Inhabitable_NPC *pUser, float fDeltaTime ) override {}
	bool NeedsLOSCheck() override { return false; }

	bool m_bAnyoneFound;
#endif
};
#endif
