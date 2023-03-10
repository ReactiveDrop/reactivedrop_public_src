#include "cbase.h"
#include "asw_spitter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_spitter, CASW_Spitter );

BEGIN_DATADESC( CASW_Spitter )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Spitter, DT_ASW_Spitter )
END_SEND_TABLE()
#endif
