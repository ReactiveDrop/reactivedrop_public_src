#pragma once

#include "steam/steam_api.h"

struct ReportingServerSnapshot_t;

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
	// Returns true if the player was connected to a server within the past 5 minutes.
	bool HasRecentServer() const;

	// Update cached server info.
	void UpdateServerInfo();

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
	bool PrepareReportForSend( const char *szCategory, const char *szDescription, CSteamID reportedPlayer, CUtlVector<CUtlBuffer> &screenshots, bool bShowWaitScreen );

	// Latest progress message.
	wchar_t m_wszLastMessage[2048]{};

private:
	bool m_bShowWaitScreen{ false };
	CUtlBuffer m_Buffer{ 0, 0, CUtlBuffer::TEXT_BUFFER };
	CUtlVectorAutoPurge<ReportingServerSnapshot_t *> m_RecentData;
	HAuthTicket m_hTicket{ k_HAuthTicketInvalid };

	STEAM_CALLBACK( CRD_Player_Reporting, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
	CCallResult<CRD_Player_Reporting, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );

	void ShowWaitScreen();

	ReportingServerSnapshot_t *NewSnapshot() const;
	ReportingServerSnapshot_t *GetRelevantOrLatestSnapshot( CSteamID player ) const;
	void DeleteExpiredSnapshots();
};

extern CRD_Player_Reporting g_RD_Player_Reporting;
