#include "cbase.h"
#include "ai_networkmanager.h"
#include "npc_strider.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRD_NetworkBuildHelper : public CAI_NetworkBuildHelper
{
	DECLARE_CLASS( CRD_NetworkBuildHelper, CAI_NetworkBuildHelper );

	void PostInitNodePosition( CAI_Network *pNetwork, CAI_Node *pNode ) override
	{
		AdjustStriderNodePosition( pNetwork, pNode );
	}
};

LINK_ENTITY_TO_CLASS( ai_network_build_helper, CRD_NetworkBuildHelper );
