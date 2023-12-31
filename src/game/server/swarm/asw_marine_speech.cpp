#include "cbase.h"
#include "asw_marine_speech.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_gamerules.h"
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "soundchars.h"
#include "ndebugoverlay.h"
#include "asw_game_resource.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ISoundEmitterSystemBase *soundemitterbase;

// chatter probabilites (1/x)
#define BASE_CHATTER_SELECTION_CHANCE 5.0f
#define CHATTER_INTERVAL 3.0f
#define ASW_INTERVAL_CHATTER_INCOMING 30.0f
#define ASW_INTERVAL_CHATTER_NO_AMMO 30.0f
#define ASW_INTERVAL_FF 12.0f

#define ASW_CALM_CHATTER_TIME 10.0f

// chance of doing a direction hold position shout (from 0 to 1)
#define ASW_DIRECTIONAL_HOLDING_CHATTER 0.5f

ConVar asw_debug_marine_chatter("asw_debug_marine_chatter", "0", 0, "Show debug info about when marine chatter is triggered");
ConVar rd_marine_chatter_enabled("rd_marine_chatter_enabled", "1", 0, "If 0 marines will not speak anything");

#define ACTOR_SARGE (1 << 0)
#define ACTOR_WILDCAT (1 << 1)
#define ACTOR_FAITH (1 << 2)
#define ACTOR_CRASH (1 << 3)
#define ACTOR_JAEGER (1 << 4)
#define ACTOR_WOLFE (1 << 5)
#define ACTOR_BASTILLE (1 << 6)
#define ACTOR_VEGAS (1 << 7)
#define ACTOR_FLYNN (1 << 8)
#define ACTOR_ALL (ACTOR_SARGE | ACTOR_JAEGER | ACTOR_WILDCAT | ACTOR_WOLFE | ACTOR_FAITH | ACTOR_BASTILLE | ACTOR_CRASH | ACTOR_FLYNN)

int CASW_MarineSpeech::s_CurrentConversation = CONV_NONE;
int CASW_MarineSpeech::s_CurrentConversationChatterStage = -1;
int CASW_MarineSpeech::s_iCurrentConvLine = -1;
CHandle<CASW_Marine> CASW_MarineSpeech::s_hActor1 = NULL;
CHandle<CASW_Marine> CASW_MarineSpeech::s_hActor2 = NULL;
CHandle<CASW_Marine> CASW_MarineSpeech::s_hCurrentActor = NULL;
static int s_Actor1Indices[ASW_NUM_CONVERSATIONS]={
	0, // CONV_NONE,
	ACTOR_CRASH | ACTOR_VEGAS, // CONV_SYNUP,
	ACTOR_CRASH, // CONV_CRASH_COMPLAIN,
	ACTOR_FAITH | ACTOR_BASTILLE, // CONV_MEDIC_COMPLAIN,
	ACTOR_BASTILLE, // CONV_HEALING_CRASH
	ACTOR_VEGAS, // CONV_TEQUILA,
	ACTOR_CRASH, // CONV_CRASH_IDLE,
	ACTOR_FAITH | ACTOR_BASTILLE, // CONV_SERIOUS_INJURY,
	ACTOR_JAEGER, // CONV_STILL_BREATHING,
	ACTOR_SARGE, // CONV_SARGE_IDLE,
	ACTOR_CRASH, // CONV_BIG_ALIEN,
	ACTOR_WILDCAT | ACTOR_WOLFE, // CONV_AUTOGUN
	ACTOR_WOLFE, // CONV_WOLFE_BEST,
	ACTOR_SARGE, // CONV_SARGE_JAEGER_CONV1, (jaeger starts)
	ACTOR_JAEGER, // CONV_SARGE_JAEGER_CONV2,  (sarge starts)
	ACTOR_WILDCAT, // CONV_WILDCAT_KILL,
	ACTOR_WOLFE, // CONV_WOLFE_KILL
	ACTOR_SARGE, //CONV_COMPLIMENT_JAEGER, // 17
	ACTOR_JAEGER, //CONV_COMPLIMENT_SARGE, // 18
	ACTOR_WILDCAT, //CONV_COMPLIMENT_WOLFE, // 19
	ACTOR_WOLFE, //CONV_COMPLIMENT_WILDCAT, // 20
};

static int s_Actor2Indices[ASW_NUM_CONVERSATIONS]={
	0, // CONV_NONE,
	ACTOR_FAITH | ACTOR_BASTILLE, // CONV_SYNUP,
	ACTOR_BASTILLE, // CONV_CRASH_COMPLAIN,
	ACTOR_ALL, // CONV_MEDIC_COMPLAIN,
	ACTOR_CRASH, // CONV_HEALING_CRASH
	ACTOR_ALL &~ ACTOR_FLYNN, // CONV_TEQUILA,
	ACTOR_SARGE | ACTOR_JAEGER | ACTOR_BASTILLE, // CONV_CRASH_IDLE,
	ACTOR_ALL, // CONV_SERIOUS_INJURY,
	ACTOR_ALL &~ (ACTOR_SARGE | ACTOR_FLYNN | ACTOR_WOLFE), // CONV_STILL_BREATHING,
	ACTOR_SARGE | ACTOR_CRASH, // CONV_SARGE_IDLE,
	ACTOR_SARGE, // CONV_BIG_ALIEN,	
	ACTOR_SARGE | ACTOR_JAEGER, // CONV_AUTOGUN
	ACTOR_WILDCAT, // CONV_WOLFE_BEST,

	ACTOR_JAEGER, // CONV_SARGE_JAEGER_CONV1,  (sarge is 2nd)
	ACTOR_SARGE, // CONV_SARGE_JAEGER_CONV2, (jaeger is 2nd)
	ACTOR_WOLFE, // CONV_WILDCAT_KILL,
	ACTOR_WILDCAT, // CONV_WOLFE_KILL
	ACTOR_JAEGER, //CONV_COMPLIMENT_JAEGER, // 17
	ACTOR_SARGE, //CONV_COMPLIMENT_SARGE, // 18
	ACTOR_WOLFE, //CONV_COMPLIMENT_WOLFE, // 19
	ACTOR_WILDCAT, //CONV_COMPLIMENT_WILDCAT, // 20
};

static int s_ConvChatterLine1[ASW_NUM_CONVERSATIONS]={
	-1, // CONV_NONE,
	CHATTER_SYNUP_SPOTTED, // CONV_SYNUP,
	CHATTER_CRASH_COMPLAIN, // CONV_CRASH_COMPLAIN,
	CHATTER_MEDIC_COMPLAIN, // CONV_MEDIC_COMPLAIN,
	CHATTER_HEALING_CRASH, // CONV_HEALING_CRASH
	CHATTER_TEQUILA_START, // CONV_TEQUILA,
	CHATTER_CRASH_IDLE, // CONV_CRASH_IDLE,
	CHATTER_SERIOUS_INJURY, // CONV_SERIOUS_INJURY,
	CHATTER_STILL_BREATHING, // CONV_STILL_BREATHING,
	CHATTER_SARGE_IDLE, // CONV_SARGE_IDLE,
	CHATTER_BIG_ALIEN_DEAD, // CONV_BIG_ALIEN,
	CHATTER_AUTOGUN, // CONV_AUTOGUN
	CHATTER_WOLFE_BEST, // CONV_WOLFE_BEST,
	CHATTER_SARGE_JAEGER_CONV_1, // CONV_SARGE_JAEGER_CONV1, (jaeger starts)
	CHATTER_SARGE_JAEGER_CONV_2, // CONV_SARGE_JAEGER_CONV2,  (sarge starts)
	CHATTER_WILDCAT_KILL, // CONV_WILDCAT_KILL,
	CHATTER_WOLFE_KILL, // CONV_WOLFE_KILL	
	CHATTER_COMPLIMENTS_JAEGER, //CONV_COMPLIMENT_JAEGER, // 17
	CHATTER_COMPLIMENTS_SARGE, //CONV_COMPLIMENT_SARGE, // 18
	CHATTER_COMPLIMENTS_WOLFE, //CONV_COMPLIMENT_WOLFE, // 19
	CHATTER_COMPLIMENTS_WILDCAT, //CONV_COMPLIMENT_WILDCAT, // 20
};

static int s_ConvChatterLine2[ASW_NUM_CONVERSATIONS]={
	-1, // CONV_NONE,
	CHATTER_SYNUP_REPLY, // CONV_SYNUP,
	CHATTER_CRASH_COMPLAIN_REPLY, // CONV_CRASH_COMPLAIN,
	CHATTER_MEDIC_COMPLAIN_REPLY, // CONV_MEDIC_COMPLAIN,
	CHATTER_HEALING_CRASH_REPLY, // CONV_HEALING_CRASH
	CHATTER_TEQUILA_REPLY, // CONV_TEQUILA,
	CHATTER_CRASH_IDLE_REPLY, // CONV_CRASH_IDLE,
	CHATTER_SERIOUS_INJURY_REPLY, // CONV_SERIOUS_INJURY,
	CHATTER_STILL_BREATHING_REPLY, // CONV_STILL_BREATHING,
	CHATTER_SARGE_IDLE_REPLY, // CONV_SARGE_IDLE,
	CHATTER_BIG_ALIEN_REPLY, // CONV_BIG_ALIEN,
	CHATTER_AUTOGUN_REPLY, // CONV_AUTOGUN
	CHATTER_WOLFE_BEST_REPLY, // CONV_WOLFE_BEST,
	CHATTER_SARGE_JAEGER_CONV_1_REPLY, // CONV_SARGE_JAEGER_CONV1, (jaeger starts)
	CHATTER_SARGE_JAEGER_CONV_2_REPLY, // CONV_SARGE_JAEGER_CONV2,  (sarge starts)
	CHATTER_WILDCAT_KILL_REPLY_AHEAD, // CONV_WILDCAT_KILL,  NOTE: OR BEHIND...
	CHATTER_WOLFE_KILL_REPLY_AHEAD, // CONV_WOLFE_KILL
	-1, //CONV_COMPLIMENT_JAEGER, // 17
	-1, //CONV_COMPLIMENT_SARGE, // 18
	-1, //CONV_COMPLIMENT_WOLFE, // 19
	-1, //CONV_COMPLIMENT_WILDCAT, // 20
};

static int s_ConvNumStages[ASW_NUM_CONVERSATIONS]={
	0, // CONV_NONE,
	2, // CONV_SYNUP,
	3, // CONV_CRASH_COMPLAIN,
	2, // CONV_MEDIC_COMPLAIN,
	3, // CONV_HEALING_CRASH
	3, // CONV_TEQUILA,
	2, // CONV_CRASH_IDLE,
	3, // CONV_SERIOUS_INJURY,
	3, // CONV_STILL_BREATHING,
	3, // CONV_SARGE_IDLE,
	3, // CONV_BIG_ALIEN,		// todo:should be 4?
	3, // CONV_AUTOGUN
	3, // CONV_WOLFE_BEST,
	3, // CONV_SARGE_JAEGER_CONV1, (jaeger starts)
	3, // CONV_SARGE_JAEGER_CONV2,  (sarge starts)
	3, // CONV_WILDCAT_KILL,  NOTE: OR BEHIND...
	3, // CONV_WOLFE_KILL
	1, //CONV_COMPLIMENT_JAEGER, // 17
	1, //CONV_COMPLIMENT_SARGE, // 18
	1, //CONV_COMPLIMENT_WOLFE, // 19
	1, //CONV_COMPLIMENT_WILDCAT, // 20
};

/*
	CHATTER_TEQUILA_REPLY_SARGE, // Vegas only
	CHATTER_TEQUILA_REPLY_JAEGER, // Vegas only
	CHATTER_TEQUILA_REPLY_WILDCAT, // Vegas only
	CHATTER_TEQUILA_REPLY_WOLFE, // Vegas only
	CHATTER_TEQUILA_REPLY_FAITH, // Vegas only
	CHATTER_TEQUILA_REPLY_BASTILLE, // Vegas only
		
	CHATTER_SERIOUS_INJURY_FOLLOW_UP,   // faith/bastille only

	CHATTER_FIRST_BLOOD_START,	// vegas only
	CHATTER_FIRST_BLOOD_WIN,	// vegas only
		
	CHATTER_WILDCAT_KILL_REPLY_BEHIND,	
	CHATTER_WOLFE_KILL_REPLY_BEHIND,
*/

bool CASW_MarineSpeech::StartConversation(int iConversation, CASW_Marine *pMarine, CASW_Marine *pSecondMarine)
{
	if (!pMarine || !pMarine->GetMarineProfile() || iConversation < 0 || iConversation > CONV_COMPLIMENT_WILDCAT
		|| pMarine->GetHealth() <= 0 || pMarine->IsInfested() || pMarine->IsOnFire())
		return false;

	if ( !ASWGameResource() )
		return false;
	
	CASW_Game_Resource *pGameResource = ASWGameResource();

	// check this marine can start this conversation
	if (! ((1 << pMarine->GetMarineProfile()->m_ProfileIndex) & s_Actor1Indices[iConversation]) )
		return false;

	CASW_Marine *pChosen = NULL;
	if (pSecondMarine != NULL && pSecondMarine->GetMarineProfile()!=NULL)
	{
		// if the other actor was specified, check they can do the conversation
		if (! ((1 << pSecondMarine->GetMarineProfile()->m_ProfileIndex) & s_Actor2Indices[iConversation]) )
			return false;
		if (pSecondMarine->IsInfested() || pMarine->IsOnFire())
			return false;
		pChosen = pSecondMarine;
	}
	else
	{
		// try and find another actor to chat with
		int iNearby = 0;
		for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
			CASW_Marine *pOtherMarine = pMR ? pMR->GetMarineEntity() : NULL;
			if (pOtherMarine && pOtherMarine != pMarine && (pOtherMarine->GetAbsOrigin().DistTo(pMarine->GetAbsOrigin()) < 600)
						&& pOtherMarine->GetHealth() > 0 && !pOtherMarine->IsInfested() && !pOtherMarine->IsOnFire() && pOtherMarine->GetMarineProfile() 
						&& ((1 << pOtherMarine->GetMarineProfile()->m_ProfileIndex) & s_Actor2Indices[iConversation]) )
				iNearby++;
		}
		if (iNearby > 0)
		{
			int iChosen = random->RandomInt(0, iNearby-1);		
			for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
				CASW_Marine *pOtherMarine = pMR ? pMR->GetMarineEntity() : NULL;
				if (pOtherMarine && pOtherMarine != pMarine && (pOtherMarine->GetAbsOrigin().DistTo(pMarine->GetAbsOrigin()) < 600)
							&& pOtherMarine->GetHealth() > 0 && !pOtherMarine->IsInfested() && !pOtherMarine->IsOnFire() && pOtherMarine->GetMarineProfile() 
							&& ((1 << pOtherMarine->GetMarineProfile()->m_ProfileIndex) & s_Actor2Indices[iConversation]) )
				{
					if (iChosen <= 0)
					{
						pChosen = pOtherMarine;
						break;
					}
					iChosen--;
				}
			}
		}
	}
	if (!pChosen && iConversation != CONV_SYNUP)	// synup doesn't need 2 people to start the conv
		return false;

	if (asw_debug_marine_chatter.GetBool())
		Msg("%f: Starting conv %d successfully, between marines %s and %s\n", gpGlobals->curtime, iConversation, pMarine->GetMarineProfile()->m_ShortName,
			pChosen ? pChosen->GetMarineProfile()->m_ShortName : "no-one");
	s_CurrentConversation = iConversation;
	s_CurrentConversationChatterStage = 0;
	s_hCurrentActor = pMarine;
	s_hActor1 = pMarine;
	s_hActor2 = pChosen;
	s_iCurrentConvLine = s_ConvChatterLine1[iConversation];
	pMarine->GetMarineSpeech()->QueueChatter(s_iCurrentConvLine, gpGlobals->curtime, gpGlobals->curtime + 5.0f);
	return true;
}

void CASW_MarineSpeech::StopConversation()
{
	if (asw_debug_marine_chatter.GetBool())
		Msg("Stopping conversation");
	s_CurrentConversation = CONV_NONE;
	s_CurrentConversationChatterStage = 0;
	s_hCurrentActor = NULL;
	s_hActor1 = NULL;
	s_hActor2 = NULL;
}

CASW_MarineSpeech::CASW_MarineSpeech(CASW_Marine *pMarine)
{	
	m_pMarine = pMarine;
	m_fPersonalChatterTime = 0;
	m_SpeechQueue.Purge();
	m_fFriendlyFireTime = 0;
}

CASW_MarineSpeech::~CASW_MarineSpeech()
{

}

bool CASW_MarineSpeech::TranslateChatter(int &iChatterType, CASW_Marine_Profile *pProfile)
{	
	if (!pProfile)
		return false;
	// special holding position translations
	if (iChatterType == CHATTER_HOLDING_POSITION)
	{
		if (random->RandomFloat() < ASW_DIRECTIONAL_HOLDING_CHATTER)
			FindDirectionalHoldingChatter(iChatterType);
	}
	// random chance of playing some chatters
	else if (iChatterType == CHATTER_SELECTION)
	{	
		// check for using injured version
		if (m_pMarine->IsWounded())
			iChatterType = CHATTER_SELECTION_INJURED;
	}	

	return true;
}

bool CASW_MarineSpeech::ForceChatter(int iChatterType, int iTimerType)
{
	if (!m_pMarine || !m_pMarine->GetMarineResource() || (m_pMarine->GetHealth()<=0 && iChatterType != CHATTER_DIE))	
		return false;
	
	CASW_Marine_Profile *pProfile = m_pMarine->GetMarineResource()->GetProfile();
	if (!pProfile)
		return false;

	if (!TranslateChatter(iChatterType, pProfile))
		return false;

	// check for discarding chatter if it only has a random chance of playing and we don't hit
	if (random->RandomFloat() > pProfile->m_fChatterChance[iChatterType])
		return true;

	// if marine doesn't have this line, then don't try playing it
	const char *szChatter = pProfile->m_Chatter[iChatterType];
	if (szChatter[0] == '\0')
		return false;
		
	InternalPlayChatter(m_pMarine, szChatter, iTimerType, iChatterType);
	return true;
}

bool CASW_MarineSpeech::Chatter(int iChatterType, int iSubChatter, CBasePlayer *pOnlyForPlayer)
{
	if (iChatterType < 0 || iChatterType >= NUM_CHATTER_LINES)
	{
		AssertMsg1( false, "Faulty chatter type %d\n", iChatterType );
		return false;
	}

	if (!m_pMarine || !m_pMarine->GetMarineResource() || (m_pMarine->GetHealth()<=0 && iChatterType != CHATTER_DIE))	
	{
		AssertMsg( false, "Absent marine tried to chatter\n" );
		return false;
	}
	
	CASW_Marine_Profile *pProfile = m_pMarine->GetMarineResource()->GetProfile();
	if (!pProfile)
	{
		AssertMsg1( false, "Marine %s is missing a profile\n", m_pMarine->GetDebugName() );
		return false;
	}

	if (gpGlobals->curtime < GetTeamChatterTime())
		return false;
		
	if (!TranslateChatter(iChatterType, pProfile))
		return false;

	// check for discarding chatter if some marines are in the middle of a conversation
	if (s_CurrentConversation != CONV_NONE)
	{
		CASW_Marine *pCurrentActor = (s_CurrentConversationChatterStage == 1) ? s_hActor2.Get() : s_hActor1.Get();
		int iCurrentConvLine = s_iCurrentConvLine;
		// if this isn't the right speaker saying the right line...
		if (iChatterType != iCurrentConvLine || m_pMarine != pCurrentActor)
			return false;
	}

	// check for discarding chatter if it only has a random chance of playing and we don't hit
	if (random->RandomFloat() > pProfile->m_fChatterChance[iChatterType])
		return true;

	if (iChatterType == CHATTER_FIRING_AT_ALIEN)
	{
		if (gpGlobals->curtime < GetIncomingChatterTime())
			return false;

		SetIncomingChatterTime(gpGlobals->curtime + ASW_INTERVAL_CHATTER_INCOMING);
	}

	// if marine doesn't have this line, then don't try playing it
	const char *szChatter = pProfile->m_Chatter[iChatterType];
	if (szChatter[0] == '\0')
		return false;

	InternalPlayChatter(m_pMarine, szChatter, ASW_CHATTER_TIMER_TEAM, iChatterType, iSubChatter, pOnlyForPlayer);
	return true;
}

bool CASW_MarineSpeech::PersonalChatter(int iChatterType)
{
	if (iChatterType < 0 || iChatterType >= NUM_CHATTER_LINES)
		return false;

	if (!m_pMarine || !m_pMarine->GetMarineResource() || (m_pMarine->GetHealth()<=0 && iChatterType != CHATTER_DIE))
		return false;
	
	CASW_Marine_Profile *pProfile = m_pMarine->GetMarineResource()->GetProfile();
	if (!pProfile)
		return false;

	if (gpGlobals->curtime < GetPersonalChatterTime())
		return false;	
	
	if (!TranslateChatter(iChatterType, pProfile))
		return false;

	// check for discarding chatter if it only has a random chance of playing and we don't hit
	if (random->RandomFloat() > pProfile->m_fChatterChance[iChatterType])
		return true;

	// if marine doesn't have this line, then don't try playing it
	const char *szChatter = pProfile->m_Chatter[iChatterType];
	if (szChatter[0] == '\0')
		return false;

	InternalPlayChatter(m_pMarine, szChatter, ASW_CHATTER_TIMER_PERSONAL, iChatterType);
	return true;
}


void CASW_MarineSpeech::InternalPlayChatter(CASW_Marine* pMarine, const char* szSoundName, int iSetTimer, int iChatterType, int iSubChatter, CBasePlayer *pOnlyForPlayer)
{
	if (!rd_marine_chatter_enabled.GetBool())
		return;

	if (!pMarine || !ASWGameRules() || iSubChatter >= NUM_SUB_CHATTERS)
		return;

	if (ASWGameRules()->GetGameState() > ASW_GS_INGAME)
		return;

	// only allow certain chatters when marine is on fire
	if (pMarine->IsOnFire())
	{
		if (iChatterType != CHATTER_FRIENDLY_FIRE &&
			iChatterType != CHATTER_MEDIC &&
			iChatterType != CHATTER_INFESTED &&
			iChatterType != CHATTER_PAIN_SMALL &&
			iChatterType != CHATTER_PAIN_LARGE &&
			iChatterType != CHATTER_DIE &&
			iChatterType != CHATTER_ON_FIRE)
			return;
	}

	if (asw_debug_marine_chatter.GetBool())
		Msg("%s playing chatter %s %d\n", pMarine->GetMarineProfile()->m_ShortName, szSoundName, iChatterType);

	// select a sub chatter
	int iMaxChatter = pMarine->GetMarineProfile()->m_iChatterCount[iChatterType];
	if (iMaxChatter <= 0)
	{
		Msg("Warning, couldn't play chatter of type %d as no sub chatters of that type for this marine\n", iChatterType);
		return;
	}
	if (iSubChatter < 0)
		iSubChatter = random->RandomInt(0, iMaxChatter - 1);

	// asw temp comment
	//CPASAttenuationFilter filter( pMarine );		
	//CSoundParameters params;
	//char chatterbuffer[128];
	//Q_snprintf(chatterbuffer, sizeof(chatterbuffer), "%s%d", szSoundName, iSubChatter);
	//if ( pMarine->GetParametersForSound( chatterbuffer, params, NULL ) )
	{
		//if (!params.soundname[0])
			//return;

		if (iSetTimer != ASW_CHATTER_TIMER_NONE)
		{
			// asw temp comment
			//char *skipped = PSkipSoundChars( params.soundname );
			//float duration = enginesound->GetSoundDuration( skipped );
			//Msg("ingame duration for sound %s is %f\n", skipped, duration);
			float duration = pMarine->GetMarineProfile()->m_fChatterDuration[iChatterType][iSubChatter];
			if (iSetTimer == ASW_CHATTER_TIMER_TEAM)
			{
				// make sure no one else talks until this is done
				SetTeamChatterTime(gpGlobals->curtime + duration);
				SetPersonalChatterTime(gpGlobals->curtime + duration);
			}
			else	// ASW_CHATTER_TIMER_PERSONAL
			{
				SetPersonalChatterTime(gpGlobals->curtime + duration);
			}
		}

		//EmitSound_t ep( params );
		//pMarine->EmitSound( filter, pMarine->entindex(), ep );
		
		
		// user messages based speech
		CEffectData	data;

		data.m_vOrigin = pMarine->GetAbsOrigin();
		data.m_nEntIndex = pMarine->entindex();
		data.m_nDamageType = iChatterType;
		data.m_nMaterial = iSubChatter;

		if (!pOnlyForPlayer)
		{
			CPASFilter filter( data.m_vOrigin );
			filter.SetIgnorePredictionCull(true);
			DispatchEffect( filter, 0.0, "ASWSpeech", data );
		}
		else
		{
			CSingleUserRecipientFilter filter( pOnlyForPlayer );
			filter.SetIgnorePredictionCull(true);
			DispatchEffect( filter, 0.0, "ASWSpeech", data );
		}
	}
}

void CASW_MarineSpeech::Precache()
{
	if (!m_pMarine || !m_pMarine->GetMarineResource())
	{
		//Msg("Error: Attempted to precache marine speech when either marine is null or marine has no marine resource.");
		return;
	}
	
	CASW_Marine_Profile *profile= m_pMarine->GetMarineResource()->GetProfile();
	// asw temp comment of speech
	profile->PrecacheSpeech(m_pMarine);
}

float CASW_MarineSpeech::GetTeamChatterTime()
{
	if (!ASWGameRules())
		return 0;

	return ASWGameRules()->GetChatterTime();
}

float CASW_MarineSpeech::GetIncomingChatterTime()
{
	if (!ASWGameRules())
		return 0;

	return ASWGameRules()->GetIncomingChatterTime();
}

void CASW_MarineSpeech::SetTeamChatterTime(float f)
{
	if ( asw_debug_marine_chatter.GetBool() )
		Msg( "%f:SetTeamChatterTime %f\n", gpGlobals->curtime, f );
	if (ASWGameRules())
		ASWGameRules()->SetChatterTime(f);
}

void CASW_MarineSpeech::SetIncomingChatterTime(float f)
{
	if ( asw_debug_marine_chatter.GetBool() )
		Msg( "%f:SetIncomingChatterTime %f\n", gpGlobals->curtime, f );
	if (ASWGameRules())
		ASWGameRules()->SetIncomingChatterTime(f);
}

void CASW_MarineSpeech::SetPersonalChatterTime(float f)
{
	if ( asw_debug_marine_chatter.GetBool() )
		Msg( "%f:SetPersonalChatterTime %f\n", gpGlobals->curtime, f );
	m_fPersonalChatterTime = f;
}

bool CASW_MarineSpeech::FindDirectionalHoldingChatter(int &iChatterType)
{
	if ( !m_pMarine )
		return false;

	Vector vecFacing;
	QAngle angHolding(0, m_pMarine->GetHoldingYaw(), 0);
	AngleVectors(angHolding, &vecFacing);

	// first check there's no other marines standing in front of us
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return false;
	int m = pGameResource->GetMaxMarineResources();
	for (int i=0;i<m;i++)
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
		if (!pMR || !pMR->GetMarineEntity())
			continue;

		CASW_Marine *pOtherMarine = pMR->GetMarineEntity();
		Vector diff = pOtherMarine->GetAbsOrigin() - m_pMarine->GetAbsOrigin();
		if (diff.Length() > 1000)	// if they're too far away, don't count them as blocking our covering area
			continue;

		// check they're in front
		diff.z = 0;
		diff.NormalizeInPlace();
		if (diff.Dot(vecFacing) < 0.3f)
			continue;

		// this marine is in front, so we can't do a directional shout
		return false;
	}

	// do a few traces to roughly check we're not facing a wall
	trace_t tr;
	if (asw_debug_marine_chatter.GetBool())
		NDebugOverlay::Line( m_pMarine->WorldSpaceCenter(), m_pMarine->WorldSpaceCenter() + vecFacing * 300.0f, 255, 0, 0, false, 5.0f );
	UTIL_TraceLine(m_pMarine->WorldSpaceCenter(), m_pMarine->WorldSpaceCenter() + vecFacing * 300.0f, MASK_SOLID, m_pMarine, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction < 1.0f)
		return false;

	angHolding[YAW] += 30;
	AngleVectors(angHolding, &vecFacing);
	if (asw_debug_marine_chatter.GetBool())
		NDebugOverlay::Line( m_pMarine->WorldSpaceCenter(), m_pMarine->WorldSpaceCenter() + vecFacing * 200.0f, 255, 0, 0, false, 5.0f );
	UTIL_TraceLine(m_pMarine->WorldSpaceCenter(), m_pMarine->WorldSpaceCenter() + vecFacing * 200.0f, MASK_SOLID, m_pMarine, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction < 1.0f)
		return false;

	angHolding[YAW] -= 60;
	AngleVectors(angHolding, &vecFacing);
	if (asw_debug_marine_chatter.GetBool())
		NDebugOverlay::Line( m_pMarine->WorldSpaceCenter(), m_pMarine->WorldSpaceCenter() + vecFacing * 200.0f, 255, 0, 0, false, 5.0f );
	UTIL_TraceLine(m_pMarine->WorldSpaceCenter(), m_pMarine->WorldSpaceCenter() + vecFacing * 200.0f, MASK_SOLID, m_pMarine, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction < 1.0f)
		return false;

	// okay, we're good to do a directional, let's see which one to do
	float fYaw = anglemod(m_pMarine->GetHoldingYaw());
	if (fYaw < 40 || fYaw > 320)
		iChatterType = CHATTER_HOLDING_EAST;
	else if (fYaw > 50 && fYaw < 130)
		iChatterType = CHATTER_HOLDING_NORTH;
	else if (fYaw > 140 && fYaw < 220)
		iChatterType = CHATTER_HOLDING_WEST;
	else if (fYaw > 230 && fYaw < 310)
		iChatterType = CHATTER_HOLDING_SOUTH;
	else
		return false;	// wasn't facing close enough to a cardinal
	
	return true;
}

void CASW_MarineSpeech::QueueChatter(int iChatterType, float fPlayTime, float fMaxPlayTime, CBasePlayer *pOnlyForPlayer)
{
	if (!m_pMarine || m_pMarine->GetHealth() <= 0)
		return;
	// check this speech isn't already in the queue
	int iCount = m_SpeechQueue.Count();
	for (int i=iCount-1;i>=0;i--)
	{
		if (m_SpeechQueue[i].iChatterType == iChatterType)
			return;
	}

	if (iChatterType == CHATTER_HOLDING_POSITION)		// if marine is holding
	{
		for (int i=iCount-1;i>=0;i--)
		{
			if (m_SpeechQueue[i].iChatterType == CHATTER_USE)		// then remove any 'confirming move order' type speech queued up
				m_SpeechQueue.Remove(i);
		}
	}
	else if (iChatterType == CHATTER_USE)		// if marine is confirming move order
	{
		for (int i=iCount-1;i>=0;i--)
		{
			if (m_SpeechQueue[i].iChatterType == CHATTER_HOLDING_POSITION)		// then remove any holding position speech queued up
				m_SpeechQueue.Remove(i);
		}
	}

	// don't FF fire too frequently
	if (iChatterType == CHATTER_FRIENDLY_FIRE)
	{
		if (gpGlobals->curtime < m_fFriendlyFireTime)
			return;

		m_fFriendlyFireTime = gpGlobals->curtime + ASW_INTERVAL_FF;
	}
	if (asw_debug_marine_chatter.GetBool())
		Msg("%f: queuing speech %d at time %f\n", gpGlobals->curtime, iChatterType, fPlayTime);
	CASW_Queued_Speech entry;
	entry.iChatterType = iChatterType;
	entry.fPlayTime = fPlayTime;
	entry.fMaxPlayTime = fMaxPlayTime;
	entry.hOnlyForPlayer = pOnlyForPlayer;
	m_SpeechQueue.AddToTail(entry);
}

void CASW_MarineSpeech::Update()
{
	if (!m_pMarine)
		return;

	int iCount = m_SpeechQueue.Count();
	
	// loop backwards because we might remove one
	for (int i=iCount-1;i>=0;i--)
	{
		if (gpGlobals->curtime >= m_SpeechQueue[i].fPlayTime)
		{
			// try to play the queued speech
			if (asw_debug_marine_chatter.GetBool())
				Msg("%f: trying to play queued speech %d\n", gpGlobals->curtime, m_SpeechQueue[i].iChatterType);
			bool bPlay = true;

			if ((m_SpeechQueue[i].iChatterType == CHATTER_HOLDING_POSITION
					|| m_SpeechQueue[i].iChatterType == CHATTER_USE)
					&& m_pMarine->IsInhabited())
			{
				bPlay = false;
			}

			if (!bPlay)
			{
				if (asw_debug_marine_chatter.GetBool())
					Msg(" removing queued speech; inappropriate to play it now (marine probably inhabited and this is an AI line)\n");
				m_SpeechQueue.Remove(i);
				continue;
			}

			if (Chatter(m_SpeechQueue[i].iChatterType, -1, m_SpeechQueue[i].hOnlyForPlayer.Get()))
			{
				if (s_CurrentConversation != CONV_NONE)
				{
					CASW_Marine *pCurrentActor = (s_CurrentConversationChatterStage == 1) ? s_hActor2.Get() : s_hActor1.Get();
					int iCurrentConvLine = s_iCurrentConvLine;
					// if we just finished the currently queued conversation line...
					if (m_SpeechQueue[i].iChatterType == iCurrentConvLine && m_pMarine == pCurrentActor)
					{
						s_CurrentConversationChatterStage++;
						int iNumStages = s_ConvNumStages[s_CurrentConversation];
						

/*
	CHATTER_WILDCAT_KILL_REPLY_BEHIND,	
	CHATTER_WOLFE_KILL_REPLY_BEHIND,
*/

						// check all the actors are alive still, etc and we're not at the end of the conversation
						if (s_CurrentConversationChatterStage >= iNumStages
							|| !s_hActor1.Get() || s_hActor1.Get()->GetHealth()<=0
							|| !s_hActor2.Get() || s_hActor2.Get()->GetHealth()<=0)
						{
							StopConversation();
						}
						else
						{
							// queue up the next line
							pCurrentActor = (s_CurrentConversationChatterStage == 1) ? s_hActor2.Get() : s_hActor1.Get();
							iCurrentConvLine = (s_CurrentConversationChatterStage == 0) ? s_ConvChatterLine1[s_CurrentConversation] : s_ConvChatterLine2[s_CurrentConversation];
							// make wolfe/wildcat pick the right line in their conv
							if (s_CurrentConversation == CONV_WILDCAT_KILL)
							{
								if (asw_debug_marine_chatter.GetBool())
									Msg("CONV_WILDCAT_KILL, stage %d\n", s_CurrentConversationChatterStage);
								if (s_CurrentConversationChatterStage == 1)	// wolfe replying, does he have more or less kills
								{
									if (GetWildcatAhead())
										iCurrentConvLine = CHATTER_WILDCAT_KILL_REPLY_BEHIND;
									else
										iCurrentConvLine = CHATTER_WILDCAT_KILL_REPLY_AHEAD;
								}
								else if (s_CurrentConversationChatterStage == 2)	// wildcat following up
								{
									if (GetWildcatAhead())
										iCurrentConvLine = CHATTER_WILDCAT_KILL_REPLY_AHEAD;
									else
										iCurrentConvLine = CHATTER_WILDCAT_KILL_REPLY_BEHIND;
								}
							}
							else if (s_CurrentConversation == CONV_WOLFE_KILL)
							{
								if (asw_debug_marine_chatter.GetBool())
									Msg("CONV_WILDCAT_KILL, stage %d\n", s_CurrentConversationChatterStage);
								if (s_CurrentConversationChatterStage == 1)	// wildcat replying, does he have more or less kills
								{
									if (GetWildcatAhead())
										iCurrentConvLine = CHATTER_WOLFE_KILL_REPLY_AHEAD;
									else
										iCurrentConvLine = CHATTER_WOLFE_KILL_REPLY_BEHIND;
								}
								else if (s_CurrentConversationChatterStage == 2)	// wolfe following up
								{
									if (GetWildcatAhead())
										iCurrentConvLine = CHATTER_WOLFE_KILL_REPLY_BEHIND;
									else
										iCurrentConvLine = CHATTER_WOLFE_KILL_REPLY_AHEAD;
								}
							}
						
							if (s_CurrentConversationChatterStage == 2)
							{
								if (s_CurrentConversation == CONV_TEQUILA && s_hActor2.Get()->GetMarineProfile())
								{
									int iProfileIndex = s_hActor2.Get()->GetMarineProfile()->m_ProfileIndex;
									// change the line to one appropriate for who we're talking to
									if (iProfileIndex == 0) iCurrentConvLine = CHATTER_TEQUILA_REPLY_SARGE;
									else if (iProfileIndex == 4) iCurrentConvLine = CHATTER_TEQUILA_REPLY_JAEGER;
									else if (iProfileIndex == 1) iCurrentConvLine = CHATTER_TEQUILA_REPLY_WILDCAT;
									else if (iProfileIndex == 5) iCurrentConvLine = CHATTER_TEQUILA_REPLY_WOLFE;
									else if (iProfileIndex == 2) iCurrentConvLine = CHATTER_TEQUILA_REPLY_FAITH;
									else if (iProfileIndex == 6) iCurrentConvLine = CHATTER_TEQUILA_REPLY_BASTILLE;
									else iCurrentConvLine = -1;
								}
								else if (s_CurrentConversation == CONV_SERIOUS_INJURY)
								{
									iCurrentConvLine = CHATTER_SERIOUS_INJURY_FOLLOW_UP;
								}
							}

							if (iCurrentConvLine == -1 || s_hActor2.Get()==NULL)
								StopConversation();
							else
							{
								if (asw_debug_marine_chatter.GetBool())
									Msg("Queuing up the next line in this conversation %d\n", iCurrentConvLine);
								s_iCurrentConvLine = iCurrentConvLine;
								pCurrentActor->GetMarineSpeech()->QueueChatter(iCurrentConvLine, GetTeamChatterTime() + 0.05f, GetTeamChatterTime() + 60.0f);
							}
						}
					}
				}
				if (asw_debug_marine_chatter.GetBool())
					Msg(" and removing the speech (%d) since it played ok (or failed the chance to play)\n", m_SpeechQueue[i].iChatterType);
				m_SpeechQueue.Remove(i);
			}
			else
			{
				if (asw_debug_marine_chatter.GetBool())
					Msg(" didn't play ok, trying to requeue\n");
				m_SpeechQueue[i].fPlayTime = GetTeamChatterTime() + 0.05f;		// try and queue it up for when we next have a gap
				if (m_SpeechQueue[i].fPlayTime > m_SpeechQueue[i].fMaxPlayTime)	// if it's too late, then abort
				{
					if (asw_debug_marine_chatter.GetBool())
						Msg(" but out of time!\n");
					m_SpeechQueue.Remove(i);
				}
			}
		}
	}
}

bool CASW_MarineSpeech::GetWildcatAhead()
{
	int iWildcatKills = 0;
	int iWolfeKills = 0;

	if ( !ASWGameResource() )
		return false;
	
	CASW_Game_Resource *pGameResource = ASWGameResource();
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);

		if (pMR && pMR->GetProfile() && pMR->GetProfile()->m_VoiceType == ASW_VOICE_WILDCAT)
			iWildcatKills = pMR->m_iAliensKilled;
		else if (pMR && pMR->GetProfile() && pMR->GetProfile()->m_VoiceType == ASW_VOICE_WOLFE)
			iWolfeKills = pMR->m_iAliensKilled;
	}
	return iWildcatKills > iWolfeKills;
}

struct MarineFlavorSpeech
{
	unsigned char uchChatterType;
	unsigned char uchSubChatter;
};

const int g_nNumFlavorSpeech = 25;

MarineFlavorSpeech g_MarineFlavorSpeech[ g_nNumFlavorSpeech ][ ASW_VOICE_TYPE_TOTAL ] =
{
	{
		{ CHATTER_BIOMASS, 1 }, // Sarge: Signs of infestation.
		{ CHATTER_SELECTION_INJURED, 0 }, // Jaeger: Never thought it would end like this.
		{ CHATTER_IDLE, 1 }, // Wildcat: I say we set some T-75 explosives and get the hell out!
		{ CHATTER_SELECTION, 0 }, // Wolfe: Locked and loaded.
		{ CHATTER_SELECTION, 1 }, // Faith: What do you need?
		{ CHATTER_USE, 2 }, // Bastille: Good thinking.
		{ CHATTER_SELECTION, 1 }, // Crash: Ready to get it on!
		{ CHATTER_SELECTION, 0 }, // Flynn
		{ CHATTER_SELECTION, 1 }, // Vegas: Wanna play a little game?
	},
	{
		{ CHATTER_MISC, 0 }, // Sarge: Squad, form up.
		{ CHATTER_IDLE, 5 }, // Jaeger: I've got a bad feeling about this place.
		{ CHATTER_IDLE, 2 }, // Wildcat: Let's get this party started.
		{ CHATTER_SELECTION_INJURED, 0 }, // Wolfe: Ain't got time to bleed.
		{ CHATTER_SELECTION_INJURED, 3 }, // Faith: Let's be careful, okay?
		{ CHATTER_USE, 3 }, // Bastille: Immediatement.
		{ CHATTER_SELECTION_INJURED, 1 }, // Crash: I didn't sign up for this!
		{ CHATTER_SELECTION_INJURED, 1 }, // Flynn
		{ CHATTER_SELECTION_INJURED, 2 }, // Vegas: I still got an ace up my sleeve.
	},
	{
		{ CHATTER_MISC, 2 }, // Sarge: Marines, into positions.
		{ CHATTER_IDLE, 10 }, // Jaeger: There's no bug trouble that can't be cured by a large case of explosives.
		{ CHATTER_IDLE, 4 }, // Wildcat: Peace through superior firepower.
		{ CHATTER_SELECTION_INJURED, 3 }, // Wolfe: It's time we hit em where it hurts most. All over.
		{ CHATTER_USE, 2 }, // Faith: Excellent.
		{ CHATTER_IDLE, 0 }, // Bastille: Careful, they could be anywhere.
		{ CHATTER_USE, 0 }, // Crash: I got skills.
		{ CHATTER_IDLE, 0 }, // Flynn
		{ CHATTER_USE, 1 }, // Vegas: I'm on it.
	},
	{
		{ CHATTER_SARGE_IDLE_REPLY, 0 }, // Sarge: Watch your mouth, Marine.
		{ CHATTER_IDLE, 12 }, // Jaeger: This isn't a good day to run out of smokes.
		{ CHATTER_IDLE, 5 }, // Wildcat: Who dares, wins.
		{ CHATTER_USE, 0 }, // Wolfe: All right!
		{ CHATTER_USE, 0 }, // Faith: Absolutely.
		{ CHATTER_IDLE, 8 }, // Bastille: Ignore the rumors. Stimpacks aren't that addictive.
		{ CHATTER_IDLE, 3 }, // Crash: I shoulda stolen more money to put up with this shit.
		{ CHATTER_IDLE, 2 }, // Flynn
		{ CHATTER_IDLE, 0 }, // Vegas: 4 to 1, eh? I like the odds.
	},
	{
		{ CHATTER_SARGE_IDLE_REPLY, 1 }, // Sarge: You secure that shit.
		{ CHATTER_IDLE, 13 }, // Jaeger: Why can't we set fire to this whole damn colony?
		{ CHATTER_IDLE, 6 }, // Wildcat: Shouldn't we be wasting bugs instead of cooling our heels?
		{ CHATTER_IDLE, 1 }, // Wolfe: I smell bugs. They're nasty and almost as ugly as me.
		{ CHATTER_IDLE, 4 }, // Faith: It's way too quiet.
		{ CHATTER_IDLE, 10 }, // Bastille: I hope we send every last one of these bastards straight to hell.
		{ CHATTER_IDLE, 4 }, // Crash: I tell you what, you guys are lucky I'm here.
		{ CHATTER_MISC, 0 }, // Flynn
		{ CHATTER_IDLE, 2 }, // Vegas: Anyone got a deck of cards?
	},
	{
		{ CHATTER_SARGE_JAEGER_CONV_1_REPLY, 1 }, // Sarge: I need a drink.
		{ CHATTER_IDLE, 9 }, // Jaeger: Stay sharp, people, these guys don't take prisoners.
		{ CHATTER_SELECTION, 0 }, // Wildcat: Anytime, anywhere.
		{ CHATTER_IDLE, 0 }, // Wolfe: Don't get too close. I got a thing for collateral damage.
		{ CHATTER_IDLE, 7 }, // Faith: This all seems like a horrible nightmare.
		{ CHATTER_IDLE, 11 }, // Bastille: Just how many more bugs are on this rock?
		{ CHATTER_SUPPLIES_AMMO, 2 }, // Crash: Oh yeah, that's what I like to see.
		{ CHATTER_IDLE, 5 }, // Flynn
		{ CHATTER_IDLE, 8 }, // Vegas: Who's up for some tequila?
	},
	{
		{ CHATTER_USE, 2 }, // Sarge: Looks good.
		{ CHATTER_IDLE, 7 }, // Jaeger: Just another day in the Corps.
		{ CHATTER_SELECTION_INJURED, 2 }, // Wildcat: They'll pay for this.
		{ CHATTER_IDLE, 2 }, // Wolfe: I got some scores to settle.
		{ CHATTER_IDLE, 3 }, // Faith: I hope we pull through without too many injuries.
		{ CHATTER_IDLE, 13 }, // Bastille: Stay sharp, mes amis.
		{ CHATTER_MAD_FIRING, 3 }, // Crash: You want some?
		{ CHATTER_IDLE, 3 }, // Flynn
		{ CHATTER_IDLE, 5 }, // Vegas: If I were a betting man, I'd say the worst is yet to come.
	},
	{
		{ CHATTER_IDLE, 5 }, // Sarge: Stay sharp, look alive.
		{ CHATTER_IDLE, 6 }, // Jaeger: Jesus, I need a vacation.
		{ CHATTER_TIME_TO_LEAVE, 1 }, // Wildcat: Time to get the hell out.
		{ CHATTER_IDLE, 3 }, // Wolfe: The only good bug is a dead bug.
		{ CHATTER_IDLE, 8 }, // Faith: This place isn't secure.
		{ CHATTER_IDLE, 0 }, // Bastille: Careful, they could be anywhere.
		{ CHATTER_HACK_STARTED, 4 }, // Crash: Time for me to work my magic.
		{ CHATTER_IDLE, 1 }, // Flynn
		{ CHATTER_FOLLOW_ME, 3 }, // Vegas: Time to roll the dice, Compadre.
	},
	{
		{ CHATTER_IDLE, 7 }, // Sarge: I love this job.
		{ CHATTER_IDLE, 2 }, // Jaeger: All this fighting makes a Marine hungry.
		{ CHATTER_MAD_FIRING, 1 }, // Wildcat: Payback's a bitch, ain't it.
		{ CHATTER_IDLE, 6 }, // Wolfe: This is getting more fun by the minute.
		{ CHATTER_IDLE, 9 }, // Faith: Wait, I hear something.
		{ CHATTER_IDLE, 15 }, // Bastille: What was that?
		{ CHATTER_HACK_LONG_STARTED, 1 }, // Crash: Got my back, right?
		{ CHATTER_FIRING_AT_ALIEN, 3 }, // Flynn
		{ CHATTER_TIME_TO_LEAVE, 1 }, // Vegas: Sweet! Tequila, here I come.
	},
	{
		{ CHATTER_IDLE, 8 }, // Sarge: Ah, another fine day in the Interstellar Armed Forces.
		{ CHATTER_FIRING_AT_ALIEN, 2 }, // Jaeger: Eat this!
		{ CHATTER_COMPLIMENTS, 0 }, // Wildcat: Nice job!
		{ CHATTER_IDLE, 4 }, // Wolfe: Remember, you don't hurt em, if you don't hit em.
		{ CHATTER_MAD_FIRING, 2 }, // Faith: I'll kill you all!
		{ CHATTER_IDLE, 5 }, // Bastille: Great, now I'm hearing things.
		{ CHATTER_HACK_FINISHED, 4 }, // Crash: Oh yeah, who's the man?
		{ CHATTER_HACK_FINISHED, 0 }, // Flynn
		{ CHATTER_SHIELDBUG, 2 }, // Vegas: Madre de dios!
	},
	{
		{ CHATTER_IDLE, 12 }, // Sarge: This is the Interstellar Armed Forces, not a Gaddamned picnic.
		{ CHATTER_MAD_FIRING, 3 }, // Jaeger: Rock and roll!
		{ CHATTER_IDLE, 3 }, // Wildcat: Meow.
		{ CHATTER_IDLE, 5 }, // Wolfe: There's only one way to solve this, and I think I'm gonna enjoy it.
		{ CHATTER_MAD_FIRING, 3 }, // Faith: Payback's a bitch, ain't it?
		{ CHATTER_TIME_TO_LEAVE, 2 }, // Bastille: Time to make an exit.
		{ CHATTER_HACK_FINISHED, 4 }, // Crash: Oh yeah, who's the man?
		{ CHATTER_COMPLIMENTS, 0 }, // Flynn
		{ CHATTER_MAD_FIRING, 2 }, // Vegas: Oh, you wanna play rough.
	},
	{
		{ CHATTER_IDLE, 2 }, // Sarge: Watch your spacing, team.
		{ CHATTER_MAD_FIRING, 4 }, // Jaeger: You want some?
		{ CHATTER_AUTOGUN_REPLY, 1 }, // Wildcat: Hell, yeah!
		{ CHATTER_IDLE, 6 }, // Wolfe: This is getting more fun by the minute.
		{ CHATTER_MEDS_NONE, 2 }, // Faith: Out of meds. Try not to get hurt.
		{ CHATTER_MAD_FIRING, 1 }, // Bastille: Eat this, alien scum!
		{ CHATTER_COMPLIMENTS, 2 }, // Crash: Smooth.
		{ CHATTER_MISC, 0 }, // Flynn
		{ CHATTER_MAD_FIRING, 0 }, // Vegas: Adios, muchachos.
	},
	{
		{ CHATTER_IDLE, 9 }, // Sarge: So far, so good.
		{ CHATTER_AUTOGUN_REPLY, 0 }, // Jaeger: That's the spirit.
		{ CHATTER_IMPATIENCE, 0 }, // Wildcat: Christ, what's taking so long?
		{ CHATTER_COMPLIMENTS_WILDCAT, 2 }, // Wolfe: Nice shooting, Kid.
		{ CHATTER_MEDIC_COMPLAIN, 1 }, // Faith: You just love getting in harm's way, don't you?
		{ CHATTER_HEALING, 13 }, // Bastille: Why, you should be dead, but you're not.
		{ CHATTER_CRASH_COMPLAIN, 0 }, // Crash: I can't stand this place much longer.
		{ CHATTER_MEDIC_COMPLAIN_REPLY, 0 }, // Flynn
		{ CHATTER_MAD_FIRING, 3 }, // Vegas: You think you can take me?
	},
	{
		{ CHATTER_SHIELDBUG, 1 }, // Sarge: Holy shit!
		{ CHATTER_STILL_BREATHING, 0 }, // Jaeger: You guys still breathing?
		{ CHATTER_FRIENDLY_FIRE, 0 }, // Wildcat: Christ, man.
		{ CHATTER_MISC, 3 }, // Wolfe: You saved my life.
		{ CHATTER_SELECTION, 1 }, // Faith: What do you need?
		{ CHATTER_HEALING, 18 }, // Bastille: You're still alive, well, I think.
		{ CHATTER_CRASH_IDLE, 0 }, // Crash: Does anyone have any idea what we are doing here?
		{ CHATTER_SELECTION, 0 }, // Flynn
		{ CHATTER_HACK_STARTED, 0 }, // Vegas: Ahh, finally something worthy of my time.
	},
	{
		{ CHATTER_GRENADE, 2 }, // Sarge: Eat this!
		{ CHATTER_SELECTION_INJURED, 0 }, // Jaeger: Never thought it would end like this.
		{ CHATTER_IDLE, 1 }, // Wildcat: I say we set some T-75 explosives and get the hell out!
		{ CHATTER_WOLFE_BEST_REPLY, 8 }, // Wolfe: Well, that sounds like a challenge.
		{ CHATTER_SELECTION_INJURED, 3 }, // Faith: Let's be careful, okay?
		{ CHATTER_HEALING, 10 }, // Bastille: That should help.
		{ CHATTER_USE, 1 }, // Crash: No problem.
		{ CHATTER_MEDIC_COMPLAIN_REPLY, 0 }, // Flynn
		{ CHATTER_HACK_LONG_STARTED, 7 }, // Vegas: Watch my back, ok?
	},
	{
		{ CHATTER_IDLE, 13 }, // Sarge: Well, time to earn those paychecks.
		{ CHATTER_IDLE, 5 }, // Jaeger: I've got a bad feeling about this place.
		{ CHATTER_IDLE, 2 }, // Wildcat: Let's get this party started.
		{ CHATTER_MEDIC_COMPLAIN_REPLY, 0 }, // Wolfe: Is that a trick question?
		{ CHATTER_USE, 2 }, // Faith: Excellent.
		{ CHATTER_MEDS_NONE, 0 }, // Bastille: I'm out of meds.
		{ CHATTER_IDLE, 5 }, // Crash: Wow, this place has seen some action.
		{ CHATTER_SELECTION_INJURED, 1 }, // Flynn
		{ CHATTER_HACK_FINISHED, 4 }, // Vegas: Too easy!
	},
	{
		{ CHATTER_IDLE, 14 }, // Sarge: Check your corners.
		{ CHATTER_IDLE, 10 }, // Jaeger: There's no bug trouble that can't be cured by a large case of explosives.
		{ CHATTER_IDLE, 4 }, // Wildcat: Peace through superior firepower.
		{ CHATTER_SELECTION, 0 }, // Wolfe: Locked and loaded.
		{ CHATTER_USE, 0 }, // Faith: Absolutely.
		{ CHATTER_COMPLIMENTS, 0 }, // Bastille: Good kill, mon ami!
		{ CHATTER_SELECTION, 1 }, // Crash: Ready to get it on!
		{ CHATTER_IDLE, 0 }, // Flynn
		{ CHATTER_COMPLIMENTS, 0 }, // Vegas: Good job, amigo.
	},
	{
		{ CHATTER_IDLE, 14 }, // Sarge: Check your corners.
		{ CHATTER_IDLE, 12 }, // Jaeger: This isn't a good day to run out of smokes.
		{ CHATTER_IDLE, 5 }, // Wildcat: Who dares, wins.
		{ CHATTER_SELECTION_INJURED, 0 }, // Wolfe: Ain't got time to bleed.
		{ CHATTER_IDLE, 4 }, // Faith: It's way too quiet.
		{ CHATTER_MISC, 11 }, // Bastille: You're starting to be annoying.
		{ CHATTER_SELECTION_INJURED, 3 }, // Crash
		{ CHATTER_IDLE, 2 }, // Flynn
		{ CHATTER_COMPLIMENTS, 1 }, // Vegas: Impressive.
	},
	{
		{ CHATTER_IDLE, 16 }, // Sarge: Proceed by squads, we got a job to do.
		{ CHATTER_IDLE, 13 }, // Jaeger: Why can't we set fire to this whole damn colony?
		{ CHATTER_IDLE, 6 }, // Wildcat: Shouldn't we be wasting bugs instead of cooling our heels?
		{ CHATTER_SELECTION_INJURED, 3 }, // Wolfe: It's time we hit em where it hurts most. All over.
		{ CHATTER_IDLE, 7 }, // Faith: This all seems like a horrible nightmare.
		{ CHATTER_MISC, 7 }, // Bastille: If we're lucky the wounds will be healed when we wake up from cryosleep.
		{ CHATTER_USE, 0 }, // Crash: I got skills.
		{ CHATTER_SELECTION_INJURED, 1 }, // Flynn
		{ CHATTER_COMPLIMENTS, 2 }, // Vegas: Nice one, amigo.
	},
	{
		{ CHATTER_IDLE, 17 }, // Sarge: Let's do this by the book.
		{ CHATTER_IDLE, 9 }, // Jaeger: Stay sharp, people, these guys don't take prisoners.
		{ CHATTER_SELECTION, 0 }, // Wildcat: Anytime, anywhere.
		{ CHATTER_USE, 0 }, // Wolfe: All right!
		{ CHATTER_IDLE, 3 }, // Faith: I hope we pull through without too many injuries.
		{ CHATTER_CRASH_COMPLAIN_REPLY, 0 }, // Bastille: Maybe you should report a nervous breakdown.
		{ CHATTER_IDLE, 3 }, // Crash: I shoulda stolen more money to put up with this shit.
		{ CHATTER_IDLE, 5 }, // Flynn
		{ CHATTER_IMPATIENCE, 0 }, // Vegas: Clock is ticking, amigo.
	},
	{
		{ CHATTER_COMPLIMENTS, 1 }, // Sarge: That's what I like to hear.
		{ CHATTER_IDLE, 7 }, // Jaeger: Just another day in the Corps.
		{ CHATTER_SELECTION_INJURED, 2 }, // Wildcat: They'll pay for this.
		{ CHATTER_IDLE, 1 }, // Wolfe: I smell bugs. They're nasty and almost as ugly as me.
		{ CHATTER_IDLE, 8 }, // Faith: This place isn't secure.
		{ CHATTER_MEDIC_COMPLAIN, 1 }, // Bastille: Can't you not get hurt for a change?
		{ CHATTER_IDLE, 4 }, // Crash: I tell you what, you guys are lucky I'm here.
		{ CHATTER_IDLE, 3 }, // Flynn
		{ CHATTER_FIRST_BLOOD_WIN, 1 }, // Vegas: Lady luck is on my side.
	},
	{
		{ CHATTER_COMPLIMENTS, 2 }, // Sarge: Damn fine work, marine.
		{ CHATTER_IDLE, 6 }, // Jaeger: Jesus, I need a vacation.
		{ CHATTER_TIME_TO_LEAVE, 1 }, // Wildcat: Time to get the hell out.
		{ CHATTER_IDLE, 0 }, // Wolfe: Don't get too close. I got a thing for collateral damage.
		{ CHATTER_IDLE, 0 }, // Faith: I don't like it down here.
		{ CHATTER_HEALING_CRASH_REPLY, 0 }, // Bastille: I'm saving them for when you really get hurt.
		{ CHATTER_SUPPLIES_AMMO, 2 }, // Crash: Oh yeah, that's what I like to see.
		{ CHATTER_IDLE, 1 }, // Flynn
		{ CHATTER_FIRST_BLOOD_WIN, 0 }, // Vegas: Haha, fortune favours Vegas today.
	},
	{
		{ CHATTER_SELECTION_INJURED, 1 }, // Sarge: We should pull back and reassess this.
		{ CHATTER_IDLE, 2 }, // Jaeger: All this fighting makes a Marine hungry.
		{ CHATTER_MAD_FIRING, 1 }, // Wildcat: Payback's a bitch, ain't it.
		{ CHATTER_IDLE, 2 }, // Wolfe: I got some scores to settle.
		{ CHATTER_IDLE, 9 }, // Faith: Wait, I hear something.
		{ CHATTER_SERIOUS_INJURY, 1 }, // Bastille: Good news, you're off the next mission.
		{ CHATTER_MAD_FIRING, 3 }, // Crash: You want some?
		{ CHATTER_FIRING_AT_ALIEN, 3 }, // Flynn
		{ CHATTER_TEQUILA_START, 0 }, // Vegas: I could do with some tequila right about now.
	},
	{
		{ CHATTER_SELECTION_INJURED, 3 }, // Sarge: This is tough.
		{ CHATTER_FIRING_AT_ALIEN, 2 }, // Jaeger: Eat this!
		{ CHATTER_COMPLIMENTS, 0 }, // Wildcat: Nice job!
		{ CHATTER_IDLE, 4 }, // Wolfe: Remember, you don't hurt em, if you don't hit em.
		{ CHATTER_MAD_FIRING, 2 }, // Faith: I'll kill you all!
		{ CHATTER_SERIOUS_INJURY_REPLY, 0 }, // Bastille: All right, if you say so.
		{ CHATTER_HACK_STARTED, 4 }, // Crash: Time for me to work my magic.
		{ CHATTER_HACK_FINISHED, 0 }, // Flynn
		{ CHATTER_SERIOUS_INJURY_REPLY, 0 }, // Vegas: Ah man, I should have stayed in Culiacán, Sinaloa.
	},
	{
		{ CHATTER_GOT_REAR, 2 }, // Sarge: I've got your back.
		{ CHATTER_MAD_FIRING, 3 }, // Jaeger: Rock and roll!
		{ CHATTER_IDLE, 3 }, // Wildcat: Meow.
		{ CHATTER_IDLE, 5 }, // Wolfe: There's only one way to solve this, and I think I'm gonna enjoy it.
		{ CHATTER_MAD_FIRING, 3 }, // Faith: Payback's a bitch, ain't it?
		{ CHATTER_USE, 2 }, // Bastille: Good thinking.
		{ CHATTER_HACK_LONG_STARTED, 1 }, // Crash: Got my back, right?
		{ CHATTER_COMPLIMENTS, 0 }, // Flynn
		{ CHATTER_STILL_BREATHING_REPLY, 0 }, // Vegas: Why do I always get the crazy ones?
	},
};

bool CASW_MarineSpeech::ClientRequestChatter(int iChatterType, int iSubChatter, CBasePlayer *pOnlyForPlayer)
{
	if (!m_pMarine || !m_pMarine->GetMarineResource() || (m_pMarine->GetHealth() <= 0 && iChatterType != CHATTER_DIE))
	{
		AssertMsg(false, "Absent marine tried to chatter\n");
		return false;
	}

	CASW_Marine_Profile *pProfile = m_pMarine->GetMarineResource()->GetProfile();
	if ( !pProfile )
	{
		return false;
	}

	if ( iChatterType == -1 )
	{
		int nRandomSpeech = RandomInt( 0, g_nNumFlavorSpeech - 1 );
		iChatterType = g_MarineFlavorSpeech[ nRandomSpeech ][ pProfile->GetVoiceType() ].uchChatterType;
		iSubChatter = g_MarineFlavorSpeech[ nRandomSpeech ][ pProfile->GetVoiceType() ].uchSubChatter;
	}

	// todo: disallow some chatter types (misleading or annoying ones like onfire, infested, etc)
	if (iChatterType < 0 || iChatterType >= NUM_CHATTER_LINES)
	{
		AssertMsg1( false, "Faulty chatter type %d\n", iChatterType );
		return false;
	}

	if (gpGlobals->curtime < GetTeamChatterTime())
	{
		return false;
	}
		
	//if (!TranslateChatter(iChatterType, pProfile))
		//return false;

	// if marine doesn't have this line, then don't try playing it
	const char *szChatter = pProfile->m_Chatter[iChatterType];
	if (szChatter[0] == '\0')
	{
		return false;
	}

	InternalPlayChatter(m_pMarine, szChatter, ASW_CHATTER_TIMER_TEAM, iChatterType, iSubChatter, pOnlyForPlayer);
	return true;
}

bool CASW_MarineSpeech::AllowCalmConversations(int iConversation)
{
	if (ASWGameRules() && ASWGameRules()->m_fLastFireTime > gpGlobals->curtime - ASW_CALM_CHATTER_TIME)
		return false;

	return true;
}