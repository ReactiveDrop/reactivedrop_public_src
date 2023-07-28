#pragma once

#include "steam/steam_api.h"

class CRD_Player_Reporting
{
public:
	// Returns true if a report is currently being sent.
	bool IsInProgress();

	// Updates the cached server info. Call this before PrepareReportForSend while connected to a server.
	void UpdateServerInfo( bool bForce = false );

	// Prepares to send a user-provided category, description, and player ID,
	// as well as zero or more JPEG screenshots, to the player reporting API.
	//
	// Returns true if the reporting process has been started.
	//
	// Can return false for two reasons:
	// 1. Only one report can be in-transit at a time.
	// 2. Can only send a report if we are in a server and also connected to Steam.
	bool PrepareReportForSend( const char *szCategory, const char *szDescription, CSteamID reportedPlayer, CUtlVector<CUtlBuffer> &screenshots, bool bShowWaitScreen );

	// Latest progress message.
	wchar_t m_wszLastMessage[2048]{};

private:
	bool m_bShowWaitScreen{ false };
	CUtlBuffer m_Buffer{ 0, 0, CUtlBuffer::TEXT_BUFFER };
	CUtlBuffer m_ServerInfoCache{ 0, 0, CUtlBuffer::TEXT_BUFFER };
	HAuthTicket m_hTicket{ k_HAuthTicketInvalid };

	STEAM_CALLBACK( CRD_Player_Reporting, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
	CCallResult<CRD_Player_Reporting, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );

	void ShowWaitScreen();
};

extern CRD_Player_Reporting g_RD_Player_Reporting;
