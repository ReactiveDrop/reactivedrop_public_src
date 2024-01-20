#include "cbase.h"
#include "vgenericchoicelist.h"
#include "nb_header_footer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

GenericChoiceList::GenericChoiceList( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName }
{
	SetProportional( true );
	SetDeleteSelfOnClose( true );
	SetCancelButtonEnabled( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( false );
	m_pHeaderFooter->SetMovieEnabled( false );
	m_pHeaderFooter->SetGradientBarEnabled( true );
}

GenericChoiceList::~GenericChoiceList()
{
}

void GenericChoiceList::SetDataSettings( KeyValues *pSettings )
{
	Assert( !m_pSettings );
	Assert( m_Buttons.Count() == 0 );
	m_pSettings.Assign( pSettings );

	FOR_EACH_SUBKEY( pSettings, pAction )
	{
		Assert( !V_stricmp( pAction->GetName(), "Action" ) );
		if ( V_stricmp( pAction->GetName(), "Action" ) )
		{
			continue;
		}

		ChoiceButton *pButton = new ChoiceButton( this, "ChoiceButton", pAction->GetWString( "label" ), ( ChoiceCallback_t )pAction->GetPtr( "do" ), pAction );
		pButton->SetConsoleStylePanel( true );
		pButton->SetStyle( BaseModHybridButton::BUTTON_FLYOUTITEM );
		m_Buttons.AddToTail( pButton );
	}

	InvalidateLayout();
}

void GenericChoiceList::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_Buttons.Count() == 0 )
	{
		return;
	}

	int iTotalTall = m_Buttons.Count() * m_iButtonTall + ( m_Buttons.Count() - 1 ) * m_iButtonSpacing;

	int y = ( GetTall() - iTotalTall ) / 2;

	m_pHeaderFooter->SetBackgroundStyle( ( NB_Background_Style )m_iBackgroundStyle );
	m_pHeaderFooter->SetGradientBarPos( vgui::scheme()->GetProportionalNormalizedValue( y - m_iPaddingY ), vgui::scheme()->GetProportionalNormalizedValue( iTotalTall + m_iPaddingY * 2 ) );
	m_pHeaderFooter->SetGradientBarWide( vgui::scheme()->GetProportionalNormalizedValue( m_iButtonWide + m_iPaddingX * 2 ) );
	m_pHeaderFooter->SetGradientBarColor( m_InnerColor );

	ChoiceButton *pLastButton = NULL;
	FOR_EACH_VEC( m_Buttons, i )
	{
		ChoiceButton *pButton = m_Buttons[i];
		pButton->SetBounds( ( GetWide() - m_iButtonWide ) / 2, y, m_iButtonWide, m_iButtonTall );
		y += m_iButtonTall + m_iButtonSpacing;

		if ( pLastButton )
		{
			pButton->SetNavUp( pLastButton );
			pLastButton->SetNavDown( pButton );
		}
		else
		{
			SetNavDown( pButton );
		}
		pLastButton = pButton;
	}

	SetNavUp( pLastButton );
}

void GenericChoiceList::OnKeyCodePressed( vgui::KeyCode keycode )
{
	BaseClass::OnKeyCodePressed( keycode );

	vgui::KeyCode code = GetBaseButtonCode( keycode );
	if ( code == KEY_XBUTTON_B || code == KEY_ESCAPE || code == KEY_XBUTTON_START )
	{
		Close();
	}
}

ChoiceButton::ChoiceButton( vgui::Panel *parent, const char *panelName, const wchar_t *wszLabel, ChoiceCallback_t callback, KeyValues *pAction )
	: BaseClass{ parent, panelName, wszLabel, this, "ChoiceButtonPressed" }
{
	m_callback = callback;
	m_pAction = pAction;

	SetEnabled( m_callback != NULL );
}

ChoiceButton::~ChoiceButton()
{
}

void ChoiceButton::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void ChoiceButton::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "ChoiceButtonPressed" ) )
	{
		if ( m_callback )
		{
			m_callback( m_pAction );
		}

		assert_cast< GenericChoiceList * >( GetParent() )->Close();

		return;
	}

	BaseClass::OnCommand( command );
}
