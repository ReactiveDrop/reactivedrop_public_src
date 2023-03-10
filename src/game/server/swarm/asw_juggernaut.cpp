#include "cbase.h"
#include "asw_juggernaut.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_juggernaut, CASW_Juggernaut );

BEGIN_DATADESC( CASW_Juggernaut )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Juggernaut, DT_ASW_Juggernaut )
END_SEND_TABLE()
#endif
