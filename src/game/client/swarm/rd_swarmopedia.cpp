#include "cbase.h"
#include "rd_swarmopedia.h"
#include "animation.h"
#include "gameui_interface.h"
#include "gameui/swarm/vfooterpanel.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "vgui/ILocalize.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/TextImage.h>
#include <missionchooser/iasw_mission_chooser.h>
#include <missionchooser/iasw_mission_chooser_source.h>
#include "rd_challenges_shared.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

#define SWARMOPEDIA_PATH "resource/swarmopedia.txt"

DECLARE_BUILD_FACTORY( Swarmopedia_Model_Panel );
DECLARE_BUILD_FACTORY( Swarmopedia );

Swarmopedia_Model_Panel::Swarmopedia_Model_Panel( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
}

Swarmopedia_Model_Panel::~Swarmopedia_Model_Panel()
{
}

void Swarmopedia_Model_Panel::ApplyConfig( KeyValues *pKV )
{
	// Set the first model for scaling.
	SetMDL( pKV->GetString( "Model/ModelName" ) );

	FOR_EACH_SUBKEY( pKV, pModel )
	{
		if ( !V_stricmp( pModel->GetName(), "Model" ) )
		{
			int iModel = m_Models.AddToTail();
			if ( !m_Models.IsValidIndex( iModel ) )
			{
				DevWarning( "Could not add model\n" );
				return;
			}

			const model_t *pWorldModel = modelinfo->FindOrLoadModel( pModel->GetString( "ModelName" ) );
			MDLHandle_t hStudioHdr = pWorldModel ? modelinfo->GetCacheHandle( pWorldModel ) : MDLHANDLE_INVALID;
			const studiohdr_t *pStudioHdr = hStudioHdr == MDLHANDLE_INVALID ? NULL : mdlcache->GetStudioHdr( hStudioHdr );
			if ( !pStudioHdr )
			{
				DevWarning( "Could not load model %s\n", pModel->GetString( "ModelName" ) );
				KeyValuesDumpAsDevMsg( pKV );
				continue;
			}
			CStudioHdr studioHdr( pStudioHdr, mdlcache );

			m_Models[iModel].m_MDL.SetMDL( hStudioHdr );
			m_Models[iModel].m_MDL.m_nSequence = LookupSequence( &studioHdr, pModel->GetString( "Animation" ) );
			m_Models[iModel].m_MDL.m_nSkin = pModel->GetInt( "Skin", -1 );
			m_Models[iModel].m_MDL.m_Color = pModel->GetColor( "Color", Color( 255, 255, 255, 255 ) );

			const QAngle angles( pModel->GetFloat( "pitch" ), pModel->GetFloat( "yaw" ), pModel->GetFloat( "roll" ) );
			const Vector position( pModel->GetFloat( "x" ), pModel->GetFloat( "y" ), pModel->GetFloat( "z" ) );
			matrix3x4_t anglePos, scale;
			AngleMatrix( angles, position, anglePos );
			SetScaleMatrix( pModel->GetFloat( "scale", 1.0f ), scale );
			ConcatTransforms( scale, anglePos, m_Models[iModel].m_MDLToWorld );

			FOR_EACH_SUBKEY( pModel, pKey )
			{
				if ( !V_stricmp( pKey->GetName(), "BodyGroup" ) )
				{
					::SetBodygroup( &studioHdr, m_Models[iModel].m_MDL.m_nBody, pKey->GetInt( "Group" ), pKey->GetInt( "Value" ) );
				}
			}
		}
	}
}

void Swarmopedia_Model_Panel::OnPaint3D()
{
	Vector vecPos;
	Vector vecCenter;
	float flRadius;
	QAngle angRot( 32.0, 0.0, 0.0 );
	Vector vecOffset;

	AngleVectors( angRot, &vecOffset );
	GetBoundingSphere( vecCenter, flRadius );
	VectorMA( vecCenter, -3.5f * flRadius, vecOffset, vecPos );

	float flTime = Plat_FloatTime();

	SetCameraPositionAndAngles( vecPos, angRot );
	SetModelAnglesAndPosition( QAngle( 0.0f, flTime * 30.0f , 0.0f ), vec3_origin );

	FOR_EACH_VEC( m_Models, i )
	{
		matrix3x4_t mat;
		ConcatTransforms( m_RootMDL.m_MDLToWorld, m_Models[i].m_MDLToWorld, mat );
		m_Models[i].m_MDL.m_flTime = flTime;
		m_Models[i].m_MDL.Draw( mat );
	}
}

Swarmopedia::Swarmopedia( Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_iSelectedAlien = 0;
	m_iSelectedModel = 0;

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pModelCaption = new CNB_Button( this, "ModelCaption", "", this, "NextModel" );
	m_pModelCaption->SetControllerButton( KEY_XBUTTON_X );
	m_pLblName = new Label( this, "LblName", "" );
	m_pLblAbilities = new Label( this, "LblAbilities", "" );
	m_pGplAliens = new GenericPanelList( this, "GplAliens", GenericPanelList::ISM_ELEVATOR );
	m_pGplParagraphs = new GenericPanelList( this, "GplParagraphs", GenericPanelList::ISM_ELEVATOR );
	m_pGplAppears = new GenericPanelList( this, "GplAppears", GenericPanelList::ISM_ELEVATOR );

	LoadAllAliens();

	LoadControlSettings( "Resource/UI/BaseModUI/Swarmopedia.res" );

	m_pHeaderFooter->SetTitle( "#asw_swarmopedia" );
}

Swarmopedia::~Swarmopedia()
{
	GameUI().AllowEngineHideGameUI();
}

void Swarmopedia::Clear()
{
	m_pGplAliens->RemoveAllPanelItems();
	m_Aliens.PurgeAndDeleteElements();
}

static void LoadAliensCallback( const char *pszPath, KeyValues *pKV, void *pUserData )
{
	DevMsg( "Loading aliens for Swarmopedia from filesystem: %s\n", pszPath );

	static_cast< Swarmopedia * >( pUserData )->LoadAliens( pKV );
}

void Swarmopedia::LoadAllAliens()
{
	// Unlike most files, the Swarmopedia is constructed from *all* paths, not just the last one.

	Clear();

	UTIL_RD_LoadAllKeyValues( SWARMOPEDIA_PATH, "GAME", "Swarmopedia", &LoadAliensCallback, this );

	FOR_EACH_VEC( m_Aliens, i )
	{
		Button *pButton = m_pGplAliens->AddPanelItem<Button>( "BtnAlienSelection", m_Aliens[i]->m_szName.Get(), this, CFmtStr( "SelectAlien%d", i ) );
		pButton->SetPaintBackgroundEnabled( false );
	}

	InvalidateLayout( true, true );
}

void Swarmopedia::LoadAliens( KeyValues *pKV )
{
	FOR_EACH_TRUE_SUBKEY( pKV, pAlien )
	{
		if ( V_stricmp( pAlien->GetName(), "ALIEN" ) )
		{
			continue;
		}

		const char *szID = pAlien->GetString( "ID", NULL );

		if ( !szID )
		{
			DevWarning( "Alien missing ID!\n" );
			KeyValuesDumpAsDevMsg( pAlien );
			continue;
		}

		bool bFound = false;
		FOR_EACH_VEC( m_Aliens, i )
		{
			if ( !V_stricmp( m_Aliens[i]->m_szID.Get(), szID ) )
			{
				m_Aliens[i]->Merge( this, pAlien );
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			m_Aliens.AddToTail( new Alien_t( this, pAlien ) );
		}
	}
}

static void WrapLabel( Label *pLabel )
{
	pLabel->SetWrap( true );
	pLabel->GetParent()->GetParent()->InvalidateLayout( true );
	pLabel->GetTextImage()->ResizeImageToContentMaxWidth( pLabel->GetWide() );
	int wide, tall;
	pLabel->GetContentSize( wide, tall );
	pLabel->SetSize( wide, tall );
}

void Swarmopedia::SetSelectedAlien( int iSelection )
{
	Assert( iSelection >= 0 && iSelection < m_Aliens.Count() );

	if ( m_Aliens[m_iSelectedAlien]->m_DisplayPanel.Count() > m_iSelectedModel )
	{
		m_Aliens[m_iSelectedAlien]->m_DisplayPanel[m_iSelectedModel]->SetVisible( false );
	}

	m_iSelectedAlien = iSelection;
	m_iSelectedModel = 0;

	if ( m_Aliens[m_iSelectedAlien]->m_DisplayPanel.Count() != 0 )
	{
		m_Aliens[m_iSelectedAlien]->m_DisplayPanel[m_iSelectedModel]->SetVisible( true );
		m_pModelCaption->SetText( m_Aliens[m_iSelectedAlien]->m_DisplayCaption[m_iSelectedModel] );
		m_pModelCaption->SetVisible( true );
	}
	else
	{
		m_pModelCaption->SetVisible( false );
	}

	m_pLblName->SetText( m_Aliens[m_iSelectedAlien]->m_szName.Get() );
	wchar_t wszAbilities[2048] = { 0 };
	wchar_t wszAbility[512];
	FOR_EACH_VEC( m_Aliens[m_iSelectedAlien]->m_Abilities, i )
	{
		if ( i != 0 )
		{
			int len = V_wcslen( wszAbilities );
			V_wcsncpy( wszAbilities + len, L" \u2022 ", sizeof( wszAbilities ) - len * sizeof( wchar_t ) );
		}
		if ( *m_Aliens[m_iSelectedAlien]->m_Abilities[i] == '#' )
		{
			if ( wchar_t *wszTranslatedAbility = g_pVGuiLocalize->Find( m_Aliens[m_iSelectedAlien]->m_Abilities[i] ) )
			{
				int len = V_wcslen( wszAbilities );
				V_wcsncpy( wszAbilities + len, wszTranslatedAbility, sizeof( wszAbilities ) - len * sizeof( wchar_t ) );
				continue;
			}
		}
		int len = V_wcslen( wszAbilities );
		g_pVGuiLocalize->ConvertANSIToUnicode( m_Aliens[m_iSelectedAlien]->m_Abilities[i], wszAbility, sizeof( wszAbility ) );
		V_wcsncpy( wszAbilities + len, wszAbility, sizeof( wszAbilities ) - len * sizeof( wchar_t ) );
	}
	m_pLblAbilities->SetText( wszAbilities );

	m_pGplParagraphs->RemoveAllPanelItems();

	FOR_EACH_VEC( m_Aliens[m_iSelectedAlien]->m_Paragraphs, i )
	{
		Label *pLabel = m_pGplParagraphs->AddPanelItem<Label>( "LblParagraph", m_Aliens[m_iSelectedAlien]->m_Paragraphs[i] );
		WrapLabel( pLabel );
	}

	m_pGplAppears->RemoveAllPanelItems();

	FOR_EACH_VEC( m_Aliens[m_iSelectedAlien]->m_AppearsIn, i )
	{
		static const char *s_szDifficulty[5] =
		{
			"#asw_difficulty_easy",
			"#asw_difficulty_normal",
			"#asw_difficulty_hard",
			"#asw_difficulty_insane",
			"#asw_difficulty_imba",
		};

		const Alien_t::AppearsIn_t & appears = m_Aliens[m_iSelectedAlien]->m_AppearsIn[i];

		const char *szCampaign = NULL;
		const char *szMap = NULL;
		const char *szChallenge = NULL;

		if ( !appears.m_szCampaign.IsEmpty() && missionchooser && missionchooser->LocalMissionSource() )
		{
			szCampaign = missionchooser->LocalMissionSource()->GetPrettyCampaignName( appears.m_szCampaign.Get() );
		}

		if ( !appears.m_szMap.IsEmpty() && missionchooser && missionchooser->LocalMissionSource() )
		{
			szMap = missionchooser->LocalMissionSource()->GetPrettyMissionName( appears.m_szMap.Get() );
		}

		if ( !appears.m_szChallenge.IsEmpty() )
		{
			szChallenge = ReactiveDropChallenges::DisplayName( appears.m_szChallenge.Get() );
		}

		wchar_t wszAppears[512];

		if ( szCampaign && szMap )
		{
			wchar_t wszBuffer1[256];
			const wchar_t *wszCampaign = NULL;
			if ( *szCampaign == '#' )
			{
				wszCampaign = g_pVGuiLocalize->Find( szCampaign );
			}
			if ( !wszCampaign )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( szCampaign, wszBuffer1, sizeof( wszBuffer1 ) );
				wszCampaign = wszBuffer1;
			}

			wchar_t wszBuffer2[256];
			const wchar_t *wszMap = NULL;
			if ( *szMap == '#' )
			{
				wszMap = g_pVGuiLocalize->Find( szMap );
			}
			if ( !wszMap )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( szMap, wszBuffer2, sizeof( wszBuffer2 ) );
				wszMap = wszBuffer2;
			}

			g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_campaign_map" ), 2, wszCampaign, wszMap );
		}
		else if ( szCampaign )
		{
			const wchar_t *wszCampaign = NULL;
			if ( *szCampaign == '#' )
			{
				wszCampaign = g_pVGuiLocalize->Find( szCampaign );
			}
			if ( wszCampaign )
			{
				V_wcsncpy( wszAppears, wszCampaign, sizeof( wszAppears ) );
			}
			else
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( szCampaign, wszAppears, sizeof( wszAppears ) );
			}
		}
		else if ( szMap )
		{
			const wchar_t *wszMap = NULL;
			if ( *szMap == '#' )
			{
				wszMap = g_pVGuiLocalize->Find( szMap );
			}
			if ( wszMap )
			{
				V_wcsncpy( wszAppears, wszMap, sizeof( wszAppears ) );
			}
			else
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( szMap, wszAppears, sizeof( wszAppears ) );
			}
		}

		if ( szChallenge )
		{
			wchar_t wszBuffer[256];
			const wchar_t *wszChallenge = NULL;
			if ( appears.m_szChallenge == "0" )
			{
				wszChallenge = g_pVGuiLocalize->Find( "#rd_so_appears_no_challenge" );
			}
			else if ( *szChallenge == '#' )
			{
				wszChallenge = g_pVGuiLocalize->Find( szChallenge );
			}
			if ( !wszChallenge )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( szChallenge, wszBuffer, sizeof( wszBuffer ) );
				wszChallenge = wszBuffer;
			}

			if ( szCampaign || szMap )
			{
				wchar_t wszAppears_temp[NELEMS( wszAppears )];
				V_wcsncpy( wszAppears_temp, wszAppears, sizeof( wszAppears_temp ) );
				g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_mission_challenge" ), 2, wszAppears_temp, wszChallenge );
			}
			else
			{
				V_wcsncpy( wszAppears, wszChallenge, sizeof( wszAppears ) );
			}
		}

		if ( !szCampaign && !szMap && !szChallenge )
		{
			V_wcsncpy( wszAppears, g_pVGuiLocalize->FindSafe( "#rd_so_appears_everywhere" ), sizeof( wszAppears ) );
		}

		if ( appears.m_iMinDifficulty != 1 || appears.m_iMaxDifficulty != NELEMS( s_szDifficulty ) )
		{
			wchar_t wszAppears_temp[NELEMS( wszAppears )];
			V_wcsncpy( wszAppears_temp, wszAppears, sizeof( wszAppears_temp ) );

			if ( appears.m_iMinDifficulty == appears.m_iMaxDifficulty )
			{
				g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_difficulty_single" ), 2, wszAppears_temp, g_pVGuiLocalize->FindSafe( s_szDifficulty[appears.m_iMinDifficulty - 1] ) );
			}
			else if ( appears.m_iMinDifficulty == 1 )
			{
				g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_difficulty_max" ), 2, wszAppears_temp, g_pVGuiLocalize->FindSafe( s_szDifficulty[appears.m_iMaxDifficulty - 1] ) );
			}
			else if ( appears.m_iMaxDifficulty == NELEMS( s_szDifficulty ) )
			{
				g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_difficulty_min" ), 2, wszAppears_temp, g_pVGuiLocalize->FindSafe( s_szDifficulty[appears.m_iMinDifficulty - 1] ) );
			}
			else
			{
				g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_difficulty_range" ), 3, wszAppears_temp, g_pVGuiLocalize->FindSafe( s_szDifficulty[appears.m_iMinDifficulty - 1] ), g_pVGuiLocalize->FindSafe( s_szDifficulty[appears.m_iMaxDifficulty - 1] ) );
			}
		}

		if ( appears.m_bOnslaughtOnly )
		{
			wchar_t wszAppears_temp[NELEMS( wszAppears )];
			V_wcsncpy( wszAppears_temp, wszAppears, sizeof( wszAppears_temp ) );
			g_pVGuiLocalize->ConstructString( wszAppears, sizeof( wszAppears ), g_pVGuiLocalize->FindSafe( "#rd_so_appears_onslaught_only" ), 1, wszAppears_temp );
		}

		Label *pLabel = m_pGplAppears->AddPanelItem<Label>( "LblAppears", "" );
		pLabel->SetText( wszAppears );
		WrapLabel( pLabel );
	}
}

void Swarmopedia::UpdateFooter()
{
	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		footer->SetButtons( FB_BBUTTON, FF_B_ONLY, false );
		footer->SetButtonText( FB_BBUTTON, "#L4D360UI_Back" );
	}
}

void Swarmopedia::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetSelectedAlien( 0 );
}

static void SetScrollBarVisibleAuto( GenericPanelList *pGPL )
{
	if ( pGPL->GetPanelItemCount() == 0 )
	{
		pGPL->SetScrollBarVisible( false );
		return;
	}

	pGPL->ScrollToPanelItem( 0 );
	pGPL->SetScrollBarVisible( pGPL->GetLastVisibleItemNumber( true ) != pGPL->GetPanelItemCount() - 1 );
}

void Swarmopedia::PerformLayout()
{
	BaseClass::PerformLayout();

	SetScrollBarVisibleAuto( m_pGplAliens );
	SetScrollBarVisibleAuto( m_pGplParagraphs );
	SetScrollBarVisibleAuto( m_pGplAppears );
}

void Swarmopedia::OnOpen()
{
	BaseClass::OnOpen();

	SetSelectedAlien( 0 );
}

void Swarmopedia::OnCommand( const char *command )
{
	if ( !V_strcmp( command, "Back" ) )
	{
		// Act as though 360 back button was pressed
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( !V_strcmp( command, "NextModel" ) )
	{
		if ( m_Aliens[m_iSelectedAlien]->m_DisplayPanel.Count() == 0 )
		{
			return;
		}
		m_Aliens[m_iSelectedAlien]->m_DisplayPanel[m_iSelectedModel]->SetVisible( false );
		m_iSelectedModel = ( m_iSelectedModel + 1 ) % m_Aliens[m_iSelectedAlien]->m_DisplayPanel.Count();
		m_Aliens[m_iSelectedAlien]->m_DisplayPanel[m_iSelectedModel]->SetVisible( true );
		m_pModelCaption->SetText( m_Aliens[m_iSelectedAlien]->m_DisplayCaption[m_iSelectedModel] );
	}
	else if ( const char *szAlienIndex = StringAfterPrefix( command, "SelectAlien" ) )
	{
		SetSelectedAlien( atoi( szAlienIndex ) );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void Swarmopedia::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
		if ( m_iSelectedAlien != 0 )
			SetSelectedAlien( m_iSelectedAlien - 1 );
		break;
	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
		if ( m_iSelectedAlien + 1 < m_Aliens.Count() )
			SetSelectedAlien( m_iSelectedAlien + 1 );
		break;
	case KEY_XBUTTON_X:
		OnCommand( "NextModel" );
		break;
	case KEY_XSTICK2_UP:
	case KEY_XSTICK2_DOWN:
		m_pGplParagraphs->OnKeyCodePressed( keycode );
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

Swarmopedia::Alien_t::Alien_t( Swarmopedia *pSwarmopedia, KeyValues *pKV ) : m_szID( pKV->GetString( "ID" ) )
{
	Merge( pSwarmopedia, pKV );
}

Swarmopedia::Alien_t::~Alien_t()
{
}

void Swarmopedia::Alien_t::Merge( Swarmopedia *pSwarmopedia, KeyValues *pKV )
{
	Assert( m_szID == pKV->GetString( "ID" ) );

	if ( const char *szName = pKV->GetString( "Name", NULL ) )
	{
		if ( !m_szName.IsEmpty() && V_strcmp( szName, m_szName ) )
		{
			DevWarning( "Overriding name of %s from %s to %s!\n", m_szID.Get(), m_szName.Get(), szName );
		}
		m_szName = szName;
	}

	FOR_EACH_VALUE( pKV, pAbility )
	{
		if ( V_stricmp( pAbility->GetName(), "Ability" ) )
		{
			continue;
		}

		bool bFound = false;
		FOR_EACH_VEC( m_Abilities, i )
		{
			if ( !V_strcmp( m_Abilities[i], pAbility->GetString() ) )
			{
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			m_Abilities.CopyAndAddToTail( pAbility->GetString() );
		}
	}

	FOR_EACH_TRUE_SUBKEY( pKV, pDisplay )
	{
		if ( V_stricmp( pDisplay->GetName(), "Display" ) )
		{
			continue;
		}

		m_DisplayCaption.CopyAndAddToTail( pDisplay->GetString( "Caption", "" ) );
		Swarmopedia_Model_Panel *pPanel = new Swarmopedia_Model_Panel( pSwarmopedia, "ModelPanel" );
		pPanel->ApplyConfig( pDisplay );
		pPanel->SetBounds( ScreenWidth() - YRES( 300 ), YRES( 30 ), YRES( 266 ), YRES( 200 ) );
		pPanel->SetVisible( false );
		m_DisplayPanel.AddToTail( pPanel );
	}

	FOR_EACH_TRUE_SUBKEY( pKV, pAppears )
	{
		if ( V_stricmp( pAppears->GetName(), "AppearsIn" ) )
		{
			continue;
		}

		m_AppearsIn.AddToTail( pAppears );
	}

	FOR_EACH_VALUE( pKV, pParagraph )
	{
		if ( V_stricmp( pParagraph->GetName(), "Paragraph" ) )
		{
			continue;
		}

		m_Paragraphs.CopyAndAddToTail( pParagraph->GetString() );
	}
}

Swarmopedia::Alien_t::AppearsIn_t::AppearsIn_t( KeyValues *pKV )
{
	m_szCampaign = pKV->GetString( "Campaign" );
	m_szMap = pKV->GetString( "Map" );
	m_szChallenge = pKV->GetString( "Challenge" );
	m_iMinDifficulty = clamp( pKV->GetInt( "MinDifficulty", 1 ), 1, 5 );
	m_iMaxDifficulty = clamp( pKV->GetInt( "MaxDifficulty", 5 ), 1, 5 );
	m_bOnslaughtOnly = pKV->GetBool( "OnslaughtOnly" );
}

CON_COMMAND( rd_swarmopedia, "" )
{
	CBaseModPanel::GetSingleton().OpenWindow( WT_SWARMOPEDIA, CBaseModPanel::GetSingleton().GetWindow( CBaseModPanel::GetSingleton().GetActiveWindowType() ), true );
}
