#include "cbase.h"
#include "rd_vgui_leaderboard_panel.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/AnimationController.h>
#include "asw_marine_profile.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "c_asw_steamstats.h"
#include "rd_text_filtering.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CReactiveDrop_VGUI_Leaderboard_Panel::CReactiveDrop_VGUI_Leaderboard_Panel( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_lblTitle = new vgui::Label( this, "LblTitle", "Leaderboard" );
	m_gplLeaderboard = new BaseModUI::GenericPanelList( this, "GplLeaderboard", BaseModUI::GenericPanelList::ISM_ELEVATOR );
	m_bOverrideEntry = false;
	m_eDisplayType = k_ELeaderboardDisplayTypeTimeMilliSeconds;
}

CReactiveDrop_VGUI_Leaderboard_Panel::~CReactiveDrop_VGUI_Leaderboard_Panel()
{
}

void CReactiveDrop_VGUI_Leaderboard_Panel::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	LoadControlSettings( "resource/UI/RDLeaderboardPanel.res" );
}

void CReactiveDrop_VGUI_Leaderboard_Panel::SetTitle( const char *szTitle )
{
	m_lblTitle->SetText( szTitle );
}

void CReactiveDrop_VGUI_Leaderboard_Panel::SetTitle( const wchar_t *wszTitle )
{
	m_lblTitle->SetText( wszTitle );
}

void CReactiveDrop_VGUI_Leaderboard_Panel::SetEntries( const CUtlVector<RD_LeaderboardEntry_t> & entries )
{
	CSteamID localUserID = SteamUser()->GetSteamID();

	m_gplLeaderboard->RemoveAllPanelItems();

	FOR_EACH_VEC( entries, i )
	{
		CReactiveDrop_VGUI_Leaderboard_Entry *pEntry = m_gplLeaderboard->AddPanelItem<CReactiveDrop_VGUI_Leaderboard_Entry>( "Entry" );
		pEntry->SetDisplayType( m_eDisplayType );
		pEntry->SetEntry( entries[i] );

		if ( entries[i].entry.m_steamIDUser == localUserID )
		{
			m_gplLeaderboard->ScrollToPanelItem( i );
		}
	}

	if ( m_bOverrideEntry )
	{
		DoOverrideEntry();
	}
}

void CReactiveDrop_VGUI_Leaderboard_Panel::OverrideEntry( const RD_LeaderboardEntry_t & entry )
{
	Assert( !m_bOverrideEntry );
	m_OverrideEntry = entry;
	m_bOverrideEntry = true;

	DoOverrideEntry();
}

static int __cdecl SortByRank( vgui::Panel *const*a, vgui::Panel *const*b )
{
	return assert_cast<CReactiveDrop_VGUI_Leaderboard_Entry *>( *a )->m_nRank - assert_cast<CReactiveDrop_VGUI_Leaderboard_Entry *>( *b )->m_nRank;
}

void CReactiveDrop_VGUI_Leaderboard_Panel::DoOverrideEntry()
{
	CReactiveDrop_VGUI_Leaderboard_Entry *pEntry = NULL;

	for ( unsigned short i = 0; i < m_gplLeaderboard->GetPanelItemCount(); i++ )
	{
		CReactiveDrop_VGUI_Leaderboard_Entry *pPanel = assert_cast<CReactiveDrop_VGUI_Leaderboard_Entry *>( m_gplLeaderboard->GetPanelItem( i ) );
		if ( pPanel->m_SteamID == m_OverrideEntry.entry.m_steamIDUser )
		{
			pEntry = pPanel;
			break;
		}
	}

	if ( !pEntry )
	{
		pEntry = m_gplLeaderboard->AddPanelItem<CReactiveDrop_VGUI_Leaderboard_Entry>( "Entry" );
	}

	pEntry->SetBgColor( Color( 0, 0, 0, 175 ) );

	pEntry->SetEntry( m_OverrideEntry );

	m_gplLeaderboard->SortPanelItems( &SortByRank );

	unsigned short index;
	if ( m_gplLeaderboard->GetPanelItemIndex( pEntry, index ) )
	{
		m_gplLeaderboard->ScrollToPanelItem( index );
	}
}

void CReactiveDrop_VGUI_Leaderboard_Panel::SetScrollable( bool bScrollable )
{
	m_gplLeaderboard->SetScrollBarVisible( bScrollable );
}

CReactiveDrop_VGUI_Leaderboard_Entry::CReactiveDrop_VGUI_Leaderboard_Entry( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_nRank = 0;
	m_nScore = 0;
	m_lblRank = new vgui::Label( this, "LblRank", "" );
	m_imgAvatar = new CAvatarImagePanel( this, "ImgAvatar" );
	m_lblName = new vgui::Label( this, "LblName", "" );
	m_lblScore = new vgui::Label( this, "LblScore", "" );
	m_imgMarine = new vgui::ImagePanel( this, "ImgMarine" );
	m_imgPrimaryWeapon = new vgui::ImagePanel( this, "ImgPrimaryWeapon" );
	m_imgSecondaryWeapon = new vgui::ImagePanel( this, "ImgSecondaryWeapon" );
	m_imgExtraWeapon = new vgui::ImagePanel( this, "ImgExtraWeapon" );
	m_lblSquadMembers = new vgui::Label( this, "LblSquadMembers", "" );
	m_lblCountry = new vgui::Label( this, "LblCountry", "" );
	m_lblDifficulty = new vgui::Label( this, "LblDifficulty", "" );
	m_lblOnslaught = new vgui::Label( this, "LblOnslaught", "#nb_onslaught_title" );
	m_lblHardcoreFF = new vgui::Label( this, "LblHardcoreFF", "#L4D360UI_HardcoreFF" );
	m_eDisplayType = k_ELeaderboardDisplayTypeTimeMilliSeconds;
}

CReactiveDrop_VGUI_Leaderboard_Entry::~CReactiveDrop_VGUI_Leaderboard_Entry()
{
}

void CReactiveDrop_VGUI_Leaderboard_Entry::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	LoadControlSettings( "resource/UI/RDLeaderboardEntry.res" );
}

void CReactiveDrop_VGUI_Leaderboard_Entry::SetEntry( const RD_LeaderboardEntry_t &entry )
{
	m_nRank = entry.entry.m_nGlobalRank;
	m_nScore = entry.entry.m_nScore;
	m_SteamID = entry.entry.m_steamIDUser;

	m_lblRank->SetText( VarArgs( "%d", entry.entry.m_nGlobalRank ) );

	CSteamID steamIDCopy = entry.entry.m_steamIDUser;
	m_imgAvatar->SetAvatarBySteamID( &steamIDCopy );

	int wide, tall;
	m_imgAvatar->GetSize( wide, tall );
	if ( ( ( CAvatarImage * )m_imgAvatar->GetImage() ) )
	{
		( ( CAvatarImage * )m_imgAvatar->GetImage() )->SetAvatarSize( wide, tall );
		( ( CAvatarImage * )m_imgAvatar->GetImage() )->SetPos( -AVATAR_INDENT_X, -AVATAR_INDENT_Y );
	}

	wchar_t wszName[k_cwchPersonaNameMax];
	Q_UTF8ToUnicode( SteamFriends()->GetFriendPersonaName( entry.entry.m_steamIDUser ), wszName, sizeof( wszName ) );
	g_RDTextFiltering.FilterTextName( wszName, entry.entry.m_steamIDUser );
	m_lblName->SetText( wszName );

	if ( m_eDisplayType == k_ELeaderboardDisplayTypeTimeMilliSeconds )
	{
		int32 milliseconds = entry.entry.m_nScore % 1000;
		int32 seconds = ( entry.entry.m_nScore / 1000 ) % 60;
		int32 minutes = ( entry.entry.m_nScore / 60 / 1000 ) % 60;
		int32 hours = entry.entry.m_nScore / 60 / 60 / 1000;
		if ( hours > 0 )
		{
			m_lblScore->SetText( VarArgs( "%d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds ) );
		}
		else
		{
			m_lblScore->SetText( VarArgs( "%d:%02d.%03d", minutes, seconds, milliseconds ) );
		}
	}
	else if ( m_eDisplayType == k_ELeaderboardDisplayTypeTimeSeconds )
	{
		int32 seconds = ( entry.entry.m_nScore ) % 60;
		int32 minutes = ( entry.entry.m_nScore / 60 ) % 60;
		int32 hours = entry.entry.m_nScore / 60 / 60;
		if ( hours > 0 )
		{
			m_lblScore->SetText( VarArgs( "%d:%02d:%02d", hours, minutes, seconds ) );
		}
		else
		{
			m_lblScore->SetText( VarArgs( "%d:%02d", minutes, seconds ) );
		}
	}
	else
	{
		m_lblScore->SetText( VarArgs( "%d", m_nScore ) );
	}

	if ( !MarineProfileList() )
	{
		return;
	}

	switch ( entry.details.version )
	{
	case 1:
	{
		if ( CASW_Marine_Profile *pMarine = MarineProfileList()->GetProfile( entry.details.v1.m_iMarine ) )
		{
			m_imgMarine->SetImage( VarArgs( "briefing/face_%s", pMarine->m_PortraitName ) );
		}
		if ( CASW_EquipItem *pPrimaryWeapon = g_ASWEquipmentList.GetRegular( entry.details.v1.m_iPrimaryWeapon ) )
		{
			if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pPrimaryWeapon->m_szEquipClass ) )
			{
				m_imgPrimaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
			}
		}
		if ( CASW_EquipItem *pSecondaryWeapon = g_ASWEquipmentList.GetRegular( entry.details.v1.m_iSecondaryWeapon ) )
		{
			if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pSecondaryWeapon->m_szEquipClass ) )
			{
				m_imgSecondaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
			}
		}
		if ( CASW_EquipItem *pExtraWeapon = g_ASWEquipmentList.GetExtra( entry.details.v1.m_iExtraWeapon ) )
		{
			if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pExtraWeapon->m_szEquipClass ) )
			{
				m_imgExtraWeapon->SetImage( pWeaponInfo->szEquipIcon );
			}
		}
		if ( entry.details.v1.m_iSquadSize > 1 )
		{
			m_lblSquadMembers->SetText( VarArgs( "+%d", entry.details.v1.m_iSquadSize - 1 ) );
		}
		wchar_t wszCountry[3] = { 0 };
		wszCountry[0] = entry.details.v1.m_CountryCode[0];
		wszCountry[1] = entry.details.v1.m_CountryCode[1];
		m_lblCountry->SetText( wszCountry );
		switch ( entry.details.v1.m_iDifficulty )
		{
		case 1:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_easy" );
			break;
		case 2:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_normal" );
			break;
		case 3:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_hard" );
			break;
		case 4:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_insane" );
			break;
		case 5:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_imba" );
			break;
		}
		m_lblOnslaught->SetVisible( ( entry.details.v1.m_iModeFlags & 1 ) != 0 );
		m_lblHardcoreFF->SetVisible( ( entry.details.v1.m_iModeFlags & 2 ) != 0 );
	}
	break;

	case 2:
	{
		if ( CASW_Marine_Profile *pMarine = MarineProfileList()->GetProfile( entry.details.v2.m_iMarine ) )
		{
			m_imgMarine->SetImage( VarArgs( "briefing/face_%s", pMarine->m_PortraitName ) );
		}
		if ( CASW_EquipItem *pPrimaryWeapon = g_ASWEquipmentList.GetRegular( entry.details.v2.m_iPrimaryWeapon ) )
		{
			if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pPrimaryWeapon->m_szEquipClass ) )
			{
				m_imgPrimaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
			}
		}
		if ( CASW_EquipItem *pSecondaryWeapon = g_ASWEquipmentList.GetRegular( entry.details.v2.m_iSecondaryWeapon ) )
		{
			if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pSecondaryWeapon->m_szEquipClass ) )
			{
				m_imgSecondaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
			}
		}
		if ( CASW_EquipItem *pExtraWeapon = g_ASWEquipmentList.GetExtra( entry.details.v2.m_iExtraWeapon ) )
		{
			if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( pExtraWeapon->m_szEquipClass ) )
			{
				m_imgExtraWeapon->SetImage( pWeaponInfo->szEquipIcon );
			}
		}
		if ( entry.details.v2.m_iSquadSize > 1 )
		{
			m_lblSquadMembers->SetText( VarArgs( "+%d", entry.details.v2.m_iSquadSize - 1 ) );
		}
		wchar_t wszCountry[3] = { 0 };
		wszCountry[0] = entry.details.v2.m_CountryCode[0];
		wszCountry[1] = entry.details.v2.m_CountryCode[1];
		m_lblCountry->SetText( wszCountry );
		switch ( entry.details.v2.m_iDifficulty )
		{
		case 1:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_easy" );
			break;
		case 2:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_normal" );
			break;
		case 3:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_hard" );
			break;
		case 4:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_insane" );
			break;
		case 5:
			m_lblDifficulty->SetText( "#L4D360UI_Difficulty_imba" );
			break;
		}
		m_lblOnslaught->SetVisible( ( entry.details.v2.m_iModeFlags & 1 ) != 0 );
		m_lblHardcoreFF->SetVisible( ( entry.details.v2.m_iModeFlags & 2 ) != 0 );
		// TODO: do something if mission failed ( ( entry.details.v1.m_iModeFlags & 4 ) != 0 )
	}
	break;
	}
}

CReactiveDrop_VGUI_Leaderboard_Panel_Points::CReactiveDrop_VGUI_Leaderboard_Panel_Points( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_lblTitle = new vgui::Label( this, "LblTitle", "Leaderboard" );
	m_gplLeaderboard = new BaseModUI::GenericPanelList( this, "GplLeaderboard", BaseModUI::GenericPanelList::ISM_ELEVATOR );
	m_bOverrideEntry = false;
}

CReactiveDrop_VGUI_Leaderboard_Panel_Points::~CReactiveDrop_VGUI_Leaderboard_Panel_Points()
{
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	LoadControlSettings( "resource/UI/RDLeaderboardPanel_Points.res" );
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::SetTitle( const char *szTitle )
{
	m_lblTitle->SetText( szTitle );
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::SetTitle( const wchar_t *wszTitle )
{
	m_lblTitle->SetText( wszTitle );
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::SetEntries( const CUtlVector<RD_LeaderboardEntry_Points_t> & entries )
{
	CSteamID localUserID = SteamUser()->GetSteamID();

	m_gplLeaderboard->RemoveAllPanelItems();

	FOR_EACH_VEC( entries, i )
	{
		CReactiveDrop_VGUI_Leaderboard_Entry_Points *pEntry = m_gplLeaderboard->AddPanelItem<CReactiveDrop_VGUI_Leaderboard_Entry_Points>( "Entry" );
		pEntry->SetEntry( entries[ i ] );

		if ( entries[ i ].entry.m_steamIDUser == localUserID )
		{
			m_gplLeaderboard->ScrollToPanelItem( i );
		}
	}

	if ( m_bOverrideEntry )
	{
		DoOverrideEntry();
	}
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::OverrideEntry( const RD_LeaderboardEntry_Points_t & entry )
{
	Assert( !m_bOverrideEntry );
	m_OverrideEntry = entry;
	m_bOverrideEntry = true;

	DoOverrideEntry();
}

static int __cdecl SortByRank_Points( vgui::Panel *const*a, vgui::Panel *const*b )
{
	return assert_cast<CReactiveDrop_VGUI_Leaderboard_Entry_Points *>(*a)->m_nRank - assert_cast<CReactiveDrop_VGUI_Leaderboard_Entry_Points *>(*b)->m_nRank;
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::DoOverrideEntry()
{
	CReactiveDrop_VGUI_Leaderboard_Entry_Points *pEntry = NULL;

	for ( unsigned short i = 0; i < m_gplLeaderboard->GetPanelItemCount(); i++ )
	{
		CReactiveDrop_VGUI_Leaderboard_Entry_Points *pPanel = assert_cast<CReactiveDrop_VGUI_Leaderboard_Entry_Points *>(m_gplLeaderboard->GetPanelItem( i ));
		if ( pPanel->m_SteamID == m_OverrideEntry.entry.m_steamIDUser )
		{
			pEntry = pPanel;
			break;
		}
	}

	if ( !pEntry )
	{
		pEntry = m_gplLeaderboard->AddPanelItem<CReactiveDrop_VGUI_Leaderboard_Entry_Points>( "Entry" );
	}

	pEntry->SetBgColor( Color( 0, 0, 0, 175 ) );

	pEntry->SetEntry( m_OverrideEntry );

	m_gplLeaderboard->SortPanelItems( &SortByRank_Points );

	unsigned short index;
	if ( m_gplLeaderboard->GetPanelItemIndex( pEntry, index ) )
	{
		m_gplLeaderboard->ScrollToPanelItem( index );
	}
}

void CReactiveDrop_VGUI_Leaderboard_Panel_Points::SetScrollable( bool bScrollable )
{
	m_gplLeaderboard->SetScrollBarVisible( bScrollable );
}

CReactiveDrop_VGUI_Leaderboard_Entry_Points::CReactiveDrop_VGUI_Leaderboard_Entry_Points( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_nRank = 0;
	m_nScore = 0;
	m_lblRank = new vgui::Label(this, "LblRank", "");
	m_imgAvatar = new CAvatarImagePanel(this, "ImgAvatar");
	m_lblName = new vgui::Label(this, "LblName", "");
	m_lblScore_Points = new vgui::Label(this, "LblScore_Points", "");
	m_lblScore_AlienKills = new vgui::Label(this, "LblScore_AlienKills", "");
	m_lblScore_PlayerKills = new vgui::Label(this, "LblScore_PlayerKills", "");
	m_lblScore_GamesWon = new vgui::Label(this, "LblScore_GamesWon", "");
	m_lblScore_GamesLost = new vgui::Label(this, "LblScore_GamesLost", "");
	m_lblScore_GamesTotal = new vgui::Label(this, "LblScore_GamesTotal", "");
	m_lblCountry = new vgui::Label( this, "LblCountry", "" );
}

CReactiveDrop_VGUI_Leaderboard_Entry_Points::~CReactiveDrop_VGUI_Leaderboard_Entry_Points()
{
}

void CReactiveDrop_VGUI_Leaderboard_Entry_Points::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings(scheme);

	LoadControlSettings("resource/UI/RDLeaderboardEntry_Points.res");
}

void CReactiveDrop_VGUI_Leaderboard_Entry_Points::SetEntry( const RD_LeaderboardEntry_Points_t & entry )
{
	m_nRank = entry.entry.m_nGlobalRank;
	m_nScore = entry.entry.m_nScore;
	m_SteamID = entry.entry.m_steamIDUser;

	m_lblRank->SetText(VarArgs("%d", entry.entry.m_nGlobalRank));

	CSteamID steamIDCopy = entry.entry.m_steamIDUser;
	m_imgAvatar->SetAvatarBySteamID(&steamIDCopy);

	int wide, tall;
	m_imgAvatar->GetSize(wide, tall);
	if (((CAvatarImage*)m_imgAvatar->GetImage()))
	{
		((CAvatarImage*)m_imgAvatar->GetImage())->SetAvatarSize(wide, tall);
		((CAvatarImage*)m_imgAvatar->GetImage())->SetPos(-AVATAR_INDENT_X, -AVATAR_INDENT_Y);
	}

	wchar_t wszName[k_cwchPersonaNameMax];
	Q_UTF8ToUnicode(SteamFriends()->GetFriendPersonaName(entry.entry.m_steamIDUser), wszName, sizeof(wszName));
	g_RDTextFiltering.FilterTextName( wszName, entry.entry.m_steamIDUser );
	m_lblName->SetText(wszName);

	m_lblScore_Points->SetText( VarArgs( "%d", m_nScore ) );

	wchar_t wszCountry[ 3 ];
	wszCountry[ 0 ] = entry.details.m_CountryCode[ 0 ];
	wszCountry[ 1 ] = entry.details.m_CountryCode[ 1 ];
	wszCountry[ 2 ] = 0;
	m_lblCountry->SetText( wszCountry );

	m_lblScore_AlienKills->SetText( VarArgs( "%d", entry.details.m_iAlienKills));
	m_lblScore_PlayerKills->SetText( VarArgs( "%d", entry.details.m_iPlayerKills ) );
	m_lblScore_GamesWon->SetText( VarArgs( "%d", entry.details.m_iGamesWon ) );
	m_lblScore_GamesLost->SetText( VarArgs( "%d", entry.details.m_iGamesLost ) );
	m_lblScore_GamesTotal->SetText( VarArgs( "%d", entry.details.m_iGamesWon + entry.details.m_iGamesLost ) );
}
