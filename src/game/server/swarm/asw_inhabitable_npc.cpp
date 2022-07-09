#include "cbase.h"
#include "asw_inhabitable_npc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( funCASW_Inhabitable_NPC, CASW_Inhabitable_NPC );

IMPLEMENT_SERVERCLASS_ST(CASW_Inhabitable_NPC, DT_ASW_Inhabitable_NPC)
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Inhabitable_NPC )
END_DATADESC()

CASW_Inhabitable_NPC::CASW_Inhabitable_NPC()
{
}


CASW_Inhabitable_NPC::~CASW_Inhabitable_NPC()
{
}
