#ifndef _DEFINED_ASW_COMPUTER_AREA_H
#define _DEFINED_ASW_COMPUTER_AREA_H

#include "asw_use_area.h"
#include "asw_shareddefs.h"

class CASW_Player;
class CASW_Hack_Computer;
class CASW_Marine;
class CASW_PointCamera;
class CASW_Remote_Turret;
class CRD_Computer_VScript;

class CASW_Computer_Area : public CASW_Use_Area
{
	DECLARE_CLASS( CASW_Computer_Area, CASW_Use_Area );
public:
	CASW_Computer_Area();
	virtual void Spawn( void );
	virtual void Precache();
	void FindTurretsAndCams();
	void ActivateUnlockedComputer( CASW_Marine *pMarine );
	bool KeyValue( const char *szKeyName, const char *szValue );
	CASW_Hack_Computer *GetCurrentHack();
	virtual bool IsLocked() { return m_bIsLocked.Get(); }
	virtual bool HasDownloadObjective();

	Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_COMPUTER_AREA; }

	static bool WaitingForInputVismonEvaluator( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer );
	static bool WaitingForInputVismonCallback( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer );

	void OnComputerDataDownloaded( CASW_Marine *pMarine );
	void Override( CASW_Marine *pMarine );

	// viewing mail
	void OnViewMail( CASW_Marine *pMarine, int iMail );
	bool m_bViewingMail;

	virtual void ActivateMultiTrigger( CBaseEntity *pActivator );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNetworkVar( int, m_iHackLevel );
	CNetworkVar( float, m_fDownloadTime );
	CNetworkVar( bool, m_bIsLocked );
	CNetworkVar( bool, m_bWaitingForInput );
	CNetworkVar( bool, m_bLoggedIn );
	CHandle<CASW_Hack_Computer> m_hComputerHack;

	string_t		m_CustomHackName;
	string_t		m_SecurityCam1Name;
	string_t		m_SecurityCam2Name;
	string_t		m_SecurityCam3Name;
	string_t		m_Turret1Name;
	string_t		m_Turret2Name;
	string_t		m_Turret3Name;
	string_t		m_CustomScreen1Name;
	string_t		m_CustomScreen2Name;
	string_t		m_CustomScreen3Name;
	string_t		m_CustomScreen4Name;
	string_t		m_CustomScreen5Name;
	string_t		m_CustomScreen6Name;

	CNetworkString( m_MailFile, 255 );
	CNetworkString( m_NewsFile, 255 );
	CNetworkString( m_StocksSeed, 255 );
	CNetworkString( m_WeatherSeed, 255 );
	CNetworkString( m_PlantFile, 255 );
	CNetworkString( m_PDAName, 255 );

	CNetworkString( m_SecurityCamLabel1, 255 );
	CNetworkString( m_SecurityCamLabel2, 255 );
	CNetworkString( m_SecurityCamLabel3, 255 );

	CNetworkString( m_DownloadObjectiveName, 255 );
	CNetworkVar( bool, m_bDownloadedDocs );

	bool m_bDownloadLocked;
	bool m_bSecurityCam1Locked;
	bool m_bSecurityCam2Locked;
	bool m_bSecurityCam3Locked;
	bool m_bTurret1Locked;
	bool m_bTurret2Locked;
	bool m_bTurret3Locked;
	bool m_bMailFileLocked;
	bool m_bMail1Locked;
	bool m_bMail2Locked;
	bool m_bMail3Locked;
	bool m_bMail4Locked;
	bool m_bNewsFileLocked;
	bool m_bStocksFileLocked;
	bool m_bWeatherFileLocked;
	bool m_bPlantFileLocked;
	bool m_bCustomScreen1Locked;
	bool m_bCustomScreen2Locked;
	bool m_bCustomScreen3Locked;
	bool m_bCustomScreen4Locked;
	bool m_bCustomScreen5Locked;
	bool m_bCustomScreen6Locked;

	CNetworkVar( unsigned, m_iLockedScreens );

	CNetworkHandle( CRD_Computer_VScript, m_hCustomHack );
	CNetworkHandle( CASW_PointCamera, m_hSecurityCam1 );
	CNetworkHandle( CASW_PointCamera, m_hSecurityCam2 );
	CNetworkHandle( CASW_PointCamera, m_hSecurityCam3 );
	CNetworkHandle( CASW_Remote_Turret, m_hTurret1 );
	CNetworkHandle( CASW_Remote_Turret, m_hTurret2 );
	CNetworkHandle( CASW_Remote_Turret, m_hTurret3 );
	CNetworkHandle( CRD_Computer_VScript, m_hCustomScreen1 );
	CNetworkHandle( CRD_Computer_VScript, m_hCustomScreen2 );
	CNetworkHandle( CRD_Computer_VScript, m_hCustomScreen3 );
	CNetworkHandle( CRD_Computer_VScript, m_hCustomScreen4 );
	CNetworkHandle( CRD_Computer_VScript, m_hCustomScreen5 );
	CNetworkHandle( CRD_Computer_VScript, m_hCustomScreen6 );

	CNetworkVar( int, m_iActiveCam );
	CNetworkVar( int8_t, m_iReactorState );

	// inputs
	void InputOverrideReactorState( inputdata_t &inputdata );

	// outputs
	COutputEvent m_OnFastHackFailed;
	COutputEvent m_OnComputerHackStarted;
	COutputEvent m_OnComputerHackHalfway;
	COutputEvent m_OnComputerHackCompleted;
	COutputEvent m_OnComputerActivated;
	COutputEvent m_OnComputerDataDownloaded;
	COutputEvent m_OnComputerViewMail1;
	COutputEvent m_OnComputerViewMail2;
	COutputEvent m_OnComputerViewMail3;
	COutputEvent m_OnComputerViewMail4;

	// properties of the tumbler hack
	int m_iNumEntriesPerTumbler;
	float m_fMoveInterval;

	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType );
	virtual void NPCUsing( CASW_Inhabitable_NPC *pNPC, float deltatime );
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pNPC );
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC );
	virtual void UnlockFromHack( CASW_Marine *pMarine );
	virtual void HackHalfway( CASW_Marine *pMarine );
	virtual bool IsWaitingForInput( void ) const { return m_bWaitingForInput; }

	virtual int GetNumMenuOptions();
	float GetDownloadProgress() { return m_fDownloadProgress; }

	CNetworkVar( bool, m_bIsInUse );
	CNetworkVar( float, m_fDownloadProgress );
	bool m_bWasLocked;
	bool m_bUseAfterHack;
	float m_fAutoOverrideTime;
	float m_fLastButtonUseTime;
	int m_iAliensKilledBeforeHack;
	float m_flStopUsingTime;		// time at which to stop the marine using this computer

	// sound related
	void StopLoopingSounds();
	void PlayPositiveSound( CASW_Player *pHackingPlayer );
	void PlayNegativeSound( CASW_Player *pHackingPlayer );
	float m_fLastPositiveSoundTime;
	bool m_bPlayedHalfwayChatter;
	bool m_bDoSecureShout;
	float m_fNextSecureShoutCheck;
	void StartDownloadingSound();
	void StopDownloadingSound();
	CSoundPatch *m_pDownloadingSound;
	CSoundPatch *m_pComputerInUseSound;

	virtual void UpdateWaitingForInput();
	virtual void UpdatePanelSkin();
	virtual void UpdateLockedScreensBits();
	CASW_PointCamera *GetActiveCam();

private:
	bool ShouldShowComputer();
};

#endif /* _DEFINED_ASW_COMPUTER_AREA_H */
