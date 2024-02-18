#pragma once

#include "steam/steam_api.h"

struct ReportingServerSnapshot_t;
namespace BaseModUI
{
	class ReportProblem;
}

// Player Reporting (that is, reports made by players, not necessarily reports about players).
//
// Reports consist of a category, a written description, some diagnostic data, optional screenshots,
// and an authentication token proving that the reporting player is who they say they are.
//
// Players can start writing a report up to 5 minutes after the player they're reporting disconnects
// or after they are disconnected from the server where the incident happened.
class CRD_Player_Reporting
{
public:
	// Returns true if a report is currently being sent.
	bool IsInProgress() const;
	// Returns true if the player was connected to a dedicated server within the past 5 minutes.
	bool HasRecentServer() const;
	// Returns true if we recently filed a descriptionless report in the given category for the given player.
	// Non-const because it loads a cache and therefore has side effects.
	bool RecentlyReportedPlayer( const char *szCategory, CSteamID player );
	// Fills a vector with a list of players seen in the past 5 minutes.
	void GetRecentlyPlayedWith( CUtlVector<CSteamID> &players ) const;
	// Returns true if any player has been seen in the past 5 minutes.
	bool HasRecentlyPlayedWith() const;
	// Returns true if a player has been seen in the past 5 minutes.
	bool HasRecentlyPlayedWith( CSteamID player ) const;

	// Update cached server info.
	void UpdateServerInfo();
	// Returns a snapshot containing the player, the latest snapshot, or NULL. The snapshot stays valid until PinRelevantSnapshot is called again.
	const ReportingServerSnapshot_t *PinRelevantSnapshot( CSteamID player, bool bRequireDedicatedServer = false );

	// Prepares to send a user-provided category, description, and player ID,
	// as well as zero or more JPEG screenshots, to the player reporting API.
	//
	// Returns true if the reporting process has been started.
	//
	// szCategory cannot be null and should be from a predefined list of categories.
	// If szDescription is null, screenshots must be empty and reportedPlayer must be valid.
	//
	// Can return false for two reasons:
	// 1. Only one report can be in-transit at a time.
	// 2. Can only send a report if we are connected to Steam.
	bool PrepareReportForSend( const char *szCategory, const char *szDescription, CSteamID reportedPlayer, const CUtlVector<CUtlBuffer> &screenshots = CUtlVector<CUtlBuffer>{}, const ReportingServerSnapshot_t *pForceSnapshot = NULL );

	// Called when it's time to update the player reporting screenshot, such as when WT_INGAMEMAINMENU is opened.
	void TakeScreenshot();

	// Latest progress message.
	wchar_t m_wszLastMessage[2048]{};

private:
	CUtlBuffer m_Buffer{ 0, 0, CUtlBuffer::TEXT_BUFFER };
	CUtlBuffer m_LatestScreenshot;
	ReportingServerSnapshot_t *m_pPinnedSnapshot{ NULL };
	CUtlVectorAutoPurge<ReportingServerSnapshot_t *> m_RecentData;
	char m_szQuickReport[256]{};
	char m_szLastCategory[256]{};
	KeyValues::AutoDelete m_pRecentReports{ ( KeyValues * )NULL };
	HAuthTicket m_hTicket{ k_HAuthTicketInvalid };
	friend class BaseModUI::ReportProblem;

	STEAM_CALLBACK( CRD_Player_Reporting, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
	CCallResult<CRD_Player_Reporting, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );

	void LogProgressMessage();

	ReportingServerSnapshot_t *NewSnapshot() const;
	ReportingServerSnapshot_t *GetRelevantOrLatestSnapshot( CSteamID player, bool bRequireDedicatedServer = false ) const;
	void DeleteExpiredSnapshots();
};

extern CRD_Player_Reporting g_RD_Player_Reporting;
