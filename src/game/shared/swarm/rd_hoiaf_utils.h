#pragma once

#include "steam/isteamhttp.h"

#ifdef CLIENT_DLL
#include <vgui/VGUI.h>

class CBaseHudChat;
#endif

class SINGLE_INHERITANCE CRD_HoIAF_System : public CAutoGameSystemPerFrame
{
public:
	CRD_HoIAF_System();

	void PostInit() override;
#ifdef CLIENT_DLL
	void Update( float frametime ) override;
#else
	void PreClientUpdate() override;
#endif

	// Returns true if the current data is non-expired. May fire off an HTTP request if there isn't one active.
	bool CheckIAFIntelUpToDate();
	// Checks whether a server IP is in the HoIAF participating server list.
	bool IsRankedServerIP( uint32_t ip ) const;
	// Returns the date of the latest release.
	bool GetLastUpdateDate( int &year, int &month, int &day ) const;

	// Featured News
	int CountFeaturedNews() const;
	const char *GetFeaturedNewsCaption( int index ) const;
	const char *GetFeaturedNewsURL( int index ) const;
#ifdef CLIENT_DLL
	vgui::HTexture GetFeaturedNewsTexture( int index ) const;
#endif

	// Event Timers
	int CountEventTimers() const;
	bool IsEventTimerActive( int index ) const;
	const char *GetEventTimerCaption( int index ) const;
	const char *GetEventTimerURL( int index ) const;
	int64_t GetEventStartTime( int index ) const;
	int64_t GetEventEndTime( int index ) const;

	// Chat Announcements
#ifdef CLIENT_DLL
	void InsertChatMessages( CBaseHudChat *pChat );
#endif

	// HoIAF Mission Bounties
#ifdef CLIENT_DLL
	void MarkBountyAsCompleted( int iBountyID );
#endif

private:
	void ParseIAFIntel();
	void LoadCachedIAFIntel();
	bool RefreshIAFIntel( bool bOnlyIfExpired = true, bool bForceNewRequest = false );
	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );
	void OnRequestFailed();
	void OnNewIntelReceived();
#ifdef CLIENT_DLL
	void LoadTranslatedString( CUtlString &str, KeyValues *pKV, const char *szTemplate );
#endif

	KeyValues::AutoDelete m_pIAFIntel;
	HTTPRequestHandle m_hIAFIntelRefreshRequest{ INVALID_HTTPREQUEST_HANDLE };
	CCallResult<CRD_HoIAF_System, HTTPRequestCompleted_t> m_IAFIntelRefresh;
	int m_iExponentialBackoff{};
	uint32_t m_iBackoffUntil{};

	int64_t m_iExpireAt{};
	CUtlVector<uint32_t> m_RankedServerIPs;
	int m_iLatestPatch{};
	struct FeaturedNews_t
	{
		~FeaturedNews_t();

		CUtlString Caption;
		CUtlString URL;
#ifdef CLIENT_DLL
		CRC32_t TextureCRC;
		int TextureIndex;
		vgui::HTexture TextureHandle{ NULL };

		HTTPRequestHandle m_hTextureRequest{ INVALID_HTTPREQUEST_HANDLE };
		void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );

		CCallResult<FeaturedNews_t, HTTPRequestCompleted_t> m_TextureRequestFinished;
#endif
	};
	CUtlVectorAutoPurge<FeaturedNews_t *> m_FeaturedNews;
	struct EventTimer_t
	{
		CUtlString Caption;
		CUtlString URL;
		int64_t Starts;
		int64_t Ends;
	};
	CUtlVectorAutoPurge<EventTimer_t *> m_EventTimers;
	struct ChatAnnouncement_t
	{
		CUtlString ID;
		CUtlString Text;
		CUtlString Zbalermorna;
		int64_t NotBefore;
		int64_t NotAfter;
		Color Color;
#ifdef CLIENT_DLL
		vgui::HFont Font;
#endif
	};
	CUtlVectorAutoPurge<ChatAnnouncement_t *> m_ChatAnnouncements;
	struct HoIAFMissionBounty_t
	{
		int ID;
		int64_t Starts;
		int64_t Ends;
		CUtlString Map;
		int Points;
	};
	CUtlVectorAutoPurge<HoIAFMissionBounty_t *> m_HoIAFMissionBounties;
#ifdef CLIENT_DLL
	CUtlSymbolTable m_ChatAnnouncementSeen;
#endif
};

CRD_HoIAF_System *HoIAF();
