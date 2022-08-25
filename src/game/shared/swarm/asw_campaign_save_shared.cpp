#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_asw_campaign_save.h"
#define CASW_Campaign_Save C_ASW_Campaign_Save
#else
#include "asw_campaign_save.h"
#endif
#include "asw_gamerules.h"
#include "rd_missions_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool CASW_Campaign_Save::IsMissionLinkedToACompleteMission( int i, const RD_Campaign_t *pCampaignInfo )
{
	if ( !pCampaignInfo || !ASWGameRules() )
		return false;

	const RD_Campaign_Mission_t *pMission = pCampaignInfo->GetMission( i );
	if ( !pMission )
		return false;

	// last mission, don't reveal it unless all other missions are complete
	if ( i == pCampaignInfo->Missions.Count() - 1 )
	{
		if ( ASWGameRules()->CampaignMissionsLeft() > 1 )
			return false;
	}

	FOR_EACH_VEC( pMission->Links, k )
	{
		int iLinked = pMission->Links[k];
		if ( iLinked >= 0 && iLinked < ASW_MAX_MISSIONS_PER_CAMPAIGN && ( m_MissionComplete[iLinked] == 1 || iLinked == 0 ) )
			return true;
	}

	return false;
}

bool CASW_Campaign_Save::IsMarineAlive(int iProfileIndex)
{
	if (iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES)
		return false;

	return !m_bMarineDead[iProfileIndex];
}

bool CASW_Campaign_Save::IsMarineWounded(int iProfileIndex)
{
	if (iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES)
		return false;

	return m_bMarineWounded[iProfileIndex];
}
	