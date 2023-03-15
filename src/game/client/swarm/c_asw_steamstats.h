#ifndef DEFINED_C_ASW_STEAMSTATS_H_
#define DEFINED_C_ASW_STEAMSTATS_H_

#include "steam/steam_api.h"
#include "platform.h"
#include "c_asw_player.h"
#include "utlvector.h"

struct DifficultyStats_t
{
	bool FetchDifficultyStats( CSteamID playerSteamID, int iDifficulty );
	void PrepStatsForSend( CASW_Player *pPlayer ); 

	int32 m_iGamesTotal;
	int32 m_iGamesSuccess;
	float32 m_fGamesSuccessPercent;
	int32 m_iKillsTotal;
	int32 m_iDamageTotal;
	int32 m_iFFTotal;
	float32 m_fAccuracyAvg;
	int32 m_iShotsFiredTotal;
	int32 m_iShotsHitTotal;
	int32 m_iHealingTotal;
};

struct MissionStats_t
{
	bool FetchMissionStats( CSteamID playerSteamID );
	void PrepStatsForSend( CASW_Player *pPlayer ); 

	int32 m_iGamesTotal;
	int32 m_iGamesSuccess;
	float32 m_fGamesSuccessPercent;
	int32 m_iKillsTotal;
	int32 m_iDamageTotal;
	int32 m_iFFTotal;
	float32 m_fKillsAvg;
	float32 m_fDamageAvg;
	float32 m_fFFAvg;
	int32 m_iTimeTotal;
	int32 m_iTimeSuccess;
	int32 m_iTimeAvg;
	int32 m_iHighestDifficulty;
	int32 m_iBestSpeedrunTimes[5];
	int32 m_iScoreTotal;
	int32 m_iScoreSuccess;
	int32 m_iBestHighScores[5];
};

struct WeaponStats_t
{
	bool FetchWeaponStats( CSteamID playerSteamID, const char *szClassName );
	void PrepStatsForSend( CASW_Player *pPlayer ); 

	int32 m_iWeaponIndex;
	int32 m_iDamage;
	int32 m_iFFDamage;
	int32 m_iShotsFired;
	int32 m_iShotsHit;
	int32 m_iKills;
	bool  m_bIsExtra;
	const char *m_szClassName;
};

#pragma pack(push, 1)
struct LeaderboardScoreDetails_v1_t
{
	uint16 m_iVersion;
	int16 m_iMarine;
	int16 m_iSquadSize;
	int16 m_iPrimaryWeapon;
	int16 m_iSecondaryWeapon;
	int16 m_iExtraWeapon;
	uint64 m_iChallenge;
	uint64 m_iTimestamp;
	char m_CountryCode[2];
	uint8 m_iDifficulty;
	uint8 m_iModeFlags;
};
struct LeaderboardScoreDetails_v2_t
{
	uint16 m_iVersion;
	uint8 m_iMarine;
	uint8 m_iPrimaryWeapon;
	uint8 m_iSecondaryWeapon;
	uint8 m_iExtraWeapon;
	uint8 m_iSquadDead;
	uint8 m_iSquadSize;
	uint64 m_iTimestamp;
	uint64 m_iSquadMarineSteam[7];
	uint8 m_iSquadMarine[7];
	uint8 m_iSquadPrimaryWeapon[7];
	uint8 m_iSquadSecondaryWeapon[7];
	uint8 m_iSquadExtraWeapon[7];
	char m_CountryCode[2];
	uint8 m_iDifficulty;
	uint8 m_iModeFlags;
	uint32 m_iGameVersion;
};
struct LeaderboardScoreDetails_Points_t
{
	int16 m_iVersion;
	char m_CountryCode[2];
	int32 m_iAlienKills;
	int32 m_iPlayerKills;
	int32 m_iGamesWon;
	int32 m_iGamesLost;	
};
#pragma pack(pop)
ASSERT_INVARIANT( sizeof( LeaderboardScoreDetails_v1_t ) % sizeof( int32 ) == 0 );
ASSERT_INVARIANT( sizeof( LeaderboardScoreDetails_v1_t ) / sizeof( int32 ) <= k_cLeaderboardDetailsMax );
ASSERT_INVARIANT( ASW_NUM_MARINE_PROFILES - 1 == 7 );
ASSERT_INVARIANT( sizeof( LeaderboardScoreDetails_v2_t ) % sizeof( int32 ) == 0 );
ASSERT_INVARIANT( sizeof( LeaderboardScoreDetails_v2_t ) / sizeof( int32 ) <= k_cLeaderboardDetailsMax );
ASSERT_INVARIANT( sizeof( LeaderboardScoreDetails_Points_t ) % sizeof( int32 ) == 0 );								// jh: what is this i dont know, but i copypaste and hope it work!
ASSERT_INVARIANT( sizeof( LeaderboardScoreDetails_Points_t ) / sizeof( int32 ) <= k_cLeaderboardDetailsMax );

struct RD_LeaderboardEntry_t
{
	LeaderboardEntry_t entry;
	union
	{
		uint16 version;
		LeaderboardScoreDetails_v1_t v1;
		LeaderboardScoreDetails_v2_t v2;
	} details;
};

struct RD_LeaderboardEntry_Points_t
{
	LeaderboardEntry_t entry;
	LeaderboardScoreDetails_Points_t details;
};


class CASW_Steamstats
{
public:
	// Fetch the client's steamstats
	bool FetchStats( CSteamID playerSteamID, CASW_Player *pPlayer );

	// Send the client's stats off to steam
	void PrepStatsForSend( CASW_Player *pPlayer ); 

	bool IsOfficialCampaign();
	bool IsLBWhitelisted( const char *name );

	// Send leaderboard entries to Steam
	void PrepStatsForSend_Leaderboard( CASW_Player *pPlayer, bool bUnofficial );

	void SpeedRunLeaderboardName( char *szBuf, size_t bufSize, const char *szMap, PublishedFileId_t nMapID = 0, const char *szChallenge = "0", PublishedFileId_t nChallengeID = 0, ELeaderboardSortMethod *pESortMethod = NULL, ELeaderboardDisplayType *pEDisplayType = NULL );
	void DifficultySpeedRunLeaderboardName( char *szBuf, size_t bufSize, int iSkill, const char *szMap, PublishedFileId_t nMapID = 0, const char *szChallenge = "0", PublishedFileId_t nChallengeID = 0 );

	void ReadDownloadedLeaderboard( CUtlVector<RD_LeaderboardEntry_t> & entries, SteamLeaderboardEntries_t hEntries, int nCount );
	void ReadDownloadedLeaderboard( CUtlVector<RD_LeaderboardEntry_Points_t> & entries, SteamLeaderboardEntries_t hEntries, int nCount );

private:
	int32 m_iTotalKills;
	float32 m_fAccuracy;
	int32 m_iFriendlyFire;
	int32 m_iDamage;
	int32 m_iShotsFired;
	int32 m_iShotsHit;
	int32 m_iAliensBurned;
	int32 m_iBiomassIgnited;
	int32 m_iHealing;
	int32 m_iFastHacksLegacy;
	int32 m_iFastHacksWire;
	int32 m_iFastHacksComputer;
	int32 m_iGamesTotal;
	int32 m_iGamesSuccess;
	float32 m_fGamesSuccessPercent;
	int32	m_iAmmoDeployed;
	int32	m_iSentryGunsDeployed;
	int32	m_iSentryFlamerDeployed;
	int32	m_iSentryFreezeDeployed;
	int32	m_iSentryCannonDeployed;
	int32	m_iSentryRailgunDeployed;
	int32	m_iMedkitsUsed;
	int32	m_iFlaresUsed;
	int32	m_iAdrenalineUsed;
	int32	m_iTeslaTrapsDeployed;
	int32	m_iFreezeGrenadesThrown;
	int32	m_iElectricArmorUsed;
	int32	m_iHealGunHeals;
	int32	m_iHealBeaconHeals;
	int32	m_iHealGunHeals_Self;
	int32	m_iHealBeaconHeals_Self;
	int32	m_iDamageAmpsUsed;
	int32	m_iHealBeaconsDeployed;
	int32	m_iMedkitHeals_Self;
	int32	m_iGrenadeExtinguishMarine;
	int32	m_iGrenadeFreezeAlien;
	int32	m_iDamageAmpAmps;
	int32	m_iNormalArmorReduction;
	int32	m_iElectricArmorReduction;
	int32	m_iHealAmpGunHeals;
	int32	m_iHealAmpGunAmps;
	int32	m_iMedRifleHeals;
	int32	m_iCryoCannonFreezeAlien;
	int32	m_iPlasmaThrowerExtinguishMarine;
	int32	m_iHackToolWireHacksTech;
	int32	m_iHackToolWireHacksOther;
	int32	m_iHackToolComputerHacksTech;
	int32	m_iHackToolComputerHacksOther;
	int32	m_iEnergyShieldProjectilesDestroyed;
	int32	m_iReanimatorRevivesOfficer;
	int32	m_iReanimatorRevivesSpecialWeapons;
	int32	m_iReanimatorRevivesMedic;
	int32	m_iReanimatorRevivesTech;
	int32	m_iSpeedBoostsUsed;
	int32	m_iShieldBubblesThrown;
	int32	m_iShieldBubblePushedEnemy;
	int32	m_iShieldBubbleDamageAbsorbed;
	int32	m_iLeadershipProcsAccuracy;
	int32	m_iLeadershipProcsResist;
	int32	m_iLeadershipDamageAccuracy;
	int32	m_iLeadershipDamageResist;
	int32	m_iTotalPlayTime;
	
	typedef CUtlVector<int32> StatList_Int_t;
	typedef CUtlVector<WeaponStats_t> WeaponStatList_t;

	StatList_Int_t m_PrimaryEquipCounts;
	StatList_Int_t m_SecondaryEquipCounts;
	StatList_Int_t m_ExtraEquipCounts;
	StatList_Int_t m_MarineSelectionCounts;
	StatList_Int_t m_MissionPlayerCounts;
	StatList_Int_t m_DifficultyCounts;

	DifficultyStats_t m_DifficultyStats[5];
	MissionStats_t m_MissionStats;
	WeaponStatList_t m_WeaponStats;
	
	int GetFavoriteEquip( int iSlot );
	int GetFavoriteMarine( void );
	int GetFavoriteMarineClass( void );
	int GetFavoriteDifficulty( void );

	float GetFavoriteEquipPercent( int iSlot );
	float GetFavoriteMarinePercent( void );
	float GetFavoriteMarineClassPercent( void );
	float GetFavoriteDifficultyPercent( void );

	CCallResult<CASW_Steamstats, LeaderboardFindResult_t> m_LeaderboardFindResultCallback;
	void LeaderboardFindResultCallback( LeaderboardFindResult_t *pResult, bool bIOFailure );
	CCallResult<CASW_Steamstats, LeaderboardFindResult_t> m_LeaderboardDifficultyFindResultCallback;
	void LeaderboardDifficultyFindResultCallback( LeaderboardFindResult_t *pResult, bool bIOFailure );
	CCallResult<CASW_Steamstats, LeaderboardScoreUploaded_t> m_LeaderboardScoreUploadedCallback;
	void LeaderboardScoreUploadedCallback( LeaderboardScoreUploaded_t *pResult, bool bIOFailure );
	CCallResult<CASW_Steamstats, LeaderboardScoreUploaded_t> m_LeaderboardDifficultyScoreUploadedCallback;
	void LeaderboardDifficultyScoreUploadedCallback( LeaderboardScoreUploaded_t *pResult, bool bIOFailure );

	int32 m_iLeaderboardScore;
	LeaderboardScoreDetails_v2_t m_LeaderboardScoreDetails;
};

extern CASW_Steamstats g_ASW_Steamstats;

#endif // #ifndef DEFINED_C_ASW_STEAMSTATS_H_
