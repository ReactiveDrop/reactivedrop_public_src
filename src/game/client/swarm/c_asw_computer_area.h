#ifndef _DEFINED_C_ASW_COMPUTER_AREA_H
#define _DEFINED_C_ASW_COMPUTER_AREA_H

#include "c_asw_use_area.h"
#include "c_asw_point_camera.h"
#include "asw_remote_turret_shared.h"

class C_ASW_Door;
class C_ASW_Player;

class C_ASW_Computer_Area : public C_ASW_Use_Area
{
	DECLARE_CLASS( C_ASW_Computer_Area, C_ASW_Use_Area );
	DECLARE_CLIENTCLASS();
public:
	C_ASW_Computer_Area();

	bool IsLocked() { return m_bIsLocked; }
	bool IsLoggedIn() { return m_bLoggedIn; }
	int GetHackLevel() { return m_iHackLevel; }
	C_ASW_Door* GetDoor();

	Class_T		Classify( void ) { return (Class_T) CLASS_ASW_COMPUTER_AREA; }
	
	// accessors for icons
	int GetLockedIconTextureID();
	const char* GetLockedIconText() { return "#asw_attempt_access"; }
	int GetOpenIconTextureID();
	const char* GetOpenIconText() { return "#asw_open"; }
	int GetCloseIconTextureID();
	const char* GetCloseIconText() { return "#asw_close"; }
	int GetUseIconTextureID();
	const char* GetUseIconText() { return "#asw_access_terminal"; }
	int GetHackIconTextureID();
	const char* GetHackIconText() { return "#asw_access_terminal"; }
	int GetUseIconPDATextureID();
	const char* GetUseIconPDAText() { return "#asw_access_pda"; }

	virtual float GetTumblerProgress(C_ASW_Marine *pUser);
	virtual bool GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser);
	virtual void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) { }
	virtual C_BaseEntity* GetGlowEntity() { return m_hPanelProp.Get(); }

	int GetNumMenuOptions();

	CNetworkString( m_MailFile, 255 );
	CNetworkString( m_NewsFile, 255 );
	CNetworkString( m_StocksSeed, 255 );
	CNetworkString( m_WeatherSeed, 255 );
	CNetworkString( m_PlantFile, 255 );
	CNetworkString( m_PDAName, 255 );

	CNetworkString( m_SecurityCamLabel1, 255 );
	CNetworkString( m_SecurityCamLabel2, 255 );
	CNetworkString( m_SecurityCamLabel3, 255 );
	CNetworkString( m_TurretLabel1, 255 );
	CNetworkString( m_TurretLabel2, 255 );
	CNetworkString( m_TurretLabel3, 255 );

	CNetworkString( m_DownloadObjectiveName, 255 );
	CNetworkVar( bool, m_bDownloadedDocs );

	CNetworkVar( bool, m_bSecurityCam1Locked );
	CNetworkVar( bool, m_bSecurityCam2Locked );
	CNetworkVar( bool, m_bSecurityCam3Locked );
	CNetworkVar( bool, m_bTurret1Locked );
	CNetworkVar( bool, m_bTurret2Locked );
	CNetworkVar( bool, m_bTurret3Locked );
	CNetworkVar( bool, m_bMailFileLocked );
	CNetworkVar( bool, m_bNewsFileLocked );
	CNetworkVar( bool, m_bStocksFileLocked );
	CNetworkVar( bool, m_bWeatherFileLocked );
	CNetworkVar( bool, m_bPlantFileLocked );

	CNetworkHandle( C_ASW_PointCamera, m_hSecurityCam1 );
	CNetworkHandle( C_ASW_PointCamera, m_hSecurityCam2 );
	CNetworkHandle( C_ASW_PointCamera, m_hSecurityCam3 );
	CNetworkHandle( CASW_Remote_Turret, m_hTurret1 );
	CNetworkHandle( CASW_Remote_Turret, m_hTurret2 );
	CNetworkHandle( CASW_Remote_Turret, m_hTurret3 );

	CNetworkVar( int8_t, m_iReactorState );

	// traditional Swarm hacking
	float GetDownloadProgress() { return m_fDownloadProgress; }
	CNetworkVar(bool, m_bIsInUse);
	CNetworkVar(bool, m_bLoggedIn);
	CNetworkVar(float, m_fDownloadProgress);

	bool IsWaitingForInput( void ) const { return m_bWaitingForInput; }
	int m_iActiveCam;

	// does this computer area represent a PDA instead of a typical computer?
	bool IsPDA();

	void PlayPositiveSound(C_ASW_Player *pPlayer);
	void PlayNegativeSound(C_ASW_Player *pPlayer);
	float m_fLastPositiveSoundTime;
	C_ASW_PointCamera *GetActiveCam();
	const char *GetPlantFileName();

protected:
	bool m_bIsLocked;
	bool m_bWaitingForInput;
	bool m_bOldWaitingForInput;
	int m_iHackLevel;
	float m_fDownloadTime;
	C_ASW_Computer_Area( const C_ASW_Computer_Area & ) = delete; // not defined, not accessible

	// icons used to interact with computers
	static bool s_bLoadedLockedIconTexture;
	static int s_nLockedIconTextureID;

	static bool s_bLoadedOpenIconTexture;
	static int s_nOpenIconTextureID;

	static bool s_bLoadedCloseIconTexture;
	static int s_nCloseIconTextureID;

	static bool s_bLoadedUseIconTexture;
	static int s_nUseIconTextureID;

	static bool s_bLoadedHackIconTexture;
	static int s_nHackIconTextureID;

	static bool s_bLoadedUseIconPDA;
	static int s_nUseIconPDA;
};

#endif /* _DEFINED_C_ASW_COMPUTER_AREA_H */