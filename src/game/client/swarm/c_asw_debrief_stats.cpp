#include "cbase.h"
#include "c_asw_debrief_stats.h"
#include "MissionStatsPanel.h"
#include "iclientmode.h"
#include "c_asw_player.h"
#include <vgui_controls/Frame.h>
#include "asw_medal_store.h"
#include "asw_gamerules.h"
#include "c_asw_game_resource.h"
#include "c_asw_marine_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Debrief_Stats, DT_ASW_Debrief_Stats, CASW_Debrief_Stats)
	RecvPropArray( RecvPropInt( RECVINFO(m_iKills[0]) ), m_iKills ),
	RecvPropArray( RecvPropFloat( RECVINFO(m_fAccuracy[0]) ), m_fAccuracy ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iFF[0]) ), m_iFF ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamage[0]) ), m_iDamage ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFired[0]) ), m_iShotsFired ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsHit[0]) ), m_iShotsHit ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iWounded[0]) ), m_iWounded ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iAliensBurned[0]) ), m_iAliensBurned ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealthHealed[0]) ), m_iHealthHealed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iFastHacksWire[0]) ), m_iFastHacksWire ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iFastHacksComputer[0]) ), m_iFastHacksComputer ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSkillPointsAwarded[0]) ), m_iSkillPointsAwarded ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iStartingEquip0[0]) ), m_iStartingEquip0 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iStartingEquip1[0]) ), m_iStartingEquip1 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iStartingEquip2[0]) ), m_iStartingEquip2 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iAmmoDeployed[0]) ), m_iAmmoDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSentryGunsDeployed[0]) ), m_iSentryGunsDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSentryFlamerDeployed[0]) ), m_iSentryFlamerDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSentryFreezeDeployed[0]) ), m_iSentryFreezeDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSentryCannonDeployed[0]) ), m_iSentryCannonDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSentryRailgunDeployed[0]) ), m_iSentryRailgunDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iMedkitsUsed[0]) ), m_iMedkitsUsed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iFlaresUsed[0]) ), m_iFlaresUsed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iAdrenalineUsed[0]) ), m_iAdrenalineUsed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iTeslaTrapsDeployed[0]) ), m_iTeslaTrapsDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iFreezeGrenadesThrown[0]) ), m_iFreezeGrenadesThrown ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iElectricArmorUsed[0]) ), m_iElectricArmorUsed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealGunHeals[0]) ), m_iHealGunHeals ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealBeaconHeals[0]) ), m_iHealBeaconHeals ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealGunHeals_Self[0]) ), m_iHealGunHeals_Self ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealBeaconHeals_Self[0]) ), m_iHealBeaconHeals_Self ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAmpsUsed[0]) ), m_iDamageAmpsUsed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealBeaconsDeployed[0]) ), m_iHealBeaconsDeployed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iMedkitHeals_Self[0]) ), m_iMedkitHeals_Self ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iGrenadeExtinguishMarine[0]) ), m_iGrenadeExtinguishMarine ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iGrenadeFreezeAlien[0]) ), m_iGrenadeFreezeAlien ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAmpAmps[0]) ), m_iDamageAmpAmps ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iNormalArmorReduction[0]) ), m_iNormalArmorReduction ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iElectricArmorReduction[0]) ), m_iElectricArmorReduction ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealAmpGunAmps[0]) ), m_iHealAmpGunHeals ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHealAmpGunHeals[0]) ), m_iHealAmpGunAmps ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iMedRifleHeals[0]) ), m_iMedRifleHeals ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iCryoCannonFreezeAlien[0]) ), m_iCryoCannonFreezeAlien ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iPlasmaThrowerExtinguishMarine[0]) ), m_iPlasmaThrowerExtinguishMarine ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHackToolWireHacksTech[0]) ), m_iHackToolWireHacksTech ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHackToolWireHacksOther[0]) ), m_iHackToolWireHacksOther ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHackToolComputerHacksTech[0]) ), m_iHackToolComputerHacksTech ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iHackToolComputerHacksOther[0]) ), m_iHackToolComputerHacksOther ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iEnergyShieldProjectilesDestroyed[0]) ), m_iEnergyShieldProjectilesDestroyed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iReanimatorRevivesOfficer[0]) ), m_iReanimatorRevivesOfficer ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iReanimatorRevivesSpecialWeapons[0]) ), m_iReanimatorRevivesSpecialWeapons ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iReanimatorRevivesMedic[0]) ), m_iReanimatorRevivesMedic ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iReanimatorRevivesTech[0]) ), m_iReanimatorRevivesTech ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iSpeedBoostsUsed[0]) ), m_iSpeedBoostsUsed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShieldBubblesThrown[0]) ), m_iShieldBubblesThrown ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShieldBubblePushedEnemy[0]) ), m_iShieldBubblePushedEnemy ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShieldBubbleDamageAbsorbed[0]) ), m_iShieldBubbleDamageAbsorbed ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iBiomassIgnited[0]) ), m_iBiomassIgnited ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iLeadershipProcsAccuracy[0]) ), m_iLeadershipProcsAccuracy ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iLeadershipProcsResist[0]) ), m_iLeadershipProcsResist ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iLeadershipDamageAccuracy[0]) ), m_iLeadershipDamageAccuracy ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iLeadershipDamageResist[0]) ), m_iLeadershipDamageResist ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills0[0]) ), m_iWeaponClassAndKills0 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF0[0]) ), m_iDamageAndFF0 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit0[0]) ), m_iShotsFiredAndHit0 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills1[0]) ), m_iWeaponClassAndKills1 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF1[0]) ), m_iDamageAndFF1 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit1[0]) ), m_iShotsFiredAndHit1 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills2[0]) ), m_iWeaponClassAndKills2 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF2[0]) ), m_iDamageAndFF2 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit2[0]) ), m_iShotsFiredAndHit2 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills3[0]) ), m_iWeaponClassAndKills3 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF3[0]) ), m_iDamageAndFF3 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit3[0]) ), m_iShotsFiredAndHit3 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills4[0]) ), m_iWeaponClassAndKills4 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF4[0]) ), m_iDamageAndFF4 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit4[0]) ), m_iShotsFiredAndHit4 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills5[0]) ), m_iWeaponClassAndKills5 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF5[0]) ), m_iDamageAndFF5 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit5[0]) ), m_iShotsFiredAndHit5 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills6[0]) ), m_iWeaponClassAndKills6 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF6[0]) ), m_iDamageAndFF6 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit6[0]) ), m_iShotsFiredAndHit6 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iWeaponClassAndKills7[0]) ), m_iWeaponClassAndKills7 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iDamageAndFF7[0]) ), m_iDamageAndFF7 ),
	RecvPropArray( RecvPropInt( RECVINFO(m_iShotsFiredAndHit7[0]) ), m_iShotsFiredAndHit7 ),

	RecvPropArray( RecvPropInt( RECVINFO(m_iLeaderboardScore[0]) ), m_iLeaderboardScore ),

	RecvPropFloat(RECVINFO(m_fTimeTaken)),
	RecvPropInt(RECVINFO(m_iTotalKills)),
	RecvPropInt(RECVINFO(m_iEggKills)),
	RecvPropInt(RECVINFO(m_iParasiteKills)),
	RecvPropInt(RECVINFO(m_iDroneKills)),
	RecvPropInt(RECVINFO(m_iShieldbugKills)),

	RecvPropString( RECVINFO( m_DebriefText1 ) ),
	RecvPropString( RECVINFO( m_DebriefText2 ) ),
	RecvPropString( RECVINFO( m_DebriefText3 ) ),

	RecvPropBool( RECVINFO( m_bBeatSpeedrunTime ) ),

	RecvPropFloat(RECVINFO(m_fBestTimeTaken)),
	RecvPropInt(RECVINFO(m_iBestKills)),
	RecvPropInt(RECVINFO(m_iSpeedrunTime)),
END_RECV_TABLE()

C_ASW_Debrief_Stats *g_pDebriefStats = NULL;
C_ASW_Debrief_Stats *GetDebriefStats() { return g_pDebriefStats; }

C_ASW_Debrief_Stats::C_ASW_Debrief_Stats()
{
	m_bCreated = false;
	g_pDebriefStats = this;
}

C_ASW_Debrief_Stats::~C_ASW_Debrief_Stats()
{
	if ( g_pDebriefStats == this )
	{
		g_pDebriefStats = NULL;
	}
}

void C_ASW_Debrief_Stats::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
	if ( type == DATA_UPDATE_CREATED )
	{
		m_bCreated = true;

		// notify the debrief stats page that all data is here and it should start counting numbers/bars up		
		HACK_GETLOCALPLAYER_GUARD( "MissionCompleteFrame needs to be a child of the main client .dll viewport (now a parent to both client mode viewports)" );
		MissionStatsPanel *pStatsPanel = dynamic_cast< MissionStatsPanel * >( GetClientMode()->GetViewport()->FindChildByName( "MissionStatsPanel", true ) );
		if ( pStatsPanel )
		{
			pStatsPanel->InitFrom( this );
		}

		// update our kill counts
#ifdef USE_MEDAL_STORE
		if ( GetMedalStore() && ASWGameRules() && ASWGameResource() && !ASWGameRules()->m_bCheated
			&& !engine->IsPlayingDemo() )
		{
			C_ASW_Game_Resource *pGameResource = ASWGameResource();
			C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
			int iMissions = ASWGameRules()->GetMissionSuccess() ? 1 : 0;		// award 1 extra mission if it was a success
			int iKills = 0;
			// go through each marine belonging to the local player and increment kills
			for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
			{
				C_ASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( pMR && pMR->GetCommanderIndex() == pPlayer->entindex() )
					iKills += m_iKills[i];
			}
			if ( iKills > 0 )	// only increment their counts if they actually killed something
				GetMedalStore()->OnIncreaseCounts( iMissions, 0, iKills, ( gpGlobals->maxClients <= 1 ) );
		}
#endif
	}
}

int C_ASW_Debrief_Stats::GetHighestKills()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetKills( i );
		if ( k > best )
		{
			best = k;
		}
	}
	return best;
}

float C_ASW_Debrief_Stats::GetHighestAccuracy()
{
	float best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		float k = GetAccuracy( i );
		if ( k > best )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestFriendlyFire()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetFriendlyFire( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestDamageTaken()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetDamageTaken( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestShotsFired()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetShotsFired( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestAliensBurned()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetAliensBurned( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestHealthHealed()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetHealthHealed( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestFastHacks()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetFastHacksWire( i ) + GetFastHacksComputer( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetHighestSkillPointsAwarded()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetSkillPointsAwarded( i );
		if ( k != 0 && ( k > best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetLowestFriendlyFire()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetFriendlyFire( i );
		if ( k != 0 && ( k < best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

int C_ASW_Debrief_Stats::GetLowestDamageTaken()
{
	int best = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		int k = GetDamageTaken( i );
		if ( k != 0 && ( k < best || best == 0 ) )
		{
			best = k;
		}
	}
	return best;
}

bool C_ASW_Debrief_Stats::GetWeaponStats( int iMarineIndex, int iEquipIndex, int &iDamage, int &iFFDamage, int &iShotsFired, int &iShotsHit, int &iKills )
{
	if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills0.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF0.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF0.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit0.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit0.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills0.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills1.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF1.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF1.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit1.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit1.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills1.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills2.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF2.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF2.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit2.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit2.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills2.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills3.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF3.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF3.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit3.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit3.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills3.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills4.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF4.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF4.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit4.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit4.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills4.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills5.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF5.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF5.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit5.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit5.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills5.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills6.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF6.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF6.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit6.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit6.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills6.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else if ( ( unsigned )iEquipIndex == ( ( m_iWeaponClassAndKills7.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF ) )
	{
		iDamage = ( m_iDamageAndFF7.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iFFDamage = m_iDamageAndFF7.Get( iMarineIndex ) & 0x0000FFFF;
		iShotsFired = ( m_iShotsFiredAndHit7.Get( iMarineIndex ) >> 16 ) & 0x0000FFFF;
		iShotsHit = m_iShotsFiredAndHit7.Get( iMarineIndex ) & 0x0000FFFF;
		iKills = m_iWeaponClassAndKills7.Get( iMarineIndex ) & 0x0000FFFF;
		return true;
	}
	else
		return false;
}
