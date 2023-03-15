#include "cbase.h"
#include "asw_tech_marine_req.h"
#include "asw_gamerules.h"
#include "asw_game_resource.h"
#include "asw_marine_resource.h"
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_tech_marine_req, CASW_Tech_Marine_Req );

BEGIN_DATADESC( CASW_Tech_Marine_Req )
	DEFINE_INPUTFUNC( FIELD_VOID,	"DisableTechMarineReq",	InputDisableTechMarineReq ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"EnableTechMarineReq",	InputEnableTechMarineReq ),
END_DATADESC()


void CASW_Tech_Marine_Req::Spawn()
{
	BaseClass::Spawn();

	//Msg("CASW_Tech_Marine_Req::Spawn setting mission requires tech to true\n");
	if (ASWGameRules())
	{
		ASWGameRules()->m_bMissionRequiresTech = true;
	}
}

void CASW_Tech_Marine_Req::InputDisableTechMarineReq( inputdata_t &inputdata )
{
	//Msg("CASW_Tech_Marine_Req::InputDisableTechMarineReq setting mission requires tech to false\n");
	if (ASWGameRules())
	{
		ASWGameRules()->m_bMissionRequiresTech = false;
	}
}

void CASW_Tech_Marine_Req::InputEnableTechMarineReq( inputdata_t &inputdata )
{
	//Msg("CASW_Tech_Marine_Req::InputEnableTechMarineReq setting mission requires tech to true\n");
	if (ASWGameRules())
	{
		ASWGameRules()->m_bMissionRequiresTech = true;

		// the tech could have died since we were last active; check now if we're in-game
		if ( ASWGameRules()->GetGameState() == ASW_GS_INGAME && ASWGameRules()->MissionRequiresTech() )
		{
			CASW_Game_Resource *pGameResource = ASWGameResource();
			if ( pGameResource )
			{
				// count number of live techs
				bool bTech = false;
				for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
				{
					CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
					if ( pMR && pMR->GetHealthPercent() > 0 && pMR->GetProfile() && pMR->GetProfile()->CanHack() )
					{
						bTech = true;
						break;
					}
				}
				if ( !bTech && pGameResource->CountAllAliveMarines() > 0 )
				{
					float flDelay = gEntList.FindEntityByClassname( NULL, "asw_weapon_hack_tool" ) ? 5.0f : 1.5f;
					ASWGameRules()->ScheduleTechFailureRestart( gpGlobals->curtime + flDelay );
				}
			}
		}
	}
}