#include "cbase.h"
#include "asw_voiceover_dialogue.h"
#include "engine/IEngineSound.h"
#include "soundchars.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ISoundEmitterSystemBase *soundemitterbase;

// Simple entity to replace logic_choreographed_scene when only one globally-audible voice line is needed.
LINK_ENTITY_TO_CLASS( asw_voiceover_dialogue, CASW_Voiceover_Dialogue );

BEGIN_DATADESC( CASW_Voiceover_Dialogue )
	DEFINE_KEYFIELD( m_iszSoundName, FIELD_SOUNDNAME, "soundname" ),
	DEFINE_KEYFIELD( m_iszActorName, FIELD_STRING, "actorname" ),
	DEFINE_KEYFIELD( m_bBusyActor, FIELD_BOOLEAN, "busyactor" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Start", InputStart ),

	DEFINE_OUTPUT( m_OnStart, "OnStart" ),
	DEFINE_OUTPUT( m_OnCompletion, "OnCompletion" ),
END_DATADESC();

CASW_Voiceover_Dialogue::CASW_Voiceover_Dialogue()
{
	m_iszSoundName = NULL_STRING;
	m_hSoundScriptHandle = -1;
	m_flDuration = 0;
	m_iszActorName = NULL_STRING;
	m_bBusyActor = true;
}

CASW_Voiceover_Dialogue::~CASW_Voiceover_Dialogue()
{
}

void CASW_Voiceover_Dialogue::Precache()
{
	Assert( m_iszSoundName != NULL_STRING );
	if ( m_iszSoundName == NULL_STRING )
	{
		Warning( "asw_voiceover_dialogue(#%d %s) LEVEL DESIGNER ERROR: Missing sound name. Deleting.\n", GetHammerID(), GetDebugName() );
		UTIL_Remove( this );
		return;
	}

	m_hSoundScriptHandle = PrecacheScriptSound( STRING( m_iszSoundName ) );
	Assert( m_hSoundScriptHandle != -1 );
	m_flDuration = enginesound->GetSoundDuration( PSkipSoundChars( soundemitterbase->GetWavFileForSound( STRING( m_iszSoundName ), GENDER_NONE ) ) );

	BaseClass::Precache();
}

void CASW_Voiceover_Dialogue::Spawn()
{
	Precache();

	BaseClass::Spawn();
}

void CASW_Voiceover_Dialogue::InputStart( inputdata_t &inputdata )
{
	CAlienSwarm *pGameRules = ASWGameRules();
	Assert( pGameRules );
	if ( !pGameRules )
	{
		Warning( "Cannot play asw_voiceover_dialogue: missing ASWGameRules.\n" );
		return;
	}

	EmitSound_t es;
	CReliableBroadcastRecipientFilter filter;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = CBaseEntity::Instance( i );
		if ( !pPlayer || !pPlayer->IsPlayer() )
			continue;

		const char *szSkipDialogue = engine->GetClientConVarValue( i, "rd_skip_all_dialogue" );
		if ( szSkipDialogue && atoi( szSkipDialogue ) )
		{
			filter.RemoveRecipientByPlayerIndex( i );
		}
		else
		{
			es.m_UtlVecSoundOrigin.AddToTail( pPlayer->EarPosition() );
		}
	}

	float flStart = gpGlobals->curtime;
	if ( m_iszActorName != NULL_STRING )
	{
		unsigned short iActor = pGameRules->m_ActorSpeakingUntil.Find( m_iszActorName );
		if ( iActor != pGameRules->m_ActorSpeakingUntil.InvalidIndex() )
		{
			float &flUntil = pGameRules->m_ActorSpeakingUntil[iActor];
			if ( m_bBusyActor )
			{
				flStart = MAX( flUntil, flStart );
			}

			flUntil = MAX( flUntil, flStart + m_flDuration + 1.0f );
		}
		else
		{
			pGameRules->m_ActorSpeakingUntil.Insert( m_iszActorName, flStart + m_flDuration + 1.0f );
		}
	}

	es.m_nChannel = CHAN_VOICE;
	es.m_pSoundName = STRING( m_iszSoundName );
	es.m_SoundLevel = SNDLVL_TALKING;
	es.m_flSoundTime = flStart;
	es.m_pflSoundDuration = &m_flDuration;
	es.m_bWarnOnMissingCloseCaption = true;
	es.m_bWarnOnDirectWaveReference = true;
	es.m_hSoundScriptHandle = m_hSoundScriptHandle;

	EmitSound( filter, 0, es, m_hSoundScriptHandle );

	float flDelay = flStart - gpGlobals->curtime;

	m_OnStart.FireOutput( inputdata.pActivator, inputdata.pCaller, flDelay );
	m_OnCompletion.FireOutput( inputdata.pActivator, inputdata.pCaller, flDelay + m_flDuration );
}
