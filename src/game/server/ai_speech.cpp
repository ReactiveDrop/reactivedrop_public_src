//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "ai_speech.h"

#include "game.h"
#include "engine/ienginesound.h"
#include "keyvalues.h"
#include "ai_basenpc.h"
#include "ai_criteria.h"
#include "isaverestore.h"
#include "sceneentity.h"
#include "ai_speechqueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define DEBUG_AISPEECH 1
#ifdef DEBUG_AISPEECH
ConVar ai_debug_speech( "ai_debug_speech", "0" );
#define DebuggingSpeech() ai_debug_speech.GetBool()
#else
inline void SpeechMsg( ... ) {}
#define DebuggingSpeech() (false)
#endif

extern ConVar rr_debugresponses;

//-----------------------------------------------------------------------------

CAI_TimedSemaphore g_AIFriendliesTalkSemaphore;
CAI_TimedSemaphore g_AIFoesTalkSemaphore;

ConceptHistory_t::~ConceptHistory_t()
{
}

ConceptHistory_t::ConceptHistory_t( const ConceptHistory_t& src )
{
	timeSpoken = src.timeSpoken;
	m_response = src.m_response ;
}

ConceptHistory_t& ConceptHistory_t::operator =( const ConceptHistory_t& src )
{
	if ( this == &src )
		return *this;

	timeSpoken = src.timeSpoken;
	m_response = src.m_response ;

	return *this;
}

BEGIN_SIMPLE_DATADESC( ConceptHistory_t )
	DEFINE_FIELD( timeSpoken,	FIELD_TIME ),  // Relative to server time
	// DEFINE_EMBEDDED( response,	FIELD_??? ),	// This is manually saved/restored by the ConceptHistory saverestore ops below
END_DATADESC()

class CConceptHistoriesDataOps : public CDefSaveRestoreOps
{
public:
	virtual void Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
	{
		CUtlDict< ConceptHistory_t, int > *ch = ((CUtlDict< ConceptHistory_t, int > *)fieldInfo.pField);
		int count = ch->Count();
		pSave->WriteInt( &count );
		for ( int i = 0 ; i < count; i++ )
		{
			ConceptHistory_t *pHistory = &(*ch)[ i ];

			pSave->StartBlock();
			{

				// Write element name
				pSave->WriteString( ch->GetElementName( i ) );

				// Write data
				pSave->WriteAll( pHistory );
				// Write response blob
				bool hasresponse = !pHistory->m_response.IsEmpty() ;
				pSave->WriteBool( &hasresponse );
				if ( hasresponse )
				{
					pSave->WriteAll( &pHistory->m_response );
				}
				// TODO: Could blat out pHistory->criteria pointer here, if it's needed
			}
			pSave->EndBlock();
		}
	}
	
	virtual void Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
	{
		CUtlDict< ConceptHistory_t, int > *ch = ((CUtlDict< ConceptHistory_t, int > *)fieldInfo.pField);

		int count = pRestore->ReadInt();
		Assert( count >= 0 );
		for ( int i = 0 ; i < count; i++ )
		{
			char conceptname[ 512 ];
			conceptname[ 0 ] = 0;
			ConceptHistory_t history;

			pRestore->StartBlock();
			{
				pRestore->ReadString( conceptname, sizeof( conceptname ), 0 );

				pRestore->ReadAll( &history );

				bool hasresponse = false;

				pRestore->ReadBool( &hasresponse );
				if ( hasresponse )
				{
					history.m_response;
					pRestore->ReadAll( &history.m_response );
				}
				else
				{
					history.m_response.Invalidate();
				}
			}

			pRestore->EndBlock();

			// TODO: Could restore pHistory->criteria pointer here, if it's needed

			// Add to utldict
			if ( conceptname[0] != 0 )
			{
				ch->Insert( conceptname, history );
			}
			else
			{
				Assert( !"Error restoring ConceptHistory_t, discarding!" );
			}
		}
	}
	
	virtual void MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
	}

	virtual bool IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		CUtlDict< ConceptHistory_t, int > *ch = ((CUtlDict< ConceptHistory_t, int > *)fieldInfo.pField);
		return ch->Count() == 0 ? true : false;
	}
};

CConceptHistoriesDataOps g_ConceptHistoriesSaveDataOps;

/////////////////////////////////////////////////
// context operators
RR::CApplyContextOperator RR::sm_OpCopy(0); // "
RR::CIncrementOperator RR::sm_OpIncrement(2); // "++"
RR::CDecrementOperator RR::sm_OpDecrement(2); // "--"
RR::CToggleOperator RR::sm_OpToggle(1); // "!"

RR::CApplyContextOperator *RR::CApplyContextOperator::FindOperator( const char *pContextString )
{
	if ( !pContextString || pContextString[0] == 0 )
	{
		return &sm_OpCopy;
	}

	if ( pContextString[0] == '+' && pContextString [1] == '+' && pContextString[2] != '\0' )
	{
		return &sm_OpIncrement;
	}
	else if ( pContextString[0] == '-' && pContextString [1] == '-' && pContextString[2] != '\0' )
	{
		return &sm_OpDecrement;
	}
	else if ( pContextString[0] == '!' )
	{
		return &sm_OpToggle;
	}
	else
	{
		return &sm_OpCopy;
	}
}

// default is just copy
bool RR::CApplyContextOperator::Apply( const char *pOldValue, const char *pOperator, char *pNewValue, int pNewValBufSize )
{
	Assert( pOperator && pNewValue && pNewValBufSize > 0 );
	Assert( m_nSkipChars == 0 );
	if ( pOperator )
	{
		V_strncpy( pNewValue, pOperator, pNewValBufSize );
	}
	else
	{
		*pNewValue = 0;
	}
	return true;
}

bool RR::CIncrementOperator::Apply( const char *pOldValue, const char *pOperator, char *pNewValue, int pNewValBufSize )
{
	Assert( pOperator[0] == '+' && pOperator[1] == '+' );
	// parse out the old value as a numeric
	int nOld = pOldValue ? V_atoi(pOldValue) : 0;
	int nInc = V_atoi( pOperator+m_nSkipChars );
	V_snprintf( pNewValue, pNewValBufSize, "%d", nOld+nInc );
	return true;
}

bool RR::CDecrementOperator::Apply( const char *pOldValue, const char *pOperator, char *pNewValue, int pNewValBufSize )
{
	Assert( pOperator[0] == '-' && pOperator[1] == '-' );
	// parse out the old value as a numeric
	int nOld = pOldValue ? V_atoi(pOldValue) : 0;
	int nInc = V_atoi( pOperator+m_nSkipChars );
	V_snprintf( pNewValue, pNewValBufSize, "%d", nOld-nInc );
	return true;
}

bool RR::CToggleOperator::Apply( const char *pOldValue, const char *pOperator, char *pNewValue, int pNewValBufSize )
{
	Assert( pOperator[0] == '!' );
	// parse out the old value as a numeric
	int nOld = pOldValue ? V_atoi(pOldValue) : 0;
	V_snprintf( pNewValue, pNewValBufSize, "%d", nOld ? 0 : 1 );
	return true;
}


//-----------------------------------------------------------------------------
//
// CLASS: CAI_Expresser
//

BEGIN_SIMPLE_DATADESC( CAI_Expresser )
	//									m_pSink		(reconnected on load)
//	DEFINE_FIELD( m_pOuter, CHandle < CBaseFlex > ),
	DEFINE_CUSTOM_FIELD( m_ConceptHistories,	&g_ConceptHistoriesSaveDataOps ),
	DEFINE_FIELD(		m_flStopTalkTime,		FIELD_TIME		),
	DEFINE_FIELD(		m_flStopTalkTimeWithoutDelay, FIELD_TIME		),
	DEFINE_FIELD(		m_flBlockedTalkTime, 	FIELD_TIME		),
	DEFINE_FIELD(		m_voicePitch,			FIELD_INTEGER	),
	DEFINE_FIELD(		m_flLastTimeAcceptedSpeak, 	FIELD_TIME		),
END_DATADESC()

//-------------------------------------

bool CAI_Expresser::SemaphoreIsAvailable( CBaseEntity *pTalker )
{
	if ( !GetSink()->UseSemaphore() )
		return true;

	CAI_TimedSemaphore *pSemaphore = GetMySpeechSemaphore( pTalker->MyNPCPointer() );
	return (pSemaphore ? pSemaphore->IsAvailable( pTalker ) : true);
}

//-------------------------------------

float CAI_Expresser::GetSemaphoreAvailableTime( CBaseEntity *pTalker )
{
	CAI_TimedSemaphore *pSemaphore = GetMySpeechSemaphore( pTalker->MyNPCPointer() );
	return (pSemaphore ? pSemaphore->GetReleaseTime() : 0);
}

//-------------------------------------

int CAI_Expresser::GetVoicePitch() const
{
	return m_voicePitch + random->RandomInt(0,3);
}

#ifdef DEBUG
static int g_nExpressers;
#endif

/*
inline bool ShouldBeInExpresserQueue( CBaseFlex *pOuter )
{
	return true; // return IsTerrorPlayer( pOuter, TEAM_SURVIVOR );
}
*/

CAI_Expresser::CAI_Expresser( CBaseFlex *pOuter )
 :	m_pOuter( pOuter ),
	m_pSink( NULL ),
	m_flStopTalkTime( 0 ),
	m_flBlockedTalkTime( 0 ),
	m_flStopTalkTimeWithoutDelay( 0 ),
	m_voicePitch( 100 ),
	m_flLastTimeAcceptedSpeak( 0 )
{
#ifdef DEBUG
	g_nExpressers++;
#endif
	if (m_pOuter)
	{
		// register me with the global expresser queue.

		// L4D: something a little ass backwards is happening here. We only want 
		// survivors to be in the queue. However, the team number isn't 
		// specified yet. So, we actually need to do this in the player's ChangeTeam.
		g_ResponseQueueManager.GetQueue()->AddExpresserHost(m_pOuter);

	}
}

CAI_Expresser::~CAI_Expresser()
{
	m_ConceptHistories.Purge();

	CBaseFlex *RESTRICT outer = GetOuter();
	if ( outer )
	{
		CAI_TimedSemaphore *pSemaphore = GetMySpeechSemaphore( outer );
		if ( pSemaphore )
		{
			if ( pSemaphore->GetOwner() == outer )
				pSemaphore->Release();

#ifdef DEBUG
			g_nExpressers--;
			if ( g_nExpressers == 0 && pSemaphore->GetOwner() )
				DevMsg( 2, "Speech semaphore being held by non-talker entity\n" );
#endif
		}

		g_ResponseQueueManager.GetQueue()->RemoveExpresserHost(outer);
	}
}

//-----------------------------------------------------------------------------
void CAI_Expresser::TestAllResponses()
{
	IResponseSystem *pResponseSystem = GetOuter()->GetResponseSystem();
	if ( pResponseSystem )
	{
		CUtlVector<AI_Response> responses;
		pResponseSystem->GetAllResponses( &responses );
		for ( int i = 0; i < responses.Count(); i++ )
		{
			char response[ 256 ];
			responses[i].GetResponse( response, sizeof( response ) );

			Msg( "Response: %s\n", response );
			AIConcept_t concept;
			SpeakDispatchResponse( concept, &responses[i], NULL );
		}
	}
}

//-----------------------------------------------------------------------------
void CAI_Expresser::SetOuter( CBaseFlex *pOuter )	
{ 
	// if we're changing outers (which is a strange thing to do)
	// unregister the old one from the queue.
	if ( m_pOuter && ( m_pOuter != pOuter  ) )
	{
		AssertMsg2( false, "Expresser is switching its Outer from %s to %s. Why?", m_pOuter->GetDebugName(), pOuter->GetDebugName() );
		// unregister me with the global expresser queue
		g_ResponseQueueManager.GetQueue()->RemoveExpresserHost(m_pOuter);
	}

	m_pOuter = pOuter; 
}

//-----------------------------------------------------------------------------

static const int LEN_SPECIFIC_SCENE_MODIFIER = strlen( AI_SPECIFIC_SCENE_MODIFIER );


// This function appends "Global world" criteria that are always added to 
// any character doing any match. This represents global concepts like weather, who's
// alive, etc.
static void ModifyOrAppendGlobalCriteria( AI_CriteriaSet * RESTRICT outputSet )
{
	return;
}


void CAI_Expresser::GatherCriteria( AI_CriteriaSet * RESTRICT outputSet, const AIConcept_t &concept, const char * RESTRICT modifiers )
{
	// Always include the concept name
	outputSet->AppendCriteria( "concept", concept, CONCEPT_WEIGHT );

#if 1
	outputSet->Merge( modifiers );
#else
	// Always include any optional modifiers
	if ( modifiers != NULL )
	{
		char copy_modifiers[ 255 ];
		const char *pCopy;
		char key[ 128 ] = { 0 };
		char value[ 128 ] = { 0 };

		Q_strncpy( copy_modifiers, modifiers, sizeof( copy_modifiers ) );
		pCopy = copy_modifiers;

		while( pCopy )
		{
			pCopy = SplitContext( pCopy, key, sizeof( key ), value, sizeof( value ), NULL, modifiers );

			if( *key && *value )
			{
				outputSet->AppendCriteria( key, value, CONCEPT_WEIGHT );
			}
		}
	}
#endif

	// include any global criteria
	ModifyOrAppendGlobalCriteria( outputSet );

	// Let our outer fill in most match criteria
	GetOuter()->ModifyOrAppendCriteria( *outputSet );

	// Append local player criteria to set, but not if this is a player doing the talking
	if ( !GetOuter()->IsPlayer() )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( 1 );
		if( pPlayer )
			pPlayer->ModifyOrAppendPlayerCriteria( *outputSet );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Searches for a possible response
// Input  : concept - 
//			NULL - 
// Output : AI_Response
//-----------------------------------------------------------------------------
// AI_Response *CAI_Expresser::SpeakFindResponse( AIConcept_t concept, const char *modifiers /*= NULL*/ )
bool CAI_Expresser::FindResponse( AI_Response &outResponse, AIConcept_t &concept, AI_CriteriaSet *criteria )
{
	VPROF("CAI_Expresser::FindResponse");
	IResponseSystem *rs = GetOuter()->GetResponseSystem();
	if ( !rs )
	{
		Assert( !"No response system installed for CAI_Expresser::GetOuter()!!!" );
		return NULL;
	}

	// if I'm dead, I can't possibly match dialog.
	if ( !GetOuter()->IsAlive() )
	{
		return false;
	}

#if 0 // this is the old technique, where we always gathered criteria in this function
	AI_CriteriaSet set;
	// Always include the concept name
	set.AppendCriteria( "concept", concept, CONCEPT_WEIGHT );

	// Always include any optional modifiers
	if ( modifiers != NULL )
	{
		char copy_modifiers[ 255 ];
		const char *pCopy;
		char key[ 128 ] = { 0 };
		char value[ 128 ] = { 0 };

		Q_strncpy( copy_modifiers, modifiers, sizeof( copy_modifiers ) );
		pCopy = copy_modifiers;

		while( pCopy )
		{
			pCopy = SplitContext( pCopy, key, sizeof( key ), value, sizeof( value ), NULL, modifiers );

			if( *key && *value )
			{
				set.AppendCriteria( key, value, CONCEPT_WEIGHT );
			}
		}
	}

	// Let our outer fill in most match criteria
	GetOuter()->ModifyOrAppendCriteria( set );

	// Append local player criteria to set, but not if this is a player doing the talking
	if ( !GetOuter()->IsPlayer() )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( 1 );
		if( pPlayer )
			pPlayer->ModifyOrAppendPlayerCriteria( set );
	}
#else
	AI_CriteriaSet localCriteriaSet; // put it on the stack so we don't deal with new/delete
	if (criteria == NULL)
	{
		GatherCriteria( &localCriteriaSet, concept, NULL );
		criteria = &localCriteriaSet;
	}
#endif 

	/// intercept any deferred criteria that are being sent to world
	AI_CriteriaSet worldWritebackCriteria;
	AI_CriteriaSet::InterceptWorldSetContexts( criteria, &worldWritebackCriteria );

	// Now that we have a criteria set, ask for a suitable response
	bool found = rs->FindBestResponse( *criteria, outResponse, this );

	if ( rr_debugresponses.GetInt() == 4 )
	{
		if ( ( GetOuter()->MyNPCPointer() && GetOuter()->m_debugOverlays & OVERLAY_NPC_SELECTED_BIT ) || GetOuter()->IsPlayer() )
		{
			const char *pszName;
			if ( GetOuter()->IsPlayer() )
			{
				pszName = ((CBasePlayer*)GetOuter())->GetPlayerName();
			}
			else
			{
				pszName = GetOuter()->GetDebugName();
			}

			if ( found )
			{
				char response[ 256 ];
				outResponse.GetResponse( response, sizeof( response ) );

				Warning( "RESPONSERULES: %s spoke '%s'. Found response '%s'.\n", pszName, (const char*)concept, response );
			}
			else
			{
				Warning( "RESPONSERULES: %s spoke '%s'. Found no matching response.\n", pszName, (const char*)concept );
			}
		}
	}

	if ( !found )
	{
		return false;
	}
	else if ( worldWritebackCriteria.GetCount() > 0 )
	{
		Assert( CBaseEntity::Instance( INDEXENT( 0 ) )->IsWorld( ) );
		worldWritebackCriteria.WriteToEntity( CBaseEntity::Instance( INDEXENT( 0 ) ) );
	}

	if ( outResponse.IsEmpty() )
	{
		// AssertMsg2( false, "RR: %s got empty but valid response for %s", GetOuter()->GetDebugName(), concept.GetStringConcept() );
		return false;
	}
	else
	{
		return true;
	}
}

#if 0
//-----------------------------------------------------------------------------
// Purpose: Searches for a possible response; writes it into a response passed as
//			parameter rather than new'ing one up.
// Input  : concept - 
//			NULL - 
// Output : bool : true on success, false on fail 
//-----------------------------------------------------------------------------
AI_Response *CAI_Expresser::SpeakFindResponse( AI_Response *result, AIConcept_t &concept, AI_CriteriaSet *criteria )
{
	Assert(response);

	IResponseSystem *rs = GetOuter()->GetResponseSystem();
	if ( !rs )
	{
		Assert( !"No response system installed for CAI_Expresser::GetOuter()!!!" );
		return NULL;
	}

#if 0
	AI_CriteriaSet set;
	// Always include the concept name
	set.AppendCriteria( "concept", concept, CONCEPT_WEIGHT );

	// Always include any optional modifiers
	if ( modifiers != NULL )
	{
		char copy_modifiers[ 255 ];
		const char *pCopy;
		char key[ 128 ] = { 0 };
		char value[ 128 ] = { 0 };

		Q_strncpy( copy_modifiers, modifiers, sizeof( copy_modifiers ) );
		pCopy = copy_modifiers;

		while( pCopy )
		{
			pCopy = SplitContext( pCopy, key, sizeof( key ), value, sizeof( value ), NULL, modifiers );

			if( *key && *value )
			{
				set.AppendCriteria( key, value, CONCEPT_WEIGHT );
			}
		}
	}

	// Let our outer fill in most match criteria
	GetOuter()->ModifyOrAppendCriteria( set );

	// Append local player criteria to set, but not if this is a player doing the talking
	if ( !GetOuter()->IsPlayer() )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( 1 );
		if( pPlayer )
			pPlayer->ModifyOrAppendPlayerCriteria( set );
	}
#else
	AI_CriteriaSet &set = *criteria;
#endif 

	// Now that we have a criteria set, ask for a suitable response
	bool found = rs->FindBestResponse( set, *result, this );

	if ( rr_debugresponses.GetInt() == 4 )
	{
		if ( ( GetOuter()->MyNPCPointer() && GetOuter()->m_debugOverlays & OVERLAY_NPC_SELECTED_BIT ) || GetOuter()->IsPlayer() )
		{
			const char *pszName;
			if ( GetOuter()->IsPlayer() )
			{
				pszName = ((CBasePlayer*)GetOuter())->GetPlayerName();
			}
			else
			{
				pszName = GetOuter()->GetDebugName();
			}

			if ( found )
			{
				char response[ 256 ];
				result->GetResponse( response, sizeof( response ) );

				Warning( "RESPONSERULES: %s spoke '%s'. Found response '%s'.\n", pszName, concept, response );
			}
			else
			{
				Warning( "RESPONSERULES: %s spoke '%s'. Found no matching response.\n", pszName, concept );
			}
		}
	}

	if ( !found )
	{
		//Assert( !"rs->FindBestResponse: Returned a NULL AI_Response!" );
		return false;
	}

	char response[ 256 ];
	result->GetResponse( response, sizeof( response ) );

	if ( !response[0] )
	{
		return false;
	}

	return true;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Dispatches the result
// Input  : *response - 
//-----------------------------------------------------------------------------
bool CAI_Expresser::SpeakDispatchResponse( AIConcept_t &concept, AI_Response *result,  AI_CriteriaSet *criteria, IRecipientFilter *filter /* = NULL */ )
{
	char response[ 256 ];
	result->GetResponse( response, sizeof( response ) );

	float delay = result->GetDelay();
	
	bool spoke = false;

	soundlevel_t soundlevel = result->GetSoundLevel();

	if ( IsSpeaking() && concept[0] != 0 && result->GetType() != ResponseRules::RESPONSE_PRINT )
	{
		const char *entityName = STRING( GetOuter()->GetEntityName() );
		if ( GetOuter()->IsPlayer() )
		{
			entityName = ToBasePlayer( GetOuter() )->GetPlayerName();
		}
		DevMsg( 2, "SpeakDispatchResponse:  Entity ( %i/%s ) already speaking, forcing '%s'\n", GetOuter()->entindex(), entityName ? entityName : "UNKNOWN", (const char*)concept );

		// Tracker 15911:  Can break the game if we stop an imported map placed lcs here, so only
		//  cancel actor out of instanced scripted scenes.  ywb
		RemoveActorFromScriptedScenes( GetOuter(), true /*instanced scenes only*/ );
		GetOuter()->SentenceStop();

		if ( IsRunningScriptedScene( GetOuter() ) )
		{
			DevMsg( "SpeakDispatchResponse:  Entity ( %i/%s ) refusing to speak due to scene entity, tossing '%s'\n", GetOuter()->entindex(), entityName ? entityName : "UNKNOWN", (const char*)concept );
			return false;
		}
	}

	switch ( result->GetType() )
	{
	default:
	case ResponseRules::RESPONSE_NONE:
		break;

	case ResponseRules::RESPONSE_SPEAK:
		{
			if ( !result->ShouldntUseScene() )
			{
				// This generates a fake CChoreoScene wrapping the sound.txt name
				spoke = SpeakAutoGeneratedScene( response, delay );
			}
			else
			{
				float speakTime = GetResponseDuration( result );
				GetOuter()->EmitSound( response );

				DevMsg( 2, "SpeakDispatchResponse:  Entity ( %i/%s ) playing sound '%s'\n", GetOuter()->entindex(), STRING( GetOuter()->GetEntityName() ), response );
				NoteSpeaking( speakTime, delay );
				spoke = true;
			}
		}
		break;

	case ResponseRules::RESPONSE_SENTENCE:
		{
			spoke = ( -1 != SpeakRawSentence( response, delay, VOL_NORM, soundlevel ) ) ? true : false;
		}
		break;

	case ResponseRules::RESPONSE_SCENE:
		{
			spoke = SpeakRawScene( response, delay, result, filter );
		}
		break;

	case ResponseRules::RESPONSE_RESPONSE:
		{
			// This should have been recursively resolved already
			Assert( 0 );
		}
		break;
	case ResponseRules::RESPONSE_PRINT:
		{
			if ( g_pDeveloper->GetInt() > 0 )
			{
				Vector vPrintPos;
				GetOuter()->CollisionProp()->NormalizedToWorldSpace( Vector(0.5,0.5,1.0f), &vPrintPos );
				NDebugOverlay::Text( vPrintPos, response, true, 1.5 );
			}
				spoke = true;
			}
		break;
	case ResponseRules::RESPONSE_ENTITYIO:
		{
			return FireEntIOFromResponse( response, GetOuter() );
		}
		break;
	}

	if ( spoke )
	{
		m_flLastTimeAcceptedSpeak = gpGlobals->curtime;
		if ( DebuggingSpeech() && g_pDeveloper->GetInt() > 0 && response && result->GetType() != ResponseRules::RESPONSE_PRINT )
		{
			Vector vPrintPos;
			GetOuter()->CollisionProp()->NormalizedToWorldSpace( Vector(0.5,0.5,1.0f), &vPrintPos );
			NDebugOverlay::Text( vPrintPos, CFmtStr( "%s: %s", (const char*)concept, response ), true, 1.5 );
		}

		if ( result->IsApplyContextToWorld() )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( INDEXENT( 0 ) );
			if ( pEntity )
			{
				pEntity->AddContext( result->GetContext() );
			}
		}
		else
		{
			GetOuter()->AddContext( result->GetContext() );
		}
		SetSpokeConcept( concept, result );
	}
	else
	{
	}

	return spoke;
}

bool CAI_Expresser::FireEntIOFromResponse( char *response, CBaseEntity *pInitiator )
{
	// find the space-separator in the response name, then split into entityname, input, and parameter
	// may barf in linux; there, should make some StringTokenizer() class that wraps the strtok_s behavior, etc.
	char *pszEntname;
	char *pszInput; 
	char *pszParam;
	char *strtokContext;

	pszEntname = strtok_s( response, " ", &strtokContext );
	if ( !pszEntname )
	{
		Warning( "Response was entityio but had bad value %s\n", response );
		return false;
	}

	pszInput = strtok_s( NULL, " ", &strtokContext );
	if ( !pszInput )
	{
		Warning( "Response was entityio but had bad value %s\n", response );
		return false;
	}

	pszParam =  strtok_s( NULL, " ", &strtokContext );

	// poke entity io
	CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, pszEntname, pInitiator );
	if ( !pTarget )
	{
		Msg( "Response rule targeted %s with entityio, but that doesn't exist.\n", pszEntname );
		// but this is actually a legit use case, so return true (below).
	}
	else
	{
		// pump the action into the target
		variant_t variant;
		if ( pszParam )
		{
			variant.SetString( MAKE_STRING(pszParam) );
		}
		pTarget->AcceptInput( pszInput, pInitiator, pInitiator, variant, 0 );

	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *response - 
// Output : float
//-----------------------------------------------------------------------------
float CAI_Expresser::GetResponseDuration( AI_Response *result )
{
	Assert( result );
	char response[ 256 ];
	result->GetResponse( response, sizeof( response ) );

	switch ( result->GetType() )
	{
	case ResponseRules::RESPONSE_SPEAK:
		{
			return GetOuter()->GetSoundDuration( response, STRING( GetOuter()->GetModelName() ) );
		}
		break;
	case ResponseRules::RESPONSE_SENTENCE:
		{
			Assert( 0 );
			return 999.0f;
		}
		break;
	case ResponseRules::RESPONSE_SCENE:
		{
			return GetSceneDuration( response );
		}
		break;
	case ResponseRules::RESPONSE_RESPONSE:
		{
			// This should have been recursively resolved already
			Assert( 0 );
		}
		break;
	case ResponseRules::RESPONSE_PRINT:
		{
			return 1.0;
		}
		break;
	default:
	case ResponseRules::RESPONSE_NONE:
	case ResponseRules::RESPONSE_ENTITYIO:
		return 0.0f;
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Placeholder for rules based response system
// Input  : concept - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_Expresser::Speak( AIConcept_t &concept, const char *modifiers /*= NULL*/, char *pszOutResponseChosen /* = NULL*/, size_t bufsize /* = 0 */, IRecipientFilter *filter /* = NULL */ )
{
	concept.SetSpeaker(GetOuter());
	AI_CriteriaSet criteria;
	GatherCriteria(&criteria, concept, modifiers);

	return Speak( concept, &criteria, pszOutResponseChosen, bufsize, filter );
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_Expresser::Speak( AIConcept_t &concept, AI_CriteriaSet * RESTRICT criteria, char *pszOutResponseChosen , size_t bufsize , IRecipientFilter *filter  )
{
	VPROF("CAI_Expresser::Speak");
	if ( IsSpeechGloballySuppressed() )
	{
		return false;
	}

	GetOuter()->ModifyOrAppendDerivedCriteria(*criteria);
	AI_Response result;
	if ( !FindResponse( result, concept, criteria ) )
	{
		return false;
	}

	SpeechMsg( GetOuter(), "%s (%x) spoke %s (%f)", STRING(GetOuter()->GetEntityName()), GetOuter(), (const char*)concept, gpGlobals->curtime );
	// Msg( "%s:%s to %s:%s\n", GetOuter()->GetDebugName(), concept.GetStringConcept(), criteria.GetValue(criteria.FindCriterionIndex("Subject")), pTarget ? pTarget->GetDebugName() : "none" );

	bool spoke = SpeakDispatchResponse( concept, &result, criteria, filter );
	if ( pszOutResponseChosen )
	{
		result.GetResponse( pszOutResponseChosen, bufsize );
	}

	return spoke;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAI_Expresser::SpeakRawScene( const char *pszScene, float delay, AI_Response *response, IRecipientFilter *filter /* = NULL */ )
{
	float sceneLength = GetOuter()->PlayScene( pszScene, delay, response, filter );
	if ( sceneLength > 0 )
	{
		SpeechMsg( GetOuter(), "SpeakRawScene( %s, %f) %f\n", pszScene, delay, sceneLength );

#if defined( HL2_EPISODIC )
		char szInstanceFilename[256];
		GetOuter()->GenderExpandString( pszScene, szInstanceFilename, sizeof( szInstanceFilename ) );
		// Only mark ourselves as speaking if the scene has speech
		if ( GetSceneSpeechCount(szInstanceFilename) > 0 )
		{
			NoteSpeaking( sceneLength, delay );
		}
#else
		NoteSpeaking( sceneLength, delay );
#endif

		return true;
	}
	return false;
}

// This will create a fake .vcd/CChoreoScene to wrap the sound to be played
bool CAI_Expresser::SpeakAutoGeneratedScene( char const *soundname, float delay )
{
	float speakTime = GetOuter()->PlayAutoGeneratedSoundScene( soundname );
	if ( speakTime > 0 )
	{
		SpeechMsg( GetOuter(), "SpeakAutoGeneratedScene( %s, %f) %f\n", soundname, delay, speakTime );
		NoteSpeaking( speakTime, delay );
		return true;
	}
	return false;
}

//-------------------------------------

int CAI_Expresser::SpeakRawSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, CBaseEntity *pListener )
{
	int sentenceIndex = -1;

	if ( !pszSentence )
		return sentenceIndex;

	if ( pszSentence[0] == AI_SP_SPECIFIC_SENTENCE )
	{
		sentenceIndex = SENTENCEG_Lookup( pszSentence );

		if( sentenceIndex == -1 )
		{
			// sentence not found
			return -1;
		}

		CPASAttenuationFilter filter( GetOuter(), soundlevel );
		CBaseEntity::EmitSentenceByIndex( filter, GetOuter()->entindex(), CHAN_VOICE, sentenceIndex, volume, soundlevel, 0, GetVoicePitch());
	}
	else
	{
		sentenceIndex = SENTENCEG_PlayRndSz( GetOuter()->NetworkProp()->edict(), pszSentence, volume, soundlevel, 0, GetVoicePitch() );
	}

	SpeechMsg( GetOuter(), "SpeakRawSentence( %s, %f) %f\n", pszSentence, delay, engine->SentenceLength( sentenceIndex ) );
	NoteSpeaking( engine->SentenceLength( sentenceIndex ), delay );

	return sentenceIndex;
}

//-------------------------------------

void CAI_Expresser::BlockSpeechUntil( float time ) 	
{ 
	SpeechMsg( GetOuter(), "BlockSpeechUntil(%f) %f\n", time, time - gpGlobals->curtime );
	m_flBlockedTalkTime = time; 
}


//-------------------------------------

void CAI_Expresser::NoteSpeaking( float duration, float delay )
{
	duration += delay;
	
	GetSink()->OnStartSpeaking();

	if ( duration <= 0 )
	{
		// no duration :( 
		m_flStopTalkTime = gpGlobals->curtime + 3;
		duration = 0;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->curtime + duration;
	}

	m_flStopTalkTimeWithoutDelay = m_flStopTalkTime - delay;

	SpeechMsg( GetOuter(), "NoteSpeaking( %f, %f ) (stop at %f)\n", duration, delay, m_flStopTalkTime );

	if ( GetSink()->UseSemaphore() )
	{
		CAI_TimedSemaphore *pSemaphore = GetMySpeechSemaphore( GetOuter() );
		if ( pSemaphore )
		{
			pSemaphore->Acquire( duration, GetOuter() );
		}
	}
}

//-------------------------------------

void CAI_Expresser::ForceNotSpeaking( void )
{
	if ( IsSpeaking() )
	{
		m_flStopTalkTime = gpGlobals->curtime;
		m_flStopTalkTimeWithoutDelay = gpGlobals->curtime;

		CAI_TimedSemaphore *pSemaphore = GetMySpeechSemaphore( GetOuter() );
		if ( pSemaphore )
		{
			if ( pSemaphore->GetOwner() == GetOuter() )
			{
				pSemaphore->Release();
			}
		}
	}
}

//-------------------------------------

bool CAI_Expresser::IsSpeaking( void )
{
	if ( m_flStopTalkTime > gpGlobals->curtime )
		SpeechMsg( GetOuter(), "IsSpeaking() %f\n", m_flStopTalkTime - gpGlobals->curtime );

	if ( m_flLastTimeAcceptedSpeak == gpGlobals->curtime ) // only one speak accepted per think
		return true;

	return ( m_flStopTalkTime > gpGlobals->curtime );
}

//-------------------------------------

bool CAI_Expresser::CanSpeak()
{
	if ( m_flLastTimeAcceptedSpeak == gpGlobals->curtime ) // only one speak accepted per think
		return false;

	float timeOk = MAX( m_flStopTalkTime, m_flBlockedTalkTime );
	return ( timeOk <= gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if it's ok for this entity to speak after himself.
//			The base CanSpeak() includes the default speech delay, and won't
//			return true until that delay time has passed after finishing the
//			speech. This returns true as soon as the speech finishes.
//-----------------------------------------------------------------------------
bool CAI_Expresser::CanSpeakAfterMyself()
{
	if ( m_flLastTimeAcceptedSpeak == gpGlobals->curtime ) // only one speak accepted per think
		return false;

	float timeOk = MAX( m_flStopTalkTimeWithoutDelay, m_flBlockedTalkTime );
	return ( timeOk <= gpGlobals->curtime );
}

//-------------------------------------
bool CAI_Expresser::CanSpeakConcept( AIConcept_t concept )
{
	// Not in history?
	int iter = m_ConceptHistories.Find( concept );
	if ( iter == m_ConceptHistories.InvalidIndex() )
	{
		return true;
	}

	ConceptHistory_t *history = &m_ConceptHistories[iter];
	Assert( history );

	const AI_Response &response = history->m_response;
	if ( response.IsEmpty() )
		return true;

	if ( response.GetSpeakOnce() ) 
		return false;

	float respeakDelay = response.GetRespeakDelay();

	if ( respeakDelay != 0.0f )
	{
		if ( history->timeSpoken != -1 && ( gpGlobals->curtime < history->timeSpoken + respeakDelay ) )
			return false;
	}

	return true;
}

//-------------------------------------

bool CAI_Expresser::SpokeConcept( AIConcept_t concept )
{
	return GetTimeSpokeConcept( concept ) != -1.f;
}

//-------------------------------------

float CAI_Expresser::GetTimeSpokeConcept( AIConcept_t concept )
{
	int iter = m_ConceptHistories.Find( concept );
	if ( iter == m_ConceptHistories.InvalidIndex() ) 
		return -1;
	
	ConceptHistory_t *h = &m_ConceptHistories[iter];
	return h->timeSpoken;
}

//-------------------------------------

void CAI_Expresser::SetSpokeConcept( AIConcept_t concept, AI_Response *response, bool bCallback )
{
	int idx = m_ConceptHistories.Find( concept );
	if ( idx == m_ConceptHistories.InvalidIndex() )
	{
		ConceptHistory_t h;
		h.timeSpoken = gpGlobals->curtime;
		idx = m_ConceptHistories.Insert( concept, h );
	}

	ConceptHistory_t *slot = &m_ConceptHistories[ idx ];

	slot->timeSpoken = gpGlobals->curtime;
	// Update response info
	if ( response )
	{
		slot->m_response = *response;
	}

	if ( bCallback )
		GetSink()->OnSpokeConcept( concept, response );
}

//-------------------------------------

void CAI_Expresser::ClearSpokeConcept( AIConcept_t concept )
{
	m_ConceptHistories.Remove( concept );
}

//-------------------------------------

void CAI_Expresser::DumpHistories()
{
	int c = 1;
	for ( int i = m_ConceptHistories.First(); i != m_ConceptHistories.InvalidIndex(); i = m_ConceptHistories.Next(i ) )
	{
		ConceptHistory_t *h = &m_ConceptHistories[ i ];

		DevMsg( "%i: %s at %f\n", c++, m_ConceptHistories.GetElementName( i ), h->timeSpoken );
	}
}

//-------------------------------------

bool CAI_Expresser::IsValidResponse( ResponseType_t type, const char *pszValue )
{
	if ( type == ResponseRules::RESPONSE_SCENE )
	{
		char szInstanceFilename[256];
		GetOuter()->GenderExpandString( pszValue, szInstanceFilename, sizeof( szInstanceFilename ) );
		return ( GetSceneDuration( szInstanceFilename ) > 0 );
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAI_TimedSemaphore *CAI_Expresser::GetMySpeechSemaphore( CBaseEntity *pNpc ) 
{
	if ( !pNpc->MyNPCPointer() )
		return NULL;

	return (pNpc->MyNPCPointer()->IsPlayerAlly() ? &g_AIFriendliesTalkSemaphore : &g_AIFoesTalkSemaphore );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_Expresser::SpeechMsg( CBaseEntity *pFlex, const char *pszFormat, ... )
{
	if ( !DebuggingSpeech() )
		return;

	va_list args;

	va_start( args, pszFormat );
	if ( pFlex->MyNPCPointer() )
	{

		DevMsg( pFlex->MyNPCPointer(), "%s", CFmtStr( &pszFormat, args ).Access() );
	}
	else 
	{
		DevMsg( "%s", CFmtStr( &pszFormat, args ).Access() );
	}
	char szFormat[] = "%s"; // UTIL_LogPrintf wants a mutable format string for some reason
	UTIL_LogPrintf( szFormat, CFmtStr( &pszFormat, args ).Access() );
	va_end( args );
}

//-----------------------------------------------------------------------------
// Purpose: returns true when l4d is in credits screen or some other
// speech-forbidden state
//-----------------------------------------------------------------------------
bool CAI_Expresser::IsSpeechGloballySuppressed()
{
	return false;
}

//-----------------------------------------------------------------------------

void CAI_ExpresserHost_NPC_DoModifyOrAppendCriteria( CAI_BaseNPC *pSpeaker, AI_CriteriaSet& set )
{
	// Append current activity name
	const char *pActivityName = pSpeaker->GetActivityName( pSpeaker->GetActivity() );
	if ( pActivityName )
	{
  		set.AppendCriteria( "activity", pActivityName );
	}

	static const char *pStateNames[] = { "None", "Idle", "Alert", "Combat", "Scripted", "PlayDead", "Dead" };
	if ( (int)pSpeaker->m_NPCState < ARRAYSIZE(pStateNames) )
	{
		set.AppendCriteria( "npcstate", UTIL_VarArgs( "[NPCState::%s]", pStateNames[pSpeaker->m_NPCState] ) );
	}

	if ( pSpeaker->GetEnemy() )
	{
		set.AppendCriteria( "enemy", pSpeaker->GetEnemy()->GetClassname() );
		set.AppendCriteria( "timesincecombat", "-1" );
	}
	else
	{
		if ( pSpeaker->GetLastEnemyTime() == 0.0 )
			set.AppendCriteria( "timesincecombat", "999999.0" );
		else
			set.AppendCriteria( "timesincecombat", UTIL_VarArgs( "%f", gpGlobals->curtime - pSpeaker->GetLastEnemyTime() ) );
	}

	set.AppendCriteria( "speed", UTIL_VarArgs( "%.3f", pSpeaker->GetSmoothedVelocity().Length() ) );

	CBaseCombatWeapon *weapon = pSpeaker->GetActiveWeapon();
	if ( weapon )
	{
		set.AppendCriteria( "weapon", weapon->GetClassname() );
	}
	else
	{
		set.AppendCriteria( "weapon", "none" );
	}

	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer )
	{
		Vector distance = pPlayer->GetAbsOrigin() - pSpeaker->GetAbsOrigin();

		set.AppendCriteria( "distancetoplayer", UTIL_VarArgs( "%f", distance.Length() ) );

	}
	else
	{
		set.AppendCriteria( "distancetoplayer", UTIL_VarArgs( "%i", MAX_COORD_RANGE ) );
	}

	if ( pSpeaker->HasCondition( COND_SEE_PLAYER ) )
	{
		set.AppendCriteria( "seeplayer", "1" );
	}
	else
	{
		set.AppendCriteria( "seeplayer", "0" );
	}

	if ( pPlayer && pPlayer->FInViewCone( pSpeaker ) && pPlayer->FVisible( pSpeaker ) )
	{
		set.AppendCriteria( "seenbyplayer", "1" );
	}
	else
	{
		set.AppendCriteria( "seenbyplayer", "0" );
	}
}

//-----------------------------------------------------------------------------

CON_COMMAND_F( npc_speakall, "Force the npc to try and speak all their responses", FCVAR_CHEAT )
{
	CBaseEntity *pEntity;

	if ( args[1] && *args[1] )
	{
		pEntity = gEntList.FindEntityByName( NULL, args[1], NULL );
		if ( !pEntity )
		{
			pEntity = gEntList.FindEntityByClassname( NULL, args[1] );
		}
	}
	else
	{
		pEntity = UTIL_GetCommandClient() ? UTIL_GetCommandClient()->FindPickerEntity() : NULL;
	}
		
	if ( pEntity )
	{
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
		if (pNPC)
		{
			if ( pNPC->GetExpresser() )
			{
				bool save = engine->LockNetworkStringTables( false );
				pNPC->GetExpresser()->TestAllResponses();
				engine->LockNetworkStringTables( save );
			}
		}
	}
}
//-----------------------------------------------------------------------------

CMultiplayer_Expresser::CMultiplayer_Expresser( CBaseFlex *pOuter ) : CAI_ExpresserWithFollowup( pOuter )
{
	m_bAllowMultipleScenes = false;
}

bool CMultiplayer_Expresser::IsSpeaking( void )
{
	if ( m_bAllowMultipleScenes )
	{
		return false;
	}

	return CAI_Expresser::IsSpeaking();
}


void CMultiplayer_Expresser::AllowMultipleScenes()
{
	m_bAllowMultipleScenes = true;
}

void CMultiplayer_Expresser::DisallowMultipleScenes()
{
	m_bAllowMultipleScenes = false;
}
