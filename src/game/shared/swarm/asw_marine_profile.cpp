#include "cbase.h"
#include "asw_marine_profile.h"
#include "filesystem.h"
#include <KeyValues.h>
#include "asw_util_shared.h"
#include "stringpool.h"
#ifdef CLIENT_DLL
	#include <vgui/ISurface.h>
	#include <vgui/ISystem.h>
	#include <vgui_controls/Panel.h>
#else
	#include "engine/IEngineSound.h"
	#include "SoundEmitterSystem/isoundemittersystembase.h"
	#include "soundchars.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL
ConVar asw_precache_speech("asw_precache_speech", "0", FCVAR_CHEAT, "If set, server will precache all speech files (increases load times a lot!)");
#endif


const char *const g_szMarineClassLabel[NUM_MARINE_CLASSES] =
{
	"#marine_class_officer",
	"#marine_class_sw_short",
	"#marine_class_medic",
	"#marine_class_tech",
};
const char *const g_szMarineClassImage[NUM_MARINE_CLASSES] =
{
	"swarm/ClassIcons/NCOClassIcon",
	"swarm/ClassIcons/SpecialWeaponsClassIcon",
	"swarm/ClassIcons/MedicClassIcon",
	"swarm/ClassIcons/TechClassIcon",
};

COMPILE_TIME_ASSERT( ASW_SKILL_SLOT_SPARE == ASW_NUM_SKILL_SLOTS - 1 );
static const char *const s_szSkillNumber[ASW_NUM_MARINE_SKILLS]
{
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17",
};
ConVar asw_marine_skill[ASW_NUM_MARINE_PROFILES][ASW_NUM_SKILL_SLOTS - 1]
{
	{
		{ "asw_marine_skill_sarge_0", s_szSkillNumber[ASW_MARINE_SKILL_LEADERSHIP], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Sarge's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_sarge_1", s_szSkillNumber[ASW_MARINE_SKILL_VINDICATOR], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Sarge's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_sarge_2", s_szSkillNumber[ASW_MARINE_SKILL_GRENADES], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Sarge's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_sarge_3", s_szSkillNumber[ASW_MARINE_SKILL_HEALTH], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Sarge's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_sarge_4", s_szSkillNumber[ASW_MARINE_SKILL_ACCURACY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Sarge's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_wildcat_0", s_szSkillNumber[ASW_MARINE_SKILL_AUTOGUN], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wildcat's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wildcat_1", s_szSkillNumber[ASW_MARINE_SKILL_PIERCING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wildcat's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wildcat_2", s_szSkillNumber[ASW_MARINE_SKILL_ACCURACY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wildcat's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wildcat_3", s_szSkillNumber[ASW_MARINE_SKILL_RELOADING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wildcat's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wildcat_4", s_szSkillNumber[ASW_MARINE_SKILL_AGILITY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wildcat's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_faith_0", s_szSkillNumber[ASW_MARINE_SKILL_HEALING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Faith's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_faith_1", s_szSkillNumber[ASW_MARINE_SKILL_XENOWOUNDS], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Faith's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_faith_2", s_szSkillNumber[ASW_MARINE_SKILL_ACCURACY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Faith's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_faith_3", s_szSkillNumber[ASW_MARINE_SKILL_RELOADING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Faith's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_faith_4", s_szSkillNumber[ASW_MARINE_SKILL_AGILITY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Faith's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_crash_0", s_szSkillNumber[ASW_MARINE_SKILL_SCANNER], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Crash's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_crash_1", s_szSkillNumber[ASW_MARINE_SKILL_ENGINEERING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Crash's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_crash_2", s_szSkillNumber[ASW_MARINE_SKILL_ACCURACY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Crash's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_crash_3", s_szSkillNumber[ASW_MARINE_SKILL_RELOADING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Crash's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_crash_4", s_szSkillNumber[ASW_MARINE_SKILL_GRENADES], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Crash's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_jaeger_0", s_szSkillNumber[ASW_MARINE_SKILL_LEADERSHIP], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Jaeger's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_jaeger_1", s_szSkillNumber[ASW_MARINE_SKILL_VINDICATOR], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Jaeger's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_jaeger_2", s_szSkillNumber[ASW_MARINE_SKILL_GRENADES], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Jaeger's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_jaeger_3", s_szSkillNumber[ASW_MARINE_SKILL_HEALTH], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Jaeger's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_jaeger_4", s_szSkillNumber[ASW_MARINE_SKILL_MELEE], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Jaeger's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_wolfe_0", s_szSkillNumber[ASW_MARINE_SKILL_AUTOGUN], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wolfe's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wolfe_1", s_szSkillNumber[ASW_MARINE_SKILL_PIERCING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wolfe's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wolfe_2", s_szSkillNumber[ASW_MARINE_SKILL_ACCURACY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wolfe's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wolfe_3", s_szSkillNumber[ASW_MARINE_SKILL_RELOADING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wolfe's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_wolfe_4", s_szSkillNumber[ASW_MARINE_SKILL_HEALTH], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Wolfe's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_bastille_0", s_szSkillNumber[ASW_MARINE_SKILL_HEALING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Bastille's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_bastille_1", s_szSkillNumber[ASW_MARINE_SKILL_XENOWOUNDS], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Bastille's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_bastille_2", s_szSkillNumber[ASW_MARINE_SKILL_ACCURACY], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Bastille's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_bastille_3", s_szSkillNumber[ASW_MARINE_SKILL_RELOADING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Bastille's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_bastille_4", s_szSkillNumber[ASW_MARINE_SKILL_DRUGS], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Bastille's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
	{
		{ "asw_marine_skill_vegas_0", s_szSkillNumber[ASW_MARINE_SKILL_SCANNER], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Vegas's first skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_vegas_1", s_szSkillNumber[ASW_MARINE_SKILL_ENGINEERING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Vegas's second skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_vegas_2", s_szSkillNumber[ASW_MARINE_SKILL_HEALTH], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Vegas's third skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_vegas_3", s_szSkillNumber[ASW_MARINE_SKILL_RELOADING], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Vegas's fourth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
		{ "asw_marine_skill_vegas_4", s_szSkillNumber[ASW_MARINE_SKILL_MELEE], FCVAR_CHEAT | FCVAR_REPLICATED, "index of Vegas's fifth skill slot type", true, 0, true, ASW_NUM_MARINE_SKILLS - 1 },
	},
};
ConVar asw_marine_skill_points[ASW_NUM_MARINE_PROFILES][ASW_NUM_SKILL_SLOTS - 1]
{
	{
		{ "asw_marine_skill_points_sarge_0", "5", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Sarge's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_sarge_1", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Sarge's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_sarge_2", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Sarge's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_sarge_3", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Sarge's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_sarge_4", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Sarge's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_wildcat_0", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wildcat's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wildcat_1", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wildcat's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wildcat_2", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wildcat's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wildcat_3", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wildcat's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wildcat_4", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wildcat's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_faith_0", "5", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Faith's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_faith_1", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Faith's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_faith_2", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Faith's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_faith_3", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Faith's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_faith_4", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Faith's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_crash_0", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Crash's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_crash_1", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Crash's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_crash_2", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Crash's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_crash_3", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Crash's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_crash_4", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Crash's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_jaeger_0", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Jaeger's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_jaeger_1", "5", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Jaeger's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_jaeger_2", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Jaeger's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_jaeger_3", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Jaeger's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_jaeger_4", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Jaeger's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_wolfe_0", "5", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wolfe's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wolfe_1", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wolfe's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wolfe_2", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wolfe's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wolfe_3", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wolfe's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_wolfe_4", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Wolfe's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_bastille_0", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Bastille's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_bastille_1", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Bastille's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_bastille_2", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Bastille's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_bastille_3", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Bastille's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_bastille_4", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Bastille's fifth skill slot", true, 0, true, 5 },
	},
	{
		{ "asw_marine_skill_points_vegas_0", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Vegas's first skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_vegas_1", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Vegas's second skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_vegas_2", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Vegas's third skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_vegas_3", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Vegas's fourth skill slot", true, 0, true, 5 },
		{ "asw_marine_skill_points_vegas_4", "4", FCVAR_CHEAT | FCVAR_REPLICATED, "number of points in Vegas's fifth skill slot", true, 0, true, 5 },
	},
};

ASW_Voice_Type GetASWVoiceType( const char *szVoiceType )
{
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_SARGE" ) )			return ASW_VOICE_SARGE;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_JAEGER" ) )		return ASW_VOICE_JAEGER;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_WILDCAT" ) )		return ASW_VOICE_WILDCAT;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_WOLFE" ) )			return ASW_VOICE_WOLFE;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_FAITH" ) )			return ASW_VOICE_FAITH;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_BASTILLE" ) )		return ASW_VOICE_BASTILLE;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_CRASH" ) )			return ASW_VOICE_CRASH;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_FLYNN" ) )			return ASW_VOICE_FLYNN;
	if ( !V_stricmp( szVoiceType, "ASW_VOICE_VEGAS" ) )			return ASW_VOICE_VEGAS;

	return ASW_VOICE_SARGE;
}

const char *GetVoiceShortName( ASW_Voice_Type voice )
{
	switch ( voice )
	{
	case ASW_VOICE_SARGE: return "Sarge";
	case ASW_VOICE_JAEGER: return "Jaeger";
	case ASW_VOICE_WILDCAT: return "Wildcat";
	case ASW_VOICE_WOLFE: return "Wolfe";
	case ASW_VOICE_FAITH: return "Faith";
	case ASW_VOICE_BASTILLE: return "Bastille";
	case ASW_VOICE_CRASH: return "Crash";
	case ASW_VOICE_FLYNN: return "Flynn";
	case ASW_VOICE_VEGAS: return "Vegas";
	}

	return "Sarge";
}

#define MARINE_HELPER( name ) "#asw_rank_" #name, "#asw_bio_" #name, "#asw_firstname_" #name, "#asw_lastname_" #name, "#asw_name_" #name, #name
static CASW_Marine_ProfileList s_MarineProfileList
{
	{
		{
			ASW_MARINE_PROFILE_SARGE, MARINE_CLASS_NCO, ASW_VOICE_SARGE,
			MARINE_HELPER( sarge ), true, 43, false,
			"Officer1", "models/swarm/marine/femalemarine.mdl", 1,
		},
		{
			ASW_MARINE_PROFILE_WILDCAT, MARINE_CLASS_SPECIAL_WEAPONS, ASW_VOICE_WILDCAT,
			MARINE_HELPER( wildcat ), true, 23, true,
			"Spec1", "models/swarm/marine/femalemarine.mdl", 3,
		},
		{
			ASW_MARINE_PROFILE_FAITH, MARINE_CLASS_MEDIC, ASW_VOICE_FAITH,
			MARINE_HELPER( faith ), true, 28, true,
			"Medic1", "models/swarm/marine/femalemarine.mdl", 2,
		},
		{
			ASW_MARINE_PROFILE_CRASH, MARINE_CLASS_TECH, ASW_VOICE_CRASH,
			MARINE_HELPER( crash ), true, 22, false,
			"Tech1", "models/swarm/marine/femalemarine.mdl", 4,
		},
		{
			ASW_MARINE_PROFILE_JAEGER, MARINE_CLASS_NCO, ASW_VOICE_JAEGER,
			MARINE_HELPER( jaeger ), false, 32, false,
			"Officer2", "models/swarm/marine/marine.mdl", 1,
		},
		{
			ASW_MARINE_PROFILE_WOLFE, MARINE_CLASS_SPECIAL_WEAPONS, ASW_VOICE_WOLFE,
			MARINE_HELPER( wolfe ), false, 26, false,
			"Spec2", "models/swarm/marine/marine.mdl", 3,
		},
		{
			ASW_MARINE_PROFILE_BASTILLE, MARINE_CLASS_MEDIC, ASW_VOICE_BASTILLE,
			MARINE_HELPER( bastille ), false, 32, false,
			"Medic2", "models/swarm/marine/marine.mdl", 2,
		},
		{
			ASW_MARINE_PROFILE_VEGAS, MARINE_CLASS_TECH, ASW_VOICE_VEGAS,
			MARINE_HELPER( vegas ), true, 34, false,
			"Tech3", "models/swarm/marine/marine.mdl", 4,
		},
	},
};

CASW_Marine_ProfileList *MarineProfileList()
{
	if ( !s_MarineProfileList.m_bInitOnce )
	{
		for ( int i=0; i < ASW_NUM_MARINE_PROFILES;i++ )
		{
			CASW_Marine_Profile *pProfile = &s_MarineProfileList.m_Profiles[i];
			pProfile->InitChatterNames( GetVoiceShortName( pProfile->m_VoiceType ) );
			pProfile->LoadTextures();
		}

		s_MarineProfileList.m_bInitOnce = true;
	}

	return &s_MarineProfileList;
}

void CASW_Marine_Profile::LoadTextures()
{
	#ifdef CLIENT_DLL
		char buffer[64];
		// load the portrait textures
		m_nPortraitTextureID = vgui::surface()->CreateNewTextureID();
		Q_snprintf(buffer, sizeof(buffer), "vgui/briefing/face_%s", m_PortraitName);
		vgui::surface()->DrawSetTextureFile( m_nPortraitTextureID, buffer, true, false);
		m_nPortraitLitTextureID = vgui::surface()->CreateNewTextureID();
		Q_snprintf(buffer, sizeof(buffer), "vgui/briefing/face_%s_lit", m_PortraitName);
		vgui::surface()->DrawSetTextureFile( m_nPortraitLitTextureID, buffer, true, false);
		
		if (GetMarineClass() == MARINE_CLASS_NCO)
			Q_snprintf(buffer, sizeof(buffer), "vgui/swarm/ClassIcons/NCOClassIcon");
		else if (GetMarineClass() == MARINE_CLASS_SPECIAL_WEAPONS)
			Q_snprintf(buffer, sizeof(buffer), "vgui/swarm/ClassIcons/SpecialWeaponsClassIcon");
		else if (GetMarineClass() == MARINE_CLASS_MEDIC)
			Q_snprintf(buffer, sizeof(buffer), "vgui/swarm/ClassIcons/MedicClassIcon");
		else if (GetMarineClass() == MARINE_CLASS_TECH)
			Q_snprintf(buffer, sizeof(buffer), "vgui/swarm/ClassIcons/TechClassIcon");
		m_nClassTextureID = vgui::surface()->CreateNewTextureID();				
		vgui::surface()->DrawSetTextureFile( m_nClassTextureID, buffer, true, false);
		
	#endif
}

void CASW_Marine_Profile::InitChatterNames( const char *szMarineName )
{
	for ( int i = 0; i < NUM_CHATTER_LINES; i++ )
	{
		m_Chatter[i][0] = '\0';
		m_iChatterCount[i] = 0;
#ifndef CLIENT_DLL
		m_fChatterChance[i] = 1.0f;
		for ( int j = 0; j < NUM_SUB_CHATTERS; j++ )
		{
			m_fChatterDuration[i][j] = 0;
		}
#endif
	}

	V_snprintf( m_Chatter[CHATTER_SELECTION], CHATTER_STRING_SIZE, "%s.Selection", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SELECTION_INJURED], CHATTER_STRING_SIZE, "%s.InjuredSelection", szMarineName );
	V_snprintf( m_Chatter[CHATTER_USE], CHATTER_STRING_SIZE, "%s.Use", szMarineName );
	V_snprintf( m_Chatter[CHATTER_IDLE], CHATTER_STRING_SIZE, "%s.Idle", szMarineName );
	V_snprintf( m_Chatter[CHATTER_FRIENDLY_FIRE], CHATTER_STRING_SIZE, "%s.FF", szMarineName );
	V_snprintf( m_Chatter[CHATTER_FIRING_AT_ALIEN], CHATTER_STRING_SIZE, "%s.FiringAtAlien", szMarineName );
	V_snprintf( m_Chatter[CHATTER_FOLLOW_ME], CHATTER_STRING_SIZE, "%s.FollowMe", szMarineName );
	V_snprintf( m_Chatter[CHATTER_HOLD_POSITION], CHATTER_STRING_SIZE, "%s.HoldPosition", szMarineName );
	V_snprintf( m_Chatter[CHATTER_NEED_AMMO], CHATTER_STRING_SIZE, "%s.NeedAmmo", szMarineName );
	V_snprintf( m_Chatter[CHATTER_NO_AMMO], CHATTER_STRING_SIZE, "%s.NoAmmo", szMarineName );
	V_snprintf( m_Chatter[CHATTER_MEDIC], CHATTER_STRING_SIZE, "%s.Medic", szMarineName );
	V_snprintf( m_Chatter[CHATTER_RELOADING], CHATTER_STRING_SIZE, "%s.Reloading", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SENTRY], CHATTER_STRING_SIZE, "%s.SentryInPlace", szMarineName );
	V_snprintf( m_Chatter[CHATTER_AMMO], CHATTER_STRING_SIZE, "%s.Ammo", szMarineName );
	V_snprintf( m_Chatter[CHATTER_MEDKIT], CHATTER_STRING_SIZE, "%s.Medkit", szMarineName );
	V_snprintf( m_Chatter[CHATTER_THANKS], CHATTER_STRING_SIZE, "%s.Thanks", szMarineName );
	V_snprintf( m_Chatter[CHATTER_QUESTION], CHATTER_STRING_SIZE, "%s.Question", szMarineName );
	if ( m_VoiceType != ASW_VOICE_SARGE )
		V_snprintf( m_Chatter[CHATTER_SARGE], CHATTER_STRING_SIZE, "%s.CallSarge", szMarineName );
	if ( m_VoiceType != ASW_VOICE_JAEGER )
		V_snprintf( m_Chatter[CHATTER_JAEGER], CHATTER_STRING_SIZE, "%s.CallJaeger", szMarineName );
	if ( m_VoiceType != ASW_VOICE_WILDCAT )
		V_snprintf( m_Chatter[CHATTER_WILDCAT], CHATTER_STRING_SIZE, "%s.CallWildcat", szMarineName );
	if ( m_VoiceType != ASW_VOICE_WOLFE )
		V_snprintf( m_Chatter[CHATTER_WOLFE], CHATTER_STRING_SIZE, "%s.CallWolfe", szMarineName );
	if ( m_VoiceType != ASW_VOICE_FAITH )
		V_snprintf( m_Chatter[CHATTER_FAITH], CHATTER_STRING_SIZE, "%s.CallFaith", szMarineName );
	if ( m_VoiceType != ASW_VOICE_BASTILLE )
		V_snprintf( m_Chatter[CHATTER_BASTILLE], CHATTER_STRING_SIZE, "%s.CallBastille", szMarineName );
	if ( m_VoiceType != ASW_VOICE_CRASH )
		V_snprintf( m_Chatter[CHATTER_CRASH], CHATTER_STRING_SIZE, "%s.CallCrash", szMarineName );
	if ( m_VoiceType != ASW_VOICE_FLYNN )
		V_snprintf( m_Chatter[CHATTER_FLYNN], CHATTER_STRING_SIZE, "%s.CallFlynn", szMarineName );
	if ( m_VoiceType != ASW_VOICE_VEGAS )
		V_snprintf( m_Chatter[CHATTER_VEGAS], CHATTER_STRING_SIZE, "%s.CallVegas", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SUPPLIES], CHATTER_STRING_SIZE, "%s.Supplies", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SUPPLIES_AMMO], CHATTER_STRING_SIZE, "%s.SuppliesAmmo", szMarineName );
	if ( GetMarineClass() != MARINE_CLASS_TECH )	// techs don't have these lines
	{
		V_snprintf( m_Chatter[CHATTER_LOCKED_TERMINAL], CHATTER_STRING_SIZE, "%s.LockedTerminal", szMarineName );
		V_snprintf( m_Chatter[CHATTER_LOCKED_TERMINAL_CRASH], CHATTER_STRING_SIZE, "%s.LockedTerminalCrash", szMarineName );
		V_snprintf( m_Chatter[CHATTER_LOCKED_TERMINAL_FLYNN], CHATTER_STRING_SIZE, "%s.LockedTerminalFlynn", szMarineName );
		V_snprintf( m_Chatter[CHATTER_LOCKED_TERMINAL_VEGAS], CHATTER_STRING_SIZE, "%s.LockedTerminalVegas", szMarineName );
	}
	V_snprintf( m_Chatter[CHATTER_HOLDING_POSITION], CHATTER_STRING_SIZE, "%s.HoldingPosition", szMarineName );
	V_snprintf( m_Chatter[CHATTER_HOLDING_NORTH], CHATTER_STRING_SIZE, "%s.HoldingNorth", szMarineName );
	V_snprintf( m_Chatter[CHATTER_HOLDING_SOUTH], CHATTER_STRING_SIZE, "%s.HoldingSouth", szMarineName );
	V_snprintf( m_Chatter[CHATTER_HOLDING_EAST], CHATTER_STRING_SIZE, "%s.HoldingEast", szMarineName );
	V_snprintf( m_Chatter[CHATTER_HOLDING_WEST], CHATTER_STRING_SIZE, "%s.HoldingWest", szMarineName );
	V_snprintf( m_Chatter[CHATTER_GOT_POINT], CHATTER_STRING_SIZE, "%s.GotPoint", szMarineName );
	V_snprintf( m_Chatter[CHATTER_GOT_REAR], CHATTER_STRING_SIZE, "%s.GotRear", szMarineName );
	V_snprintf( m_Chatter[CHATTER_REQUEST_SEAL_DOOR], CHATTER_STRING_SIZE, "%s.RequestSealDoor", szMarineName );
	V_snprintf( m_Chatter[CHATTER_REQUEST_CUT_DOOR], CHATTER_STRING_SIZE, "%s.RequestCutDoor", szMarineName );
	V_snprintf( m_Chatter[CHATTER_REQUEST_SHOOT_DOOR], CHATTER_STRING_SIZE, "%s.RequestShootDoor", szMarineName );
	V_snprintf( m_Chatter[CHATTER_CUTTING_DOOR], CHATTER_STRING_SIZE, "%s.CuttingDoor", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SEALING_DOOR], CHATTER_STRING_SIZE, "%s.SealingDoor", szMarineName );
	V_snprintf( m_Chatter[CHATTER_BIOMASS], CHATTER_STRING_SIZE, "%s.Biomass", szMarineName );
	V_snprintf( m_Chatter[CHATTER_TIME_TO_LEAVE], CHATTER_STRING_SIZE, "%s.TimeToLeave", szMarineName );
	V_snprintf( m_Chatter[CHATTER_WATCH_OUT], CHATTER_STRING_SIZE, "%s.WatchOut", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SHIELDBUG], CHATTER_STRING_SIZE, "%s.Shieldbug", szMarineName );
	V_snprintf( m_Chatter[CHATTER_SHIELDBUG_HINT], CHATTER_STRING_SIZE, "%s.ShieldbugHint", szMarineName );
	V_snprintf( m_Chatter[CHATTER_PARASITE], CHATTER_STRING_SIZE, "%s.Parasite", szMarineName );
	V_snprintf( m_Chatter[CHATTER_INFESTED], CHATTER_STRING_SIZE, "%s.BeenInfested", szMarineName );
	V_snprintf( m_Chatter[CHATTER_EGGS], CHATTER_STRING_SIZE, "%s.SwarmEggs", szMarineName );
	V_snprintf( m_Chatter[CHATTER_GRENADE], CHATTER_STRING_SIZE, "%s.Grenade", szMarineName );
	V_snprintf( m_Chatter[CHATTER_ALIEN_TOO_CLOSE], CHATTER_STRING_SIZE, "%s.AlienTooClose", szMarineName );
	V_snprintf( m_Chatter[CHATTER_MAD_FIRING], CHATTER_STRING_SIZE, "%s.MadFiring", szMarineName );
	V_snprintf( m_Chatter[CHATTER_BREACHED_DOOR], CHATTER_STRING_SIZE, "%s.BreachedDoor", szMarineName );
	V_snprintf( m_Chatter[CHATTER_MARINE_DOWN], CHATTER_STRING_SIZE, "%s.MarineDown", szMarineName );
	V_snprintf( m_Chatter[CHATTER_PAIN_SMALL], CHATTER_STRING_SIZE, "%s.SmallPain", szMarineName );
	V_snprintf( m_Chatter[CHATTER_PAIN_LARGE], CHATTER_STRING_SIZE, "%s.LargePain", szMarineName );
	V_snprintf( m_Chatter[CHATTER_DIE], CHATTER_STRING_SIZE, "%s.Dead", szMarineName );
	V_snprintf( m_Chatter[CHATTER_ON_FIRE], CHATTER_STRING_SIZE, "%s.OnFire", szMarineName );
	if ( GetVoiceType() != ASW_VOICE_JAEGER && GetVoiceType() != ASW_VOICE_FAITH && GetVoiceType() != ASW_VOICE_WOLFE )	// everyone but jaeger/faith/wolfe
		V_snprintf( m_Chatter[CHATTER_COMPLIMENTS], CHATTER_STRING_SIZE, "%s.Compliments", szMarineName );
	V_snprintf( m_Chatter[CHATTER_STIM_NOW], CHATTER_STRING_SIZE, "%s.ActivateStims", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_WILDCAT || GetVoiceType() == ASW_VOICE_VEGAS )	// sarge, wildcat and vegas only
		V_snprintf( m_Chatter[CHATTER_IMPATIENCE], CHATTER_STRING_SIZE, "%s.Impatience", szMarineName );

	// don't print the following lines for marines that don't have them
	if ( GetVoiceType() == ASW_VOICE_SARGE )
		V_snprintf( m_Chatter[CHATTER_COMPLIMENTS_JAEGER], CHATTER_STRING_SIZE, "%s.ComplimentsJaeger", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_JAEGER )
		V_snprintf( m_Chatter[CHATTER_COMPLIMENTS_SARGE], CHATTER_STRING_SIZE, "%s.ComplimentsSarge", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WOLFE )
		V_snprintf( m_Chatter[CHATTER_COMPLIMENTS_WILDCAT], CHATTER_STRING_SIZE, "%s.ComplimentsWildcat", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WILDCAT )
		V_snprintf( m_Chatter[CHATTER_COMPLIMENTS_WOLFE], CHATTER_STRING_SIZE, "%s.ComplimentsWolfe", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WILDCAT )
		V_snprintf( m_Chatter[CHATTER_COMPLIMENTS_CRASH], CHATTER_STRING_SIZE, "%s.ComplimentsCrash", szMarineName );

	// medics only
	if ( GetVoiceType() == ASW_VOICE_FAITH || GetVoiceType() == ASW_VOICE_BASTILLE )
	{
		V_snprintf( m_Chatter[CHATTER_HEALING], CHATTER_STRING_SIZE, "%s.Healing", szMarineName );
		if ( GetVoiceType() == ASW_VOICE_BASTILLE )	// bastille only
			V_snprintf( m_Chatter[CHATTER_MEDS_LOW], CHATTER_STRING_SIZE, "%s.LowOnMeds", szMarineName );
		V_snprintf( m_Chatter[CHATTER_MEDS_NONE], CHATTER_STRING_SIZE, "%s.NoMeds", szMarineName );
	}

	// techs only
	if ( GetMarineClass() == MARINE_CLASS_TECH )
	{
		V_snprintf( m_Chatter[CHATTER_HACK_STARTED], CHATTER_STRING_SIZE, "%s.StartedHack", szMarineName );
		V_snprintf( m_Chatter[CHATTER_HACK_LONG_STARTED], CHATTER_STRING_SIZE, "%s.StartedLongHack", szMarineName );
		V_snprintf( m_Chatter[CHATTER_HACK_HALFWAY], CHATTER_STRING_SIZE, "%s.HalfwayHack", szMarineName );
		V_snprintf( m_Chatter[CHATTER_HACK_FINISHED], CHATTER_STRING_SIZE, "%s.FinishedHack", szMarineName );
		V_snprintf( m_Chatter[CHATTER_HACK_BUTTON_FINISHED], CHATTER_STRING_SIZE, "%s.FinishedHackButton", szMarineName );

		if ( GetVoiceType() != ASW_VOICE_VEGAS )
			V_snprintf( m_Chatter[CHATTER_SCANNER], CHATTER_STRING_SIZE, "%s.Scanner", szMarineName );
		V_snprintf( m_Chatter[CHATTER_SCANNER_MULTIPLE], CHATTER_STRING_SIZE, "%s.ScannerMultiple", szMarineName );
	}

	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_JAEGER )
		V_snprintf( m_Chatter[CHATTER_MINE_DEPLOYED], CHATTER_STRING_SIZE, "%s.MineDeployed", szMarineName );

	if ( GetVoiceType() == ASW_VOICE_CRASH || GetVoiceType() == ASW_VOICE_VEGAS )
		V_snprintf( m_Chatter[CHATTER_SYNUP_SPOTTED], CHATTER_STRING_SIZE, "%s.SynUpSpotted", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_FAITH || GetVoiceType() == ASW_VOICE_BASTILLE )
		V_snprintf( m_Chatter[CHATTER_SYNUP_REPLY], CHATTER_STRING_SIZE, "%s.SynUpReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_CRASH )
		V_snprintf( m_Chatter[CHATTER_CRASH_COMPLAIN], CHATTER_STRING_SIZE, "%s.CrashComplain", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_CRASH || GetVoiceType() == ASW_VOICE_BASTILLE )
		V_snprintf( m_Chatter[CHATTER_CRASH_COMPLAIN_REPLY], CHATTER_STRING_SIZE, "%s.CrashComplainReply", szMarineName );
	V_snprintf( m_Chatter[CHATTER_MEDIC_COMPLAIN], CHATTER_STRING_SIZE, "%s.MedicComplain", szMarineName );
	V_snprintf( m_Chatter[CHATTER_MEDIC_COMPLAIN_REPLY], CHATTER_STRING_SIZE, "%s.MedicComplainReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_BASTILLE )
		V_snprintf( m_Chatter[CHATTER_HEALING_CRASH], CHATTER_STRING_SIZE, "%s.HealingCrash", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_CRASH || GetVoiceType() == ASW_VOICE_BASTILLE )
		V_snprintf( m_Chatter[CHATTER_HEALING_CRASH_REPLY], CHATTER_STRING_SIZE, "%s.HealingCrashReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_VEGAS )
	{
		V_snprintf( m_Chatter[CHATTER_TEQUILA_START], CHATTER_STRING_SIZE, "%s.TequilaStart", szMarineName );
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY_SARGE], CHATTER_STRING_SIZE, "%s.TequilaReplySarge", szMarineName );
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY_JAEGER], CHATTER_STRING_SIZE, "%s.TequilaReplyJaeger", szMarineName );
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY_WILDCAT], CHATTER_STRING_SIZE, "%s.TequilaReplyWildcat", szMarineName );
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY_WOLFE], CHATTER_STRING_SIZE, "%s.TequilaReplyWolfe", szMarineName );
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY_FAITH], CHATTER_STRING_SIZE, "%s.TequilaReplyFaith", szMarineName );
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY_BASTILLE], CHATTER_STRING_SIZE, "%s.TequilaBastille", szMarineName );
	}
	if ( GetVoiceType() != ASW_VOICE_FLYNN )	// all but flynn
		V_snprintf( m_Chatter[CHATTER_TEQUILA_REPLY], CHATTER_STRING_SIZE, "%s.TequilaReply", szMarineName );

	if ( GetVoiceType() == ASW_VOICE_CRASH )	 // crash only
		V_snprintf( m_Chatter[CHATTER_CRASH_IDLE], CHATTER_STRING_SIZE, "%s.CrashIdle", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_JAEGER || GetVoiceType() == ASW_VOICE_BASTILLE )	 // sarge/jaeger/bastille
		V_snprintf( m_Chatter[CHATTER_CRASH_IDLE_REPLY], CHATTER_STRING_SIZE, "%s.CrashIdleReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_FAITH || GetVoiceType() == ASW_VOICE_BASTILLE )	// medics only
	{
		V_snprintf( m_Chatter[CHATTER_SERIOUS_INJURY], CHATTER_STRING_SIZE, "%s.SeriousInjury", szMarineName );
		V_snprintf( m_Chatter[CHATTER_SERIOUS_INJURY_FOLLOW_UP], CHATTER_STRING_SIZE, "%s.SeriousInjuryFollowUp", szMarineName );
	}
	V_snprintf( m_Chatter[CHATTER_SERIOUS_INJURY_REPLY], CHATTER_STRING_SIZE, "%s.SeriousInjuryReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_JAEGER )	// jaeger only
		V_snprintf( m_Chatter[CHATTER_STILL_BREATHING], CHATTER_STRING_SIZE, "%s.StillBreathing", szMarineName );
	if ( GetVoiceType() != ASW_VOICE_SARGE && GetVoiceType() != ASW_VOICE_WOLFE && GetVoiceType() != ASW_VOICE_FLYNN )	// everyone but sarge/flynn/wolfe
		V_snprintf( m_Chatter[CHATTER_STILL_BREATHING_REPLY], CHATTER_STRING_SIZE, "%s.StillBreathingReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE )	 // sarge only
		V_snprintf( m_Chatter[CHATTER_SARGE_IDLE], CHATTER_STRING_SIZE, "%s.SargeIdle", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_CRASH )	 // sarge/crash only
		V_snprintf( m_Chatter[CHATTER_SARGE_IDLE_REPLY], CHATTER_STRING_SIZE, "%s.SargeIdleReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_CRASH )	 // crash only
		V_snprintf( m_Chatter[CHATTER_BIG_ALIEN_DEAD], CHATTER_STRING_SIZE, "%s.BigAlienDead", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_CRASH || GetVoiceType() == ASW_VOICE_SARGE )	 // crash/sarge only
		V_snprintf( m_Chatter[CHATTER_BIG_ALIEN_REPLY], CHATTER_STRING_SIZE, "%s.BigAlienDeadReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WILDCAT || GetVoiceType() == ASW_VOICE_WOLFE )	// wildcat/wolfe only
		V_snprintf( m_Chatter[CHATTER_AUTOGUN], CHATTER_STRING_SIZE, "%s.Autogun", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_JAEGER || GetVoiceType() == ASW_VOICE_WILDCAT || GetVoiceType() == ASW_VOICE_WOLFE )	// sarge/jaeger/wildcat/wolfe only
		V_snprintf( m_Chatter[CHATTER_AUTOGUN_REPLY], CHATTER_STRING_SIZE, "%s.AutogunReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WOLFE )	// wolfe only
		V_snprintf( m_Chatter[CHATTER_WOLFE_BEST], CHATTER_STRING_SIZE, "%s.WolfeBest", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WILDCAT || GetVoiceType() == ASW_VOICE_WOLFE )	// wildcat/wolfe only
		V_snprintf( m_Chatter[CHATTER_WOLFE_BEST_REPLY], CHATTER_STRING_SIZE, "%s.WolfeBestReply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_VEGAS ) // vegas only
	{
		V_snprintf( m_Chatter[CHATTER_FIRST_BLOOD_START], CHATTER_STRING_SIZE, "%s.FirstBloodStart", szMarineName );
		V_snprintf( m_Chatter[CHATTER_FIRST_BLOOD_WIN], CHATTER_STRING_SIZE, "%s.FirstBloodWin", szMarineName );
	}
	if ( GetVoiceType() == ASW_VOICE_SARGE )	// sarge only
		V_snprintf( m_Chatter[CHATTER_SARGE_JAEGER_CONV_1], CHATTER_STRING_SIZE, "%s.SargeJaegerConv1", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_JAEGER )	// sarge/jaeger only
		V_snprintf( m_Chatter[CHATTER_SARGE_JAEGER_CONV_1_REPLY], CHATTER_STRING_SIZE, "%s.SargeJaegerConv1Reply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_JAEGER )	// jaeger only
		V_snprintf( m_Chatter[CHATTER_SARGE_JAEGER_CONV_2], CHATTER_STRING_SIZE, "%s.SargeJaegerConv2", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_SARGE || GetVoiceType() == ASW_VOICE_JAEGER )	// sarge/jaeger only
		V_snprintf( m_Chatter[CHATTER_SARGE_JAEGER_CONV_2_REPLY], CHATTER_STRING_SIZE, "%s.SargeJaegerConv2Reply", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WILDCAT ) // wildcat only
		V_snprintf( m_Chatter[CHATTER_WILDCAT_KILL], CHATTER_STRING_SIZE, "%s.WildcatKillStart", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WOLFE ) // wolfe only
		V_snprintf( m_Chatter[CHATTER_WOLFE_KILL], CHATTER_STRING_SIZE, "%s.WolfeKillStart", szMarineName );
	if ( GetVoiceType() == ASW_VOICE_WILDCAT || GetVoiceType() == ASW_VOICE_WOLFE )	// wildcat/wofle only
	{
		V_snprintf( m_Chatter[CHATTER_WILDCAT_KILL_REPLY_AHEAD], CHATTER_STRING_SIZE, "%s.WildcatKillReplyAhead", szMarineName );
		V_snprintf( m_Chatter[CHATTER_WILDCAT_KILL_REPLY_BEHIND], CHATTER_STRING_SIZE, "%s.WildcatKillReplyBehind", szMarineName );
		V_snprintf( m_Chatter[CHATTER_WOLFE_KILL_REPLY_AHEAD], CHATTER_STRING_SIZE, "%s.WolfeKillReplyAhead", szMarineName );
		V_snprintf( m_Chatter[CHATTER_WOLFE_KILL_REPLY_BEHIND], CHATTER_STRING_SIZE, "%s.WolfeKillReplyBehind", szMarineName );
	}
	V_snprintf( m_Chatter[CHATTER_MISC], CHATTER_STRING_SIZE, "%s.Misc", szMarineName );

	// read speech counts in
	char szKeyName[64];
	char szFileName[128];
	V_snprintf( szKeyName, sizeof( szKeyName ), "%sChatterCount", szMarineName );
	V_snprintf( szFileName, sizeof( szFileName ), "scripts/asw_speech_count_%s.txt", szMarineName );
	KeyValues::AutoDelete pCountKeyValues{ szKeyName };
	if ( UTIL_RD_LoadKeyValuesFromFile( pCountKeyValues, filesystem, szFileName ) )
	{
		// now go down all the chatters, grabbing the count
		for ( int i = 0; i < NUM_CHATTER_LINES; i++ )
		{
			int iCount = pCountKeyValues->GetInt( m_Chatter[i] );
			iCount = MIN( iCount, NUM_SUB_CHATTERS );
			m_iChatterCount[i] = iCount;
		}
	}

#ifndef CLIENT_DLL
	// setup chatter chances
	m_fChatterChance[CHATTER_HOLDING_POSITION] = 0.2f;
	m_fChatterChance[CHATTER_HOLDING_NORTH] = 0.2f;
	m_fChatterChance[CHATTER_HOLDING_SOUTH] = 0.2f;
	m_fChatterChance[CHATTER_HOLDING_EAST] = 0.2f;
	m_fChatterChance[CHATTER_HOLDING_WEST] = 0.2f;
	//m_fChatterChance[CHATTER_SYNUP_SPOTTED] = 1.0f;
	m_fChatterChance[CHATTER_HEALING] = 0.5f;

	// read speech durations in
	V_snprintf( szKeyName, sizeof( szKeyName ), "%sChatterDuration", szMarineName );
	V_snprintf( szFileName, sizeof( szFileName ), "scripts/asw_speech_duration_%s.txt", szMarineName );
	KeyValues::AutoDelete pDurationKeyValues{ szKeyName };
	if ( UTIL_RD_LoadKeyValuesFromFile( pDurationKeyValues, filesystem, szFileName ) )
	{
		// now go down all the chatters, grabbing the count
		for ( int i = 0; i < NUM_CHATTER_LINES; i++ )
		{
			for ( int j = 0; j < m_iChatterCount[i]; j++ )
			{
				char chatterbuffer[128];
				V_snprintf( chatterbuffer, sizeof( chatterbuffer ), "%s%d", m_Chatter[i], j );
				m_fChatterDuration[i][j] = pDurationKeyValues->GetFloat( chatterbuffer );
			}
		}
	}

#endif

	// clear the lines that some marines don't have
	if ( GetVoiceType() == ASW_VOICE_SARGE )
	{
		m_Chatter[CHATTER_SARGE][0] = '\0';
		m_Chatter[CHATTER_COMPLIMENTS_SARGE][0] = '\0';
		m_Chatter[CHATTER_COMPLIMENTS_WILDCAT][0] = '\0';
		m_Chatter[CHATTER_COMPLIMENTS_WOLFE][0] = '\0';
		m_Chatter[CHATTER_COMPLIMENTS_CRASH][0] = '\0';
	}
}

#ifndef CLIENT_DLL
void CASW_Marine_Profile::PrecacheSpeech(CBaseEntity *pEnt)
{
	if ( asw_precache_speech.GetBool() )
	{
		for ( int i = 0; i < NUM_CHATTER_LINES; i++ )
		{
			for ( int j = 0; j < m_iChatterCount[i]; j++ )
			{
				if ( m_Chatter[i][0] != '\0' )
				{
					char buffer[128];
					V_snprintf( buffer, sizeof( buffer ), "%s%d", m_Chatter[i], j );
					pEnt->PrecacheScriptSound( buffer );
				}
			}
		}
	}

	m_nResponseRulesName = ResponseRules::CriteriaSet::ComputeCriteriaSymbol( m_ResponseRulesName );

	// precache the model here too
	pEnt->PrecacheModel( m_ModelName );
}
#endif

void CASW_Marine_ProfileList::PrecacheSpeech( CBaseEntity *pEnt )
{
#ifndef CLIENT_DLL
	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		m_Profiles[i].PrecacheSpeech( pEnt );
	}
#endif
}

#ifndef CLIENT_DLL
// These functions measure the duration of each speech line and save it to a file.
//   This is done so the server can know the duration of the speech wavs without actually loading in all the wav files.
void CASW_Marine_ProfileList::SaveSpeechDurations( CBaseEntity *pEnt )
{
	if ( !asw_precache_speech.GetBool() )
	{
		Msg( "Please set asw_precache_speech to 1 and reload the map before building speech durations.  If you do not do this, they will build incorrectly with zero duration." );
		return;
	}
	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		m_Profiles[i].SaveSpeechDurations( pEnt );
	}
}

// saves out a file containing speech durations
void CASW_Marine_Profile::SaveSpeechDurations( CBaseEntity *pEnt )
{
	// precache all speech
	for ( int i = 0; i < NUM_CHATTER_LINES; i++ )
	{
		for ( int j = 0; j < m_iChatterCount[i]; j++ )
		{
			if ( m_Chatter[i][0] != '\0' )
			{
				char chatterbuffer[128];
				V_snprintf( chatterbuffer, sizeof( chatterbuffer ), "%s%d", m_Chatter[i], j );
				pEnt->PrecacheScriptSound( chatterbuffer );
			}
		}
	}
	// create keyvalues
	char szKeyName[64];
	char szFileName[128];
	V_snprintf( szKeyName, sizeof( szKeyName ), "%sChatterDuration", GetVoiceShortName( GetVoiceType() ) );
	V_snprintf( szFileName, sizeof( szFileName ), "scripts/asw_speech_duration_%s.txt", GetVoiceShortName( GetVoiceType() ) );
	KeyValues::AutoDelete pDurationKeyValues{ szKeyName };

	// now go down all the chatters, grabbing the duration
	for ( int i = 0; i < NUM_CHATTER_LINES; i++ )
	{
		for ( int j = 0; j < m_iChatterCount[i]; j++ )
		{
			char chatterbuffer[128];
			V_snprintf( chatterbuffer, sizeof( chatterbuffer ), "%s%d", m_Chatter[i], j );
			float duration = 1.0f;
			CSoundParameters params;
			if ( pEnt->GetParametersForSound( chatterbuffer, params, NULL ) )
			{
				if ( !params.soundname[0] )
				{
					pDurationKeyValues->deleteThis();
					return;
				}

				char *skipped = PSkipSoundChars( params.soundname );
				duration = enginesound->GetSoundDuration( skipped );
				Msg( "Duration for sound %s is %f\n", skipped, duration );
			}
			pDurationKeyValues->SetFloat( chatterbuffer, duration );
		}
	}
	pDurationKeyValues->SaveToFile( filesystem, szFileName );
}
#endif
ASW_Marine_Class CASW_Marine_Profile::GetMarineClass()
{
	return (ASW_Marine_Class) m_iMarineClass;
}
ASW_Voice_Type CASW_Marine_Profile::GetVoiceType()
{
	return m_VoiceType;
}

ASW_Skill CASW_Marine_Profile::GetSkillMapping( int nSkillSlot )
{
	if ( nSkillSlot < 0 || nSkillSlot >= ASW_NUM_SKILL_SLOTS )
		return ASW_MARINE_SKILL_INVALID;

	if ( nSkillSlot == ASW_SKILL_SLOT_SPARE )
		return ASW_MARINE_SKILL_SPARE;

	return ASW_Skill( asw_marine_skill[m_ProfileIndex][nSkillSlot].GetInt() );
}

int CASW_Marine_Profile::GetStaticSkillPoints( int nSkillSlot )
{
	if ( nSkillSlot < 0 || nSkillSlot >= ASW_NUM_SKILL_SLOTS )
		return 0;

	if ( nSkillSlot == ASW_SKILL_SLOT_SPARE )
		return 0;

	return ASW_Skill( asw_marine_skill_points[m_ProfileIndex][nSkillSlot].GetInt() );
}
