#include "cbase.h"
#include "asw_objective.h"
#include "asw_game_resource.h"
#include "asw_mission_manager.h"
#include "asw_gamerules.h"
#include "asw_player.h"
#include "asw_marker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_mission_objective, CASW_Objective );

IMPLEMENT_SERVERCLASS_ST( CASW_Objective, DT_ASW_Objective )
	SendPropStringT( SENDINFO( m_ObjectiveTitle ) ),
	SendPropStringT( SENDINFO( m_ObjectiveDescription1 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveDescription2 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveDescription3 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveDescription4 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveImage ) ),
	SendPropStringT( SENDINFO( m_ObjectiveInfoIcon1 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveInfoIcon2 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveInfoIcon3 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveInfoIcon4 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveInfoIcon5 ) ),
	SendPropStringT( SENDINFO( m_ObjectiveIcon ) ),
	SendPropStringT( SENDINFO( m_LegacyMapMarkings ) ),
	SendPropInt( SENDINFO( m_Priority ) ),
	SendPropBool( SENDINFO( m_bComplete ) ),
	SendPropBool( SENDINFO( m_bFailed ) ),
	SendPropBool( SENDINFO( m_bOptional ) ),
	SendPropBool( SENDINFO( m_bDummy ) ),
	SendPropBool( SENDINFO( m_bVisible ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Objective )
	DEFINE_KEYFIELD( m_ObjectiveTitle, FIELD_STRING, "objectivetitle" ),
	DEFINE_KEYFIELD( m_ObjectiveDescription1, FIELD_STRING, "objectivedescription1" ),
	DEFINE_KEYFIELD( m_ObjectiveDescription2, FIELD_STRING, "objectivedescription2" ),
	DEFINE_KEYFIELD( m_ObjectiveDescription3, FIELD_STRING, "objectivedescription3" ),
	DEFINE_KEYFIELD( m_ObjectiveDescription4, FIELD_STRING, "objectivedescription4" ),
	DEFINE_KEYFIELD( m_ObjectiveImage, FIELD_STRING, "objectiveimage" ),
	DEFINE_KEYFIELD( m_ObjectiveInfoIcon1, FIELD_STRING, "objectiveinfoicon1" ),
	DEFINE_KEYFIELD( m_ObjectiveInfoIcon2, FIELD_STRING, "objectiveinfoicon2" ),
	DEFINE_KEYFIELD( m_ObjectiveInfoIcon3, FIELD_STRING, "objectiveinfoicon3" ),
	DEFINE_KEYFIELD( m_ObjectiveInfoIcon4, FIELD_STRING, "objectiveinfoicon4" ),
	DEFINE_KEYFIELD( m_ObjectiveInfoIcon5, FIELD_STRING, "objectiveinfoicon5" ),
	DEFINE_KEYFIELD( m_ObjectiveIcon, FIELD_STRING, "objectiveicon" ),
	DEFINE_KEYFIELD( m_LegacyMapMarkings, FIELD_STRING, "mapmarkings" ),
	DEFINE_KEYFIELD( m_Priority, FIELD_INTEGER, "Priority" ),
	DEFINE_KEYFIELD( m_bOptional, FIELD_BOOLEAN, "Optional" ),
	DEFINE_KEYFIELD( m_bVisible, FIELD_BOOLEAN, "Visible" ),
	DEFINE_FIELD( m_bFailed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bComplete, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDummy, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetVisible", InputSetVisible ),
	DEFINE_OUTPUT( m_OnObjectiveComplete, "OnObjectiveComplete" ),
END_DATADESC()

CASW_Objective::CASW_Objective()
{
	m_ObjectiveTitle = NULL_STRING;
	m_ObjectiveDescription1 = NULL_STRING;
	m_ObjectiveDescription2 = NULL_STRING;
	m_ObjectiveDescription3 = NULL_STRING;
	m_ObjectiveDescription4 = NULL_STRING;
	m_ObjectiveImage = NULL_STRING;
	m_ObjectiveInfoIcon1 = NULL_STRING;
	m_ObjectiveInfoIcon2 = NULL_STRING;
	m_ObjectiveInfoIcon3 = NULL_STRING;
	m_ObjectiveInfoIcon4 = NULL_STRING;
	m_ObjectiveInfoIcon5 = NULL_STRING;
	m_ObjectiveIcon = NULL_STRING;
	m_LegacyMapMarkings = NULL_STRING;

	m_Priority = 0;
	m_bOptional = false;
	m_bVisible = true;
	m_bFailed = false;
	m_bComplete = false;
	m_bDummy = false;
}

CASW_Objective::~CASW_Objective()
{
}

// always send this info to players
int CASW_Objective::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

void CASW_Objective::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Game.ObjectiveComplete" );
}

void CASW_Objective::Spawn( void )
{
	BaseClass::Spawn();

	if ( ASWGameResource() )
	{
		// objective was created after level load; re-init objective list
		ASWGameResource()->FindObjectives();
	}
}

void CASW_Objective::SetComplete( bool bComplete )
{
	bool bOld = m_bComplete;
	m_bComplete = bComplete;

	if ( !bOld && bComplete )
	{
		m_OnObjectiveComplete.FireOutput( this, this );

		CAlienSwarm *pGameRules = ASWGameRules();
		CASW_Mission_Manager *pManager = pGameRules ? pGameRules->GetMissionManager() : NULL;
		if ( pManager && pGameRules->GetGameState() == ASW_GS_INGAME ) // only emit objective complete sounds in the middle of a mission
		{
			if ( !pManager->CheckMissionComplete() )
			{
				// play objective complete sound
				pGameRules->BroadcastSound( "Game.ObjectiveComplete" );
			}
		}
	}
}

void CASW_Objective::SetFailed( bool bFailed )
{
	m_bFailed = bFailed;

	CAlienSwarm *pGameRules = ASWGameRules();
	CASW_Mission_Manager *pManager = pGameRules ? pGameRules->GetMissionManager() : NULL;
	if ( pManager )
	{
		pManager->CheckMissionComplete();
	}
}

void CASW_Objective::InputSetVisible( inputdata_t &inputdata )
{
	m_bVisible = inputdata.value.Bool();

	CReliableBroadcastRecipientFilter filter;
	UserMessageBegin( filter, "ShowObjectives" );
	WRITE_FLOAT( 5.0f );
	MessageEnd();
}
