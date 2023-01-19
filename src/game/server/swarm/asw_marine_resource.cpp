#include "cbase.h"
#include "asw_marine_resource.h"
#include "asw_marine_profile.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_equipment_list.h"
#include "asw_weapon.h"
#include "asw_weapon_parse.h"
#include "asw_door.h"
#include "asw_director.h"
#include "asw_gamestats.h"
#include "asw_gamerules.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_marine_resource, CASW_Marine_Resource );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Marine_Resource )
	DEFINE_FIELD( m_MarineProfileIndex, FIELD_INTEGER ),
	DEFINE_FIELD( m_MarineEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_Commander, FIELD_EHANDLE),	
	DEFINE_AUTO_ARRAY( m_iWeaponsInSlots, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY( m_iInitialWeaponsInSlots, FIELD_INTEGER ),
	DEFINE_FIELD( m_vecOutOfAmmoSpot, FIELD_VECTOR ),
	DEFINE_FIELD( m_iHealCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_bInfested, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bInhabited, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iServerFiring, FIELD_INTEGER ),	
	DEFINE_FIELD( m_iCommanderIndex, FIELD_INTEGER ),
	DEFINE_FIELD( m_iShotsFired, FIELD_INTEGER ),
	DEFINE_FIELD( m_iPlayerShotsFired, FIELD_INTEGER ),
	DEFINE_FIELD( m_iPlayerShotsMissed, FIELD_INTEGER ),
	DEFINE_FIELD( m_fDamageTaken, FIELD_FLOAT ),
	DEFINE_FIELD( m_iAliensKilled, FIELD_INTEGER ),
	DEFINE_FIELD( m_vecDeathPosition, FIELD_VECTOR ),
	DEFINE_FIELD( m_fDeathTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_fFriendlyFireDamageDealt, FIELD_FLOAT ),
	DEFINE_FIELD( m_iShieldbugsKilled, FIELD_INTEGER ),
	DEFINE_FIELD( m_iParasitesKilled, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSingleGrenadeKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iKickedGrenadeKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSavedLife, FIELD_INTEGER ),
	DEFINE_FIELD( m_bHurt, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iMadFiring, FIELD_INTEGER ),
	DEFINE_FIELD( m_iMadFiringAutogun, FIELD_INTEGER ),
	DEFINE_FIELD( m_bOnlyWeaponExtra, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iOnlyWeaponEquipIndex, FIELD_INTEGER ),	
	//DEFINE_FIELD( m_iLastStandKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iMedicHealing, FIELD_INTEGER ),
	DEFINE_FIELD( m_iCuredInfestation, FIELD_INTEGER ),
	DEFINE_FIELD( m_iAliensBurned, FIELD_INTEGER ),
	DEFINE_FIELD( m_iMineKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSentryKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iEggKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iGrubKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iBarrelKills, FIELD_INTEGER ),
	DEFINE_FIELD( m_iFiresPutOut, FIELD_INTEGER ),
	DEFINE_FIELD( m_bDidFastReloadsInARow, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDamageAmpMedal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bKilledBoomerEarly, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bStunGrenadeMedal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFreezeGrenadeMedal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDodgedRanger, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bProtectedTech, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bMeleeParasiteKill, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iAliensKicked, FIELD_INTEGER ),
	DEFINE_FIELD( m_iFastDoorHacks, FIELD_INTEGER ),
	DEFINE_FIELD( m_iFastComputerHacks, FIELD_INTEGER ),
	DEFINE_FIELD( m_iAliensKilledByBouncingBullets, FIELD_INTEGER ),
	DEFINE_FIELD( m_iScore, FIELD_INTEGER ),
	DEFINE_FIELD( m_flFinishedMissionTime, FIELD_FLOAT ),
END_DATADESC()

void *SendProxy_SendMarineResourceTimelinesDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	if ( ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME )
	{
		// Don't send timeline data while in the mission
		pRecipients->ClearAllRecipients();
	}

	return ( void * )pVarData;
}

// Only send active weapon index to local player
BEGIN_SEND_TABLE_NOBASE( CASW_Marine_Resource, DT_MR_Timelines )
	SendPropDataTable( SENDINFO_DT( m_TimelineFriendlyFire ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
	SendPropDataTable( SENDINFO_DT( m_TimelineKillsTotal ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
	SendPropDataTable( SENDINFO_DT( m_TimelineHealth ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
	SendPropDataTable( SENDINFO_DT( m_TimelineAmmo ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
	SendPropDataTable( SENDINFO_DT( m_TimelinePosX ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
	SendPropDataTable( SENDINFO_DT( m_TimelinePosY ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
	SendPropDataTable( SENDINFO_DT( m_TimelineScore ), &REFERENCE_SEND_TABLE(DT_Timeline) ),
END_SEND_TABLE();

IMPLEMENT_SERVERCLASS_ST(CASW_Marine_Resource, DT_ASW_Marine_Resource)
	// Timeline data only gets sent at mission end
	SendPropDataTable( "mr_timelines", 0, &REFERENCE_SEND_TABLE(DT_MR_Timelines), SendProxy_SendMarineResourceTimelinesDataTable ),
	SendPropInt		(SENDINFO(m_MarineProfileIndex), 10 ),
	SendPropEHandle (SENDINFO(m_MarineEntity) ),
	SendPropEHandle (SENDINFO(m_Commander) ),
	SendPropInt		(SENDINFO(m_iCommanderIndex), 10),
	SendPropArray( SendPropInt( SENDINFO_ARRAY( m_iWeaponsInSlots ), 30 ), m_iWeaponsInSlots ),
	SendPropBool	(SENDINFO(m_bInfested) ),
	SendPropBool	(SENDINFO(m_bInhabited) ),
	SendPropInt		(SENDINFO(m_iServerFiring), 8 ),
	//SendPropFloat		(SENDINFO(m_fDamageTaken) ),
	SendPropInt		(SENDINFO(m_iAliensKilled) ),
	SendPropBool	(SENDINFO(m_bTakenWoundDamage) ),
	SendPropBool	(SENDINFO(m_bHealthHalved) ),	
	SendPropString	(SENDINFO(m_MedalsAwarded)),
	SendPropEHandle	(SENDINFO(m_hWeldingDoor)),
	SendPropBool	(SENDINFO(m_bUsingEngineeringAura)),
	SendPropInt		(SENDINFO(m_iBotFrags)),
	SendPropInt		(SENDINFO(m_iScore)),
	SendPropFloat	(SENDINFO(m_flFinishedMissionTime)),
END_SEND_TABLE()

extern ConVar asw_leadership_radius;
extern ConVar asw_debug_marine_damage;
extern ConVar asw_debug_medals;
ConVar rd_damage_buff_scale( "rd_damage_buff_scale", "2.0", FCVAR_CHEAT, "Damage amplifier's damage factor. 2.0 by default" );

CASW_Marine_Resource::CASW_Marine_Resource()
{
	m_bAwardedMedals = false;
	m_MarineProfileIndex = -1;
	m_bInfested = false;
	SetCommander(NULL);
	m_bInhabited = false;
	m_iServerFiring = 0;
	m_iShotsFired = 0;
	m_iPlayerShotsFired = 0;
	m_iPlayerShotsMissed = 0;
	m_fDamageTaken = 0;
	m_iAliensKilled = 0;
	m_iAliensKilledSinceLastFriendlyFireIncident = 0;
	m_bAwardedFFPartialAchievement = false;
	m_bAwardedHealBeaconAchievement = false;
	m_bDealtNonMeleeDamage = false;
	m_iDamageTakenDuringHack = 0;
	m_iBotFrags = 0;
	m_iAliensBurned = 0;
	m_iAmmoDeployed = 0;
	m_iSentryGunsDeployed = 0;
	m_iSentryFlamerDeployed = 0;
	m_iSentryFreezeDeployed = 0;
	m_iSentryCannonDeployed = 0;
	m_iMedkitsUsed = 0;
	m_iFlaresUsed = 0;
	m_iAdrenalineUsed = 0;
	m_iTeslaTrapsDeployed = 0;
	m_iFreezeGrenadesThrown = 0;
	m_iElectricArmorUsed = 0;
	m_iHealGunHeals = 0;
	m_iHealBeaconHeals = 0;
	m_iHealGunHeals_Self = 0;
	m_iHealBeaconHeals_Self = 0;
	m_iDamageAmpsUsed = 0;
	m_iHealBeaconsDeployed = 0;
	m_iMedkitHeals_Self = 0;
	m_iGrenadeExtinguishMarine = 0;
	m_iGrenadeFreezeAlien = 0;
	m_iDamageAmpAmps = 0;
	m_iNormalArmorReduction = 0;
	m_iElectricArmorReduction = 0;
	m_iHealAmpGunHeals = 0;
	m_iHealAmpGunAmps = 0;
	m_iMedRifleHeals = 0;
	m_iBiomassIgnited = 0;
	m_iScore = -1;
	m_flFinishedMissionTime = -1;

	m_TimelineFriendlyFire.SetCompressionType( TIMELINE_COMPRESSION_SUM );
	m_TimelineKillsTotal.SetCompressionType( TIMELINE_COMPRESSION_SUM );
	m_TimelineHealth.SetCompressionType( TIMELINE_COMPRESSION_AVERAGE );
	m_TimelineAmmo.SetCompressionType( TIMELINE_COMPRESSION_AVERAGE );
	m_TimelinePosX.SetCompressionType( TIMELINE_COMPRESSION_AVERAGE );
	m_TimelinePosY.SetCompressionType( TIMELINE_COMPRESSION_AVERAGE );
	m_TimelineScore.SetCompressionType( TIMELINE_COMPRESSION_SUM );

	// BenLubar(deathmatch-improvements): if we're already in game (such as deathmatch mode), make sure we still record stats for the graph
	if ( ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME )
	{
		float flStart = ASWGameRules()->m_fMissionStartedTime;
		m_TimelineFriendlyFire.ClearValues();
		m_TimelineFriendlyFire.StartFromTime( flStart );
		m_TimelineKillsTotal.ClearValues();
		m_TimelineKillsTotal.StartFromTime( flStart );
		m_TimelineHealth.ClearValues();
		m_TimelineHealth.StartFromTime( flStart );
		m_TimelineAmmo.ClearValues();
		m_TimelineAmmo.StartFromTime( flStart );
		m_TimelinePosX.ClearValues();
		m_TimelinePosX.StartFromTime( flStart );
		m_TimelinePosY.ClearValues();
		m_TimelinePosY.StartFromTime( flStart );
		m_TimelineScore.ClearValues();
		m_TimelineScore.StartFromTime( flStart );
	}

	m_bHealthHalved = false;
	m_bTakenWoundDamage = false;
	m_iOnlyWeaponEquipIndex = -1;  // equip index of the only weapon we've used (-1 means not used a weapon yet, -2 means we've used multiple weapons)	
	memset( m_MedalsAwarded.GetForModify(), 0, sizeof(m_MedalsAwarded) );
	m_bUsingEngineeringAura = false;
	m_pIntensity = new CASW_Intensity();

	memset( m_iInitialWeaponsInSlots, -1, sizeof( m_iInitialWeaponsInSlots ) );
	memset( const_cast<int*>( m_iWeaponsInSlots.Base() ), -1, sizeof( m_iInitialWeaponsInSlots ) );
}


CASW_Marine_Resource::~CASW_Marine_Resource()
{
	if (GetMarineEntity())
		GetMarineEntity()->SetMarineResource(NULL);
	if ( m_pIntensity )
		delete m_pIntensity;
}

// always send this info to players
int CASW_Marine_Resource::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}


void CASW_Marine_Resource::SetCommander(CASW_Player* pCommander)
{	
	m_Commander = pCommander;
	m_iCommanderIndex = pCommander ? pCommander->entindex() : -1;
}

CASW_Player* CASW_Marine_Resource::GetCommander()
{
	return m_Commander;
}

void CASW_Marine_Resource::GetDisplayName( char *pchDisplayName, int nMaxBytes )
{
	wchar_t wszDisplayName[256];
	GetDisplayName( wszDisplayName, sizeof( wszDisplayName ) );

	V_UnicodeToUTF8( wszDisplayName, pchDisplayName, nMaxBytes );
}

void CASW_Marine_Resource::GetDisplayName( wchar_t *pwchDisplayName, int nMaxBytes )
{
	bool bPlayerName = false;
	const char *pchName = NULL;

	if ( gpGlobals->maxClients <= 1 )
	{
		// Always use the character name in singleplayer
		pchName = GetProfile()->GetShortName();
	}
	else
	{
		CASW_Player *pPlayer = GetCommander();
		if ( IsInhabited() && pPlayer && pPlayer->IsConnected() && pPlayer->GetNPC() == m_MarineEntity )
		{
			// Use the player name
			pchName = pPlayer->GetPlayerName();
			bPlayerName = true;
		}
		else
		{
			// Use the character name
			pchName = GetProfile()->GetShortName();
		}
	}

	const wchar_t *pwchLocalizedMarineName = NULL;

	if ( !bPlayerName )
	{
		// Find the localized character name
		pwchLocalizedMarineName = g_pVGuiLocalize->Find( pchName );
	}

	if ( pwchLocalizedMarineName )
	{
		// Copy the localized name
		V_wcsncpy( pwchDisplayName, pwchLocalizedMarineName, nMaxBytes );
	}
	else
	{
		// Copy the name
		g_pVGuiLocalize->ConvertANSIToUnicode( pchName, pwchDisplayName, nMaxBytes );
	}
}

void CASW_Marine_Resource::SetMarineEntity(CASW_Marine* marine)
{
	m_MarineEntity = marine;
}

void CASW_Marine_Resource::SetProfileIndex(int ProfileIndex)
{
	m_MarineProfileIndex = ProfileIndex;
}

int CASW_Marine_Resource::GetProfileIndex()
{
	return m_MarineProfileIndex;
}

CASW_Marine_Profile* CASW_Marine_Resource::GetProfile()
{
	Assert(MarineProfileList());

	if (m_MarineProfileIndex <0 || m_MarineProfileIndex >= MarineProfileList()->m_NumProfiles)
		return NULL;
	
	return MarineProfileList()->m_Profiles[m_MarineProfileIndex];
}


CASW_Marine* CASW_Marine_Resource::GetMarineEntity()
{
	return m_MarineEntity;
}

// updates our primary/secondary/extra indices into the equipment list with what we're currently carrying
void CASW_Marine_Resource::UpdateWeaponIndices()
{
	if ( !m_MarineEntity || !ASWEquipmentList() )
		return;

	for ( int iWpnSlot = 0; iWpnSlot < ASW_MAX_EQUIP_SLOTS; ++ iWpnSlot )
	{
		int idx = -1;
		if ( CBaseCombatWeapon* pWpn = m_MarineEntity->GetWeapon( iWpnSlot ) )
		{
			const char *szClassName = pWpn->GetClassname();
			idx = ASWEquipmentList()->GetIndexForSlot( iWpnSlot, szClassName );

			// updating current weapon into m_iWeaponsInSlots
			if ( idx != m_iWeaponsInSlots.Get( iWpnSlot ) )
			{
				m_iWeaponsInSlots.Set( iWpnSlot, idx );
			}
		}
	}

	m_TimelineAmmo.RecordValue( m_MarineEntity->GetAllAmmoCount() );
}

float CASW_Marine_Resource::GetHealthPercent()
{
	CASW_Marine_Profile *profile = GetProfile();
	CASW_Marine *marine = GetMarineEntity();
	if (!profile || !marine)
		return 0;

	float max_health = marine->GetMaxHealth();
	float health = marine->GetHealth();

	return health / max_health;
}

bool CASW_Marine_Resource::IsInhabited()
{
	return m_bInhabited;
}

void CASW_Marine_Resource::SetInhabited(bool bInhabited)
{
	m_bInhabited = bInhabited;
}

bool CASW_Marine_Resource::IsFiring()
{
	// if we can directly see the marine and his weapon, then check accurately if he's firing
	//if (GetMarineEntity() && GetMarineEntity()->GetActiveASWWeapon())
	//{
		//return GetMarineEntity()->GetActiveASWWeapon()->IsFiring();
	//}
	// if not, then see what the server says
	return (m_iServerFiring > 0);
}

// a game event has happened that is potentially affected by leadership or other damage scaling
float CASW_Marine_Resource::OnFired_GetDamageScale()
{
	float flDamageScale = 1.0f;

	// damage amp causes double damage always
	CASW_Marine *pMarine = GetMarineEntity();
	if ( pMarine && pMarine->GetDamageBuffEndTime() > gpGlobals->curtime )
	{
		flDamageScale *= rd_damage_buff_scale.GetFloat();// 2.0f;
	}

	//m_iLeadershipCount++;

	// find the shortest leadership interval of our nearby leaders
	if ( pMarine )	// BenLubar(deathmatch-improvements): fixes a crash when a marine dies on the same tick they fired a bullet
	{
		float fChance = MarineSkills()->GetHighestSkillValueNearby( pMarine->GetAbsOrigin(),
			asw_leadership_radius.GetFloat(),
			ASW_MARINE_SKILL_LEADERSHIP, ASW_MARINE_SUBSKILL_LEADERSHIP_ACCURACY_CHANCE );
		float f = random->RandomFloat();
		static int iLeadershipAccCount = 0;
		if ( f < fChance )
		{
			// TODO: leadership particle effect?

			iLeadershipAccCount++;

			flDamageScale *= 2.0f;
		}

		if ( asw_debug_marine_damage.GetBool() )
		{
			Msg("Doing leadership accuracy test.  Chance is %f random float is %f\n", fChance, f);
			Msg("  Leadership accuracy applied %d times so far\n", iLeadershipAccCount);
			Msg( "   OnFired_GetDamageScale returning scale of %f\n", flDamageScale );
		}
	}

	return flDamageScale;
}

// marine has just used the weapon specified - track stats
void CASW_Marine_Resource::UsedWeapon(int iWeaponEquipIndex, bool bWeaponExtra, int iShots)
{
	// call weapon pointer version to update hits/misses
	UsedWeapon(NULL, iShots);

	// update 'only used weapon'

	// not used a weapon yet?
	if (m_iOnlyWeaponEquipIndex == -1)
	{
		m_iOnlyWeaponEquipIndex = iWeaponEquipIndex;
		m_bOnlyWeaponExtra = bWeaponExtra;
		return;
	}

	// used a different type of weapon?
	if (bWeaponExtra != m_bOnlyWeaponExtra)
	{
		m_iOnlyWeaponEquipIndex = -2;
		return;
	}

	// used a different weapon index?
	if (iWeaponEquipIndex != m_iOnlyWeaponEquipIndex)
	{
		m_iOnlyWeaponEquipIndex = -2;
		return;
	}
}

// marine has just used the weapon specified - track stats
void CASW_Marine_Resource::UsedWeapon(CASW_Weapon *pWeapon, int iShots)
{
	// track misses
	if (GetMarineEntity())
	{
		if (asw_debug_medals.GetBool())
			Msg("used weapon: ");
		if (gpGlobals->curtime <= GetMarineEntity()->m_fLastShotAlienTime + 1.0f)	// give a 1 second window after hitting an alien to count shots as hitting
		{
			if (asw_debug_medals.GetBool())
				Msg("hit an alien (curtime=%f lastshotalientime=%f\n", gpGlobals->curtime, GetMarineEntity()->m_fLastShotAlienTime);
			// shot hit an alien
			m_iShotsFired += iShots;
			if (GetMarineEntity()->IsInhabited())
				m_iPlayerShotsFired += iShots;	
		}
		else if (gpGlobals->curtime <= GetMarineEntity()->m_fLastShotJunkTime + 1.0f)
		{
			if (asw_debug_medals.GetBool())
				Msg("hit junk (curtime=%f last shotjunktime=%f lastshotalientime=%f\n", gpGlobals->curtime,GetMarineEntity()->m_fLastShotJunkTime, GetMarineEntity()->m_fLastShotAlienTime);
			// we didn't hit an alien, but we did hit some junk, don't increase player shots
			//  so players can't shoot barrels and thinks to get their accuracy up
			m_iShotsFired += iShots;
		}
		else
		{
			if (asw_debug_medals.GetBool())
				Msg("missed (curtime=%f last shotjunktime=%f lastshotalientime=%f\n", gpGlobals->curtime,GetMarineEntity()->m_fLastShotJunkTime, GetMarineEntity()->m_fLastShotAlienTime);
			// we didn't an alien or any junk, this is a miss
			m_iShotsFired += iShots;
			m_iShotsMissed += iShots;
			if (GetMarineEntity()->IsInhabited())
			{
				m_iPlayerShotsFired += iShots;	
				m_iPlayerShotsMissed += iShots;
			}
		}
	}

	if (!pWeapon || m_iOnlyWeaponEquipIndex == -2)
		return;

	const CASW_WeaponInfo* pWpnInfo = pWeapon->GetWeaponInfo();
	// not used a weapon yet?
	if (m_iOnlyWeaponEquipIndex == -1)
	{
		m_iOnlyWeaponEquipIndex = pWeapon->GetEquipmentListIndex();
		if (pWpnInfo)
			m_bOnlyWeaponExtra = pWpnInfo->m_bExtra;
		return;
	}

	// used a different type of weapon?
	if (pWpnInfo && pWpnInfo->m_bExtra != m_bOnlyWeaponExtra)
	{
		m_iOnlyWeaponEquipIndex = -2;
		return;
	}

	// used a different weapon index?
	if (pWeapon->GetEquipmentListIndex() != m_iOnlyWeaponEquipIndex)
	{
		m_iOnlyWeaponEquipIndex = -2;
		return;
	}
}

// prints debug info to the console about various stats collected for awarding medals
void CASW_Marine_Resource::DebugMedalStats()
{
	if (!GetProfile())
		return;

	Msg("%s's stats for medals:\n", GetProfile()->m_ShortName);
	Msg(" Shots fired: %d\n", m_iShotsFired);	
	float accuracy = 0;
	if (m_iShotsFired > 0)
	{
		accuracy = float(m_iShotsFired - m_iShotsMissed) / float(m_iShotsFired);
	}
	Msg(" Shots missed: %d (accuracy = %f)\n", m_iShotsMissed, accuracy);	
	Msg(" Player shots fired: %d\n", m_iPlayerShotsFired);
	if (m_iPlayerShotsFired > 0)
	{
		accuracy = float(m_iPlayerShotsFired - m_iPlayerShotsMissed) / float(m_iPlayerShotsFired);
	}
	Msg(" Player shots missed: %d (player acc = %f)\n", m_iPlayerShotsMissed, accuracy);	
	Msg(" Damage taken: %f\n", m_fDamageTaken);
	Msg(" Alien kills: %d\n", m_iAliensKilled.Get());
	Msg(" FF damage dealt: %f\n", m_fFriendlyFireDamageDealt);
	Msg(" FF damage taken: %f\n", GetMarineEntity() ? GetMarineEntity()->m_fFriendlyFireDamage : -1);
	Msg(" Single explosion kills: %d\n", m_iSingleGrenadeKills);
	Msg(" Kicked Grenade kills: %d\n", m_iKickedGrenadeKills);
	Msg(" Shieldbugs killed: %d\n", m_iShieldbugsKilled);
	Msg(" Parasites killed: %d\n", m_iParasitesKilled);
	Msg(" Lifesaver kills: %d\n", m_iSavedLife);
	Msg(" Been hurt: %d\n", m_bHurt);
	Msg(" Been wounded: %d\n", m_bTakenWoundDamage.Get());
	Msg(" Mad Firing: %d\n", m_iMadFiring);
	Msg(" Mad Firing Autogun: %d\n", m_iMadFiringAutogun);
	//Msg(" Last Stand Kills: %d\n", m_iLastStandKills);
	Msg(" Health Healed: %d\n", m_iMedicHealing);
	Msg(" Cured Infestation: %d\n", m_iCuredInfestation);
	Msg(" Only weapon used index: %d\n", m_iOnlyWeaponEquipIndex);
	Msg(" Only weapon used extra: %d\n", m_bOnlyWeaponExtra);
	Msg(" Mine burns: %d\n", m_iMineKills);	
	Msg(" Sentry kills: %d\n", m_iSentryKills);
	Msg(" Eggs killed: %d\n", m_iEggKills);
	Msg(" Grubs killed: %d\n", m_iGrubKills);
	Msg(" Aliens killed by barrels: %d\n", m_iBarrelKills);
	Msg(" Fires put out: %d\n", m_iFiresPutOut);
	Msg(" Did several fast reloads: %s\n", m_bDidFastReloadsInARow ? "yes" : "no" );
	Msg(" Damage Amp medal: %s\n", m_bDamageAmpMedal ? "yes" : "no" );
	Msg(" Killed boomer early: %s\n", m_bKilledBoomerEarly ? "yes" : "no" );
	Msg(" Stun grenade medal: %s\n", m_bStunGrenadeMedal ? "yes" : "no" );
	Msg(" Freeze grenade medal: %s\n", m_bFreezeGrenadeMedal ? "yes" : "no" );
	Msg(" Dodged ranger: %s\n", m_bDodgedRanger ? "yes" : "no" );
	Msg(" Protected tech: %s\n", m_bProtectedTech ? "yes" : "no" );
	Msg(" Melee parasite kill: %s\n", m_bMeleeParasiteKill ? "yes" : "no" );
	Msg(" Aliens kicked: %d\n", m_iAliensKicked);
	Msg(" Fast Door Hacks: %d\n", m_iFastDoorHacks);
	Msg(" Fast Computer Hacks: %d\n", m_iFastComputerHacks);
}

void CASW_Marine_Resource::DebugMedalStatsOnScreen()
{
	int i=2;
	float accuracy = 0;
	float player_acc = 0;
	if (m_iShotsFired > 0)
	{
		accuracy = float(m_iShotsFired - m_iShotsMissed) / float(m_iShotsFired);
	}
	if (m_iPlayerShotsFired > 0)
	{
		player_acc = float(m_iPlayerShotsFired - m_iPlayerShotsMissed) / float(m_iPlayerShotsFired);
	}

	engine->Con_NPrintf( i++, "Shots fired: (%d)", m_iShotsFired);
	engine->Con_NPrintf( i++, "Shots missed: %d (accuracy = %f)", m_iShotsMissed, accuracy);
	engine->Con_NPrintf( i++, "Player Shots fired: (%d)", m_iPlayerShotsFired);
	engine->Con_NPrintf( i++, "Player Shots missed: %d (accuracy = %f)", m_iPlayerShotsMissed, player_acc);	
	engine->Con_NPrintf( i++, "Damage taken: %f\n", m_fDamageTaken);
	engine->Con_NPrintf( i++, "Alien kills: %d\n", m_iAliensKilled.Get());
	engine->Con_NPrintf( i++, "FF damage dealt: %f\n", m_fFriendlyFireDamageDealt);
	engine->Con_NPrintf( i++, "FF damage taken: %f\n", GetMarineEntity() ? GetMarineEntity()->m_fFriendlyFireDamage : -1);
	engine->Con_NPrintf( i++, "Single explosion kills: %d\n", m_iSingleGrenadeKills);
	engine->Con_NPrintf( i++, "Kicked Grenade kills: %d\n", m_iKickedGrenadeKills);
	engine->Con_NPrintf( i++, "Shieldbugs killed: %d\n", m_iShieldbugsKilled);
	engine->Con_NPrintf( i++, "Parasites killed: %d\n", m_iParasitesKilled);	
	engine->Con_NPrintf( i++, "Lifesaver kills: %d\n", m_iSavedLife);
	bool bHurt = m_bHurt;
	engine->Con_NPrintf( i++, "Been hurt: %d\n", bHurt);
	bool bWounded = m_bTakenWoundDamage;
	engine->Con_NPrintf( i++, "Been wounded: %d\n", bWounded );
	engine->Con_NPrintf( i++, "Mad Firing: %d\n", m_iMadFiring);
	engine->Con_NPrintf( i++, "Mad Firing Autogun: %d\n", m_iMadFiringAutogun);
	//engine->Con_NPrintf( i++, "Last Stand Kills: %d\n", m_iLastStandKills);
	engine->Con_NPrintf( i++, "Health Healed: %d\n", m_iMedicHealing);
	engine->Con_NPrintf( i++, "Cured Infestation: %d\n", m_iCuredInfestation);
	engine->Con_NPrintf( i++, "Only weapon used index: %d\n", m_iOnlyWeaponEquipIndex);
	engine->Con_NPrintf( i++, "Only weapon used extra: %d\n", m_bOnlyWeaponExtra);
	engine->Con_NPrintf( i++, "Mine burns: %d\n", m_iMineKills);	
	engine->Con_NPrintf( i++, "Sentry kills: %d\n", m_iSentryKills);
	engine->Con_NPrintf( i++, "Eggs killed: %d\n", m_iEggKills);
	engine->Con_NPrintf( i++, "Grubs killed: %d\n", m_iGrubKills);
	engine->Con_NPrintf( i++, "Aliens killed by barrels: %d\n", m_iBarrelKills);
	engine->Con_NPrintf( i++, "Fires put out: %d\n", m_iFiresPutOut);
	engine->Con_NPrintf( i++, "Aliens kicked: %d\n", m_iAliensKicked);
	engine->Con_NPrintf( i++, "Fast door hacks: %d\n", m_iFastDoorHacks);
	engine->Con_NPrintf( i++, "Fast computer hacks: %d\n", m_iFastComputerHacks);
}

bool CASW_Marine_Resource::IsReloading()
{
	if (!GetMarineEntity())
		return false;

	CASW_Weapon *pWeapon = GetMarineEntity()->GetActiveASWWeapon();
	if (!pWeapon)
		return false;

	return pWeapon->IsReloading();
}

void CASW_Marine_Resource::IncrementWeaponStats( Class_T weaponClass, int32 nDamage, int32 nFFDamage, int32 nShotsFired, int32 nShotsHit, int32 nKills )
{
	extern ConVar asw_stats_verbose;

	FOR_EACH_VEC( m_WeaponStats, i )
	{
		if ( m_WeaponStats[i].m_WeaponClass == weaponClass )
		{
			if ( asw_stats_verbose.GetBool() )
			{
				DevMsg( "marine %d weaponclass %d (d:%d+%d, f:%d+%d, s:%d+%d, h:%d+%d, k:%d+%d)\n", ASWGameResource()->GetMarineResourceIndex( this ), weaponClass,
					m_WeaponStats[i].m_nDamage, nDamage, m_WeaponStats[i].m_nFFDamage, nFFDamage,
					m_WeaponStats[i].m_nShotsFired, nShotsFired, m_WeaponStats[i].m_nShotsHit, nShotsHit,
					m_WeaponStats[i].m_nKills, nKills );
			}
			m_WeaponStats[i].m_nDamage += nDamage;
			m_WeaponStats[i].m_nFFDamage += nFFDamage;
			m_WeaponStats[i].m_nShotsFired += nShotsFired;
			m_WeaponStats[i].m_nShotsHit += nShotsHit;
			m_WeaponStats[i].m_nKills += nKills;
			return;
		}
	}

	if ( asw_stats_verbose.GetBool() )
	{
		DevMsg( "marine %d weaponclass %d (d:%d, f:%d, s:%d, h:%d, k:%d)\n", ASWGameResource()->GetMarineResourceIndex( this ), weaponClass,
			nDamage, nFFDamage, nShotsFired, nShotsHit, nKills );
	}

	WeaponStats_t weaponStats( weaponClass );
	weaponStats.m_nDamage = nDamage;
	weaponStats.m_nFFDamage = nFFDamage;
	weaponStats.m_nShotsFired = nShotsFired;
	weaponStats.m_nShotsHit = nShotsHit;
	weaponStats.m_nKills = nKills;
	m_WeaponStats.AddToTail( weaponStats );
}

int __cdecl CASW_Marine_Resource::CompareWeaponStats( const WeaponStats_t *a, const WeaponStats_t *b )
{
	return b->m_nShotsFired - a->m_nShotsFired;
}
