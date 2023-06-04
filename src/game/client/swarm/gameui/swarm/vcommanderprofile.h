#pragma once

#include "basemodui.h"
#include "rd_inventory_shared.h"
#include "c_asw_steamstats.h"

class CRD_VGUI_Commander_Mini_Profile;

namespace BaseModUI
{

class CommanderProfile : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( CommanderProfile, CBaseModFrame );

public:
	CommanderProfile( vgui::Panel *parent, const char *panelName );
	~CommanderProfile();

	void SetDataSettings( KeyValues *pSettings ) override;

	void InitForLocalPlayer();
	void InitForNetworkedPlayer( C_ASW_Player *pPlayer );
	void InitForSteamID( CSteamID steamID );

	void FetchLocalMedals();
	void FetchSteamIDMedalsAndExperience();
	void FetchHoIAFSeasonStats();

	CRD_VGUI_Commander_Mini_Profile *m_pMiniProfile;

	enum Mode_t
	{
		MODE_INVALID,
		MODE_LOCAL_PLAYER,
		MODE_NETWORKED_PLAYER,
		MODE_STEAM_ID,
	} m_Mode{ MODE_INVALID };

	CSteamID m_SteamID;
	int m_iExperience{ -1 };
	int m_iPromotion{ -1 };
	C_RD_ItemInstance m_Medals[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS]{};

	LeaderboardEntry_t m_HoIAFLeaderboardEntry;
	LeaderboardScoreDetails_Points_t m_HoIAFDetails;
	CCallResult<CommanderProfile, LeaderboardScoresDownloaded_t> m_HoIAFSeasonStatsFetched;
	void OnHoIAFSeasonStatsFetched( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure );

	HTTPRequestHandle m_hHTTPRequest{ INVALID_HTTPREQUEST_HANDLE };
	CCallResult<CommanderProfile, HTTPRequestCompleted_t> m_SteamIDProfileDataFetched;
	void OnSteamIDProfileDataFetched( HTTPRequestCompleted_t *pParam, bool bIOFailure );
};

}
