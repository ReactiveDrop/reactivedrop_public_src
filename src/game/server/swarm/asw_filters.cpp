#include "cbase.h"
#include "filters.h"
#include "asw_marine.h"
#include "asw_melee_system.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CASW_Filter_Rolling : public CBaseFilter
{
	DECLARE_CLASS(CASW_Filter_Rolling, CBaseFilter);

public:
	bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine(pEntity);
		return pMarine && pMarine->m_iMeleeAttackID.Get() == CASW_Melee_System::s_nRollAttackID;
	}
};

LINK_ENTITY_TO_CLASS(asw_filter_rolling, CASW_Filter_Rolling);

class CASW_Filter_Marine_Class : public CBaseFilter
{
	DECLARE_CLASS( CASW_Filter_Marine_Class, CBaseFilter );

public:
	DECLARE_DATADESC();

	ASW_Marine_Class m_iMarineClass;

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity )
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine( pEntity );
		CASW_Marine_Profile *pProfile = pMarine ? pMarine->GetMarineProfile() : NULL;
		return pProfile && pProfile->GetMarineClass() == m_iMarineClass;
	}
};

LINK_ENTITY_TO_CLASS( asw_filter_marine_class, CASW_Filter_Marine_Class );

BEGIN_DATADESC( CASW_Filter_Marine_Class )
	DEFINE_KEYFIELD( m_iMarineClass, FIELD_INTEGER, "MarineClass" )
END_DATADESC()
