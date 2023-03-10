#include "cbase.h"
#include "asw_watcher.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_watcher, CASW_Watcher );

BEGIN_DATADESC( CASW_Watcher )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Watcher, DT_ASW_Watcher )
END_SEND_TABLE()
#endif
