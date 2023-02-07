#include "cbase.h"
#include "asw_debrief_stats.h"
#include "asw_shareddefs.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "asw_gamerules.h"
#include "asw_debrief_info.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_debrief_stats, CASW_Debrief_Stats );
PRECACHE_REGISTER( asw_debrief_stats );

IMPLEMENT_SERVERCLASS_ST(CASW_Debrief_Stats, DT_ASW_Debrief_Stats)	
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iKills) ), m_iKills ),
	SendPropArray( SendPropFloat( SENDINFO_ARRAY(m_fAccuracy) ), m_fAccuracy ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iFF) ), m_iFF ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamage) ), m_iDamage ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFired) ), m_iShotsFired ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsHit) ), m_iShotsHit ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWounded) ), m_iWounded ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iAliensBurned) ), m_iAliensBurned ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealthHealed) ), m_iHealthHealed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iFastHacks) ), m_iFastHacks ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iSkillPointsAwarded) ), m_iSkillPointsAwarded ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iStartingEquip0) ), m_iStartingEquip0 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iStartingEquip1) ), m_iStartingEquip1 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iStartingEquip2) ), m_iStartingEquip2 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iAmmoDeployed) ), m_iAmmoDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iSentryGunsDeployed) ), m_iSentryGunsDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iSentryFlamerDeployed) ), m_iSentryFlamerDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iSentryFreezeDeployed) ), m_iSentryFreezeDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iSentryCannonDeployed) ), m_iSentryCannonDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iMedkitsUsed) ), m_iMedkitsUsed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iFlaresUsed) ), m_iFlaresUsed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iAdrenalineUsed) ), m_iAdrenalineUsed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iTeslaTrapsDeployed) ), m_iTeslaTrapsDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iFreezeGrenadesThrown) ), m_iFreezeGrenadesThrown ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iElectricArmorUsed) ), m_iElectricArmorUsed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealGunHeals) ), m_iHealGunHeals ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealBeaconHeals) ), m_iHealBeaconHeals ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealGunHeals_Self) ), m_iHealGunHeals_Self ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealBeaconHeals_Self) ), m_iHealBeaconHeals_Self ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAmpsUsed) ), m_iDamageAmpsUsed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealBeaconsDeployed) ), m_iHealBeaconsDeployed ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iMedkitHeals_Self) ), m_iMedkitHeals_Self ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iGrenadeExtinguishMarine) ), m_iGrenadeExtinguishMarine ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iGrenadeFreezeAlien) ), m_iGrenadeFreezeAlien ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAmpAmps) ), m_iDamageAmpAmps ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iNormalArmorReduction) ), m_iNormalArmorReduction ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iElectricArmorReduction) ), m_iElectricArmorReduction ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealAmpGunHeals) ), m_iHealAmpGunHeals ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iHealAmpGunAmps) ), m_iHealAmpGunAmps ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iMedRifleHeals) ), m_iMedRifleHeals ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iBiomassIgnited) ), m_iBiomassIgnited ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iLeadershipProcsAccuracy) ), m_iLeadershipProcsAccuracy ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iLeadershipProcsResist) ), m_iLeadershipProcsResist ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iLeadershipDamageAccuracy) ), m_iLeadershipDamageAccuracy ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iLeadershipDamageResist) ), m_iLeadershipDamageResist ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills0) ), m_iWeaponClassAndKills0 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF0) ), m_iDamageAndFF0 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit0) ), m_iShotsFiredAndHit0 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills1) ), m_iWeaponClassAndKills1 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF1) ), m_iDamageAndFF1 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit1) ), m_iShotsFiredAndHit1 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills2) ), m_iWeaponClassAndKills2 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF2) ), m_iDamageAndFF2 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit2) ), m_iShotsFiredAndHit2 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills3) ), m_iWeaponClassAndKills3 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF3) ), m_iDamageAndFF3 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit3) ), m_iShotsFiredAndHit3 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills4) ), m_iWeaponClassAndKills4 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF4) ), m_iDamageAndFF4 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit4) ), m_iShotsFiredAndHit4 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills5) ), m_iWeaponClassAndKills5 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF5) ), m_iDamageAndFF5 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit5) ), m_iShotsFiredAndHit5 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills6) ), m_iWeaponClassAndKills6 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF6) ), m_iDamageAndFF6 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit6) ), m_iShotsFiredAndHit6 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iWeaponClassAndKills7) ), m_iWeaponClassAndKills7 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iDamageAndFF7) ), m_iDamageAndFF7 ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iShotsFiredAndHit7) ), m_iShotsFiredAndHit7 ),

	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iLeaderboardScore) ), m_iLeaderboardScore ),

	SendPropFloat(SENDINFO(m_fTimeTaken)),
	SendPropInt(SENDINFO(m_iTotalKills)),
	SendPropInt(SENDINFO(m_iEggKills)),
	SendPropInt(SENDINFO(m_iParasiteKills)),
	SendPropInt(SENDINFO(m_iDroneKills)),
	SendPropInt(SENDINFO(m_iShieldbugKills)),

	SendPropString( SENDINFO( m_DebriefText1 ) ),
	SendPropString( SENDINFO( m_DebriefText2 ) ),
	SendPropString( SENDINFO( m_DebriefText3 ) ),

	SendPropBool( SENDINFO( m_bBeatSpeedrunTime ) ),

	SendPropFloat(SENDINFO(m_fBestTimeTaken)),
	SendPropInt(SENDINFO(m_iBestKills)),
	SendPropInt(SENDINFO(m_iSpeedrunTime)),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Debrief_Stats )
END_DATADESC()

CASW_Debrief_Stats::CASW_Debrief_Stats()
{
	//Msg("[S] CASW_Debrief_Stats created\n");
	m_bBeatSpeedrunTime = false;
}

CASW_Debrief_Stats::~CASW_Debrief_Stats()
{
	//Msg("[S] CASW_Debrief_Stats destroyed\n");
}

void CASW_Debrief_Stats::Spawn( void )
{
	BaseClass::Spawn();

	CASW_Debrief_Info* pDebriefInfo = dynamic_cast<CASW_Debrief_Info*>(gEntList.FindEntityByClassname( NULL, "asw_debrief_info" ));
	if (pDebriefInfo)
	{
		Q_strncpy( m_DebriefText1.GetForModify(), STRING( pDebriefInfo->m_DebriefText1 ), 255 );
		Q_strncpy( m_DebriefText2.GetForModify(), STRING( pDebriefInfo->m_DebriefText2 ), 255 );
		Q_strncpy( m_DebriefText3.GetForModify(), STRING( pDebriefInfo->m_DebriefText3 ), 255 );
	}
	else
	{
		Q_snprintf(m_DebriefText1.GetForModify(), 255, "");
		Q_snprintf(m_DebriefText2.GetForModify(), 255, "");
		Q_snprintf(m_DebriefText3.GetForModify(), 255, "");
	}	
}

int CASW_Debrief_Stats::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

int CASW_Debrief_Stats::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_FULLCHECK );
}

int CASW_Debrief_Stats::GetSkillPointsAwarded(int iProfileIndex)
{
	if (!ASWGameRules())
		return ASW_SKILL_POINTS_PER_MISSION;
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return ASW_SKILL_POINTS_PER_MISSION;

	for (int i=0;i<ASW_MAX_MARINE_RESOURCES;i++)
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
		if (pMR && pMR->GetProfileIndex() == iProfileIndex)
			return m_iSkillPointsAwarded[i];
	}
	return ASW_SKILL_POINTS_PER_MISSION;
}

void CASW_Debrief_Stats::SetWeaponStats( int nMarineIndex, int nWeaponIndex, int nWeaponClass, int nDamage, int nFFDamage, int nShotsFired, int nShotsHit, int nKills )
{
	// Pack each attribute into 32 bit ints
	unsigned int nClassAndKills = ( (unsigned short)nKills & 0x0000FFFF ) | ( (unsigned short)nWeaponClass << 16 );
	unsigned int nDamageAndFF = ( (unsigned short)nFFDamage & 0x0000FFFF ) | ( (unsigned short)nDamage << 16 );
	unsigned int nShotsFiredAndHit = ( (unsigned short)nShotsHit & 0x0000FFFF ) | ( (unsigned short)nShotsFired << 16 );
	
	switch( nWeaponIndex )
	{
	case 0:
		m_iWeaponClassAndKills0.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF0.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit0.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 1:
		m_iWeaponClassAndKills1.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF1.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit1.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 2:
		m_iWeaponClassAndKills2.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF2.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit2.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 3:
		m_iWeaponClassAndKills3.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF3.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit3.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 4:
		m_iWeaponClassAndKills4.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF4.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit4.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 5:
		m_iWeaponClassAndKills5.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF5.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit5.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 6:
		m_iWeaponClassAndKills6.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF6.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit6.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	case 7:
		m_iWeaponClassAndKills7.Set( nMarineIndex, nClassAndKills );
		m_iDamageAndFF7.Set( nMarineIndex, nDamageAndFF );
		m_iShotsFiredAndHit7.Set( nMarineIndex, nShotsFiredAndHit );
		break;

	default:
		return;
	}
}

CASW_Debrief_Stats* GetDebriefStats()
{
	return ASWGameRules() ? ASWGameRules()->m_hDebriefStats.Get() : NULL;
}