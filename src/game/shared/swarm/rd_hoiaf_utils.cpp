#include "cbase.h"
#include "rd_hoiaf_utils.h"
#include "filesystem.h"
#include "steam/steam_api.h"
#include "fmtstr.h"
#include "rd_workshop.h"

#ifdef CLIENT_DLL
#include <vgui/ISurface.h>
#include "hud_basechat.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/basemodframe.h"
#include "asw_medal_store.h"
#include "rd_inventory_shared.h"
#include "rd_vgui_notifications.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef CLIENT_DLL
extern ConVar hud_saytext_time;
extern ConVar rd_notification_filter_hoiaf;
static void ForceRebuildNotificationList( IConVar *var, const char *pOldValue, float flOldValue )
{
	HoIAF()->RebuildNotificationList();
}
ConVar rd_notification_debug_fake( "rd_notification_debug_fake", "0", FCVAR_NONE, "", ForceRebuildNotificationList );
#endif

CRD_HoIAF_System::CRD_HoIAF_System() :
	CAutoGameSystemPerFrame{ IsClientDll() ? "CRD_HoIAF_System (client)" : "CRD_HoIAF_System (server)" },
	m_pIAFIntel{ "II" }
{
}

void CRD_HoIAF_System::PostInit()
{
#ifdef GAME_DLL
	if ( !engine->IsDedicatedServer() )
		return;
#endif

	LoadCachedIAFIntel();
	// always grab new world state just in case we have a corrupt cached version with a ridiculous future expiration timestamp
	RefreshIAFIntel( false, true );
}

// Check data expiration timestamp once per frame.
#ifdef CLIENT_DLL
void CRD_HoIAF_System::Update( float frametime )
#else
void CRD_HoIAF_System::PreClientUpdate()
#endif
{
#ifdef GAME_DLL
	if ( !engine->IsDedicatedServer() )
		return;
#endif

	CheckIAFIntelUpToDate();
}

bool CRD_HoIAF_System::CheckIAFIntelUpToDate()
{
	return RefreshIAFIntel();
}

bool CRD_HoIAF_System::IsRankedServerIP( uint32_t ip ) const
{
	return m_RankedServerIPs.IsValidIndex( m_RankedServerIPs.Find( ip ) );
}

bool CRD_HoIAF_System::GetLastUpdateDate( int &year, int &month, int &day ) const
{
	int date = m_iLatestPatch;
	if ( !date )
	{
		return false;
	}

	day = date % 100;
	Assert( day >= 1 && day <= 31 );
	date /= 100;
	month = date % 100;
	Assert( month >= 1 && month <= 12 );
	date /= 100;
	year = date;
	Assert( year >= 2023 && year < 10000 );

	return true;
}

int CRD_HoIAF_System::CountFeaturedNews() const
{
	return m_FeaturedNews.Count();
}

const char *CRD_HoIAF_System::GetFeaturedNewsCaption( int index ) const
{
	if ( index < 0 || index >= CountFeaturedNews() )
		return NULL;

	return m_FeaturedNews[index]->Caption;
}

const char *CRD_HoIAF_System::GetFeaturedNewsURL( int index ) const
{
	if ( index < 0 || index >= CountFeaturedNews() )
		return NULL;

	return m_FeaturedNews[index]->URL;
}

#ifdef CLIENT_DLL
vgui::HTexture CRD_HoIAF_System::GetFeaturedNewsTexture( int index ) const
{
	if ( index < 0 || index >= CountFeaturedNews() )
		return NULL;

	return m_FeaturedNews[index]->TextureHandle;
}
#endif

int CRD_HoIAF_System::CountEventTimers() const
{
	return m_EventTimers.Count();
}

bool CRD_HoIAF_System::IsEventTimerActive( int index ) const
{
	int64_t iCurrentTime = SteamUtils() ? SteamUtils()->GetServerRealTime() : 0;
	return GetEventStartTime( index ) <= iCurrentTime || GetEventEndTime( index ) >= iCurrentTime;
}

const char *CRD_HoIAF_System::GetEventTimerCaption( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->Caption;
}

const char *CRD_HoIAF_System::GetEventTimerURL( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->URL;
}

int64_t CRD_HoIAF_System::GetEventStartTime( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->Starts;
}

int64_t CRD_HoIAF_System::GetEventEndTime( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->Ends;
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::InsertChatMessages( CBaseHudChat *pChat )
{
	int64_t iCurrentTime = SteamUtils() ? SteamUtils()->GetServerRealTime() : 0;
	FOR_EACH_VEC( m_ChatAnnouncements, i )
	{
		if ( m_ChatAnnouncements[i]->NotBefore > iCurrentTime )
			continue;
		if ( m_ChatAnnouncements[i]->NotAfter < iCurrentTime )
			continue;
		if ( m_ChatAnnouncementSeen.Find( m_ChatAnnouncements[i]->ID ).IsValid() )
			continue;

		m_ChatAnnouncementSeen.AddString( m_ChatAnnouncements[i]->ID );

		CBaseHudChatLine *line = pChat->FindUnusedChatLine();
		if ( !line )
			line = pChat->FindUnusedChatLine();
		Assert( line );
		if ( !line )
			continue;

		line->SetText( "" );
		line->SetExpireTime();

		line->SetVisible( false );
		line->SetNameStart( 0 );
		line->SetNameLength( 0 );

		if ( m_ChatAnnouncements[i]->Zbalermorna.IsEmpty() )
		{
			wchar_t wbuf[2048];
			V_UTF8ToUnicode( m_ChatAnnouncements[i]->Text, wbuf, sizeof( wbuf ) );

			if ( line->m_text )
			{
				delete[] line->m_text;
			}
			line->m_text = CloneWString( wbuf );
			line->m_textRanges.RemoveAll();

			TextRange range;
			range.start = 0;
			range.end = V_wcslen( wbuf );
			range.color = m_ChatAnnouncements[i]->Color;
			line->m_textRanges.AddToTail( range );

			line->Colorize();
		}
		else if ( pChat->GetChatHistory() )
		{
			pChat->GetChatHistory()->InsertString( "\n" );
			pChat->GetChatHistory()->InsertColorChange( m_ChatAnnouncements[i]->Color );
			pChat->GetChatHistory()->InsertFontChange( m_ChatAnnouncements[i]->Font );
			pChat->GetChatHistory()->InsertFade( hud_saytext_time.GetFloat(), CHAT_HISTORY_IDLE_FADE_TIME );
			pChat->GetChatHistory()->InsertZbalermornaString( m_ChatAnnouncements[i]->Zbalermorna );
			pChat->GetChatHistory()->InsertFade( -1, -1 );
		}
	}
}
#endif

const char *CRD_HoIAF_System::BountyAddonName( PublishedFileId_t addonID )
{
	if ( rd_notification_debug_fake.GetBool() )
	{
		if ( addonID == 848331447 )
		{
			return "Nest";
		}
		if ( addonID == 861497042 )
		{
			return "City 17";
		}
		if ( addonID == 936101427 )
		{
			return "Operation: Chaos Theory";
		}
		if ( addonID == 1312255876 )
		{
			return "Warehouse";
		}
		if ( addonID == 2979008182 )
		{
			return "Gluon Vengeance Bonus Mission";
		}

		Assert( addonID == k_PublishedFileIdInvalid );
		return NULL;
	}

	FOR_EACH_VEC( m_HoIAFMissionBounties, i )
	{
		if ( m_HoIAFMissionBounties[i]->AddonID == addonID && !m_HoIAFMissionBounties[i]->AddonName.IsEmpty() )
		{
			return m_HoIAFMissionBounties[i]->AddonName.Get();
		}
	}

	return NULL;
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::MarkBountyAsCompleted( int iBountyID )
{
	FOR_EACH_VEC( m_HoIAFMissionBounties, i )
	{
		if ( m_HoIAFMissionBounties[i]->ID != iBountyID )
		{
			continue;
		}

		Assert( !V_stricmp( m_HoIAFMissionBounties[i]->Map, MapName() ) );
		if ( V_stricmp( m_HoIAFMissionBounties[i]->Map, MapName() ) )
		{
			Warning( "[HoIAF:%c] Server said to mark mission bounty complete for %s, but we are on map %s!\n", IsClientDll() ? 'C' : 'S', m_HoIAFMissionBounties[i]->Map, MapName() );
			return;
		}

		if ( C_ASW_Medal_Store *pMedalStore = GetMedalStore() )
		{
			pMedalStore->OnCompletedBounty( iBountyID );
		}

		return;
	}

	Warning( "[HoIAF:%c] Server said to mark mission bounty %d complete, but we don't know about that bounty!\n", IsClientDll() ? 'C' : 'S', iBountyID );
}

static int __cdecl ItemInstancesInAcquisitionOrder( const ReactiveDropInventory::ItemInstance_t *a, const ReactiveDropInventory::ItemInstance_t *b )
{
	return int( a->Acquired ) - int( b->Acquired );
}

static int __cdecl NewAndLatestNotificationsFirst( HoIAFNotification_t *const *a, HoIAFNotification_t *const *b )
{
	if ( ( *a )->Seen == HoIAFNotification_t::SEEN_NEW && ( *b )->Seen != HoIAFNotification_t::SEEN_NEW )
		return -1;
	if ( ( *a )->Seen != HoIAFNotification_t::SEEN_NEW && ( *b )->Seen == HoIAFNotification_t::SEEN_NEW )
		return 1;

	return int( clamp<int64_t>( ( *b )->Starts - ( *a )->Starts, -1, 1 ) );
}

void CRD_HoIAF_System::RebuildNotificationList()
{
	m_Notifications.PurgeAndDeleteElements();
	for ( int i = 0; i < HoIAFNotification_t::NUM_SEEN_TYPES; i++ )
	{
		m_nSeenNotifications[i] = 0;
	}

	if ( rd_notification_debug_fake.GetBool() )
	{
		m_Notifications.AddToTail( new HoIAFNotification_t );
		m_Notifications[0]->Type = HoIAFNotification_t::NOTIFICATION_ITEM;
		m_Notifications[0]->Title = ReactiveDropInventory::GetItemDef( 7077 )->Name;
		m_Notifications[0]->Description = ReactiveDropInventory::GetItemDef( 7077 )->Description;
		m_Notifications[0]->Starts = 1706770000;
		m_Notifications[0]->Ends = 0;
		m_Notifications[0]->Seen = HoIAFNotification_t::SEEN_HOVERED;
		m_Notifications[0]->ItemDefID = 7077;
		m_Notifications[0]->ItemID = 1;

		m_Notifications.AddToTail( new HoIAFNotification_t );
		m_Notifications[1]->Type = HoIAFNotification_t::NOTIFICATION_ITEM;
		m_Notifications[1]->Title = ReactiveDropInventory::GetItemDef( 7078 )->Name;
		m_Notifications[1]->Description = ReactiveDropInventory::GetItemDef( 7078 )->Description;
		m_Notifications[1]->Starts = 1706802222;
		m_Notifications[1]->Ends = 0;
		m_Notifications[1]->Seen = HoIAFNotification_t::SEEN_VIEWED;
		m_Notifications[1]->ItemDefID = 7078;
		m_Notifications[1]->ItemID = 2;

		m_Notifications.AddToTail( new HoIAFNotification_t );
		m_Notifications[2]->Type = HoIAFNotification_t::NOTIFICATION_BOUNTY;
		m_Notifications[2]->Title = "#rd_hoiaf_bounty_title";
		m_Notifications[2]->Description = "#rd_hoiaf_bounty_fully_claimed";
		m_Notifications[2]->Starts = 1706745600;
		m_Notifications[2]->Ends = 1706832000;
		m_Notifications[2]->Seen = HoIAFNotification_t::SEEN_CLICKED;
		m_Notifications[2]->FirstBountyID = 1;
		m_Notifications[2]->BountyMissions.SetCount( 5 );
		V_strcpy( m_Notifications[2]->BountyMissions[0].MissionName, "rd-acc_complex" );
		m_Notifications[2]->BountyMissions[0].Points = 675;
		m_Notifications[2]->BountyMissions[0].AddonID = k_PublishedFileIdInvalid;
		m_Notifications[2]->BountyMissions[0].Claimed = true;
		V_strcpy( m_Notifications[2]->BountyMissions[1].MissionName, "asw_warehouse_v1" );
		m_Notifications[2]->BountyMissions[1].Points = 875;
		m_Notifications[2]->BountyMissions[1].AddonID = 1312255876;
		m_Notifications[2]->BountyMissions[1].Claimed = true;
		V_strcpy( m_Notifications[2]->BountyMissions[2].MissionName, "researchlab2" );
		m_Notifications[2]->BountyMissions[2].Points = 1250;
		m_Notifications[2]->BountyMissions[2].AddonID = 936101427;
		m_Notifications[2]->BountyMissions[2].Claimed = true;
		V_strcpy( m_Notifications[2]->BountyMissions[3].MissionName, "rd-gluoncave_short" );
		m_Notifications[2]->BountyMissions[3].Points = 625;
		m_Notifications[2]->BountyMissions[3].AddonID = 2979008182;
		m_Notifications[2]->BountyMissions[3].Claimed = true;
		V_strcpy( m_Notifications[2]->BountyMissions[4].MissionName, "nest01cave" );
		m_Notifications[2]->BountyMissions[4].Points = 750;
		m_Notifications[2]->BountyMissions[4].AddonID = 848331447;
		m_Notifications[2]->BountyMissions[4].Claimed = true;

		m_Notifications.AddToTail( new HoIAFNotification_t );
		m_Notifications[3]->Type = HoIAFNotification_t::NOTIFICATION_BOUNTY;
		m_Notifications[3]->Title = "#rd_hoiaf_bounty_title";
		m_Notifications[3]->Description = "#rd_hoiaf_bounty_desc";
		m_Notifications[3]->Starts = 1706774400;
		m_Notifications[3]->Ends = 1706860800;
		m_Notifications[3]->Seen = HoIAFNotification_t::SEEN_HOVERED;
		m_Notifications[3]->FirstBountyID = 6;
		m_Notifications[3]->BountyMissions.SetCount( 5 );
		V_strcpy( m_Notifications[3]->BountyMissions[0].MissionName, "as_city17_01" );
		m_Notifications[3]->BountyMissions[0].Points = 337;
		m_Notifications[3]->BountyMissions[0].AddonID = 861497042;
		m_Notifications[3]->BountyMissions[0].Claimed = true;
		V_strcpy( m_Notifications[3]->BountyMissions[1].MissionName, "as_city17_02" );
		m_Notifications[3]->BountyMissions[1].Points = 315;
		m_Notifications[3]->BountyMissions[1].AddonID = 861497042;
		m_Notifications[3]->BountyMissions[1].Claimed = true;
		V_strcpy( m_Notifications[3]->BountyMissions[2].MissionName, "as_city17_03" );
		m_Notifications[3]->BountyMissions[2].Points = 300;
		m_Notifications[3]->BountyMissions[2].AddonID = 861497042;
		m_Notifications[3]->BountyMissions[2].Claimed = false;
		V_strcpy( m_Notifications[3]->BountyMissions[3].MissionName, "as_city17_04" );
		m_Notifications[3]->BountyMissions[3].Points = 300;
		m_Notifications[3]->BountyMissions[3].AddonID = 861497042;
		m_Notifications[3]->BountyMissions[3].Claimed = false;
		V_strcpy( m_Notifications[3]->BountyMissions[4].MissionName, "as_city17_05" );
		m_Notifications[3]->BountyMissions[4].Points = 487;
		m_Notifications[3]->BountyMissions[4].AddonID = 861497042;
		m_Notifications[3]->BountyMissions[4].Claimed = false;

		m_Notifications.AddToTail( new HoIAFNotification_t );
		m_Notifications[4]->Type = HoIAFNotification_t::NOTIFICATION_BOUNTY;
		m_Notifications[4]->Title = "#rd_hoiaf_bounty_title";
		m_Notifications[4]->Description = "#rd_hoiaf_bounty_desc";
		m_Notifications[4]->Starts = 1706803200;
		m_Notifications[4]->Ends = 1706889600;
		m_Notifications[4]->Seen = HoIAFNotification_t::SEEN_VIEWED;
		m_Notifications[4]->FirstBountyID = 11;
		m_Notifications[4]->BountyMissions.SetCount( 3 );
		V_strcpy( m_Notifications[4]->BountyMissions[0].MissionName, "rd-bio1operationx5" );
		m_Notifications[4]->BountyMissions[0].Points = 375;
		m_Notifications[4]->BountyMissions[0].AddonID = k_PublishedFileIdInvalid;
		m_Notifications[4]->BountyMissions[0].Claimed = false;
		V_strcpy( m_Notifications[4]->BountyMissions[1].MissionName, "rd-bio2invisiblethreat" );
		m_Notifications[4]->BountyMissions[1].Points = 262;
		m_Notifications[4]->BountyMissions[1].AddonID = k_PublishedFileIdInvalid;
		m_Notifications[4]->BountyMissions[1].Claimed = false;
		V_strcpy( m_Notifications[4]->BountyMissions[2].MissionName, "rd-bio3biogenlabs" );
		m_Notifications[4]->BountyMissions[2].Points = 412;
		m_Notifications[4]->BountyMissions[2].AddonID = k_PublishedFileIdInvalid;
		m_Notifications[4]->BountyMissions[2].Claimed = false;

		m_nSeenNotifications[HoIAFNotification_t::SEEN_VIEWED] = 2;
		m_nSeenNotifications[HoIAFNotification_t::SEEN_HOVERED] = 2;
		m_nSeenNotifications[HoIAFNotification_t::SEEN_CLICKED] = 1;
	}
	else
	{
		CUtlVector<ReactiveDropInventory::ItemInstance_t> notificationItems;
		ReactiveDropInventory::GetItemsForSlot( notificationItems, "notification" );

		notificationItems.Sort( &ItemInstancesInAcquisitionOrder );
		CUtlVector<SteamItemDef_t> seenUniqueNotificationItem;
		FOR_EACH_VEC_BACK( notificationItems, i )
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( notificationItems[i].ItemDefID );
			Assert( pDef );
			if ( !pDef )
			{
				// no idea what this item is (even though we somehow categorized it!); best to not touch it
				continue;
			}

			if ( pDef->HasNotificationTag( "auto_delete_except_newest" ) )
			{
				if ( seenUniqueNotificationItem.IsValidIndex( seenUniqueNotificationItem.Find( notificationItems[i].ItemDefID ) ) )
				{
					// this is an older copy of a notification item that we only want to have one of; discard the older copy
					ReactiveDropInventory::DeleteNotificationItem( notificationItems[i].ItemID );
					continue;
				}

				seenUniqueNotificationItem.AddToTail( notificationItems[i].ItemDefID );
			}

			UtlSymId_t notificationType = pDef->Tags.Find( "notification" );
			if ( notificationType != pDef->Tags.InvalidIndex() )
			{
				bool bFiltered = false;
				FOR_EACH_VEC( pDef->Tags[notificationType], j )
				{
					ConVarRef filter{ CFmtStr{ "rd_notification_filter_%s", pDef->Tags[notificationType][j]} };
					Assert( filter.IsValid() );
					if ( filter.IsValid() && !filter.GetBool() )
					{
						bFiltered = true;
						break;
					}

					if ( !V_strcmp( pDef->Tags[notificationType][j], "crafting" ) )
					{
						// always hide crafting notifications for now
						bFiltered = true;
						break;
					}
				}

				if ( bFiltered )
				{
					continue;
				}
			}

			HoIAFNotification_t *pNotification = new HoIAFNotification_t;

			pNotification->Type = HoIAFNotification_t::NOTIFICATION_ITEM;
			pNotification->Title = pDef->Name;
			pNotification->Description = pDef->Description;
			pNotification->Starts = notificationItems[i].Acquired;
			pNotification->Ends = 0;
			pNotification->Seen = HoIAFNotification_t::SEEN_NEW;

			UtlSymId_t seen = notificationItems[i].DynamicProps.Find( "notification_seen" );
			if ( seen != notificationItems[i].DynamicProps.InvalidIndex() )
			{
				pNotification->Seen = ( HoIAFNotification_t::Seen_t )clamp<int>( V_atoi( notificationItems[i].DynamicProps[seen] ), ( int )HoIAFNotification_t::SEEN_NEW, HoIAFNotification_t::NUM_SEEN_TYPES - 1 );
			}

			m_nSeenNotifications[pNotification->Seen]++;

			pNotification->ItemDefID = notificationItems[i].ItemDefID;
			pNotification->ItemID = notificationItems[i].ItemID;

			m_Notifications.AddToTail( pNotification );
		}

		C_ASW_Medal_Store *pMedalStore = GetMedalStore();
		Assert( pMedalStore );
		if ( rd_notification_filter_hoiaf.GetBool() && pMedalStore )
		{
			HoIAFNotification_t *pBountyNotification = NULL;
			int64_t iLastStartTime = 0;
			bool bAnyUnclaimed = false;

			// bounties are grouped by start time. the server sends them pre-sorted.
			FOR_EACH_VEC( m_HoIAFMissionBounties, i )
			{
				if ( m_HoIAFMissionBounties[i]->Starts != iLastStartTime )
				{
					if ( pBountyNotification )
					{
						if ( bAnyUnclaimed )
						{
							pBountyNotification->Description = "#rd_hoiaf_bounty_desc";
						}
						else
						{
							// if we have claimed every mission bonus, mark the notification as all-but-dismissed.
							pBountyNotification->Seen = HoIAFNotification_t::SEEN_CLICKED;
						}

						m_nSeenNotifications[pBountyNotification->Seen]++;
					}

					HoIAFNotification_t::Seen_t iSeen = ( HoIAFNotification_t::Seen_t )pMedalStore->GetBountyNotificationStatus( m_HoIAFMissionBounties[i]->ID );
					if ( iSeen == HoIAFNotification_t::NUM_SEEN_TYPES )
					{
						// notification was dismissed
						pBountyNotification = NULL;
					}
					else
					{
						pBountyNotification = new HoIAFNotification_t;
						pBountyNotification->Type = HoIAFNotification_t::NOTIFICATION_BOUNTY;
						pBountyNotification->Title = "#rd_hoiaf_bounty_title";
						pBountyNotification->Description = "#rd_hoiaf_bounty_fully_claimed";
						pBountyNotification->Starts = m_HoIAFMissionBounties[i]->Starts;
						pBountyNotification->Ends = m_HoIAFMissionBounties[i]->Ends;
						pBountyNotification->FirstBountyID = m_HoIAFMissionBounties[i]->ID;

						m_Notifications.AddToTail( pBountyNotification );
					}

					bAnyUnclaimed = false;
				}

				Assert( !pBountyNotification || pBountyNotification->Ends == m_HoIAFMissionBounties[i]->Ends );

				if ( pBountyNotification )
				{
					int j = pBountyNotification->BountyMissions.AddToTail();
					V_strncpy( pBountyNotification->BountyMissions[j].MissionName, m_HoIAFMissionBounties[i]->Map, sizeof( pBountyNotification->BountyMissions[j].MissionName ) );
					pBountyNotification->BountyMissions[j].Points = m_HoIAFMissionBounties[i]->Points;
					pBountyNotification->BountyMissions[j].AddonID = m_HoIAFMissionBounties[i]->AddonID;
					pBountyNotification->BountyMissions[j].Claimed = pMedalStore->HasCompletedBounty( m_HoIAFMissionBounties[i]->ID );
					bAnyUnclaimed = bAnyUnclaimed || !pBountyNotification->BountyMissions[j].Claimed;
				}
			}

			if ( pBountyNotification )
			{
				if ( bAnyUnclaimed )
				{
					pBountyNotification->Description = "#rd_hoiaf_bounty_desc";
				}
				else
				{
					// if we have claimed every mission bonus, mark the notification as all-but-dismissed.
					pBountyNotification->Seen = HoIAFNotification_t::SEEN_CLICKED;
				}

				m_nSeenNotifications[pBountyNotification->Seen]++;
			}
		}
	}

	m_Notifications.Sort( &NewAndLatestNotificationsFirst );

	// we set a hard limit of 100 notifications in the UI list to avoid problems.
	if ( m_Notifications.Count() > 100 )
	{
		for ( int i = 100; i < m_Notifications.Count(); i++ )
		{
			delete m_Notifications[i];
		}
		m_Notifications.SetCountNonDestructively( 100 );
	}

	FOR_EACH_VEC( g_NotificationsButtons, i )
	{
		g_NotificationsButtons[i]->UpdateNotifications();
	}
	FOR_EACH_VEC( g_NotificationsLists, i )
	{
		g_NotificationsLists[i]->UpdateNotifications();
	}
}
#endif

void CRD_HoIAF_System::ParseIAFIntel()
{
	m_iExpireAt = int64_t( m_pIAFIntel->GetUint64( "expires" ) );
	m_RankedServerIPs.Purge();
	m_iLatestPatch = m_pIAFIntel->GetInt( "latestPatch" );
	m_FeaturedNews.PurgeAndDeleteElements();
	m_EventTimers.PurgeAndDeleteElements();
	m_ChatAnnouncements.PurgeAndDeleteElements();
	m_HoIAFMissionBounties.PurgeAndDeleteElements();

	FOR_EACH_SUBKEY( m_pIAFIntel, pCommand )
	{
		const char *szName = pCommand->GetName();
		if ( !V_stricmp( szName, "expires" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_UINT64 );
			// already handled above
		}
		else if ( !V_stricmp( szName, "rankedIP" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_INT );
			m_RankedServerIPs.AddToTail( uint32_t( pCommand->GetInt() ) );
		}
		else if ( !V_stricmp( szName, "latestPatch" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_INT );
			// already handled above
		}
		else if ( !V_stricmp( szName, "featuredNews" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );
			Assert( m_FeaturedNews.Count() < 5 );

			FeaturedNews_t *pNews = new FeaturedNews_t;

			LoadTranslatedString( pNews->Caption, pCommand, "caption_%s" );
			pNews->URL = pCommand->GetString( "url" );

#ifdef CLIENT_DLL
			const char *szTextureURL = pCommand->GetString( "texture_url" );
			pNews->TextureCRC = pCommand->GetInt( "texture_crc" );
			pNews->TextureIndex = pCommand->GetInt( "texture_index" );
			Assert( pNews->TextureIndex >= 1 && pNews->TextureIndex <= 5 );

			char szTextureName[MAX_PATH];
			V_snprintf( szTextureName, sizeof( szTextureName ), "materials/vgui/swarm/news_showcase_%d.vtf", pNews->TextureIndex );

			CUtlBuffer buf;
			if ( g_pFullFileSystem->ReadFile( szTextureName, "GAME", buf ) && CRC32_ProcessSingleBuffer( buf.Base(), buf.TellPut() ) == pNews->TextureCRC )
			{
				pNews->TextureHandle = vgui::surface()->CreateNewTextureID();
				vgui::surface()->DrawSetTextureFile( pNews->TextureHandle, &szTextureName[strlen( "materials/" )], 1, false );
			}
			else if ( ISteamHTTP *pHTTP = SteamHTTP() )
			{
				pNews->m_hTextureRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, szTextureURL );
				pHTTP->SetHTTPRequestUserAgentInfo( pNews->m_hTextureRequest, "Reactive Drop News Showcase Icon Fetch" );

				SteamAPICall_t hCall = k_uAPICallInvalid;
				if ( pHTTP->SendHTTPRequest( pNews->m_hTextureRequest, &hCall ) )
				{
					pNews->m_TextureRequestFinished.Set( hCall, pNews, &CRD_HoIAF_System::FeaturedNews_t::OnHTTPRequestCompleted );
				}
			}
#endif

			m_FeaturedNews.AddToTail( pNews );
		}
		else if ( !V_stricmp( szName, "eventTimer" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );
			Assert( m_EventTimers.Count() < 3 );

			EventTimer_t *pTimer = new EventTimer_t;

			LoadTranslatedString( pTimer->Caption, pCommand, "caption_%s" );
			pTimer->URL = pCommand->GetString( "url" );
			pTimer->Starts = pCommand->GetUint64( "starts" );
			pTimer->Ends = pCommand->GetUint64( "ends" );

			m_EventTimers.AddToTail( pTimer );
		}
		else if ( !V_stricmp( szName, "chatAnnouncement" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );

			ChatAnnouncement_t *pAnnouncement = new ChatAnnouncement_t;

			pAnnouncement->ID = pCommand->GetString( "id" );
			LoadTranslatedString( pAnnouncement->Text, pCommand, "text_%s" );
			pAnnouncement->Zbalermorna = pCommand->GetString( "text_zbalermorna" );
			pAnnouncement->NotBefore = pCommand->GetUint64( "not_before" );
			pAnnouncement->NotAfter = pCommand->GetUint64( "not_after" );
			pAnnouncement->Color = pCommand->GetColor( "color", Color{ 255, 0, 0, 255 } );
#ifdef CLIENT_DLL
			vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFileEx( NULL, "resource/ChatScheme.res", "ChatScheme" );
			vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( hScheme );
			pAnnouncement->Font = pScheme->GetFont( pCommand->GetString( "font", "ChatFont" ), false );
#endif

			m_ChatAnnouncements.AddToTail( pAnnouncement );
		}
		else if ( !V_stricmp( szName, "hoiafMissionBounty" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );

			HoIAFMissionBounty_t *pBounty = new HoIAFMissionBounty_t;

			pBounty->ID = pCommand->GetInt( "id" );
			pBounty->Starts = pCommand->GetUint64( "starts" );
			pBounty->Ends = pCommand->GetUint64( "ends" );
			V_strncpy( pBounty->Map, pCommand->GetString( "map" ), sizeof( pBounty->Map ) );
			pBounty->Points = pCommand->GetInt( "points" );
			pBounty->AddonID = pCommand->GetUint64( "addon", k_PublishedFileIdInvalid );
			pBounty->AddonName = pCommand->GetString( "addon_name" );

			m_HoIAFMissionBounties.AddToTail( pBounty );
		}
		else
		{
			AssertMsg1( false, "Unhandled IAF Intel command %s", szName );
		}
	}

#ifdef CLIENT_DLL
	RebuildNotificationList();

	BaseModUI::CBaseModFrame *pMainMenu = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_MAINMENU );
	if ( pMainMenu && pMainMenu->IsVisible() )
	{
		pMainMenu->Activate();
	}
#endif
}

void CRD_HoIAF_System::LoadCachedIAFIntel()
{
	Assert( g_pFullFileSystem );
	if ( !g_pFullFileSystem )
		return;

	CUtlBuffer buf;
	if ( g_pFullFileSystem->ReadFile( "cfg/iicache.dat", "MOD", buf ) )
	{
		m_pIAFIntel->Clear();
		if ( !m_pIAFIntel->ReadAsBinary( buf ) )
		{
			DevMsg( "[HoIAF:%c] Failed to load cached IAF Intel. Continuing regardless.\n", IsClientDll() ? 'C' : 'S' );
		}
	}
	else
	{
		DevMsg( "[HoIAF:%c] No cached IAF Intel found. This is a fresh install or somebody deleted it.\n", IsClientDll() ? 'C' : 'S' );
	}

	ParseIAFIntel();
}

bool CRD_HoIAF_System::RefreshIAFIntel( bool bOnlyIfExpired, bool bForceNewRequest )
{
#ifdef CLIENT_DLL
	ISteamUtils *pUtils = SteamUtils();
	ISteamHTTP *pHTTP = SteamHTTP();
#else
	ISteamUtils *pUtils = engine->IsDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	ISteamHTTP *pHTTP = engine->IsDedicatedServer() ? SteamGameServerHTTP() : SteamHTTP();
#endif
	Assert( pUtils );
	Assert( pHTTP );
	if ( !pUtils || !pHTTP )
	{
		return false;
	}

	int64_t iNow = pUtils->GetServerRealTime();
	if ( bOnlyIfExpired && iNow < m_iExpireAt )
	{
		return true;
	}

	// Call PrioritizeHTTPRequest again to check if the request still exists and didn't magically disappear.
	if ( !bForceNewRequest && m_hIAFIntelRefreshRequest != INVALID_HTTPREQUEST_HANDLE && pHTTP->PrioritizeHTTPRequest( m_hIAFIntelRefreshRequest ) )
	{
		return false;
	}

	if ( m_iBackoffUntil > iNow )
	{
		// We need updated data, but we just failed to request it. Wait until a little later to retry again.
		return false;
	}

	DevMsg( "[HoIAF:%c] Requesting updated IAF Intel; previous intel expired %lld seconds ago.%s\n", IsClientDll() ? 'C' : 'S', iNow - m_iExpireAt, bOnlyIfExpired ? "" : " (forced)" );

	if ( m_hIAFIntelRefreshRequest != INVALID_HTTPREQUEST_HANDLE )
	{
		pHTTP->ReleaseHTTPRequest( m_hIAFIntelRefreshRequest );
	}

	m_hIAFIntelRefreshRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, "https://stats.reactivedrop.com/game_dynamic_state.bin" );
	if ( m_hIAFIntelRefreshRequest == INVALID_HTTPREQUEST_HANDLE )
	{
		Warning( "[HoIAF:%c] Game global state update request: CreateHTTPRequest failed!\n", IsClientDll() ? 'C' : 'S' );
		return false;
	}

	pHTTP->SetHTTPRequestUserAgentInfo( m_hIAFIntelRefreshRequest, IsClientDll() ? "Reactive Drop Client" : "Reactive Drop Server" );
	SteamAPICall_t hCall = k_uAPICallInvalid;
	if ( !pHTTP->SendHTTPRequest( m_hIAFIntelRefreshRequest, &hCall ) )
	{
		Warning( "[HoIAF:%c] Game global state update request: SendHTTPRequest failed!\n", IsClientDll() ? 'C' : 'S' );
		return false;
	}

	// We want this before any of the optional things we might be downloading.
	pHTTP->PrioritizeHTTPRequest( m_hIAFIntelRefreshRequest );

	m_IAFIntelRefresh.Set( hCall, this, &CRD_HoIAF_System::OnHTTPRequestCompleted );

	return false;
}

void CRD_HoIAF_System::OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		Warning( "[HoIAF:%c] Game global state update request: Lost connection to Steam!\n", IsClientDll() ? 'C' : 'S' );
		OnRequestFailed();
		m_hIAFIntelRefreshRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	Assert( pParam && pParam->m_hRequest == m_hIAFIntelRefreshRequest );

#ifdef CLIENT_DLL
	ISteamUtils *pUtils = SteamUtils();
	ISteamHTTP *pHTTP = SteamHTTP();
#else
	ISteamUtils *pUtils = engine->IsDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	ISteamHTTP *pHTTP = engine->IsDedicatedServer() ? SteamGameServerHTTP() : SteamHTTP();
#endif
	Assert( pUtils );
	Assert( pHTTP );
	if ( !pHTTP )
	{
		Warning( "[HoIAF:%c] Game global state update request: Somehow lost ISteamHTTP interface!\n", IsClientDll() ? 'C' : 'S' );
		OnRequestFailed();
		// can't release request or get body, so just give up
		m_hIAFIntelRefreshRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	if ( !pParam->m_bRequestSuccessful )
	{
		Warning( "[HoIAF:%c] Game global state update request: Network failure!\n", IsClientDll() ? 'C' : 'S' );
		OnRequestFailed();
		goto cleanup;
	}

	{
		CUtlBuffer body{ 0, int( pParam->m_unBodySize ) };
		body.SeekPut( CUtlBuffer::SEEK_HEAD, pParam->m_unBodySize );
		if ( !pHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )body.Base(), pParam->m_unBodySize ) )
		{
			Warning( "[HoIAF:%c] Game global state update request: GetHTTPResponseBodyData failed!\n", IsClientDll() ? 'C' : 'S' );
			OnRequestFailed();
			goto cleanup;
		}

		if ( pParam->m_eStatusCode != k_EHTTPStatusCode200OK )
		{
			Warning( "[HoIAF:%c] Game global state update request: Received HTTP status code %d (should be 200!)\n", IsClientDll() ? 'C' : 'S', pParam->m_eStatusCode );
			OnRequestFailed();
			goto cleanup;
		}

		Assert( g_pFullFileSystem );
		if ( g_pFullFileSystem )
		{
			g_pFullFileSystem->WriteFile( "cfg/iicache.dat", "MOD", body );
		}

		KeyValues *pNewData = new KeyValues( "II" );
		if ( !pNewData->ReadAsBinary( body ) )
		{
			Warning( "[HoIAF:%c] Game global state update request: failed to parse global state data\n", IsClientDll() ? 'C' : 'S' );
			OnRequestFailed();
			pNewData->deleteThis();
			goto cleanup;
		}

		m_pIAFIntel->deleteThis();
		m_pIAFIntel.Assign( pNewData );

		ParseIAFIntel();

		int64_t iNow = pUtils ? pUtils->GetServerRealTime() : 0;
		Assert( m_iExpireAt > iNow );
		if ( m_iExpireAt > iNow )
		{
			OnNewIntelReceived();
		}
		else
		{
			OnRequestFailed();
		}

		DevMsg( "[HoIAF:%c] Received updated IAF Intel; expires in %lld seconds.\n", IsClientDll() ? 'C' : 'S', m_iExpireAt - iNow );
	}

cleanup:
	pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
	m_hIAFIntelRefreshRequest = INVALID_HTTPREQUEST_HANDLE;
}

void CRD_HoIAF_System::OnRequestFailed()
{
#ifdef CLIENT_DLL
	ISteamUtils *pUtils = SteamUtils();
#else
	ISteamUtils *pUtils = engine->IsDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
#endif
	Assert( pUtils );

	m_iExponentialBackoff++;
	if ( m_iExponentialBackoff > 12 )
		m_iExponentialBackoff = 12; // cap at just over an hour

	// if we can't get the current time, give up until the game is restarted
	m_iBackoffUntil = pUtils ? ( pUtils->GetServerRealTime() + ( 1 << m_iExponentialBackoff ) ) : ~0u;
}

void CRD_HoIAF_System::OnNewIntelReceived()
{
	m_iBackoffUntil = 0;
	m_iExponentialBackoff = 0;

#ifdef CLIENT_DLL
	C_ASW_Medal_Store *pMedalStore = GetMedalStore();
	if ( pMedalStore )
	{
		CUtlVector<int> activeBounties;
		activeBounties.SetCount( m_HoIAFMissionBounties.Count() );
		FOR_EACH_VEC( m_HoIAFMissionBounties, i )
		{
			activeBounties[i] = m_HoIAFMissionBounties[i]->ID;
		}

		pMedalStore->RemoveBountiesExcept( activeBounties );
	}
#endif
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::LoadTranslatedString( CUtlString &str, KeyValues *pKV, const char *szTemplate )
{
	char szLocalized[256], szEnglish[256];
	V_snprintf( szLocalized, sizeof( szLocalized ), szTemplate, SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "" );
	V_snprintf( szEnglish, sizeof( szEnglish ), szTemplate, "english" );
	str = pKV->GetString( szLocalized, pKV->GetString( szEnglish ) );
}
#endif

CRD_HoIAF_System::FeaturedNews_t::~FeaturedNews_t()
{
#ifdef CLIENT_DLL
	if ( TextureHandle && vgui::surface() )
	{
		vgui::surface()->DestroyTextureID( TextureHandle );
		TextureHandle = NULL;
	}

	if ( m_hTextureRequest != INVALID_HTTPREQUEST_HANDLE && SteamHTTP() )
	{
		SteamHTTP()->ReleaseHTTPRequest( m_hTextureRequest );
		m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
	}
#endif
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::FeaturedNews_t::OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (IO failure)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
		m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	Assert( m_hTextureRequest == pParam->m_hRequest );

	ISteamHTTP *pHTTP = SteamHTTP();
	Assert( pHTTP );
	if ( !pHTTP )
	{
		Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (lost ISteamHTTP interface!)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
		// can't release request or get body, so just give up
		m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	if ( !pParam->m_bRequestSuccessful )
	{
		Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (network failure)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
		goto cleanup;
	}

	{
		CUtlBuffer body{ 0, int( pParam->m_unBodySize ) };
		body.SeekPut( CUtlBuffer::SEEK_HEAD, pParam->m_unBodySize );
		if ( !pHTTP->GetHTTPResponseBodyData( m_hTextureRequest, ( uint8 * )body.Base(), pParam->m_unBodySize ) )
		{
			Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (GetHTTPResponseBodyData failed)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
			goto cleanup;
		}

		Assert( CRC32_ProcessSingleBuffer( body.Base(), body.TellPut() ) == TextureCRC );

		char szTextureName[MAX_PATH];
		V_snprintf( szTextureName, sizeof( szTextureName ), "materials/vgui/swarm/news_showcase_%d.vtf", TextureIndex );

		g_pFullFileSystem->CreateDirHierarchy( "materials/vgui/swarm", "MOD" );
		g_pFullFileSystem->WriteFile( szTextureName, "MOD", body );
		TextureHandle = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( TextureHandle, &szTextureName[strlen( "materials/" )], 1, false );

		BaseModUI::CBaseModFrame *pMainMenu = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_MAINMENU );
		if ( pMainMenu && pMainMenu->IsVisible() )
		{
			pMainMenu->Activate();
		}
	}

cleanup:
	pHTTP->ReleaseHTTPRequest( m_hTextureRequest );
	m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
}
#endif

static CRD_HoIAF_System s_HoIAFSystem;

CRD_HoIAF_System *HoIAF()
{
	return &s_HoIAFSystem;
}

#ifdef CLIENT_DLL
CON_COMMAND_F( rd_hoiaf_mark_bounty_completed, "mark a bounty as completed", FCVAR_HIDDEN | FCVAR_SERVER_CAN_EXECUTE | FCVAR_DONTRECORD )
{
	if ( args.ArgC() != 2 )
		return;

	int iBountyID = V_atoi( args[1] );
	if ( !iBountyID )
		return;

	HoIAF()->MarkBountyAsCompleted( iBountyID );
}
#endif
