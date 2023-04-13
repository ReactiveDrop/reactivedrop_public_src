//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_link.h"
#include "ai_network.h"
#include "ai_node.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_ENT_SCRIPTDESC_ROOT( CAI_Link, "AI Link class" )
	DEFINE_SCRIPT_INSTANCE_HELPER( &g_AILinkScriptInstanceHelper )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetSrcNodeID, "GetSrcNodeID", "Get the ID of the node that 'owns' this link." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetDestNodeID, "GetDestNodeID", "Get the ID of the node on the other end of the link." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetAcceptedMoveTypes, "GetAcceptedMoveTypes", "Get the Capability_T of motions acceptable for passed hull type." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetLinkInfo, "GetLinkInfo", "Get other information about this link." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetTimeStaleExpires, "GetTimeStaleExpires", "Returns the amount of time until this link is available again." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetDangerCount, "GetDangerCount", "Returns how many dangerous things are near this link." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetLinkInfo, "SetLinkInfo", "Sets information about this link." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetTimeStaleExpires, "SetTimeStaleExpires", "Sets the amount of time until this link is available again." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetDangerCount, "SetDangerCount", "Sets how many dangerous things are near this link." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetAcceptedMoveTypes, "SetAcceptedMoveTypes", "Set the Capability_T of motions acceptable for passed hull type." )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetDynamicLink, "GetDynamicLink", "Returns the info_node_link entity for this link or null if it doesn't exist." )
END_SCRIPTDESC();


ASSERT_INVARIANT( ( bits_LINK_STALE_SUGGESTED | bits_LINK_OFF ) <= 255 && ( AI_MOVE_TYPE_BITS <= 255 ) );
//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CAI_Link::CAI_Link(void)
{
	m_iSrcID			= -1;
	m_iDestID			= -1;
	m_LinkInfo			= 0;
	m_timeStaleExpires	= 0;
	m_pDynamicLink		= NULL;
	m_nDangerCount		= 0;
	for (int hull=0;hull<NUM_HULLS;hull++)
	{
		m_iAcceptedMoveTypes[hull] = 0;
	}
	m_hScriptInstance = NULL;
};

//-----------------------------------------------------------------------------
// Purpose: Destructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CAI_Link::~CAI_Link(void)
{
	if ( m_hScriptInstance )
	{
		if ( g_pScriptVM )
			g_pScriptVM->RemoveInstance( m_hScriptInstance );
		m_hScriptInstance = NULL;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HSCRIPT CAI_Link::GetScriptInstance()
{
	if ( !m_hScriptInstance )
	{
		if ( g_pScriptVM )
			m_hScriptInstance = g_pScriptVM->RegisterInstance( GetScriptDesc(), this );
	}
	return m_hScriptInstance;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CAI_Link::ScriptGetAcceptedMoveTypes( int hullType ) const
{
	if ( hullType < 0 || hullType > ( NUM_HULLS - 1 ) )
	{
		DevMsg ("Error: Invalid hullType %i in CAI_Link::GetAcceptedMoveTypes. Valid types are 0-%i\n", hullType, (NUM_HULLS - 1) );
		return -1;
	}

	return m_iAcceptedMoveTypes[hullType];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CAI_Link::ScriptSetAcceptedMoveTypes( int hullType, int moveType )
{
	if ( hullType < 0 || hullType > ( NUM_HULLS - 1 ) )
	{
		DevMsg ("Error: Invalid hullType %i in CAI_Link::SetAcceptedMoveTypes. Valid types are 0-%i\n", hullType, (NUM_HULLS - 1) );
		return;
	}

	m_iAcceptedMoveTypes[hullType] = moveType;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HSCRIPT CAI_Link::ScriptGetDynamicLink()
{
	if ( !m_pDynamicLink )
		return NULL;

	CBaseEntity *pDynamicLink = ( CBaseEntity* )m_pDynamicLink;
	return ToHScript( pDynamicLink );
}

void CAI_Link::ClearStaleLinks()
{
	for ( int i = 0; i < g_pBigAINet->NumNodes(); i++ )
	{
		CAI_Node *pNode = g_pBigAINet->GetNode( i );
		for ( int j = 0; j < pNode->NumLinks(); j++ )
		{
			pNode->GetLinkByIndex( j )->m_LinkInfo &= ~bits_LINK_STALE_SUGGESTED;
		}
	}
}
