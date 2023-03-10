#include "cbase.h"
#include "asw_detonator.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_detonator, CASW_Detonator );

BEGIN_DATADESC( CASW_Detonator )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Detonator, DT_ASW_Detonator )
END_SEND_TABLE()
#endif
