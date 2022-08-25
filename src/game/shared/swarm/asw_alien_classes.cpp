#include "cbase.h"
#include "asw_alien_classes.h"
#include "gamestringpool.h"
#include "../../game/server/ai_hull.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==================================
// == Master list of alien classes ==
// ==================================

ASW_Alien_Class_Entry::ASW_Alien_Class_Entry( const char *szClass, int nHullType )
{
	m_pszAlienClass = szClass;
	m_nHullType = nHullType;
}

// NOTE: If you add new entries to this list, update the asw_spawner choices in swarm.fgd.
//       Do not rearrange the order or you will be changing what spawns in all the maps.

ASW_Alien_Class_Entry g_Aliens[] =
{
	ASW_Alien_Class_Entry( "asw_drone", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "asw_buzzer", HULL_TINY_CENTERED ),
	ASW_Alien_Class_Entry( "asw_parasite", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_shieldbug", HULL_WIDE_SHORT ),
	ASW_Alien_Class_Entry( "asw_grub", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_drone_jumper", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "asw_harvester", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "asw_parasite_defanged", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_queen", HULL_TINY ),
	ASW_Alien_Class_Entry( "asw_boomer", HULL_LARGE ),
	ASW_Alien_Class_Entry( "asw_ranger", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "asw_mortarbug", HULL_WIDE_SHORT ),
	ASW_Alien_Class_Entry( "asw_shaman", HULL_MEDIUM ),
	ASW_Alien_Class_Entry( "asw_drone_uber", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "npc_antlionguard_normal", HULL_LARGE ),
	ASW_Alien_Class_Entry( "npc_antlionguard_cavern", HULL_LARGE ),
	ASW_Alien_Class_Entry( "npc_antlion", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "npc_antlion_worker", HULL_MEDIUMBIG ),
	ASW_Alien_Class_Entry( "npc_zombie", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "npc_zombie_torso", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "npc_poisonzombie", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "npc_fastzombie", HULL_HUMAN ),
	ASW_Alien_Class_Entry( "npc_fastzombie_torso", HULL_HUMAN )
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

class CASW_Alien_Class_Strings : public CAutoGameSystem
{
public:
	CASW_Alien_Class_Strings() : CAutoGameSystem( "CASW_Alien_Class_Strings" )
	{
	}

	virtual void LevelInitPreEntity()
	{
		for ( int i = 0; i < NELEMS( g_Aliens ); i++ )
		{
			g_Aliens[i].m_iszAlienClass = AllocPooledString( g_Aliens[i].m_pszAlienClass );
		}
	}
} s_ASW_Alien_Class_Strings;
