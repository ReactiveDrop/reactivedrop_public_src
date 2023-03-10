#include "cbase.h"
#include "asw_flock.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_flock, CASW_Flock );

BEGIN_DATADESC( CASW_Flock )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Flock, DT_ASW_Flock )
END_SEND_TABLE()
#endif
