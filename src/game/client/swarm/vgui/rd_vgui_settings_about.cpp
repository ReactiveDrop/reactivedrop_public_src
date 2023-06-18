#include "cbase.h"
#include "rd_vgui_settings.h"
#include "gameui/swarm/vgenericpanellist.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef IS_WINDOWS_PC
extern const char *( *const wine_get_version )( void );
#endif

static constexpr const char *s_szCopyrightStrings[] =
{
	"#rd_about_rdteam",
	"#rd_about_valve",
	"#GameUI_Miles_Audio",
	"#GameUI_Miles_Voice",
	"#GameUI_Bink",
	"#rd_about_havok",
	"#rd_about_nvidia",
};

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_About );

CRD_VGUI_Settings_About::CRD_VGUI_Settings_About( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_pLblBuildID = new vgui::Label( this, "LblBuildID", "" );
	m_pLblNetworkVersion = new vgui::Label( this, "LblNetworkVersion", "" );
	m_pLblCurrentBranch = new vgui::Label( this, "LblCurrentBranch", "" );
	m_pLblWineVersion = new vgui::Label( this, "LblWineVersion", "" );

	m_pImgSourceEngine = new vgui::ImagePanel( this, "ImgSourceEngine" );

	m_pCopyrightDisclaimers = new BaseModUI::GenericPanelList( this, "CopyrightDisclaimers", BaseModUI::GenericPanelList::ISM_ELEVATOR );
	for ( int i = 0; i < NELEMS( s_szCopyrightStrings ); i++ )
	{
		vgui::Label *pLabel = m_pCopyrightDisclaimers->AddPanelItem<vgui::Label>( "CopyrightDisclaimer", s_szCopyrightStrings[i] );
		pLabel->SetWrap( true );
	}

	MakeReadyForUse();
}

void CRD_VGUI_Settings_About::Activate()
{
	ISteamApps *pApps = SteamApps();
	Assert( pApps );

	wchar_t wszBuf[256];
	wchar_t wszParam[64];

	if ( !pApps )
	{
		V_wcsncpy( wszParam, L"?", sizeof( wszParam ) );
	}
	else
	{
		V_snwprintf( wszParam, NELEMS( wszParam ), L"%d", pApps->GetAppBuildId() );
	}
	g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ), g_pVGuiLocalize->Find( "#rd_about_build_id" ), 1, wszParam );
	m_pLblBuildID->SetText( wszBuf );

	V_UTF8ToUnicode( engine->GetProductVersionString(), wszParam, sizeof( wszParam ) );
	g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ), g_pVGuiLocalize->Find( "#rd_about_network_version" ), 1, wszParam );
	m_pLblNetworkVersion->SetText( wszBuf );

	char szBranch[64]{};
	if ( !pApps )
	{
		V_wcsncpy( wszParam, L"?", sizeof( wszParam ) );
	}
	else if ( pApps->GetCurrentBetaName( szBranch, sizeof( szBranch ) ) || szBranch[0] != '\0' )
	{
		V_UTF8ToUnicode( szBranch, wszParam, sizeof( wszParam ) );
	}
	else
	{
		V_wcsncpy( wszParam, L"default", sizeof( wszParam ) );
	}

	g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ), g_pVGuiLocalize->Find( "#rd_about_current_branch" ), 1, wszParam );
	m_pLblCurrentBranch->SetText( wszBuf );

#ifdef IS_WINDOWS_PC
	if ( wine_get_version )
	{
		V_UTF8ToUnicode( wine_get_version(), wszParam, sizeof( wszParam ) );
		g_pVGuiLocalize->ConstructString( wszBuf, sizeof( wszBuf ), g_pVGuiLocalize->Find( "#rd_about_wine_version" ), 1, wszParam );
		m_pLblWineVersion->SetText( wszBuf );
		m_pLblWineVersion->SetVisible( true );
	}
	else
#endif
	{
		m_pLblWineVersion->SetVisible( false );
	}
}

void CRD_VGUI_Settings_About::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );


	for ( int i = 0; i < m_pCopyrightDisclaimers->GetPanelItemCount(); i++ )
	{
		m_pCopyrightDisclaimers->GetPanelItem( i )->SetFgColor( m_pCopyrightDisclaimers->GetFgColor() );
	}
}

void CRD_VGUI_Settings_About::PerformLayout()
{
	BaseClass::PerformLayout();

	for ( int i = 0; i < m_pCopyrightDisclaimers->GetPanelItemCount(); i++ )
	{
		vgui::Label *pLabel = assert_cast< vgui::Label * >( m_pCopyrightDisclaimers->GetPanelItem( i ) );
		pLabel->SizeToContents();
	}

	m_pCopyrightDisclaimers->InvalidateLayout();
}
