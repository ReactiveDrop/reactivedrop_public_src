#include "cbase.h"
#include "rd_vgui_notifications.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "nb_button.h"
#include "MultiFontRichText.h"
#include <vgui_controls/ImagePanel.h>
#include "asw_util_shared.h"
#include "rd_inventory_shared.h"
#include "rd_missions_shared.h"
#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "asw_medal_store.h"
#include "rd_vgui_settings.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void ForceRebuildNotificationList( IConVar *var, const char *pOldValue, float flOldValue )
{
	HoIAF()->RebuildNotificationList();
}
ConVar rd_notification_filter_crafting( "rd_notification_filter_crafting", "1", FCVAR_ARCHIVE, "Should we show crafting-related notifications?", ForceRebuildNotificationList );
ConVar rd_notification_filter_hoiaf( "rd_notification_filter_hoiaf", "1", FCVAR_ARCHIVE, "Should we show HoIAF-related notifications?", ForceRebuildNotificationList );
ConVar rd_notification_filter_reports( "rd_notification_filter_reports", "1", FCVAR_ARCHIVE, "Should we show non-critical report-related notifications?", ForceRebuildNotificationList );
extern ConVar rd_notification_debug_fake;

DECLARE_BUILD_FACTORY( CRD_VGUI_Notifications_Button );
DECLARE_BUILD_FACTORY( CRD_VGUI_Notifications_List );
DECLARE_BUILD_FACTORY( CRD_VGUI_Notifications_Filters );

CUtlVector<CRD_VGUI_Notifications_Button *> g_NotificationsButtons;
CUtlVector<CRD_VGUI_Notifications_List *> g_NotificationsLists;

static vgui::Panel *GetParentFrame( vgui::Panel *pPanel )
{
	vgui::Panel *pFrame = pPanel;
	while ( !dynamic_cast< vgui::Frame * >( pFrame ) && pFrame )
	{
		pFrame = pFrame->GetParent();
	}

	return pFrame;
}

CRD_VGUI_Notifications_Button::CRD_VGUI_Notifications_Button( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName, L"", this, "NotificationsButtonClicked" }
{
	SetConsoleStylePanel( true );
	SetProportional( true );

	m_pLblCounter = new vgui::Label( this, "LblCounter", L"" );
	m_hListPopOut = NULL;

	g_NotificationsButtons.AddToTail( this );
}

CRD_VGUI_Notifications_Button::~CRD_VGUI_Notifications_Button()
{
	g_NotificationsButtons.FindAndFastRemove( this );

	if ( m_hListPopOut )
	{
		m_hListPopOut->MarkForDeletion();
		m_hListPopOut = NULL;
	}
}

void CRD_VGUI_Notifications_Button::UpdateNotifications()
{
	if ( int iSeen = HoIAF()->m_nSeenNotifications[HoIAFNotification_t::SEEN_NEW] )
	{
		m_pLblCounter->SetVisible( true );
		m_pLblCounter->SetText( UTIL_RD_CommaNumber( iSeen ) );
		m_pLblCounter->SetBgColor( m_BoldCounterColor );
	}
	else if ( ( iSeen = HoIAF()->m_nSeenNotifications[HoIAFNotification_t::SEEN_VIEWED] ) != 0 )
	{
		m_pLblCounter->SetVisible( true );
		m_pLblCounter->SetText( UTIL_RD_CommaNumber( iSeen ) );
		m_pLblCounter->SetBgColor( m_MutedCounterColor );
	}
	else
	{
		m_pLblCounter->SetVisible( false );
	}

	InvalidateLayout();
}

void CRD_VGUI_Notifications_Button::OnCommand( const char *command )
{
	if ( FStrEq( command, "NotificationsButtonClicked" ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_CLICK );

		if ( m_hListPopOut && m_hListPopOut->IsVisible() )
		{
			ReactiveDropInventory::CommitNotificationSeen();

			m_hListPopOut->SetVisible( false );

			if ( m_hListPopOut->m_hFiltersPopOut )
			{
				m_hListPopOut->m_hFiltersPopOut->SetVisible( false );
			}
		}
		else
		{
			if ( !m_hListPopOut )
			{
				m_hListPopOut = new CRD_VGUI_Notifications_List( this, "List" );
			}

			m_hListPopOut->SetVisible( true );
			m_hListPopOut->MoveToFront();

			for ( int i = 0; i < m_hListPopOut->m_pList->GetPanelItemCount(); i++ )
			{
				CRD_VGUI_Notifications_List_Item *pItem = assert_cast< CRD_VGUI_Notifications_List_Item * >( m_hListPopOut->m_pList->GetPanelItem( i ) );
				pItem->SetSeenAtLeast( HoIAFNotification_t::SEEN_VIEWED );
			}
		}
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Notifications_Button::OnThink()
{
	BaseClass::OnThink();

	if ( m_hListPopOut && m_hListPopOut->IsVisible() )
	{
		bool bAnyFocus = HasFocus() || m_hListPopOut->HasFocus() || m_hListPopOut->m_pList->HasFocus() || m_hListPopOut->m_pFiltersButton->HasFocus();

		if ( !bAnyFocus && m_hListPopOut->m_pList->GetPanelItemCount() )
		{
			for ( int i = 0; i < m_hListPopOut->m_pList->GetPanelItemCount(); i++ )
			{
				if ( m_hListPopOut->m_pList->GetPanelItem( i )->HasFocus() )
				{
					bAnyFocus = true;
					break;
				}
			}
		}

		if ( !bAnyFocus && m_hListPopOut->m_hFiltersPopOut && m_hListPopOut->m_hFiltersPopOut->IsVisible() )
		{
			bAnyFocus = m_hListPopOut->m_hFiltersPopOut->HasFocus();

			for ( int i = 0; i < m_hListPopOut->m_hFiltersPopOut->GetChildCount(); i++ )
			{
				CRD_VGUI_Option *pOption = dynamic_cast< CRD_VGUI_Option * >( m_hListPopOut->m_hFiltersPopOut->GetChild( i ) );
				if ( !pOption )
				{
					continue;
				}

				if ( pOption->HasFocus() )
				{
					bAnyFocus = true;
					break;
				}
			}
		}

		if ( !bAnyFocus )
		{
			// If focus has shifted outside of the notifications menu, close the notifications menu.
			m_hListPopOut->SetVisible( false );

			if ( m_hListPopOut->m_hFiltersPopOut )
			{
				m_hListPopOut->m_hFiltersPopOut->SetVisible( false );
			}
		}
	}
}

void CRD_VGUI_Notifications_Button::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void CRD_VGUI_Notifications_Button::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( false );
	m_pLblCounter->SetMouseInputEnabled( false );
	m_pLblCounter->SetFgColor( m_CounterFgColor );
	m_pLblCounter->SetPaintBackgroundEnabled( true );
	m_pLblCounter->SetPaintBackgroundType( 2 );
	m_pLblCounter->SetContentAlignment( vgui::Label::a_center );
	m_pLblCounter->SetFont( pScheme->GetFont( "DefaultVerySmall", IsProportional() ) );

	UpdateNotifications();
}

void CRD_VGUI_Notifications_Button::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	m_pLblCounter->OnThink();
	m_pLblCounter->GetContentSize( wide, tall );
	m_pLblCounter->SetBounds( GetWide() - YRES( 10 ) - wide, GetTall() - tall - YRES( 2 ), wide + YRES( 9 ), tall + YRES( 1 ) );
	m_pLblCounter->SetZPos( -1 );
}

CRD_VGUI_Notifications_List::CRD_VGUI_Notifications_List( vgui::Panel *parent, const char *panelName )
	: BaseClass{ GetParentFrame( parent ), panelName }
{
	m_hButton = parent;
	m_pList = new BaseModUI::GenericPanelList( this, "GplList", BaseModUI::GenericPanelList::ISM_PERITEM );
	m_pList->SetScrollBarVisible( true );
	m_pLblNone = new vgui::Label( this, "LblNone", "#rd_notification_none" );
	m_pFiltersButton = new CNB_Button( this, "BtnFilters", "#rd_notification_filter_title", this, "NotificationsFiltersClicked" );
	m_hFiltersPopOut = NULL;
	m_iLastTimerUpdate = -1;

	g_NotificationsLists.AddToTail( this );

	UpdateNotifications();
}

CRD_VGUI_Notifications_List::~CRD_VGUI_Notifications_List()
{
	g_NotificationsLists.FindAndFastRemove( this );

	ReactiveDropInventory::CommitNotificationSeen();

	if ( m_hFiltersPopOut )
	{
		m_hFiltersPopOut->MarkForDeletion();
		m_hFiltersPopOut = NULL;
	}
}

static bool __cdecl SortNotificationsByExpectedOrder( const vgui::Panel &a, const vgui::Panel &b )
{
	const CRD_VGUI_Notifications_List_Item *pA = assert_cast< const CRD_VGUI_Notifications_List_Item * >( &a );
	const CRD_VGUI_Notifications_List_Item *pB = assert_cast< const CRD_VGUI_Notifications_List_Item * >( &b );

	return pA->m_iExpectedOrder < pB->m_iExpectedOrder;
}

void CRD_VGUI_Notifications_List::UpdateNotifications()
{
	CVarBitVec foundNotification{ HoIAF()->m_Notifications.Count() ? HoIAF()->m_Notifications.Count() : 1 };
	foundNotification.ClearAll();

	m_pLblNone->SetVisible( HoIAF()->m_Notifications.Count() == 0 );

	// first pass: delete any list item that is no longer a notification we want to show
	for ( int i = m_pList->GetPanelItemCount() - 1; i >= 0; i-- )
	{
		CRD_VGUI_Notifications_List_Item *pItem = assert_cast< CRD_VGUI_Notifications_List_Item * >( m_pList->GetPanelItem( i ) );

		bool bFound = false;
		FOR_EACH_VEC( HoIAF()->m_Notifications, j )
		{
			if ( pItem->MatchesNotification( HoIAF()->m_Notifications[j] ) )
			{
				pItem->m_Notification = *HoIAF()->m_Notifications[j];
				pItem->m_iExpectedOrder = j;
				pItem->InitFromNotification();
				Assert( !foundNotification.Get( j ) );
				foundNotification.Set( j );
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			m_pList->RemovePanelItem( i, true );
		}
	}

	// second pass: add any notification that we're missing from the list
	FOR_EACH_VEC( HoIAF()->m_Notifications, i )
	{
		if ( foundNotification.Get( i ) )
		{
			continue;
		}

		CRD_VGUI_Notifications_List_Item *pItem = CreateNotificationListItem( HoIAF()->m_Notifications[i] );
		pItem->m_iExpectedOrder = i;
		pItem->InitFromNotification();
		m_pList->AddPanelItem( pItem, true );
	}

	// third pass: sort the list so it's in the same order as the notifications vector
	m_pList->Sort( &SortNotificationsByExpectedOrder );

	// force a timer update
	m_iLastTimerUpdate = -1;
	OnThink();

	// set up navigation
	if ( m_pList->GetPanelItemCount() )
	{
		m_hButton->SetNavDown( m_pList->GetPanelItem( 0 ) );
		m_pList->GetPanelItem( 0 )->SetNavUp( m_hButton );
		m_pList->GetPanelItem( m_pList->GetPanelItemCount() - 1 )->SetNavDown( m_pFiltersButton );
		m_pFiltersButton->SetNavUp( m_pList->GetPanelItem( m_pList->GetPanelItemCount() - 1 ) );
	}
	else
	{
		m_hButton->SetNavDown( m_pFiltersButton );
		m_pFiltersButton->SetNavUp( m_hButton );
	}
}

CRD_VGUI_Notifications_List_Item *CRD_VGUI_Notifications_List::CreateNotificationListItem( HoIAFNotification_t *pNotification )
{
	switch ( pNotification->Type )
	{
	case HoIAFNotification_t::NOTIFICATION_ITEM:
		return new CRD_VGUI_Notifications_List_Item_Inventory( this, "Notification", pNotification );
	case HoIAFNotification_t::NOTIFICATION_BOUNTY:
		return new CRD_VGUI_Notifications_List_Item_HoIAF_Bounty( this, "Notification", pNotification );
	default:
		// do a full game crash because this shouldn't be possible outside of dev builds
		COMPILE_TIME_ASSERT( HoIAFNotification_t::NUM_TYPES == 2 );
		Error( "Unhandled notification type in CRD_VGUI_Notifications_List::CreateNotificationListItem: %d\n", pNotification->Type );
		return NULL;
	}
}

void CRD_VGUI_Notifications_List::OnThink()
{
	BaseClass::OnThink();

	int64_t iNow = SteamUtils() ? SteamUtils()->GetServerRealTime() : 0;
	if ( iNow == m_iLastTimerUpdate )
	{
		// only update timers once per second to save some CPU cycles
		return;
	}

	m_iLastTimerUpdate = iNow;

	if ( rd_notification_debug_fake.GetBool() )
	{
		iNow = 1706803456;
	}

	for ( int i = 0; i < m_pList->GetPanelItemCount(); i++ )
	{
		CRD_VGUI_Notifications_List_Item *pItem = assert_cast< CRD_VGUI_Notifications_List_Item * >( m_pList->GetPanelItem( i ) );
		pItem->UpdateTimers( iNow );
	}
}

void CRD_VGUI_Notifications_List::OnCommand( const char *command )
{
	if ( FStrEq( command, "NotificationsFiltersClicked" ) )
	{
		if ( m_hFiltersPopOut && m_hFiltersPopOut->IsVisible() )
		{
			m_hFiltersPopOut->SetVisible( false );
		}
		else
		{
			if ( !m_hFiltersPopOut )
			{
				m_hFiltersPopOut = new CRD_VGUI_Notifications_Filters( GetParentFrame( this ), "Filters" );
			}

			m_hFiltersPopOut->SetVisible( true );
		}
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Notifications_List::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CRD_VGUI_Notifications_List.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	UpdateNotifications();
}

CRD_VGUI_Notifications_List_Item::CRD_VGUI_Notifications_List_Item( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification )
	: BaseClass{ parent, panelName }
{
	m_pNotificationText = new vgui::MultiFontRichText( this, "NotificationText" );
	m_pNotificationText->SetDrawTextOnly();
	m_pNotificationText->SetMouseInputEnabled( false );
	m_pNotificationText->SetCursor( vgui::dc_arrow );
	m_pLblAge = new vgui::Label( this, "LblAge", L"" );
	m_pLblAge->SetMouseInputEnabled( false );
	m_pLblExpires = new vgui::Label( this, "LblExpires", L"" );
	m_pLblExpires->SetMouseInputEnabled( false );
	m_hDetailsPopOut = NULL;
	m_Notification = *pNotification;
	m_iExpectedOrder = -1;
	m_hFontBold = vgui::INVALID_FONT;
	m_hFontNormal = vgui::INVALID_FONT;
}

CRD_VGUI_Notifications_List_Item::~CRD_VGUI_Notifications_List_Item()
{
	if ( m_hDetailsPopOut )
	{
		m_hDetailsPopOut->MarkForDeletion();
		m_hDetailsPopOut = NULL;
	}
}

void CRD_VGUI_Notifications_List_Item::InitFromNotification()
{
	m_pNotificationText->SetText( L"" );
	m_pNotificationText->InsertFontChange( m_hFontBold );
	m_pNotificationText->InsertColorChange( m_TitleColor );
	m_pNotificationText->InsertString( m_Notification.Title );
	m_pNotificationText->InsertString( L"\n" );
	m_pNotificationText->InsertFontChange( m_hFontNormal );
	m_pNotificationText->InsertColorChange( m_DescriptionColor );
	m_pNotificationText->InsertString( m_Notification.Description );

	UpdateBackgroundColor();

	InvalidateLayout();

	if ( m_hDetailsPopOut )
	{
		m_hDetailsPopOut->InitFromNotification();
	}
}

void CRD_VGUI_Notifications_List_Item::UpdateTimers( int64_t iNow )
{
	wchar_t wszTimer[256];

	if ( m_Notification.Starts > iNow )
	{
		m_pLblAge->SetVisible( false );
	}
	else
	{
		int64_t iSinceStart = iNow - m_Notification.Starts;
		Assert( iSinceStart >= 0 );
		if ( iSinceStart >= 2 * 24 * 60 * 60 )
		{
			int64_t iDays = iSinceStart / ( 24 * 60 * 60 );
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_age_days" ), 1, UTIL_RD_CommaNumber( iDays ) );
		}
		else if ( iSinceStart >= 2 * 60 * 60 )
		{
			int64_t iHours = iSinceStart / ( 60 * 60 );
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_age_hours" ), 1, UTIL_RD_CommaNumber( iHours ) );
		}
		else if ( iSinceStart >= 2 * 60 )
		{
			int64_t iMinutes = iSinceStart / 60;
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_age_minutes" ), 1, UTIL_RD_CommaNumber( iMinutes ) );
		}
		else
		{
			int64_t iSeconds = iSinceStart;
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_age_seconds" ), 1, UTIL_RD_CommaNumber( iSeconds ) );
		}

		m_pLblAge->SetVisible( true );
		m_pLblAge->SetText( wszTimer );
	}

	if ( m_Notification.Ends == 0 )
	{
		m_pLblExpires->SetVisible( false );
	}
	else
	{
		int64_t iUntilEnd = MAX( m_Notification.Ends - iNow, 0 );
		if ( iUntilEnd >= 2 * 24 * 60 * 60 )
		{
			int64_t iDays = iUntilEnd / ( 24 * 60 * 60 );
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_expires_in_days" ), 1, UTIL_RD_CommaNumber( iDays ) );
		}
		else if ( iUntilEnd >= 2 * 60 * 60 )
		{
			int64_t iHours = iUntilEnd / ( 60 * 60 );
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_expires_in_hours" ), 1, UTIL_RD_CommaNumber( iHours ) );
		}
		else if ( iUntilEnd >= 2 * 60 )
		{
			int64_t iMinutes = iUntilEnd / 60;
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_expires_in_minutes" ), 1, UTIL_RD_CommaNumber( iMinutes ) );
		}
		else
		{
			int64_t iSeconds = iUntilEnd;
			g_pVGuiLocalize->ConstructString( wszTimer, sizeof( wszTimer ),
				g_pVGuiLocalize->Find( "#rd_notification_expires_in_seconds" ), 1, UTIL_RD_CommaNumber( iSeconds ) );
		}

		m_pLblExpires->SetVisible( true );
		m_pLblExpires->SetText( wszTimer );
	}
}

void CRD_VGUI_Notifications_List_Item::UpdateBackgroundColor( int isFocused )
{
	if ( isFocused == -1 ? HasFocus() : isFocused )
	{
		SetBgColor( m_BackgroundColorHover );
	}
	else if ( m_Notification.Seen == HoIAFNotification_t::SEEN_VIEWED )
	{
		SetBgColor( m_BackgroundColorFresh );
	}
	else
	{
		SetBgColor( m_BackgroundColorViewed );
	}
}

void CRD_VGUI_Notifications_List_Item::SetSeenAtLeast(int iSeen)
{
	if ( rd_notification_debug_fake.GetBool() )
	{
		return;
	}

	if ( m_Notification.Seen < iSeen )
	{
		SetSeen( iSeen );
		Assert( m_Notification.Seen == iSeen );
	}
}

void CRD_VGUI_Notifications_List_Item::OnClicked()
{
	SetSeenAtLeast( HoIAFNotification_t::SEEN_CLICKED );
}

void CRD_VGUI_Notifications_List_Item::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	GetParent()->NavigateToChild( this );
}

void CRD_VGUI_Notifications_List_Item::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void CRD_VGUI_Notifications_List_Item::OnSetFocus()
{
	BaseClass::OnSetFocus();

	BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );

	SetSeenAtLeast( HoIAFNotification_t::SEEN_HOVERED );

	if ( m_hDetailsPopOut )
	{
		m_hDetailsPopOut->SetVisible( true );
	}

	UpdateBackgroundColor( true );
}

void CRD_VGUI_Notifications_List_Item::OnKillFocus()
{
	BaseClass::OnKillFocus();

	UpdateBackgroundColor( false );

	if ( m_hDetailsPopOut )
	{
		m_hDetailsPopOut->SetVisible( false );
	}
}

void CRD_VGUI_Notifications_List_Item::PerformLayout()
{
	BaseClass::PerformLayout();

	int tall = GetTall() - m_pNotificationText->GetTall();
	m_pNotificationText->OnThink();
	m_pNotificationText->SetToFullHeight();
	SetTall( tall + m_pNotificationText->GetTall() );

	GetParent()->GetParent()->InvalidateLayout();
}

void CRD_VGUI_Notifications_List_Item::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	InitFromNotification();
}

CRD_VGUI_Notifications_List_Item_Inventory::CRD_VGUI_Notifications_List_Item_Inventory( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification ) :
	BaseClass{ parent, panelName, pNotification }
{
	Assert( pNotification->Type == HoIAFNotification_t::NOTIFICATION_ITEM );
}

void CRD_VGUI_Notifications_List_Item_Inventory::InitFromNotification()
{
	BaseClass::InitFromNotification();

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_Notification.ItemDefID );
	Assert( pDef );
	const ReactiveDropInventory::ItemInstance_t *pItem = ReactiveDropInventory::GetLocalItemCache( m_Notification.ItemID );
	Assert( pItem || rd_notification_debug_fake.GetBool() );
	if ( !pDef || !pItem )
	{
		return;
	}

	m_pNotificationText->SetText( L"" );
	m_pNotificationText->InsertFontChange( m_hFontBold );
	m_pNotificationText->InsertColorChange( pDef->HasBorder ? pDef->NameColor : m_TitleColor );
	m_pNotificationText->InsertString( pDef->Name );
	m_pNotificationText->InsertString( L"\n" );
	m_pNotificationText->InsertFontChange( m_hFontNormal );
	pItem->FormatDescription( m_pNotificationText, true, m_DescriptionColor );
	m_pNotificationText->InsertString( m_Notification.Description );
}

bool CRD_VGUI_Notifications_List_Item_Inventory::MatchesNotification( const HoIAFNotification_t *pNotification ) const
{
	return pNotification->Type == HoIAFNotification_t::NOTIFICATION_ITEM && pNotification->ItemID == m_Notification.ItemID;
}

void CRD_VGUI_Notifications_List_Item_Inventory::SetSeen( int iSeen )
{
	if ( rd_notification_debug_fake.GetBool() )
	{
		return;
	}

	ReactiveDropInventory::QueueSetNotificationSeen( m_Notification.ItemID, iSeen );
	m_Notification.Seen = ( HoIAFNotification_t::Seen_t )iSeen;
}

void CRD_VGUI_Notifications_List_Item_Inventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CRD_VGUI_Notifications_List_Item_Inventory.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

CRD_VGUI_Notifications_List_Item_HoIAF_Bounty::CRD_VGUI_Notifications_List_Item_HoIAF_Bounty( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification ) :
	BaseClass{ parent, panelName, pNotification }
{
	Assert( pNotification->Type == HoIAFNotification_t::NOTIFICATION_BOUNTY );

	for ( int i = 0; i < MAX_MISSIONS_PER_BOUNTY; i++ )
	{
		m_pImgCompleted[i] = new vgui::ImagePanel( this, VarArgs( "ImgCompleted%d", i ) );
		m_pImgCompleted[i]->SetMouseInputEnabled( false );
		m_pImgMissionIcon[i] = new vgui::ImagePanel( this, VarArgs( "ImgMissionIcon%d", i ) );
		m_pImgMissionIcon[i]->SetMouseInputEnabled( false );
		m_pLblMissionPoints[i] = new vgui::Label( this, VarArgs( "LblMissionPoints%d", i ), L"" );
		m_pLblMissionPoints[i]->SetMouseInputEnabled( false );
	}

	m_hDetailsPopOut = new CRD_VGUI_Notifications_Details_HoIAF_Bounty( this, "Details" );
	m_hDetailsPopOut->SetMouseInputEnabled( false );
}

void CRD_VGUI_Notifications_List_Item_HoIAF_Bounty::InitFromNotification()
{
	BaseClass::InitFromNotification();

	FOR_EACH_VEC( m_Notification.BountyMissions, i )
	{
		if ( m_Notification.BountyMissions[i].Claimed )
		{
			m_pImgCompleted[i]->SetVisible( true );
			m_pLblMissionPoints[i]->SetAlpha( 64 );
		}
		else
		{
			m_pImgCompleted[i]->SetVisible( false );
			m_pLblMissionPoints[i]->SetAlpha( 255 );
		}

		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( m_Notification.BountyMissions[i].MissionName );
		if ( !pMission || pMission->WorkshopID != m_Notification.BountyMissions[i].AddonID )
		{
			Assert( m_Notification.BountyMissions[i].AddonID != k_PublishedFileIdInvalid );

			m_pImgMissionIcon[i]->SetImage( "swarm/missionpics/addonmissionpic" );
		}
		else
		{
			m_pImgMissionIcon[i]->SetImage( STRING( pMission->Image ) );
		}
		m_pImgMissionIcon[i]->SetVisible( true );
		m_pLblMissionPoints[i]->SetText( UTIL_RD_CommaNumber( m_Notification.BountyMissions[i].Points ) );
		m_pLblMissionPoints[i]->SetVisible( true );
	}

	for ( int i = m_Notification.BountyMissions.Count(); i < MAX_MISSIONS_PER_BOUNTY; i++ )
	{
		m_pImgCompleted[i]->SetVisible( false );
		m_pImgMissionIcon[i]->SetVisible( false );
		m_pLblMissionPoints[i]->SetVisible( false );
	}
}

bool CRD_VGUI_Notifications_List_Item_HoIAF_Bounty::MatchesNotification( const HoIAFNotification_t *pNotification ) const
{
	return pNotification->Type == HoIAFNotification_t::NOTIFICATION_BOUNTY && pNotification->Starts == m_Notification.Starts;
}

void CRD_VGUI_Notifications_List_Item_HoIAF_Bounty::SetSeen( int iSeen )
{
	if ( rd_notification_debug_fake.GetBool() )
	{
		return;
	}

	C_ASW_Medal_Store *pMedalStore = GetMedalStore();
	Assert( pMedalStore );
	if ( pMedalStore )
	{
		Assert( pMedalStore->GetBountyNotificationStatus( m_Notification.FirstBountyID ) == iSeen );
		pMedalStore->SetBountyNotificationStatus( m_Notification.FirstBountyID, iSeen );
		m_Notification.Seen = ( HoIAFNotification_t::Seen_t )iSeen;
	}
}

void CRD_VGUI_Notifications_List_Item_HoIAF_Bounty::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CRD_VGUI_Notifications_List_Item_HoIAF_Bounty.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

CRD_VGUI_Notifications_Details::CRD_VGUI_Notifications_Details( CRD_VGUI_Notifications_List_Item *listItem, const char *panelName )
	: BaseClass{ GetParentFrame( listItem ), panelName }
{
	m_hListItem = listItem;
}

void CRD_VGUI_Notifications_Details::OnThink()
{
	BaseClass::OnThink();

	if ( !m_hListItem || !m_hListItem->HasFocus() )
	{
		SetVisible( false );
	}
}

CRD_VGUI_Notifications_Details_HoIAF_Bounty::CRD_VGUI_Notifications_Details_HoIAF_Bounty( CRD_VGUI_Notifications_List_Item *listItem, const char *panelName )
	: BaseClass{ listItem, panelName }
{
}

void CRD_VGUI_Notifications_Details_HoIAF_Bounty::InitFromNotification()
{
	const CCopyableUtlVector<HoIAFNotification_t::BountyMission_t> &missions = m_hListItem->m_Notification.BountyMissions;

	DebuggerBreakIfDebugging(); // TODO

	InvalidateLayout();
}

void CRD_VGUI_Notifications_Details_HoIAF_Bounty::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CRD_VGUI_Notifications_Details_HoIAF_Bounty.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

CRD_VGUI_Notifications_Filters::CRD_VGUI_Notifications_Filters( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName }
{
	m_pSettingCrafting = new CRD_VGUI_Option( this, "SettingCrafting", "#rd_notification_filter_crafting_title", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingCrafting->LinkToConVar( "rd_notification_filter_crafting", false );
	m_pSettingCrafting->SetDefaultHint( "#rd_notification_filter_crafting_desc" );
	m_pSettingHoIAF = new CRD_VGUI_Option( this, "SettingHoIAF", "#rd_notification_filter_hoiaf_title", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingHoIAF->LinkToConVar( "rd_notification_filter_hoiaf", false );
	m_pSettingHoIAF->SetDefaultHint( "#rd_notification_filter_hoiaf_desc" );
	m_pSettingReports = new CRD_VGUI_Option( this, "SettingReports", "#rd_notification_filter_reports_title", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingReports->LinkToConVar( "rd_notification_filter_reports", false );
	m_pSettingReports->SetDefaultHint( "#rd_notification_filter_reports_desc" );
}

void CRD_VGUI_Notifications_Filters::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CRD_VGUI_Notifications_Filters.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}