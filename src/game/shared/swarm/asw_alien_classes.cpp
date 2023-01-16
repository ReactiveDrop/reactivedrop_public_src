#include "cbase.h"
#include "asw_alien_classes.h"
#include "gamestringpool.h"
#include "../../game/server/ai_hull.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==================================
// == Master list of alien classes ==
// ==================================

ASW_Alien_Class_Entry::ASW_Alien_Class_Entry( const char *szClass, int nHullType, Vector vecRealHullMins, Vector vecRealHullMaxs )
{
	m_pszAlienClass = szClass;
	m_nHullType = nHullType;
	m_vecRealHullMins = vecRealHullMins;
	m_vecRealHullMaxs = vecRealHullMins;
}

// NOTE: If you add new entries to this list, update the asw_spawner choices in swarm.fgd.
//       Do not rearrange the order or you will be changing what spawns in all the maps.

const ASW_Alien_Class_Entry g_Aliens[] =
{
	ASW_Alien_Class_Entry( "asw_drone", HULL_MEDIUMBIG, Vector( -17, -17, 0 ), Vector( 17, 17, 69 ) ),
	ASW_Alien_Class_Entry( "asw_buzzer", HULL_TINY_CENTERED, Vector( -10, -10, -10 ), Vector( 10, 10, 10 ) ),
	ASW_Alien_Class_Entry( "asw_parasite", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) ),
	ASW_Alien_Class_Entry( "asw_shieldbug", HULL_WIDE_SHORT, Vector( -40, -40, 0 ), Vector( 40, 40, 72 ) ),
	ASW_Alien_Class_Entry( "asw_grub", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) ),
	ASW_Alien_Class_Entry( "asw_drone_jumper", HULL_MEDIUMBIG, Vector( -17, -17, 0 ), Vector( 17, 17, 69 ) ),
	ASW_Alien_Class_Entry( "asw_harvester", HULL_WIDE_SHORT, Vector( -23, -23, 0 ), Vector( 23, 23, 69 ) ),
	ASW_Alien_Class_Entry( "asw_parasite_defanged", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) ),
	ASW_Alien_Class_Entry( "asw_queen", HULL_HUGE, Vector( -120, -120, 0 ), Vector( 120, 120, 160 ) ),
	ASW_Alien_Class_Entry( "asw_boomer", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) ),
	ASW_Alien_Class_Entry( "asw_ranger", HULL_MEDIUMBIG, Vector( -20, -20, 0 ), Vector( 20, 20, 69 ) ),
	ASW_Alien_Class_Entry( "asw_mortarbug", HULL_WIDE_SHORT, Vector( -23, -23, 0 ), Vector( 23, 23, 69 ) ),
	ASW_Alien_Class_Entry( "asw_shaman", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) ),
	ASW_Alien_Class_Entry( "asw_drone_uber", HULL_MEDIUMBIG, Vector( -17, -17, 0 ), Vector( 17, 17, 69 ) ),
	ASW_Alien_Class_Entry( "npc_antlionguard_normal", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) ),
	ASW_Alien_Class_Entry( "npc_antlionguard_cavern", HULL_LARGE, Vector( -30, -30, 0 ), Vector( 30, 30, 110 ) ),
	ASW_Alien_Class_Entry( "npc_antlion", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) ),
	ASW_Alien_Class_Entry( "npc_antlion_worker", HULL_MEDIUM, Vector( -16, -16, 0 ), Vector( 16, 16, 64 ) ),
	ASW_Alien_Class_Entry( "npc_zombie", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_zombie_torso", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_poisonzombie", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_fastzombie", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_fastzombie_torso", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_headcrab", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) ),
	ASW_Alien_Class_Entry( "npc_headcrab_fast", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) ),
	ASW_Alien_Class_Entry( "npc_headcrab_poison", HULL_TINY, Vector( -12, -12, 0 ), Vector( 12, 12, 12 ) ),
	ASW_Alien_Class_Entry( "npc_zombine", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_combine_s", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_combine_shotgun", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_combine_elite", HULL_HUMAN, Vector( -13, -13, 0 ), Vector( 13, 13, 72 ) ),
	ASW_Alien_Class_Entry( "npc_strider", HULL_LARGE_CENTERED, Vector( -38, -38, -38 ), Vector( 38, 38, 38 ) ),
	ASW_Alien_Class_Entry( "npc_hunter", HULL_MEDIUM_TALL, Vector( -18, -18, 0 ), Vector( 18, 18, 100 ) ),
	ASW_Alien_Class_Entry( "npc_combinedropship", HULL_LARGE_CENTERED, Vector( -38, -38, -38 ), Vector( 38, 38, 38 ) ),
	ASW_Alien_Class_Entry( "npc_combinegunship", HULL_LARGE_CENTERED, Vector( -38, -38, -38 ), Vector( 38, 38, 38 ) ),
};

int GetAlienClassIndex( CBaseEntity *pAlien )
{
	Assert( pAlien );
	if ( pAlien )
	{
		const char *szClassName = pAlien->GetClassname();
		for ( int i = 0; i < NELEMS( g_Aliens ); i++ )
		{
			if ( FStrEq( g_Aliens[i].m_pszAlienClass, szClassName ) )
			{
				return i;
			}
		}

		Assert( !"unhandled alien class" );
	}

	return -1;
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
