#include "cbase.h"
#include "vdemos.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "vfooterpanel.h"
#include "vdropdownmenu.h"
#include "vgenericconfirmation.h"
#include "vgenericpanellist.h"
#include "vpasswordentry.h"
#include "rd_demo_utils.h"
#include "rd_missions_shared.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui_controls/ImagePanel.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

extern ConVar rd_auto_record_lobbies;

class DemoInfoPanel final : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( DemoInfoPanel, vgui::EditablePanel );
public:
	DemoInfoPanel( vgui::Panel *parent, const char *panelName ) :
		BaseClass( parent, panelName )
	{
		SetProportional( true );

		m_szFileName[0] = '\0';
		m_bWatchable = false;
		m_bCurrentlySelected = false;

		m_LblName = new vgui::Label( this, "LblName", "" );
		m_LblName->SetMouseInputEnabled( false );
		m_LblFileSize = new vgui::Label( this, "LblFileSize", "" );
		m_LblFileSize->SetMouseInputEnabled( false );
		m_LblError = new vgui::Label( this, "LblError", "" );
		m_LblError->SetMouseInputEnabled( false );
		m_LblDuration = new vgui::Label( this, "LblDuration", "" );
		m_LblDuration->SetMouseInputEnabled( false );
		m_LblRecordedBy = new vgui::Label( this, "LblRecordedBy", "" );
		m_LblRecordedBy->SetMouseInputEnabled( false );
		m_LblMissionName = new vgui::Label( this, "LblMissionName", "" );
		m_LblMissionName->SetMouseInputEnabled( false );
		m_ImgMissionIcon = new vgui::ImagePanel( this, "ImgMissionIcon" );
		m_ImgMissionIcon->SetMouseInputEnabled( false );
		m_LblAutoDeletionWarning = new vgui::Label( this, "LblAutoDeletionWarning", "" );
		m_LblAutoDeletionWarning->SetMouseInputEnabled( false );
	}

	bool IsLabel() override { return false; }

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override
	{
		BaseClass::ApplySchemeSettings( pScheme );

		static KeyValues *s_pPreloadedDemoInfoItemLayout = NULL;

#ifdef _DEBUG
		if ( s_pPreloadedDemoInfoItemLayout )
		{
			s_pPreloadedDemoInfoItemLayout->deleteThis();
			s_pPreloadedDemoInfoItemLayout = NULL;
		}
#endif

		if ( !s_pPreloadedDemoInfoItemLayout )
		{
			const char *pszResource = "Resource/UI/BaseModUI/DemoListItem.res";
			s_pPreloadedDemoInfoItemLayout = new KeyValues( pszResource );
			s_pPreloadedDemoInfoItemLayout->LoadFromFile( g_pFullFileSystem, pszResource );
		}

		LoadControlSettings( "", NULL, s_pPreloadedDemoInfoItemLayout );
	}

	void OnMousePressed( vgui::MouseCode code ) override
	{
		if ( MOUSE_LEFT == code )
		{
			GenericPanelList *pGenericList = dynamic_cast< GenericPanelList * >( GetParent()->GetParent() );

			unsigned short nindex;
			if ( pGenericList && pGenericList->GetPanelItemIndex( this, nindex ) )
			{
				pGenericList->SelectPanelItem( nindex );
			}
		}

		BaseClass::OnMousePressed( code );
	}

	void OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel ) override
	{
		BaseClass::OnMessage( params, ifromPanel );

		if ( !V_strcmp( params->GetName(), "PanelSelected" ) )
		{
			m_bCurrentlySelected = true;
		}
		else if ( !V_strcmp( params->GetName(), "PanelUnSelected" ) )
		{
			m_bCurrentlySelected = false;
		}
	}

	void Paint() override
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
			vgui::surface()->DrawFilledRectFade( 0, nPanelTall - 2, 0.5f * nPanelWide, nPanelTall, 0, 255, true );
			vgui::surface()->DrawFilledRectFade( 0.5f * nPanelWide, nPanelTall - 2, nPanelWide, nPanelTall, 255, 0, true );

			// Text Blotch
			int nTextWide, nTextTall, nNameX, nNameY, nNameWide, nNameTall;
			wchar_t wszName[MAX_PATH];

			m_LblName->GetPos( nNameX, nNameY );
			m_LblName->GetSize( nNameWide, nNameTall );
			m_LblName->GetText( wszName, sizeof( wszName ) );
			vgui::surface()->GetTextSize( m_LblName->GetFont(), wszName, nTextWide, nTextTall );
			int nBlotchWide = nNameX + nTextWide + vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 75 );

			vgui::surface()->DrawFilledRectFade( 0, 2, 0.50f * nBlotchWide, nPanelTall - 2, 0, 50, true );
			vgui::surface()->DrawFilledRectFade( 0.50f * nBlotchWide, 2, nBlotchWide, nPanelTall - 2, 50, 0, true );
		}
	}

	void PerformLayout() override
	{
		BaseClass::PerformLayout();

		int w, t;
		m_LblMissionName->GetContentSize( w, t );
		int x, y;
		m_LblMissionName->GetPos( x, y );
		y += m_LblMissionName->GetTall() - t;
		m_LblMissionName->SetPos( x, y );
		m_LblMissionName->SetTall( t );
	}

	void SetDemoInfo( const RD_Demo_Info_t &info, bool bJustBaseName )
	{
		static RD_Auto_Recording_t s_AutoRecordTest;
		m_bAutoRecording = s_AutoRecordTest.Parse( info.szFileName, true );

		V_strncpy( m_szFileName, info.szFileName, sizeof( m_szFileName ) );

		wchar_t buf[255];

		if ( bJustBaseName )
		{
			char szBaseName[MAX_PATH];
			V_FileBase( info.szFileName, szBaseName, sizeof( szBaseName ) );
			V_UTF8ToUnicode( szBaseName, buf, sizeof( buf ) );
		}
		else
		{
			V_UTF8ToUnicode( info.szFileName, buf, sizeof( buf ) );
		}
		m_LblName->SetText( buf );

		wchar_t wszSize[64];
		V_snwprintf( wszSize, NELEMS( wszSize ), L"%.1f", info.nFileSize / 1024.0f / 1024.0f );
		g_pVGuiLocalize->ConstructString( buf, sizeof( buf ),
			g_pVGuiLocalize->Find( "#rd_demo_size" ), 1, wszSize );
		m_LblFileSize->SetText( buf );

		if ( info.wszCantWatchReason[0] != L'\0' )
		{
			m_bWatchable = false;
			m_LblError->SetText( info.wszCantWatchReason );

			m_LblName->SetAlpha( 128 );
			m_LblFileSize->SetAlpha( 128 );
		}
		else
		{
			m_bWatchable = true;
			V_snwprintf( buf, NELEMS( buf ), L"%d:%06.3f", Floor2Int( info.Header.playback_time / 60 ), fmodf( info.Header.playback_time, 60 ) );
			m_LblDuration->SetText( buf );
			V_UTF8ToUnicode( info.Header.clientname, buf, sizeof( buf ) );
			m_LblRecordedBy->SetText( buf );

			m_LblName->SetAlpha( 255 );
			m_LblFileSize->SetAlpha( 255 );
		}

		m_LblMissionName->SetText( info.pMission ? STRING( info.pMission->MissionTitle ) : info.Header.mapname );
		m_ImgMissionIcon->SetImage( info.pMission ? STRING( info.pMission->Image ) : "swarm/missionpics/unknownmissionpic" );
	}

	char m_szFileName[MAX_PATH];
	bool m_bWatchable;
	bool m_bCurrentlySelected;
	bool m_bAutoRecording;

	vgui::Label *m_LblName;
	vgui::Label *m_LblFileSize;
	vgui::Label *m_LblError;
	vgui::Label *m_LblDuration;
	vgui::Label *m_LblRecordedBy;
	vgui::Label *m_LblMissionName;
	vgui::ImagePanel *m_ImgMissionIcon;
	vgui::Label *m_LblAutoDeletionWarning;
};

Demos::Demos( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName, false, true )
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 75, 350 );

	m_LblNoRecordings = new vgui::Label( this, "LblNoRecordings", "#rd_demo_list_empty" );

	m_GplRecordingList = new GenericPanelList( this, "GplRecordingList", GenericPanelList::ISM_PERITEM );
	m_GplRecordingList->SetScrollBarVisible( true );

	m_DrpAutoRecord = new DropDownMenu( this, "DrpAutoRecord" );
	m_LblAutoRecordWarning = new vgui::Label( this, "LblAutoRecordWarning", "" );

	m_BtnWatch = new CNB_Button( this, "BtnWatch", "#rd_demo_action_watch" );
	m_BtnWatch->SetControllerButton( KEY_XBUTTON_A );
	m_BtnCancel = new CNB_Button( this, "BtnCancel", "#nb_back" );
	m_BtnCancel->SetControllerButton( KEY_XBUTTON_B );
	m_BtnDelete = new CNB_Button( this, "BtnDelete", "#rd_demo_action_delete" );
	m_BtnDelete->SetControllerButton( KEY_XBUTTON_X );
	m_BtnRename = new CNB_Button( this, "BtnRename", "#rd_demo_action_rename" );
	m_BtnRename->SetControllerButton( KEY_XBUTTON_Y );

	SetLowerGarnishEnabled( true );
	m_ActiveControl = m_GplRecordingList;

	LoadControlSettings( "Resource/UI/BaseModUI/demos.res" );
}

Demos::~Demos()
{
	GameUI().AllowEngineHideGameUI();
}

void Demos::Activate()
{
	BaseClass::Activate();

	MakeReadyForUse();

	CUtlVector<RD_Demo_Info_t> DemoList;
	g_RD_Auto_Record_System.ReadDemoList( DemoList );

	bool bJustBaseName = true;
	FOR_EACH_VEC( DemoList, i )
	{
		const char *szSuffix = StringAfterPrefix( DemoList[i].szFileName, "recordings/" );
		if ( !szSuffix || strchr( szSuffix, '/' ) )
		{
			bJustBaseName = false;
			break;
		}
	}

	m_GplRecordingList->RemoveAllPanelItems();
	FOR_EACH_VEC( DemoList, i )
	{
		m_GplRecordingList->AddPanelItem<DemoInfoPanel>( "DemoListItem" )->SetDemoInfo( DemoList[i], bJustBaseName );
	}

	if ( DemoList.Count() > 0 )
	{
		m_GplRecordingList->NavigateTo();
		m_GplRecordingList->SelectPanelItem( 0 );
		m_LblNoRecordings->SetVisible( false );
		m_BtnRename->SetVisible( true );
		m_BtnDelete->SetVisible( true );
	}
	else
	{
		m_LblNoRecordings->SetVisible( true );
		m_BtnWatch->SetEnabled( false );
		m_BtnRename->SetVisible( false );
		m_BtnDelete->SetVisible( false );
	}

	m_GplRecordingList->InvalidateLayout();

	m_DrpAutoRecord->SetCurrentSelection( VarArgs( "#rd_demo_auto_setting_%d", rd_auto_record_lobbies.GetInt() ) );

	UpdateWarnings();
}

void Demos::UpdateWarnings()
{
	bool bWillAutoDelete = false;
	if ( rd_auto_record_lobbies.GetInt() < 0 )
	{
		m_LblAutoRecordWarning->SetText( "#rd_demo_auto_warning_unlimited" );
	}
	else if ( rd_auto_record_lobbies.GetInt() > 0 )
	{
		m_LblAutoRecordWarning->SetText( "#rd_demo_auto_warning_generic" );
		bWillAutoDelete = true;
	}
	else
	{
		m_LblAutoRecordWarning->SetText( "" );
	}

	for ( unsigned short i = 0; i < m_GplRecordingList->GetPanelItemCount(); i++ )
	{
		DemoInfoPanel *pEntry = assert_cast<DemoInfoPanel *>( m_GplRecordingList->GetPanelItem( i ) );
		pEntry->m_LblAutoDeletionWarning->SetVisible( bWillAutoDelete && pEntry->m_bAutoRecording );
	}
}

static char s_szRecordingToRenameOrDelete[MAX_PATH];
static void DeleteRecordingCallback()
{
	g_pFullFileSystem->RemoveFile( s_szRecordingToRenameOrDelete, "MOD" );
	s_szRecordingToRenameOrDelete[0] = '\0';

	if ( CBaseModFrame *pFrame = CBaseModPanel::GetSingleton().GetWindow( WT_DEMOS ) )
		pFrame->Activate();

	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
}

static void RenameRecordingCallback()
{
	PasswordEntry *pEntry = assert_cast< PasswordEntry * >( CBaseModPanel::GetSingleton().GetWindow( WT_PASSWORDENTRY ) );
	Assert( pEntry );

	char szNewBase[MAX_PATH];
	pEntry->GetPassword( szNewBase, sizeof( szNewBase ) );

	// replace slashes with a safe character
	V_FixSlashes( szNewBase, '_' );

	CUtlString szNewName = CUtlString::PathJoin( "recordings", szNewBase ) + ".dem";

	g_pFullFileSystem->RenameFile( s_szRecordingToRenameOrDelete, szNewName, "MOD" );
	s_szRecordingToRenameOrDelete[0] = '\0';

	if ( CBaseModFrame *pFrame = CBaseModPanel::GetSingleton().GetWindow( WT_DEMOS ) )
		pFrame->Activate();

	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
}

void Demos::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	DemoInfoPanel *pEntry = assert_cast< DemoInfoPanel * >( m_GplRecordingList->GetSelectedPanelItem() );

	vgui::KeyCode code = GetBaseButtonCode( keycode );
	switch ( code )
	{
	case KEY_XBUTTON_A:
		if ( !pEntry || !pEntry->m_bWatchable )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );
			break;
		}

		engine->ClientCmd_Unrestricted( VarArgs( "playdemo \"%s\"\n", pEntry->m_szFileName ) );
		MarkForDeletion();

		break;
	case KEY_XBUTTON_X:
		if ( !pEntry )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );
			break;
		}

		{
			V_strncpy( s_szRecordingToRenameOrDelete, pEntry->m_szFileName, sizeof( s_szRecordingToRenameOrDelete ) );

			GenericConfirmation *pConfirmation = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, true ) );

			GenericConfirmation::Data_t data;

			wchar_t wszFileName[MAX_PATH];
			V_UTF8ToUnicode( pEntry->m_szFileName, wszFileName, sizeof( wszFileName ) );

			static wchar_t s_wszMessage[1024];
			g_pVGuiLocalize->ConstructString( s_wszMessage, sizeof( s_wszMessage ),
				g_pVGuiLocalize->Find( "#rd_demo_confirm_delete_message" ), 1, wszFileName );

			data.bOkButtonEnabled = true;
			data.bCancelButtonEnabled = true;
			data.pWindowTitle = "#rd_demo_confirm_delete_title";
			data.pMessageTextW = s_wszMessage;
			data.pfnOkCallback = &DeleteRecordingCallback;

			pConfirmation->SetUsageData( data );

			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
		}

		break;
	case KEY_XBUTTON_Y:
		if ( !pEntry )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );
			break;
		}

		{
			V_strncpy( s_szRecordingToRenameOrDelete, pEntry->m_szFileName, sizeof( s_szRecordingToRenameOrDelete ) );

			PasswordEntry *pTextEntry = assert_cast< PasswordEntry * >( CBaseModPanel::GetSingleton().OpenWindow( WT_PASSWORDENTRY, this, true ) );

			PasswordEntry::Data_t data;

			char szBaseName[MAX_PATH];
			V_FileBase( pEntry->m_szFileName, szBaseName, sizeof( szBaseName ) );

			data.pWindowTitle = "#rd_demo_rename_title";
			data.pMessageText = "#rd_demo_rename_message";
			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &RenameRecordingCallback;
			data.bCancelButtonEnabled = true;
			data.bShowPassword = true;
			data.m_szCurrentPW = szBaseName;

			pTextEntry->SetUsageData( data );

			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
		}

		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void Demos::OnCommand( const char *command )
{
	if ( FStrEq( command, "Back" ) )
	{
		// Act as though 360 back button was pressed
		OnKeyCodePressed( KEY_XBUTTON_B );
	}
	else if ( FStrEq( command, "Watch" ) )
	{
		OnKeyCodePressed( KEY_XBUTTON_A );
	}
	else if ( FStrEq( command, "Delete" ) )
	{
		OnKeyCodePressed( KEY_XBUTTON_X );
	}
	else if ( FStrEq( command, "Rename" ) )
	{
		OnKeyCodePressed( KEY_XBUTTON_Y );
	}
	else if ( const char *szValue = StringAfterPrefix( command, "!rd_auto_record_lobbies " ) )
	{
		rd_auto_record_lobbies.SetValue( szValue );
		engine->ExecuteClientCmd( "host_writeconfig\n" );
		UpdateWarnings();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void Demos::OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel )
{
	BaseClass::OnMessage( params, ifromPanel );

	if ( !V_strcmp( params->GetName(), "OnItemSelected" ) )
	{
		int index = const_cast< KeyValues * >( params )->GetInt( "index" );
		DemoInfoPanel *pEntry = assert_cast< DemoInfoPanel * >( m_GplRecordingList->GetPanelItem( index ) );

		m_BtnWatch->SetEnabled( pEntry->m_bWatchable );
	}
}

void Demos::OnThink()
{
	BaseClass::OnThink();
}

void Demos::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetupAsDialogStyle();
}

void Demos::PerformLayout()
{
	BaseClass::PerformLayout();

	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );
}

CON_COMMAND_F( rd_auto_record_ui, "Displays demo list.", FCVAR_NOT_CONNECTED )
{
	CBaseModPanel::GetSingleton().OpenWindow( WT_DEMOS, CBaseModPanel::GetSingleton().GetWindow( CBaseModPanel::GetSingleton().GetActiveWindowType() ) );
}
