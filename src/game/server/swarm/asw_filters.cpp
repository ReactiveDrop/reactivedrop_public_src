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
