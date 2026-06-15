#include "cbase.h"
#include "rd_collections_crafting_research.h"
#include "asw_util_shared.h"
#include "steam/isteamapps.h"
#include "rd_inventory_shared.h"
#include "fmtstr.h"
#include "vgui/ilocalize.h"
#include "vgui/isurface.h"
#include "vgui_controls/imagepanel.h"
#include "vgui_controls/label.h"
#include "nb_button.h"
#include "rd_vgui_settings.h"
#include <ctime>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CRD_Collection_Tab_Crafting_Research::CRD_Collection_Tab_Crafting_Research( TabbedGridDetails *parent, const char *szLabel )
	: BaseClass( parent, szLabel )
{
}

vgui::Panel *CRD_Collection_Tab_Crafting_Research::CreatePanel()
{
	return new CRD_Crafting_Research_Panel( this );
}

CRD_Crafting_Research_Panel::CRD_Crafting_Research_Panel( CRD_Collection_Tab_Crafting_Research *pTab )
	: BaseClass( pTab->m_pParent, "CraftingResearchPanel" )
{
	SetConsoleStylePanel( true );

	m_pParent = pTab;

	m_pLblUpdateTimer = new vgui::Label( this, "LblUpdateTimer", "" );
	m_pBackdrop = new vgui::Panel( this, "Backdrop" );
	m_pBackdropError = new vgui::Panel( this, "BackdropError" );
	m_pLblErrorMessage = new vgui::Label( this, "LblErrorMessage", "" );
	m_pBtnRetryAfterError = new CNB_Button( this, "BtnRetryAfterError", "#rd_crafting_research_retry_connection", this, "RetryConnection" );

	m_flNextUpdateTime = Plat_FloatTime() + 1.0;
	m_bErrored = false;
}

CRD_Crafting_Research_Panel::~CRD_Crafting_Research_Panel()
{
	m_pParent->m_pPanel = nullptr;

	if ( m_hGetStateRequest != INVALID_HTTPREQUEST_HANDLE )
	{
		ISteamHTTP *pSteamHTTP = SteamHTTP();
		Assert( pSteamHTTP );
		if ( pSteamHTTP )
		{
			pSteamHTTP->ReleaseHTTPRequest( m_hGetStateRequest );
		}
	}
}

void CRD_Crafting_Research_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	bool bWasVisible = IsVisible();

	LoadControlSettings( "Resource/UI/CraftingResearchPanel.res" );

	SetVisible( bWasVisible );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pBtnRetryAfterError->SetVisible( m_bErrored );
}

void CRD_Crafting_Research_Panel::PerformLayout()
{
	BaseClass::PerformLayout();

	FOR_EACH_VEC( m_VisualSlots, i )
	{
		// if we reuse this screen with a different number of slots, revisit this being hard-coded
		m_VisualSlots[i]->SetPos( YRES( 5 + i * 214 ), YRES( 16 ) );
	}
}

void CRD_Crafting_Research_Panel::OnCommand( const char *szCommand )
{
	if ( !V_stricmp( szCommand, "RetryConnection" ) )
	{
		Assert( m_bErrored );

		m_bErrored = false;

		m_pLblErrorMessage->SetText( "#rd_crafting_research_refresh_active" );
		m_pBtnRetryAfterError->SetVisible( false );

		m_flNextUpdateTime = 0.0;
		RequestResearchState();
	}
	else if ( !V_stricmp( szCommand, "ForceRefresh" ) )
	{
		m_flNextUpdateTime = 0.0;
		RequestResearchState();
	}
	else
	{
		BaseClass::OnCommand( szCommand );
	}
}

void CRD_Crafting_Research_Panel::OnThink()
{
	BaseClass::OnThink();

	if ( m_bErrored || m_hDonateModal.Get() )
	{
		m_pLblUpdateTimer->SetVisible( false );
		return;
	}

	m_pLblUpdateTimer->SetVisible( true );

	if ( m_VisualSlots.Count() == 0 )
	{
		// show *something* on the screen until the initial data is loaded
		m_pBackdropError->SetVisible( true );
		m_pLblErrorMessage->SetText( "#rd_crafting_research_refresh_active" );
	}

	double now = Plat_FloatTime();

	if ( m_flNextUpdateTime != 0.0 && m_flNextUpdateTime <= now )
	{
		Assert( m_hGetStateAuth == k_HAuthTicketInvalid );
		Assert( m_hGetStateRequest == INVALID_HTTPREQUEST_HANDLE );

		m_flNextUpdateTime = 0.0;
		m_pLblUpdateTimer->SetText( g_pVGuiLocalize->Find( "#rd_crafting_research_refresh_active" ) );
		RequestResearchState();
	}

	Assert( ( m_flNextUpdateTime > 0.0 ) != ( m_hGetStateAuth != k_HAuthTicketInvalid || m_hGetStateRequest != INVALID_HTTPREQUEST_HANDLE ) );

	if ( m_flNextUpdateTime != 0.0 )
	{
		Assert( m_flNextUpdateTime >= 0.0 );

		wchar_t wszCountdown[256];
		g_pVGuiLocalize->ConstructString( wszCountdown, sizeof( wszCountdown ),
			g_pVGuiLocalize->Find( "#rd_crafting_research_refresh_countdown" ), 1,
			UTIL_RD_CommaNumber( Ceil2Int( m_flNextUpdateTime - now ) ) );

		m_pLblUpdateTimer->SetText( wszCountdown );
	}
}

void CRD_Crafting_Research_Panel::ShowError( const char *szError )
{
	m_bErrored = true;
	m_pBackdropError->SetVisible( true );
	m_pLblErrorMessage->SetText( szError );

	FOR_EACH_VEC( m_VisualSlots, i )
	{
		m_VisualSlots[i]->MarkForDeletion();
	}
	m_VisualSlots.Purge();

	m_pBtnRetryAfterError->SetVisible( true );
	NavigateToChild( m_pBtnRetryAfterError );
}

void CRD_Crafting_Research_Panel::RequestResearchState()
{
	ISteamUser *pSteamUser = SteamUser();
	Assert( pSteamUser );
	if ( !pSteamUser )
	{
		Warning( "Missing ISteamUser! Cannot request research state.\n" );
		ShowError( "#rd_crafting_research_error_steam" );
		return;
	}

	// prove our identity to the server to get our personal contribution to the crafting project (this isn't stored or used for any other purpose)
	m_hGetStateAuth = pSteamUser->GetAuthTicketForWebApi( "ocm_research_get_state" );
}

void CRD_Crafting_Research_Panel::OnGetTicketForWebApiResponse( GetTicketForWebApiResponse_t *pParam )
{
	if ( pParam->m_hAuthTicket == m_hGetStateAuth )
	{
		m_hGetStateAuth = k_HAuthTicketInvalid;

		Assert( !m_hDonateModal.Get() );
		if ( m_hDonateModal.Get() )
		{
			// don't refresh while the donate modal is open
			m_flNextUpdateTime = Plat_FloatTime() + 60.0;
			return;
		}

		ISteamHTTP *pSteamHTTP = SteamHTTP();
		Assert( pSteamHTTP );
		if ( !pSteamHTTP )
		{
			Warning( "Missing ISteamHTTP! Cannot request research state.\n" );
			ShowError( "#rd_crafting_research_error_steam" );
			return;
		}

		char szHexTicket[sizeof( pParam->m_rgubTicket ) * 2 + 1];
		UTIL_RD_BinToHex( pParam->m_rgubTicket, pParam->m_cubTicket, szHexTicket, sizeof( szHexTicket ) );

		m_hGetStateRequest = pSteamHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://stats.reactivedrop.com/api/ocm-research/state.bin" );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hGetStateRequest, "lang", SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "" );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hGetStateRequest, "ticket", szHexTicket );
		pSteamHTTP->SetHTTPRequestUserAgentInfo( m_hGetStateRequest, "CRD_Crafting_Research_Panel" );
		SteamAPICall_t hAPICall = k_uAPICallInvalid;
		pSteamHTTP->SendHTTPRequest( m_hGetStateRequest, &hAPICall );

		m_OnGetStateRequestCompleted.Set( hAPICall, this, &CRD_Crafting_Research_Panel::OnGetStateRequestCompleted );

		m_hGetStateAuth = k_HAuthTicketInvalid;

		return;
	}
}

void CRD_Crafting_Research_Panel::OnGetStateRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	Assert( !m_hDonateModal.Get() );
	if ( m_hDonateModal.Get() )
	{
		// don't refresh while the donate modal is open
		m_flNextUpdateTime = Plat_FloatTime() + 60.0;

		if ( !bIOFailure && SteamHTTP() )
		{
			Assert( m_hGetStateRequest == pParam->m_hRequest );
			m_hGetStateRequest = INVALID_HTTPREQUEST_HANDLE;
			SteamHTTP()->ReleaseHTTPRequest( pParam->m_hRequest );
		}

		return;
	}

	if ( bIOFailure )
	{
		Warning( "Lost connection to Steam client! Cannot parse research state response.\n" );
		ShowError( "#rd_crafting_research_error_steam" );
		return;
	}

	ISteamHTTP *pSteamHTTP = SteamHTTP();
	Assert( pSteamHTTP );
	if ( !pSteamHTTP )
	{
		Warning( "Missing ISteamHTTP! Cannot parse research state response.\n" );
		ShowError( "#rd_crafting_research_error_steam" );
		return;
	}

	Assert( m_hGetStateRequest == pParam->m_hRequest );
	m_hGetStateRequest = INVALID_HTTPREQUEST_HANDLE;

	if ( !pParam->m_bRequestSuccessful )
	{
		Warning( "Could not connect to server! Cannot parse research state response.\n" );
		ShowError( "#rd_crafting_research_error_connection" );
		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		return;
	}

	if ( pParam->m_eStatusCode != k_EHTTPStatusCode200OK )
	{
		Warning( "Server returned error (%d)! Cannot parse research state response.\n", pParam->m_eStatusCode );
		ShowError( "#rd_crafting_research_error_unknown" );
		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		return;
	}

	CUtlBuffer body{ 0, int( pParam->m_unBodySize ) };
	body.SeekPut( CUtlBuffer::SEEK_HEAD, pParam->m_unBodySize );
	if ( !pSteamHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )body.Base(), pParam->m_unBodySize ) )
	{
		Warning( "Failed to get response body. Cannot parse research state response.\n" );
		ShowError( "#rd_crafting_research_error_connection" );
		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		return;
	}

	pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );

	KeyValues::AutoDelete pKV( "RDRD" );
	if ( !pKV->ReadAsBinary( body ) )
	{
		Warning( "Failed to read binary keyvalues. Cannot parse research state response.\n" );
		ShowError( "#rd_crafting_research_error_unknown" );
		return;
	}

	m_Slots.PurgeAndDeleteElements();

	FOR_EACH_TRUE_SUBKEY( pKV, pSlot )
	{
		if ( V_stricmp( pSlot->GetName(), "s" ) )
		{
			continue;
		}

		ResearchSlot_t *pResearchSlot = new ResearchSlot_t();
		pResearchSlot->ID = pSlot->GetInt( "i" );
		pResearchSlot->Title = pSlot->GetString( "t" );
		pResearchSlot->WaitDescription = pSlot->GetString( "d" );
		pResearchSlot->WaitUntil = pSlot->GetInt( "u" );

		KeyValues *pProject = pSlot->FindKey( "p" );
		if ( pProject )
		{
			pResearchSlot->ProjectID = pProject->GetInt( "i" );
			pResearchSlot->ItemForIcon = pProject->GetInt( "p" );
			( void )ReactiveDropInventory::GetItemDef( pResearchSlot->ItemForIcon ); // precache icon
			pResearchSlot->ProjectTitle = pProject->GetString( "t" );
			pResearchSlot->ProjectFlavorText = pProject->GetString( "d" );

			FOR_EACH_TRUE_SUBKEY( pProject, pComponent )
			{
				if ( V_stricmp( pComponent->GetName(), "c" ) )
				{
					continue;
				}

				ResearchComponent_t *pResearchComponent = new ResearchComponent_t();

				pResearchComponent->ID = pComponent->GetInt( "i" );
				pResearchComponent->ItemDef = pComponent->GetInt( "d" );
				( void )ReactiveDropInventory::GetItemDef( pResearchComponent->ItemDef ); // precache icon
				pResearchComponent->TotalRequirement = pComponent->GetInt( "t" );
				pResearchComponent->MaxPersonal = pComponent->GetInt( "m" );
				pResearchComponent->TotalContributed = pComponent->GetInt( "q" );
				pResearchComponent->PersonalContributed = pComponent->GetInt( "p" );

				pResearchSlot->Components.AddToTail( pResearchComponent );
			}
		}

		m_Slots.AddToTail( pResearchSlot );
	}

	if ( m_Slots.Count() != m_VisualSlots.Count() )
	{
		m_VisualSlots.EnsureCapacity( m_Slots.Count() );
		while ( m_Slots.Count() > m_VisualSlots.Count() )
		{
			int i = m_VisualSlots.Count();
			m_VisualSlots.AddToTail( new CRD_Crafting_Research_Slot( this, "ResearchSlot" ) );
			m_VisualSlots[i]->m_pParent = this;
			m_VisualSlots[i]->m_iSlotIndex = i;
		}

		for ( int i = m_Slots.Count(); i < m_VisualSlots.Count(); i++ )
		{
			m_VisualSlots[i]->DeletePanel();
		}
		m_VisualSlots.SetCountNonDestructively( m_Slots.Count() );

		InvalidateLayout();
	}

	FOR_EACH_VEC( m_VisualSlots, i )
	{
		m_VisualSlots[i]->UpdateState();
	}

	if ( m_VisualSlots.Count() > 0 )
	{
		// hide the loading text now that we have actual UI in that screen space
		m_pBackdropError->SetVisible( false );
		m_pLblErrorMessage->SetText( "" );
	}

	m_flNextUpdateTime = Plat_FloatTime() + 60.0;
}

CRD_Crafting_Research_Slot::CRD_Crafting_Research_Slot( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_pLblTitle = new vgui::Label( this, "LblTitle", "" );
	m_pLblWaitDescription = new vgui::Label( this, "LblWaitDescription", "" );
	m_pLblWaitTimer = new vgui::Label( this, "LblWaitTimer", L"0:00:00" );
	m_pImgItemIcon = new vgui::ImagePanel( this, "ImgItemIcon" );
	m_pLblProjectTitle = new vgui::Label( this, "LblProjectTitle", L"" );
	m_pLblProjectFlavorText = new vgui::Label( this, "LblProjectFlavorText", L"" );
	m_pBtnFetchProjectComponents = new CNB_Button( this, "BtnFetchProjectComponents", "#rd_crafting_research_force_refresh", parent, "ForceRefresh" );
	m_pGplComponents = new BaseModUI::GenericPanelList( this, "GplComponents", BaseModUI::GenericPanelList::ISM_ELEVATOR );
}

void CRD_Crafting_Research_Slot::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CraftingResearchSlot.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Crafting_Research_Slot::OnThink()
{
	BaseClass::OnThink();

	UpdateTimer();

	if ( m_pParent && m_pBtnFetchProjectComponents->IsVisible() )
	{
		m_pBtnFetchProjectComponents->SetEnabled( m_pParent->m_hGetStateAuth == k_HAuthTicketInvalid && m_pParent->m_hGetStateRequest == INVALID_HTTPREQUEST_HANDLE );
	}
}

void CRD_Crafting_Research_Slot::UpdateTimer()
{
	Assert( m_pParent );
	if ( !m_pParent )
		return;

	Assert( m_pParent->m_Slots.IsValidIndex( m_iSlotIndex ) );
	if ( !m_pParent->m_Slots.IsValidIndex( m_iSlotIndex ) )
		return;

	CRD_Crafting_Research_Panel::ResearchSlot_t *pSlot = m_pParent->m_Slots[m_iSlotIndex];
	Assert( pSlot );
	if ( !pSlot )
		return;

	RTime32 now = SteamUtils() ? SteamUtils()->GetServerRealTime() : std::time( nullptr );

	if ( pSlot->WaitUntil == 0 || pSlot->WaitUntil < now )
	{
		m_pLblWaitDescription->SetVisible( pSlot->ProjectID == 0 );
		m_pLblProjectFlavorText->SetVisible( pSlot->ProjectID != 0 );
		m_pLblWaitTimer->SetVisible( false );

		m_pBtnFetchProjectComponents->SetVisible( pSlot->Components.Count() == 0 && pSlot->ProjectID != 0 );
		m_pBtnFetchProjectComponents->SetEnabled( m_pParent->m_hGetStateAuth == k_HAuthTicketInvalid && m_pParent->m_hGetStateRequest == INVALID_HTTPREQUEST_HANDLE );
	}
	else
	{
		m_pLblWaitDescription->SetVisible( true );
		m_pLblProjectFlavorText->SetVisible( false );
		m_pLblWaitTimer->SetVisible( true );

		int seconds = pSlot->WaitUntil - now;

		wchar_t wszTimer[64];
		V_snwprintf( wszTimer, ARRAYSIZE( wszTimer ), L"%d:%02d:%02d", seconds / ( 60 * 60 ), ( seconds / 60 ) % 60, seconds % 60 );
		m_pLblWaitTimer->SetText( wszTimer );

		m_pBtnFetchProjectComponents->SetVisible( false );
		m_pBtnFetchProjectComponents->SetEnabled( false );
	}
}

void CRD_Crafting_Research_Slot::UpdateState()
{
	MakeReadyForUse();

	Assert( m_pParent );
	if ( !m_pParent )
		return;

	Assert( m_pParent->m_Slots.IsValidIndex( m_iSlotIndex ) );
	if ( !m_pParent->m_Slots.IsValidIndex( m_iSlotIndex ) )
		return;

	CRD_Crafting_Research_Panel::ResearchSlot_t *pSlot = m_pParent->m_Slots[m_iSlotIndex];
	Assert( pSlot );
	if ( !pSlot )
		return;


	const ReactiveDropInventory::ItemDef_t *pDef = pSlot->ItemForIcon != 0 ? ReactiveDropInventory::GetItemDef( pSlot->ItemForIcon ) : nullptr;
	if ( pDef )
	{
		m_pImgItemIcon->SetImage( pDef->Icon );
		m_pImgItemIcon->SetVisible( true );
	}
	else
	{
		m_pImgItemIcon->SetImage( ( vgui::IImage * )nullptr );
		m_pImgItemIcon->SetVisible( false );
	}

	m_pLblTitle->SetText( pSlot->Title );
	m_pLblWaitDescription->SetText( pSlot->WaitDescription );
	UpdateTimer();
	m_pLblProjectTitle->SetText( pSlot->ProjectTitle );
	m_pLblProjectFlavorText->SetText( pSlot->ProjectFlavorText );

	for ( int i = m_pGplComponents->GetPanelItemCount(); i < pSlot->Components.Count(); i++ )
	{
		CRD_Crafting_Research_Component *pComponent = m_pGplComponents->AddPanelItem<CRD_Crafting_Research_Component>( "ResearchComponent" );
		pComponent->m_pParent = this;
		pComponent->m_iComponentIndex = i;
	}

	for ( int i = m_pGplComponents->GetPanelItemCount(); i > pSlot->Components.Count(); i-- )
	{
		m_pGplComponents->RemovePanelItem( i, true );
	}

	for ( ushort i = 0; i < m_pGplComponents->GetPanelItemCount(); i++ )
	{
		assert_cast<CRD_Crafting_Research_Component *>( m_pGplComponents->GetPanelItem( i ) )->UpdateState();
	}
}

CRD_Crafting_Research_Component::CRD_Crafting_Research_Component( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_pImgItemIcon = new vgui::ImagePanel( this, "ImgItemIcon" );
	m_pLblItemName = new vgui::Label( this, "LblItemName", L"" );
	m_pCommunityProgressBar = new StatsBar( this, "CommunityProgressBar" );
	m_pLblPersonalProgress = new vgui::Label( this, "LblPersonalProgress", L"" );
	m_pBtnContribute = new CNB_Button( this, "BtnContribute", "", this, "Contribute" );

	SetPostChildPaintEnabled( true );
}

void CRD_Crafting_Research_Component::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CraftingResearchComponent.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Crafting_Research_Component::PostChildPaint()
{
	BaseClass::PostChildPaint();

	int x, y, wide, tall;
	m_pBtnContribute->GetBounds( x, y, wide, tall );
	vgui::surface()->DrawSetColor( Color( 255, 255, 255, m_pBtnContribute->IsEnabled() ? 255 : 192 ) );
	vgui::surface()->DrawSetTexture( m_nDonateTextureId );
	vgui::surface()->DrawTexturedRect( x + YRES( 2 ), y + YRES( 2 ), x + wide - YRES( 2 ), y + tall - YRES( 2 ) );
}

void CRD_Crafting_Research_Component::OnCommand( const char *szCommand )
{
	if ( !V_stricmp( szCommand, "Contribute" ) )
	{
		Assert( m_pParent );
		if ( !m_pParent )
			return;

		Assert( m_pParent->m_pParent );
		if ( !m_pParent->m_pParent )
			return;

		Assert( m_pParent->m_pParent->m_Slots.IsValidIndex( m_pParent->m_iSlotIndex ) );
		if ( !m_pParent->m_pParent->m_Slots.IsValidIndex( m_pParent->m_iSlotIndex ) )
			return;

		CRD_Crafting_Research_Panel::ResearchSlot_t *pSlot = m_pParent->m_pParent->m_Slots[m_pParent->m_iSlotIndex];
		Assert( pSlot );
		if ( !pSlot )
			return;

		Assert( pSlot->Components.IsValidIndex( m_iComponentIndex ) );
		if ( !pSlot->Components.IsValidIndex( m_iComponentIndex ) )
			return;

		CRD_Crafting_Research_Panel::ResearchComponent_t *pComponent = pSlot->Components[m_iComponentIndex];
		Assert( pComponent );
		if ( !pComponent )
			return;

		Assert( !m_pParent->m_pParent->m_hDonateModal.Get() );
		if ( m_pParent->m_pParent->m_hDonateModal.Get() )
			return;

		CRD_Crafting_Research_Donate_Modal *pDonateModal = new CRD_Crafting_Research_Donate_Modal( m_pParent->m_pParent, "DonateModal" );
		m_pParent->m_pParent->m_hDonateModal = pDonateModal;
		pDonateModal->m_pParent = m_pParent->m_pParent;
		pDonateModal->m_iSelectedProjectID = pSlot->ProjectID;
		pDonateModal->m_iDonateComponentID = pComponent->ID;
		pDonateModal->m_iDonateItemDef = pComponent->ItemDef;

		// we can only donate from one item stack at a time; the game should have automatically combined stacks, but it's possible that there are still multiple for various reasons.
		CUtlVector<ReactiveDropInventory::ItemInstance_t> items;
		ReactiveDropInventory::GetItemsForDef( items, pComponent->ItemDef );
		int iBestStackQuantity = 0;
		FOR_EACH_VEC( items, i )
		{
			if ( items[i].Quantity > iBestStackQuantity )
			{
				iBestStackQuantity = items[i].Quantity;
				pDonateModal->m_iDonateItemInstance = items[i].ItemID;
				pDonateModal->m_iDonateMaxQuantity = MIN( pComponent->MaxDonation(), items[i].Quantity );
			}
		}

		pDonateModal->MakeReadyForUse();

		FOR_EACH_VEC( m_pParent->m_pParent->m_VisualSlots, i )
		{
			CRD_Crafting_Research_Slot *pOtherSlot = m_pParent->m_pParent->m_VisualSlots[i];
			unsigned short count = pOtherSlot->m_pGplComponents->GetPanelItemCount();
			for ( unsigned short j = 0; j < count;j++ )
			{
				CRD_Crafting_Research_Component *pOtherComponent = assert_cast< CRD_Crafting_Research_Component * >( pOtherSlot->m_pGplComponents->GetPanelItem( j ) );
				pOtherComponent->m_pBtnContribute->SetEnabled( false );
			}
		}

		m_pParent->m_pParent->m_flNextUpdateTime = Plat_FloatTime() + 1.0;

		m_pParent->m_pParent->m_pBackdrop->SetVisible( false );
	}
	else
	{
		BaseClass::OnCommand( szCommand );
	}
}

void CRD_Crafting_Research_Component::UpdateState()
{
	Assert( m_pParent );
	if ( !m_pParent )
		return;

	Assert( m_pParent->m_pParent );
	if ( !m_pParent->m_pParent )
		return;

	Assert( m_pParent->m_pParent->m_Slots.IsValidIndex( m_pParent->m_iSlotIndex ) );
	if ( !m_pParent->m_pParent->m_Slots.IsValidIndex( m_pParent->m_iSlotIndex ) )
		return;

	CRD_Crafting_Research_Panel::ResearchSlot_t *pSlot = m_pParent->m_pParent->m_Slots[m_pParent->m_iSlotIndex];
	Assert( pSlot );
	if ( !pSlot )
		return;

	Assert( pSlot->Components.IsValidIndex( m_iComponentIndex ) );
	if ( !pSlot->Components.IsValidIndex( m_iComponentIndex ) )
		return;

	CRD_Crafting_Research_Panel::ResearchComponent_t *pComponent = pSlot->Components[m_iComponentIndex];
	Assert( pComponent );
	if ( !pComponent )
		return;

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( pComponent->ItemDef );
	wchar_t wszText[256];
	if ( pDef )
	{
		m_pImgItemIcon->SetImage( pDef->Icon );
		V_UTF8ToUnicode( pDef->BriefingName, wszText, sizeof( wszText ) );
		m_pLblItemName->SetText( wszText );
		m_pLblItemName->SetFgColor( pDef->NameColor );
	}
	else
	{
		m_pImgItemIcon->SetImage( ( vgui::IImage * )nullptr );
		V_snwprintf( wszText, ARRAYSIZE( wszText ), L"ITEMDEFMISSING#%d", pComponent->ItemDef );
		m_pLblItemName->SetText( wszText );
	}

	int nPlayerItemCount = 0;
	CUtlVector<ReactiveDropInventory::ItemInstance_t> items;
	ReactiveDropInventory::GetItemsForDef( items, pComponent->ItemDef );
	FOR_EACH_VEC( items, i )
	{
		nPlayerItemCount += items[i].Quantity;
	}

	m_pCommunityProgressBar->ClearMinMax();
	m_pCommunityProgressBar->Init( pComponent->TotalContributed, pComponent->TotalContributed, 1.0f, true, false );
	m_pCommunityProgressBar->SetShowMaxOnCounter( true );
	m_pCommunityProgressBar->AddMinMax( 0, pComponent->TotalRequirement );

	g_pVGuiLocalize->ConstructString( wszText, sizeof( wszText ), g_pVGuiLocalize->Find( "#rd_crafting_research_personal_progress" ), 3,
		UTIL_RD_CommaNumber( pComponent->PersonalContributed ),
		UTIL_RD_CommaNumber( pComponent->PersonalContributed + pComponent->MaxDonation() ),
		UTIL_RD_CommaNumber( nPlayerItemCount ) );
	m_pLblPersonalProgress->SetText( wszText );

	m_pBtnContribute->SetEnabled( pComponent->MaxDonation() > 0 && nPlayerItemCount > 0 );
}

CRD_Crafting_Research_Donate_Modal::CRD_Crafting_Research_Donate_Modal( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_pImgItemIcon = new vgui::ImagePanel( this, "ImgItemIcon" );
	m_pLblItemName = new vgui::Label( this, "LblItemName", L"" );
	m_pLblDisplayType = new vgui::Label( this, "LblDisplayType", L"" );
	m_pQuantitySlider = new CRD_VGUI_Option( this, "QuantitySlider", "#rd_crafting_research_quantity_to_donate", CRD_VGUI_Option::MODE_SLIDER );
	m_pQuantitySlider->AddActionSignalTarget( this );
	m_pBtnConfirm = new CNB_Button( this, "BtnConfirm", "#rd_crafting_research_donate_button", this, "ConfirmDonation" );
	m_pBtnCancel = new CNB_Button( this, "BtnCancel", "#asw_button_cancel", this, "Back" );
}

CRD_Crafting_Research_Donate_Modal::~CRD_Crafting_Research_Donate_Modal()
{
	if ( m_hDonateRequest != INVALID_HTTPREQUEST_HANDLE )
	{
		ISteamHTTP *pSteamHTTP = SteamHTTP();
		Assert( pSteamHTTP );
		if ( pSteamHTTP )
		{
			pSteamHTTP->ReleaseHTTPRequest( m_hDonateRequest );
		}
	}
}

void CRD_Crafting_Research_Donate_Modal::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "Resource/UI/CraftingResearchDonateModal.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iDonateItemDef );
	Assert( pDef );
	if ( !pDef )
		return;

	m_pImgItemIcon->SetImage( pDef->Icon );

	wchar_t wszText[256];
	V_UTF8ToUnicode( pDef->BriefingName, wszText, sizeof( wszText ) );
	m_pLblItemName->SetText( wszText );
	m_pLblItemName->SetFgColor( pDef->NameColor );

	V_UTF8ToUnicode( pDef->DisplayType, wszText, sizeof( wszText ) );
	m_pLblDisplayType->SetText( wszText );

	m_pQuantitySlider->SetSliderMinMax( 1.0f, m_iDonateMaxQuantity );
	m_pQuantitySlider->SetSliderSnap( 1.0f );
	NavigateToChild( m_pQuantitySlider );

	m_pBtnConfirm->SetEnabled( false );
}

void CRD_Crafting_Research_Donate_Modal::OnCommand( const char *szCommand )
{
	if ( !V_stricmp( szCommand, "ConfirmDonation" ) )
	{
		ISteamUser *pSteamUser = SteamUser();
		Assert( pSteamUser );
		if ( !pSteamUser )
		{
			Warning( "Missing ISteamUser! Cannot donate to crafting research.\n" );
			m_pParent->m_pBackdrop->SetVisible( true );
			m_pParent->ShowError( "#rd_crafting_research_error_steam" );
			MarkForDeletion();
			return;
		}

		// prove our identity to the server because we're going to edit our inventory (our Steam ID gets stored in the database in the record of the donation)
		m_hDonateAuth = pSteamUser->GetAuthTicketForWebApi( CFmtStr( "don8_%d_%d_%u_%d", m_iSelectedProjectID, m_iDonateComponentID, uint32_t( m_iDonateItemInstance ), m_iDonatedQuantity ) );
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );

		m_pQuantitySlider->SetEnabled( false );
		m_pBtnConfirm->SetEnabled( false );
		m_pBtnCancel->SetEnabled( false );
	}
	else if ( !V_stricmp( szCommand, "Back" ) )
	{
		m_pParent->m_pBackdrop->SetVisible( true );
		MarkForDeletion();
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_BACK );
	}
	else
	{
		BaseClass::OnCommand( szCommand );
	}
}

void CRD_Crafting_Research_Donate_Modal::OnGetTicketForWebApiResponse( GetTicketForWebApiResponse_t *pParam )
{
	if ( pParam->m_hAuthTicket == m_hDonateAuth )
	{
		m_hDonateAuth = k_HAuthTicketInvalid;

		ISteamHTTP *pSteamHTTP = SteamHTTP();
		Assert( pSteamHTTP );
		if ( !pSteamHTTP )
		{
			Warning( "Missing ISteamHTTP! Cannot donate to crafting research.\n" );
			m_pParent->ShowError( "#rd_crafting_research_error_steam" );
			MarkForDeletion();
			return;
		}

		char szHexTicket[sizeof( pParam->m_rgubTicket ) * 2 + 1];
		UTIL_RD_BinToHex( pParam->m_rgubTicket, pParam->m_cubTicket, szHexTicket, sizeof( szHexTicket ) );

		Assert( m_hDonateRequest == INVALID_HTTPREQUEST_HANDLE );
		m_hDonateRequest = pSteamHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://stats.reactivedrop.com/api/ocm-research/donate" );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hDonateRequest, "project", CFmtStr( "%d", m_iSelectedProjectID ) );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hDonateRequest, "order", CFmtStr( "%d", m_iDonateComponentID ) );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hDonateRequest, "instance", CFmtStr( "%llu", m_iDonateItemInstance ) );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hDonateRequest, "quantity", CFmtStr( "%d", m_iDonatedQuantity ) );
		pSteamHTTP->SetHTTPRequestGetOrPostParameter( m_hDonateRequest, "ticket", szHexTicket );
		pSteamHTTP->SetHTTPRequestUserAgentInfo( m_hDonateRequest, "CRD_Crafting_Research_Donate_Modal" );
		SteamAPICall_t hAPICall = k_uAPICallInvalid;
		pSteamHTTP->SendHTTPRequest( m_hDonateRequest, &hAPICall );

		m_OnDonateRequestCompleted.Set( hAPICall, this, &CRD_Crafting_Research_Donate_Modal::OnDonateRequestCompleted );

		BaseModUI::CUIGameData::Get()->OpenWaitScreen( "#rd_crafting_research_donation_in_progress" );
		BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_COLLECTIONS )->SetVisible( false );

		return;
	}
}

void CRD_Crafting_Research_Donate_Modal::OnDonateRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	BaseModUI::CUIGameData::Get()->CloseWaitScreen( NULL, NULL );
	BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_COLLECTIONS )->SetVisible( true );
	m_pParent->m_pBackdrop->SetVisible( true );

	if ( bIOFailure )
	{
		Warning( "Lost connection to Steam client! Cannot check donation result.\n" );
		m_pParent->ShowError( "#rd_crafting_research_error_steam" );
		MarkForDeletion();
		return;
	}

	ISteamHTTP *pSteamHTTP = SteamHTTP();
	Assert( pSteamHTTP );
	if ( !pSteamHTTP )
	{
		Warning( "Missing ISteamHTTP! Cannot check donation result.\n" );
		m_pParent->ShowError( "#rd_crafting_research_error_steam" );
		MarkForDeletion();
		return;
	}

	Assert( m_hDonateRequest == pParam->m_hRequest );
	m_hDonateRequest = INVALID_HTTPREQUEST_HANDLE;

	if ( !pParam->m_bRequestSuccessful )
	{
		Warning( "Could not connect to server! Cannot check donation result.\n" );
		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		m_pParent->ShowError( "#rd_crafting_research_error_connection" );
		MarkForDeletion();
		return;
	}

	if ( pParam->m_eStatusCode == k_EHTTPStatusCode202Accepted )
	{
		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		m_pParent->ShowError( "#rd_crafting_research_donation_success" );
		ReactiveDropInventory::RequestFullInventoryRefresh();
		MarkForDeletion();
		return;
	}

	if ( pParam->m_eStatusCode == k_EHTTPStatusCode409Conflict )
	{
		CUtlBuffer body{ 0, int( pParam->m_unBodySize ) };
		body.SeekPut( CUtlBuffer::SEEK_HEAD, pParam->m_unBodySize );
		if ( !pSteamHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )body.Base(), pParam->m_unBodySize ) )
		{
			Warning( "Failed to get response body. Cannot check donation result.\n" );
			pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			m_pParent->ShowError( "#rd_crafting_research_error_connection" );
			MarkForDeletion();
			return;
		}

		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );

		if ( !V_strnicmp( ( char * )body.Base(), "INVENTORY\n", pParam->m_unBodySize ) )
		{
			m_pParent->ShowError( "#rd_crafting_research_error_inventory" );
			ReactiveDropInventory::RequestFullInventoryRefresh();
			MarkForDeletion();
			return;
		}

		if ( !V_strnicmp( ( char * )body.Base(), "STALE\n", pParam->m_unBodySize ) )
		{
			m_pParent->ShowError( "#rd_crafting_research_error_stale" );
			MarkForDeletion();
			return;
		}
	}
	else
	{
		pSteamHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
	}

	Warning( "Donation request returned HTTP status code %d.\n", pParam->m_eStatusCode );
	m_pParent->ShowError( "#rd_crafting_research_error_unknown" );
	MarkForDeletion();
}

void CRD_Crafting_Research_Donate_Modal::OnCurrentOptionChanged( vgui::Panel *panel )
{
	m_iDonatedQuantity = RoundFloatToInt( m_pQuantitySlider->GetCurrentSliderValue() );
	m_pBtnConfirm->SetEnabled( true );
}
