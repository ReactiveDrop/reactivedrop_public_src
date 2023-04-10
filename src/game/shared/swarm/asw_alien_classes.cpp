#include "cbase.h"
#include "asw_alien_classes.h"
#include "gamestringpool.h"
#include "../../game/server/ai_hull.h"
#ifdef CLIENT_DLL
#include "c_asw_inhabitable_npc.h"
#include "asw_shareddefs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==================================
// == Master list of alien classes ==
// ==================================

ASW_Alien_Class_Entry::ASW_Alien_Class_Entry( const char *szClass, int nHullType, Vector vecRealHullMins, Vector vecRealHullMaxs, const char *szHordeSound )
{
	m_pszAlienClass = szClass;
	m_nHullType = nHullType;
	m_vecRealHullMins = vecRealHullMins;
	m_vecRealHullMaxs = vecRealHullMins;
	m_szHordeSound = szHordeSound;
}

// NOTE: If you add new entries to this list, update the asw_spawner choices in swarm.fgd.
//       Do not rearrange the order or you will be changing what spawns in all the maps.
const ASW_Alien_Class_Entry g_Aliens[] =
{
	{ "asw_drone", HULL_MEDIUMBIG, Vector( -17, -17, 0 ), Vector( 17, 17, 69 ) },
	{ "asw_buzzer", HULL_TINY_CENTERED, Vector( -10, -10, -10 ), Vector( 10, 10, 10 ) },
	{ "asw_parasite", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) },
	{ "asw_shieldbug", HULL_WIDE_SHORT, Vector( -40, -40, 0 ), Vector( 40, 40, 72 ) },
	{ "asw_grub", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) },
	{ "asw_drone_jumper", HULL_MEDIUMBIG, Vector( -17, -17, 0 ), Vector( 17, 17, 69 ) },
	{ "asw_harvester", HULL_WIDE_SHORT, Vector( -23, -23, 0 ), Vector( 23, 23, 69 ) },
	{ "asw_parasite_defanged", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) },
	{ "asw_queen", HULL_HUGE, Vector( -80, -80, 0 ), Vector( 80, 80, 160 ) },
	{ "asw_boomer", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) },
	{ "asw_ranger", HULL_MEDIUMBIG, Vector( -20, -20, 0 ), Vector( 20, 20, 69 ) },
	{ "asw_mortarbug", HULL_WIDE_SHORT, Vector( -23, -23, 0 ), Vector( 23, 23, 69 ) },
	{ "asw_shaman", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) },
	{ "asw_drone_uber", HULL_MEDIUMBIG, Vector( -17, -17, 0 ), Vector( 17, 17, 69 ) },
	{ "npc_antlionguard_normal", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) },
	{ "npc_antlionguard_cavern", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) },
	{ "npc_antlion", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) },
	{ "npc_antlion_worker", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) },
	{ "npc_zombie", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeZombies" },
	{ "npc_zombie_torso", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeZombies" },
	{ "npc_poisonzombie", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeZombies" },
	{ "npc_fastzombie", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeZombies" },
	{ "npc_fastzombie_torso", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeZombies" },
	{ "npc_headcrab", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) },
	{ "npc_headcrab_fast", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) },
	{ "npc_headcrab_poison", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) },
	{ "npc_zombine", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeZombies" },
	{ "npc_combine_s", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeCombine" },
	{ "npc_combine_shotgun", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeCombine" },
	{ "npc_combine_elite", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ), "Spawner.HordeCombine" },
	{ "npc_hunter", HULL_MEDIUM_TALL, Vector( -18, -18, 0 ), Vector( 18, 18, 100 ), "Spawner.HordeCombine" },
#ifdef RD_7A_ENEMIES
	{ "asw_meatbug", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) },
	{ "asw_juggernaut", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) },
	{ "asw_spitter", HULL_MEDIUMBIG, Vector( -20, -20, 0 ), Vector( 20, 20, 69 ) },
	{ "asw_detonator", HULL_MEDIUMBIG, Vector( -20, -20, 0 ), Vector( 20, 20, 69 ) },
	{ "asw_flock", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) },
	{ "asw_runner", HULL_WIDE_SHORT, Vector( -23, -23, 0 ), Vector( 23, 23, 69 ) },
	{ "asw_watcher", HULL_MEDIUM_TALL, Vector( -18, -18, 0 ), Vector( 18, 18, 100 ) },
#endif
};

// These are inhabitable NPC classes that can't be spawned via an asw_spawner.
//
// Inserted backwards into the alien class list for networking purposes; as in:
//
// -4: g_NonSpawnableAliens[2]
// -3: g_NonSpawnableAliens[1]
// -2: g_NonSpawnableAliens[0]
// -1: NULL
// 0: g_Aliens[0]
// 1: g_Aliens[1]
// 2: g_Aliens[2]
// 3: g_Aliens[3]
const ASW_Alien_Class_Entry g_NonSpawnableAliens[] =
{
	{ "asw_marine", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) },
	{ "asw_colonist", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) },
	{ "npc_strider", HULL_LARGE_CENTERED, Vector( -38, -38, -38 ), Vector( 38, 38, 38 ), "Spawner.HordeCombine" },
	{ "npc_combinegunship", HULL_LARGE_CENTERED, Vector( -38, -38, -38 ), Vector( 38, 38, 38 ), "Spawner.HordeCombine" },
	{ "npc_combinedropship", HULL_LARGE_CENTERED, Vector( -38, -38, -38 ), Vector( 38, 38, 38 ), "Spawner.HordeCombine" },
};

int GetAlienClassIndex( CBaseEntity *pAlien )
{
	Assert( pAlien );
	if ( pAlien )
	{
#ifdef CLIENT_DLL
		if ( pAlien->IsInhabitableNPC() )
		{
			Assert( assert_cast< C_ASW_Inhabitable_NPC * >( pAlien )->m_iAlienClassIndex != -1 );
			return assert_cast< C_ASW_Inhabitable_NPC * >( pAlien )->m_iAlienClassIndex;
		}

		if ( pAlien->Classify() == CLASS_ASW_GRUB )
		{
			Assert( FStrEq( g_Aliens[4].m_pszAlienClass, "asw_grub" ) );
			return 4;
		}
#else
		const char *szClassName = pAlien->GetClassname();
		for ( int i = 0; i < NELEMS( g_Aliens ); i++ )
		{
			if ( FStrEq( g_Aliens[i].m_pszAlienClass, szClassName ) )
			{
				return i;
			}
		}
		for ( int i = 0; i < NELEMS( g_NonSpawnableAliens ); i++ )
		{
			if ( FStrEq( g_NonSpawnableAliens[i].m_pszAlienClass, szClassName ) )
			{
				return -2 - i;
			}
		}
#endif

		Assert( !"unhandled alien class" );
	}

	return -1;
}

const ASW_Alien_Class_Entry *GetAlienClass( int index )
{
	if ( index == -1 )
		return NULL;

	Assert( index < NELEMS( g_Aliens ) );
	if ( index >= 0 && index < NELEMS( g_Aliens ) )
		return &g_Aliens[index];

	Assert( 2 - index < NELEMS( g_NonSpawnableAliens ) );
	if ( 2 - index >= 0 && 2 - index < NELEMS( g_NonSpawnableAliens ) )
		return &g_NonSpawnableAliens[2 - index];

	return NULL;
}

const char *GetAlienClassname( int index )
{
	if ( index == -1 )
		return NULL;

	const ASW_Alien_Class_Entry *pClass = GetAlienClass( index );
	Assert( pClass );

	return pClass ? pClass->m_pszAlienClass : NULL;
}

// Array indices of drones.  Used by carnage mode.
const int g_nDroneClassEntry = 0;
const int g_nDroneJumperClassEntry = 5;

static class CASW_Alien_Class_Strings final : public CAutoGameSystem
{
public:
	CASW_Alien_Class_Strings() : CAutoGameSystem( "CASW_Alien_Class_Strings" )
	{
	}

	void LevelInitPreEntity() override
	{
		for ( int i = 0; i < NELEMS( g_Aliens ); i++ )
		{
			g_Aliens[i].m_iszAlienClass = AllocPooledString( g_Aliens[i].m_pszAlienClass );

#ifdef GAME_DLL
			Assert( NAI_Hull::Mins( g_Aliens[i].m_nHullType ).x <= g_Aliens[i].m_vecRealHullMins.x );
			Assert( NAI_Hull::Mins( g_Aliens[i].m_nHullType ).y <= g_Aliens[i].m_vecRealHullMins.y );
			Assert( NAI_Hull::Mins( g_Aliens[i].m_nHullType ).z <= g_Aliens[i].m_vecRealHullMins.z );
			Assert( NAI_Hull::Maxs( g_Aliens[i].m_nHullType ).x >= g_Aliens[i].m_vecRealHullMaxs.x );
			Assert( NAI_Hull::Maxs( g_Aliens[i].m_nHullType ).y >= g_Aliens[i].m_vecRealHullMaxs.y );
			Assert( NAI_Hull::Maxs( g_Aliens[i].m_nHullType ).z >= g_Aliens[i].m_vecRealHullMaxs.z );
#endif
		}
	}
} s_ASW_Alien_Class_Strings;
