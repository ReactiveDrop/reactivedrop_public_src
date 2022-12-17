#include "cbase.h"
#include "rd_challenge_selection.h"
#include "rd_challenges_shared.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "nb_header_footer.h"
#include "nb_button.h"
#include "vfooterpanel.h"
#include "rd_workshop.h"
#include "filesystem.h"
#include "controller_focus.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BaseModUI::ReactiveDropChallengeSelectionListItem::ReactiveDropChallengeSelectionListItem( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	SetProportional( true );

	m_lblError = new vgui::Label( this, "LblError", "" );
	m_lblError->SetMouseInputEnabled( false );
	m_imgIcon = new vgui::ImagePanel( this, "ImgIcon" );
	m_imgIcon->SetMouseInputEnabled( false );
	m_lblName = new vgui::Label( this, "LblName", "" );
	m_lblName->SetMouseInputEnabled( false );
	m_lblSource = new vgui::Label( this, "LblSource", "" );
	m_lblSource->SetMouseInputEnabled( false );

	m_nWorkshopID = k_PublishedFileIdInvalid;

	m_bCurrentlySelected = false;
}

void BaseModUI::ReactiveDropChallengeSelectionListItem::OnMousePressed( vgui::MouseCode code )
{
	if ( MOUSE_LEFT == code )
	{
		GenericPanelList *pGenericList = dynamic_cast< GenericPanelList * >( GetParent()->GetParent() );
		Assert( pGenericList );
		if ( pGenericList )
		{
			pGenericList->SelectPanelItemByPanel( this );
		}
	}
	else if ( MOUSE_RIGHT == code && GetControllerFocus()->IsControllerMode() )
	{
		ReactiveDropChallengeSelection *pChallengeSelection = dynamic_cast< ReactiveDropChallengeSelection * >( GetParent()->GetParent()->GetParent() );
		Assert( pChallengeSelection );
		if ( pChallengeSelection )
		{
			pChallengeSelection->OnCommand( "BackButton" );
		}
	}

	BaseClass::OnMousePressed( code );
}

void BaseModUI::ReactiveDropChallengeSelectionListItem::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	static KeyValues *s_pPreloadedChallengeListItemLayout = NULL;
	if ( !s_pPreloadedChallengeListItemLayout )
	{
		const char *pszResource = "Resource/UI/BaseModUI/ReactiveDropChallengeSelectionListItem.res";
		s_pPreloadedChallengeListItemLayout = new KeyValues( pszResource );
		s_pPreloadedChallengeListItemLayout->LoadFromFile( g_pFullFileSystem, pszResource );
	}

	LoadControlSettings( "", NULL, s_pPreloadedChallengeListItemLayout );

	SetBgColor( pScheme->GetColor( "Button.BgColor", Color( 64, 64, 64, 255 ) ) );
	SetPaintBackgroundEnabled( false );

	m_hTextFont = pScheme->GetFont( "DefaultLarge", true );
}

void BaseModUI::ReactiveDropChallengeSelectionListItem::OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel )
{
	BaseClass::OnMessage( params, ifromPanel );

	if ( !V_strcmp( params->GetName(), "PanelSelected" ) ) 
	{
		m_bCurrentlySelected = true;
		SetPaintBackgroundEnabled( true );
	}
	if ( !V_strcmp( params->GetName(), "PanelUnSelected" ) ) 
	{
		m_bCurrentlySelected = false;
		SetPaintBackgroundEnabled( false );
	}
}

void BaseModUI::ReactiveDropChallengeSelectionListItem::Paint()
{
	BaseClass::Paint();

	// Draw the graded outline for the selected item only
	if ( m_bCurrentlySelected )
	{
		int nPanelWide, nPanelTall;
		GetSize( nPanelWide, nPanelTall );

		vgui::surface()->DrawSetColor( Color( 169, 213, 255, 128 ) );

		// Top lines
		vgui::surface()->DrawFilledRectFade( 0, 0, 0.5f * nPanelWide, 2, 0, 255, true );
		vgui::surface()->DrawFilledRectFade( 0.5f * nPanelWide, 0, nPanelWide, 2, 255, 0, true );

		// Bottom lines
		vgui::surface()->DrawFilledRectFade( 0, nPanelTall-2, 0.5f * nPanelWide, nPanelTall, 0, 255, true );
		vgui::surface()->DrawFilledRectFade( 0.5f * nPanelWide, nPanelTall-2, nPanelWide, nPanelTall, 255, 0, true );

		// Text Blotch
		int nTextWide, nTextTall, nNameX, nNameY, nNameWide, nNameTall;
		wchar_t wszChallengeName[256];

		m_lblName->GetPos( nNameX, nNameY );
		m_lblName->GetSize( nNameWide, nNameTall );
		m_lblName->GetText( wszChallengeName, sizeof( wszChallengeName ) );
		vgui::surface()->GetTextSize( m_hTextFont, wszChallengeName, nTextWide, nTextTall );
		int nBlotchWide = nTextWide + vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 75 );

		vgui::surface()->DrawFilledRectFade( 0, 2, 0.50f * nBlotchWide, nPanelTall-2, 0, 50, true );
		vgui::surface()->DrawFilledRectFade( 0.50f * nBlotchWide, 2, nBlotchWide, nPanelTall-2, 50, 0, true );
	}
}

void BaseModUI::ReactiveDropChallengeSelectionListItem::PopulateChallenge( const char *szName )
{
	m_szChallengeName = szName;

	if ( !Q_strcmp( szName, "0" ) )
	{
		m_lblName->SetText( "#rd_challenge_name_0" );
		m_lblName->SetVisible( true );
		m_szChallengeDescription = "#rd_challenge_desc_0";
		return;
	}

	KeyValues::AutoDelete pKV( "CHALLENGE" );
	const RD_Challenge_t *pChallenge = ReactiveDropChallenges::GetSummary( szName );
	if ( !pChallenge )
	{
		m_lblError->SetText( "#rd_challenge_selection_error" );
		m_lblError->SetVisible( true );
		return;
	}

	// get local data if available
	( void )ReactiveDropChallenges::ReadData( pKV, szName );

	m_nWorkshopID = pChallenge->WorkshopID;
	const CReactiveDropWorkshop::WorkshopItem_t & item = GetWorkshopItem();

	m_lblName->SetText( pKV->GetString( "name", pChallenge->Title ) );
	m_lblName->SetVisible( true );
	if ( const char *szIcon = pKV->GetString( "icon", NULL ) )
	{
		m_imgIcon->SetImage( "" );
		m_imgIcon->SetImage( szIcon );
		m_imgIcon->SetVisible( true );
	}
	else if ( item.pPreviewImage.IsValid() )
	{
		m_imgIcon->SetImage( const_cast<CReactiveDropWorkshopPreviewImage *>( item.pPreviewImage.GetObject() ) );
		m_imgIcon->SetVisible( true );
	}
	else
	{
		m_imgIcon->SetVisible( false );
	}
	m_szChallengeDescription = pKV->GetString( "description", item.details.m_rgchDescription[0] ? item.details.m_rgchDescription : "#rd_challenge_selection_no_description" );
	m_szChallengeAuthor = pKV->GetString( "author", item.details.m_ulSteamIDOwner ? SteamFriends()->GetFriendPersonaName( item.details.m_ulSteamIDOwner ) : "" );

	if ( m_nWorkshopID == k_PublishedFileIdInvalid )
	{
		m_lblSource->SetText( ReactiveDropChallenges::IsOfficial( szName ) ? "#rd_challenge_selection_source_official" : "#rd_challenge_selection_source_manually_installed" );
	}
	else
	{
		wchar_t wszTitle[k_cchPublishedDocumentTitleMax];
		wszTitle[0] = L'?';
		wszTitle[1] = 0;

		if ( item.details.m_rgchTitle[0] )
		{
			Q_UTF8ToUnicode( item.details.m_rgchTitle, wszTitle, sizeof( wszTitle ) );
		}

		wchar_t wszWorkshopSource[256];
		g_pVGuiLocalize->ConstructString( wszWorkshopSource, sizeof( wszWorkshopSource ), g_pVGuiLocalize->FindSafe( "#rd_challenge_selection_source_workshop" ), 1, wszTitle );
		m_lblSource->SetText( wszWorkshopSource );
	}
	m_lblSource->SetVisible( true );
}

const CReactiveDropWorkshop::WorkshopItem_t &BaseModUI::ReactiveDropChallengeSelectionListItem::GetWorkshopItem()
{
	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		const CReactiveDropWorkshop::WorkshopItem_t & enabledAddon = g_ReactiveDropWorkshop.m_EnabledAddons[i];
		if ( enabledAddon.details.m_nPublishedFileId != m_nWorkshopID )
		{
			continue;
		}

		return enabledAddon;
	}

	static CReactiveDropWorkshop::WorkshopItem_t emptyWorkshopItem;

	return emptyWorkshopItem;
}

BaseModUI::ReactiveDropChallengeSelection::ReactiveDropChallengeSelection( vgui::Panel *parent, const char *panelName, bool bDeathmatch ) : BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 75, 350 );

	m_gplChallenges = new GenericPanelList( this, "GplChallenges", GenericPanelList::ISM_PERITEM );
	m_gplChallenges->SetScrollBarVisible( true );

	m_pBackButton = new CNB_Button( this, "BackButton", "#nb_back", this, "BackButton" );
	m_pBackButton->SetControllerButton( KEY_XBUTTON_B );

	m_lblName = new vgui::Label( this, "LblName", "" );
	m_imgIcon = new vgui::ImagePanel( this, "ImgIcon" );
	m_lblDescription = new vgui::Label( this, "LblDescription", "" );
	m_lblAuthor = new vgui::Label( this, "LblAuthor", "" );

	m_bIgnoreSelectionChange = false;

	m_bDeathmatch = bDeathmatch;

	GetControllerFocus()->PushModal();
	PopulateChallenges();

	LoadControlSettings( "Resource/UI/BaseModUI/ReactiveDropChallengeSelection.res" );
}

BaseModUI::ReactiveDropChallengeSelection::~ReactiveDropChallengeSelection()
{
	m_bIgnoreSelectionChange = true;
	for ( unsigned i = 0; i < m_gplChallenges->GetPanelItemCount(); i++ )
	{
		GetControllerFocus()->RemoveFromFocusList( m_gplChallenges->GetPanelItem( i ) );
	}
	GetControllerFocus()->PopModal();
}

bool BaseModUI::ReactiveDropChallengeSelection::SetSelectedChallenge( const char *szName )
{
	for ( unsigned i = 0; i < m_gplChallenges->GetPanelItemCount(); i++ )
	{
		ReactiveDropChallengeSelectionListItem *pChallenge = assert_cast<ReactiveDropChallengeSelectionListItem *>( m_gplChallenges->GetPanelItem( i ) );
		if ( !Q_stricmp( pChallenge->m_szChallengeName, szName ) )
		{
			SetDetailsForChallenge( pChallenge );
			return m_gplChallenges->SelectPanelItem( i );
		}
	}

	return false;
}

void BaseModUI::ReactiveDropChallengeSelection::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "BackButton" ) )
	{
		MarkForDeletion();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void BaseModUI::ReactiveDropChallengeSelection::OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel )
{
	BaseClass::OnMessage( params, ifromPanel );

	if ( Q_strcmp( params->GetName(), "OnItemSelected" ) == 0 ) 
	{
		if ( m_bIgnoreSelectionChange )
		{
			return;
		}

		int index = const_cast<KeyValues *>( params )->GetInt( "index" );

		SetDetailsForChallenge( assert_cast<ReactiveDropChallengeSelectionListItem *>( m_gplChallenges->GetPanelItem( index ) ) );

		if ( GetControllerFocus()->IsControllerMode() )
		{
			// ensure we can see the previous and next item or controller navigation won't work.
			m_gplChallenges->ScrollToPanelItem( MAX( index - 1, 0 ) );
			m_gplChallenges->ScrollToPanelItem( index + 1 );
			m_gplChallenges->ScrollToPanelItem( index );
		}
	}
}

void BaseModUI::ReactiveDropChallengeSelection::PopulateChallenges()
{
	m_gplChallenges->RemoveAllPanelItems();
	ReactiveDropChallengeSelectionListItem *pDisabled = m_gplChallenges->AddPanelItem<ReactiveDropChallengeSelectionListItem>( "ReactiveDropChallengeSelectionListItem" );
	pDisabled->PopulateChallenge( "0" );
	GetControllerFocus()->AddToFocusList( pDisabled, true, true );

	int iCount = ReactiveDropChallenges::Count();
	for ( int i = 0; i < iCount; i++ )
	{
		const RD_Challenge_t *pChallenge = ReactiveDropChallenges::GetSummary( i );
		if ( m_bDeathmatch ? pChallenge->AllowDeathmatch : pChallenge->AllowCoop )
		{
			ReactiveDropChallengeSelectionListItem *pChallenge = m_gplChallenges->AddPanelItem<ReactiveDropChallengeSelectionListItem>( "ReactiveDropChallengeSelectionListItem" );
			pChallenge->PopulateChallenge( ReactiveDropChallenges::Name( i ) );
			GetControllerFocus()->AddToFocusList( pChallenge, true, true );
		}
	}
}

void BaseModUI::ReactiveDropChallengeSelection::SetDetailsForChallenge( ReactiveDropChallengeSelectionListItem *pChallenge )
{
	GetParent()->OnCommand( VarArgs( "cmd_challenge_selected_%s", pChallenge->m_szChallengeName.Get() ) );

	if ( pChallenge->m_lblError->IsVisible() )
	{
		m_lblName->SetVisible( false );
		m_imgIcon->SetVisible( false );
		m_lblDescription->SetVisible( false );
		m_lblAuthor->SetVisible( false );
		return;
	}

	wchar_t wszName[256];
	pChallenge->m_lblName->GetText( wszName, sizeof( wszName ) );
	m_lblName->SetText( wszName );
	m_lblName->SetVisible( true );

	if ( pChallenge->m_imgIcon->IsVisible() )
	{
		m_imgIcon->SetImage( pChallenge->m_imgIcon->GetImage() );
		m_imgIcon->SetVisible( true );
	}
	else
	{
		m_imgIcon->SetVisible( false );
	}

	m_lblDescription->SetText( pChallenge->m_szChallengeDescription.Get() );
	m_lblDescription->SetVisible( true );

	const CReactiveDropWorkshop::WorkshopItem_t &item = pChallenge->GetWorkshopItem();
	if ( item.details.m_nPublishedFileId )
	{
		const char *szName = SteamFriends()->GetFriendPersonaName( item.details.m_ulSteamIDOwner );
		wchar_t wszAuthorName[k_cwchPersonaNameMax];
		Q_UTF8ToUnicode( szName, wszAuthorName, sizeof( wszAuthorName ) );

		wchar_t wszAuthor[256];
		g_pVGuiLocalize->ConstructString( wszAuthor, sizeof( wszAuthor ), g_pVGuiLocalize->FindSafe( "#rd_challenge_selection_author" ), 1, wszAuthorName );

		m_lblAuthor->SetText( wszAuthor );
		m_lblAuthor->SetVisible( true );
	}
	else if ( ReactiveDropChallenges::IsOfficial( pChallenge->m_szChallengeName.Get() ) )
	{
		m_lblAuthor->SetText( "#rd_challenge_selection_author_official" );
		m_lblAuthor->SetVisible( true );
	}
	else if ( Q_strcmp( pChallenge->m_szChallengeAuthor.Get(), "" ) )
	{
		wchar_t kvzName[k_cwchPersonaNameMax];
		Q_UTF8ToUnicode( pChallenge->m_szChallengeAuthor.Get(), kvzName, sizeof( kvzName ) );
		wchar_t kvzAuthor[256];
		g_pVGuiLocalize->ConstructString( kvzAuthor, sizeof( kvzAuthor ), g_pVGuiLocalize->FindSafe( "#rd_challenge_selection_author" ), 1, kvzName );
		m_lblAuthor->SetText( kvzAuthor );
		m_lblAuthor->SetVisible( true );
	}
	else
	{
		m_lblAuthor->SetVisible( false );
	}
}
