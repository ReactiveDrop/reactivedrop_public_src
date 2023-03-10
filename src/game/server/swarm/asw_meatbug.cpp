#include "cbase.h"
#include "asw_meatbug.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_ENEMIES
LINK_ENTITY_TO_CLASS( asw_meatbug, CASW_Meatbug );

BEGIN_DATADESC( CASW_Meatbug )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Meatbug, DT_ASW_Meatbug )
END_SEND_TABLE()
#endif
