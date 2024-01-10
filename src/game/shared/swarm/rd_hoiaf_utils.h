#pragma once

#include "steam/isteamhttp.h"

#ifdef CLIENT_DLL
#include <vgui/VGUI.h>

class CBaseHudChat;
#endif

// Note: This system is documented in more detail at <https://developer.reactivedrop.com/iaf-intel.html>.
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

#ifdef CLIENT_DLL
	// Notifications
	struct Notification_t
	{
		enum Type_t
		{
			NOTIFICATION_ITEM,
			NOTIFICATION_BOUNTY,
			NUM_TYPES,
		} Type;
		CUtlString Title;
		CUtlString Description;
		int64_t Starts;
		int64_t Ends;
		enum Seen_t
		{
			SEEN_NEW,
			SEEN_VIEWED,
			SEEN_HOVERED,
			SEEN_CLICKED,
			NUM_SEEN_TYPES,
		} Seen;

		// For NOTIFICATION_ITEM
		int32_t ItemDefID;
		uint64_t ItemID;

		// For NOTIFICATION_BOUNTY
		int FirstBountyID;
		struct BountyMission_t
		{
			char MissionName[MAX_MAP_NAME];
			int Points;
			uint64_t AddonID;
			bool Claimed;
		};
		CUtlVector<BountyMission_t> BountyMissions;
	};
	// Elements from this array can be deleted at any time; do not store pointers.
	CUtlVectorAutoPurge<Notification_t *> m_Notifications;
	int m_nSeenNotifications[Notification_t::NUM_SEEN_TYPES];
	void RebuildNotificationList();
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
#ifdef CLIENT_DLL
	CUtlSymbolTable m_ChatAnnouncementSeen;
#endif
	struct HoIAFMissionBounty_t
	{
		int ID;
		int64_t Starts;
		int64_t Ends;
		char Map[MAX_MAP_NAME];
		int Points;
		uint64_t AddonID;
	};
	CUtlVectorAutoPurge<HoIAFMissionBounty_t *> m_HoIAFMissionBounties;
};

CRD_HoIAF_System *HoIAF();
