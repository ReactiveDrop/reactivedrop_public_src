#include "cbase.h"
#include "asw_runner.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_runner, CASW_Runner );

BEGIN_DATADESC( CASW_Runner )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Runner, DT_ASW_Runner )
END_SEND_TABLE()
#endif
