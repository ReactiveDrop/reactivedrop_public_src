#include "cbase.h"
#include "rd_briefing_camera.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


BEGIN_DATADESC( CRD_Briefing_Camera )
	// BenLubar: To avoid having to define a sendtable and a client class, let's just use a field that already gets networked in CBaseEntity.
	DEFINE_KEYFIELD( m_flElasticity, FIELD_FLOAT, "fov" ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( rd_briefing_camera, CRD_Briefing_Camera );

CRD_Briefing_Camera::CRD_Briefing_Camera()
{
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
}

int CRD_Briefing_Camera::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_PVSCHECK;
}
