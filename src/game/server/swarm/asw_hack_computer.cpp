#include "cbase.h"
#include "asw_marine_resource.h"
#include "asw_marine_profile.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_hack_computer.h"
#include "asw_remote_turret_shared.h"
#include "asw_point_camera.h"
#include "asw_computer_area.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_hack_computer, CASW_Hack_Computer );

IMPLEMENT_SERVERCLASS_ST(CASW_Hack_Computer, DT_ASW_Hack_Computer)	
	SendPropInt		(SENDINFO(m_iNumTumblers)),
	SendPropInt		(SENDINFO(m_iEntriesPerTumbler)),
	SendPropBool	(SENDINFO(m_bLastAllCorrect)),
	
	SendPropFloat	(SENDINFO(m_fMoveInterval)),
	SendPropFloat	(SENDINFO(m_fNextMoveTime), 0, SPROP_NOSCALE ),	
	SendPropFloat	(SENDINFO(m_fFastFinishTime), 0, SPROP_NOSCALE ),
	SendPropArray3  (SENDINFO_ARRAY3(m_iTumblerPosition), SendPropInt( SENDINFO_ARRAY(m_iTumblerPosition), 12 ) ),	
	SendPropArray3  (SENDINFO_ARRAY3(m_iTumblerCorrectNumber), SendPropInt( SENDINFO_ARRAY(m_iTumblerCorrectNumber), 12 ) ),	
	SendPropArray3  (SENDINFO_ARRAY3(m_iTumblerDirection), SendPropInt( SENDINFO_ARRAY(m_iTumblerDirection), 12 ) ),	
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Hack_Computer )
	DEFINE_FIELD(m_bSetupComputer, FIELD_BOOLEAN),
	DEFINE_FIELD(m_fFastFinishTime, FIELD_TIME),
END_DATADESC()

extern ConVar asw_debug_medals;

CASW_Hack_Computer::CASW_Hack_Computer()
{
	m_iShowOption = 0;		// the main menu
	m_bPlayedTimeOutSound = false;
	
	m_fNextMoveTime = 0;
	m_iLastNumWrong = 0;
	m_bLastAllCorrect = false;
	m_bLastHalfCorrect = false;
}

bool CASW_Hack_Computer::InitHack(CASW_Player* pHackingPlayer, CASW_Marine* pHackingMarine, CBaseEntity* pHackTarget)
{
	if ( !pHackTarget )
		return false;

	if ( pHackTarget->Classify() != CLASS_ASW_COMPUTER_AREA )
		return false;

	if ( !pHackingPlayer || !pHackingMarine || !pHackingMarine->GetMarineResource() )
		return false;

	CASW_Computer_Area* pComputer = assert_cast<CASW_Computer_Area*>(pHackTarget);

	if (!m_bSetupComputer)
	{
		m_bSetupComputer = true;
		m_iNumTumblers = pComputer->m_iHackLevel;	// hack level controls how many tumblers there should be
		if (m_iNumTumblers <= 1)
			m_iNumTumblers = 5;
		if (m_iNumTumblers > ASW_HACK_COMPUTER_MAX_TUMBLERS)
			m_iNumTumblers = ASW_HACK_COMPUTER_MAX_TUMBLERS;
		m_iEntriesPerTumbler = pComputer->m_iNumEntriesPerTumbler;	// temp for now
		if (m_iEntriesPerTumbler <= ASW_MIN_ENTRIES_PER_TUMBLER)
			m_iEntriesPerTumbler = ASW_MIN_ENTRIES_PER_TUMBLER;
		if (m_iEntriesPerTumbler >= ASW_MAX_ENTRIES_PER_TUMBLER)
			m_iEntriesPerTumbler= ASW_MAX_ENTRIES_PER_TUMBLER;
		// check it's an odd number
		int r = m_iEntriesPerTumbler % 2;
		if (r != 1)
			r -=1;
		m_fMoveInterval = pComputer->m_fMoveInterval;	// idle gap between slides (slides are 0.5f seconds long themselves)
		if (m_fMoveInterval <= 0.3)
			m_fMoveInterval = 0.3f;
		if (m_fMoveInterval > 2.0f)
			m_fMoveInterval= 2.0f;

		if ( m_iEntriesPerTumbler % 2 > 0 )
		{
			m_iEntriesPerTumbler++;
		}
		if (m_iNumTumblers <= 3)
			m_iNumTumblers = 3;

		int k=0;
		bool bIncomplete = false;
		for (int i=0;i<ASW_HACK_COMPUTER_MAX_TUMBLERS;i++)
		{
			if (random->RandomFloat() > 0.5f)
				m_iTumblerDirection.Set(i, -1);
			else
				m_iTumblerDirection.Set(i, 1);
			m_iTumblerPosition.Set(i, random->RandomInt(0, m_iEntriesPerTumbler-1));
			m_iTumblerCorrectNumber.Set(i, random->RandomInt(0, m_iEntriesPerTumbler-1));

			if ( m_iTumblerPosition.Get( i ) % 2 > 0 )
			{
				m_iTumblerPosition.Set( i, ( m_iTumblerPosition.Get( i ) + 1 ) % m_iEntriesPerTumbler.Get() );
			}

			if ( m_iTumblerCorrectNumber.Get( i ) % 2 > 0 )
			{
				m_iTumblerCorrectNumber.Set( i, ( m_iTumblerCorrectNumber.Get( i ) + 1 ) % m_iEntriesPerTumbler.Get() );
			}
			//Msg("Set %d to correct number %d\n", i, m_iTumblerCorrectNumber[i]);
			m_iNewTumblerDirection[i] = 0;
		}
		k++;
		bIncomplete = (GetTumblerProgress() < 1.0f);

		// if the generated puzzle is already solved, then flip every other tumbler
		if ( !bIncomplete )
		{
			for (int i=0;i<ASW_HACK_COMPUTER_MAX_TUMBLERS;i+=2)
			{
				m_iTumblerDirection.Set(i, m_iTumblerDirection[i] );
			}
		}

		// have to set this before checking if it's a PDA
		m_hHackTarget = pHackTarget;

		SetDefaultHackOption();
	}
	else
	{
		// make sure we're back on the right page

		// have to set this before checking if it's a PDA
		m_hHackTarget = pHackTarget;

		SetDefaultHackOption();
	}

	return BaseClass::InitHack(pHackingPlayer, pHackingMarine, pHackTarget);
}

void CASW_Hack_Computer::SelectHackOption(int i)
{
	CASW_Computer_Area* pArea = GetComputerArea();
	if (!pArea)
		return;

	int iOptionType = GetOptionTypeForEntry(i);

	// check we're allowed to do this
	if (pArea->m_bIsLocked)
	{
		if (i != ASW_HACK_OPTION_OVERRIDE)	// in a locked computer, the only valid hack menu option is the override puzzle
			return;

		if (!GetHackingMarine() || !GetHackingMarine()->GetMarineProfile()	// only a hacker can access a locked computer
				|| !GetHackingMarine()->GetMarineProfile()->CanHack())
			return;

		if (m_fFastFinishTime == 0)
		{
			float ideal_time = 0;
			float diff_factor = 1.8f;
			int iSkill = ASWGameRules()->GetSkillLevel();
			if (iSkill == 2)
				diff_factor = 1.55f;
			else if (iSkill == 3)
				diff_factor = 1.50f;
			else if (iSkill >= 4)
				diff_factor = 1.45f;
			// estimate the time for a fast hack
			// try, time taken for each column to rotate through twice
			float time_per_column = m_fMoveInterval * m_iEntriesPerTumbler;
			ideal_time = time_per_column * diff_factor * m_iNumTumblers;
			if (asw_debug_medals.GetBool())
				Msg("Fast hack time is %f, starting at %f\n", ideal_time, gpGlobals->curtime);
			m_fFastFinishTime = gpGlobals->curtime + ideal_time;
		}

		m_iShowOption = i;
		pArea->m_bLoggedIn = true;

		return;
	}
	
	if ( i == ASW_HACK_OPTION_OVERRIDE )
	{
		pArea->m_bLoggedIn = true;

		return;
	}

	if ( i == 0 && GetOptionTypeForEntry( 1 ) == ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_1 && GetOptionTypeForEntry( 2 ) == ASW_COMPUTER_OPTION_TYPE_NONE )
	{
		pArea->m_flStopUsingTime = gpGlobals->curtime;

		return;
	}

	m_iShowOption = i;

	// remember that the boot-up sequence finished
	pArea->m_bLoggedIn = true;

	// make sure the computer area knows if we're viewing mail or not
	bool bViewingMail = (iOptionType == ASW_COMPUTER_OPTION_TYPE_MAIL);
	
	pArea->m_bViewingMail = bViewingMail;

	// make sure the downloading sound isn't playing if we're not doing downloading
	if (iOptionType != ASW_COMPUTER_OPTION_TYPE_DOWNLOAD_DOCS)
	{
		pArea->StopDownloadingSound();
	}

	if (iOptionType == ASW_COMPUTER_OPTION_TYPE_DOWNLOAD_DOCS)
	{
		// note: being on this option causes the computer area's NPCUsing function to count up to downloading the files
	}
	else if (iOptionType == ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_1)
	{
		// camera activation happens in CASW_Player::SetupVisibility
		pArea->m_iActiveCam = 1;
	}
	else if (iOptionType == ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_2)
	{
		pArea->m_iActiveCam = 2;
	}
	else if (iOptionType == ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_3)
	{
		pArea->m_iActiveCam = 3;
	}
	else if (iOptionType == ASW_COMPUTER_OPTION_TYPE_TURRET_1)
	{
		if (!pArea || !pArea->m_hTurret1.Get())
		{
			Warning("Area is null or no turret handle\n");
			return;
		}

		CASW_Remote_Turret *pTurret = pArea->m_hTurret1.Get();
		if (pTurret && GetHackingMarine())
		{
			pTurret->StartedUsingTurret( GetHackingMarine(), pArea );
			GetHackingMarine()->m_hRemoteTurret = pTurret;
			return;
		}
	}
	else if (iOptionType == ASW_COMPUTER_OPTION_TYPE_TURRET_2)
	{
		if (!pArea || !pArea->m_hTurret2.Get())
		{
			Warning("Area is null or no turret handle\n");
			return;
		}

		CASW_Remote_Turret *pTurret = pArea->m_hTurret2.Get();
		if (pTurret && GetHackingMarine())
		{
			pTurret->StartedUsingTurret( GetHackingMarine(), pArea );
			GetHackingMarine()->m_hRemoteTurret = pTurret;
			return;
		}
	}
	else if (iOptionType == ASW_COMPUTER_OPTION_TYPE_TURRET_3)
	{
		if (!pArea || !pArea->m_hTurret3.Get())
		{
			Warning("Area is null or no turret handle\n");
			return;
		}

		CASW_Remote_Turret *pTurret = pArea->m_hTurret3.Get();
		if (pTurret && GetHackingMarine())
		{
			pTurret->StartedUsingTurret( GetHackingMarine(), pArea );
			GetHackingMarine()->m_hRemoteTurret = pTurret;
			return;
		}
	}
}

bool CASW_Hack_Computer::IsPDA()
{
	CASW_Computer_Area *pArea = GetComputerArea();
	bool bPDA = (pArea && pArea->m_PDAName.Get()[0] != 0);
	//Msg("bpda = %d pArea = %d pdaname[0] = %d\n", bPDA, pArea, pArea ? pArea->m_PDAName.Get()[0] : -1);
	return bPDA;
}

int CASW_Hack_Computer::GetMailOption()
{
	CASW_Computer_Area *pArea = GetComputerArea();
	if (!pArea)
	{
		return -1;
	}

	int m = pArea->GetNumMenuOptions();
	for ( int i = 0; i < m; i++ )
	{
		if ( GetOptionTypeForEntry( i + 1 ) == ASW_COMPUTER_OPTION_TYPE_MAIL )
		{
			return i + 1;
		}
	}
	return -1;
}

void CASW_Hack_Computer::SetDefaultHackOption()
{
	if ( IsPDA() )
	{
		int iMailOption = GetMailOption();
		if ( iMailOption != -1 )
			m_iShowOption = iMailOption;
		else
			m_iShowOption = 0;
	}
	else
	{
		CASW_Computer_Area *pArea = GetComputerArea();
		if ( pArea && !pArea->IsLocked() && GetOptionTypeForEntry( 1 ) == ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_1 && GetOptionTypeForEntry( 2 ) == ASW_COMPUTER_OPTION_TYPE_NONE )
		{
			pArea->m_bLoggedIn = true;
			m_iShowOption = 1; // boot directly into camera
			pArea->m_iActiveCam = 1;
		}
		else
		{
			m_iShowOption = 0;	// put us on the main menu
		}
	}
}

void CASW_Hack_Computer::MarineStoppedUsing(CASW_Marine* pMarine)
{
	m_iShowOption = 0;	// put us back on the main menu
	if (pMarine->m_hRemoteTurret)
	{
		pMarine->m_hRemoteTurret->StopUsingTurret();
		pMarine->m_hRemoteTurret = NULL;
	}

	BaseClass::MarineStoppedUsing(pMarine);
}

int CASW_Hack_Computer::GetOptionTypeForEntry(int iOption)
{
	CASW_Computer_Area *pArea = GetComputerArea();
	if (!pArea)
		return ASW_COMPUTER_OPTION_TYPE_NONE;

	int icon = 1;	
	if (pArea->m_DownloadObjectiveName.Get()[0] != NULL && pArea->GetDownloadProgress() < 1.0f)
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_DOWNLOAD_DOCS;
		}
		icon++;
	}
	if (pArea->m_hSecurityCam1.Get())
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_1;
		}
		icon++;
	}
	if (pArea->m_hSecurityCam2.Get())
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_2;
		}
		icon++;
	}
	if (pArea->m_hSecurityCam3.Get())
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_SECURITY_CAM_3;
		}
		icon++;
	}
	if (pArea->m_hTurret1.Get())
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_TURRET_1;
		}
		icon++;
	}
	if (pArea->m_hTurret2.Get())
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_TURRET_2;
		}
		icon++;
	}
	if (pArea->m_hTurret3.Get())
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_TURRET_3;
		}
		icon++;
	}
	if (pArea->m_MailFile.Get()[0] != NULL)
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_MAIL;
		}
		icon++;
	}
	if (pArea->m_NewsFile.Get()[0] != NULL)
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_NEWS;
		}
		icon++;
	}
	if (pArea->m_StocksSeed.Get()[0] != NULL)
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_STOCKS;
		}
		icon++;
	}
	if (pArea->m_WeatherSeed.Get()[0] != NULL)
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_WEATHER;
		}
		icon++;
	}
	if (pArea->m_PlantFile.Get()[0] != NULL)
	{
		if (iOption == icon)
		{
			return ASW_COMPUTER_OPTION_TYPE_PLANT;
		}
		icon++;
	}	

	return ASW_COMPUTER_OPTION_TYPE_NONE;
}

bool CASW_Hack_Computer::IsDownloadingFiles()
{
	if (!GetHackingMarine())
		return false;

	int iOptionType = GetOptionTypeForEntry(m_iShowOption);
	return (iOptionType == ASW_COMPUTER_OPTION_TYPE_DOWNLOAD_DOCS);
}
