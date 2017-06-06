#include "cbase.h"
#include "rd_vgui_leaderboard_panel.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/AnimationController.h>
#include "asw_marine_profile.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "c_asw_steamstats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CReactiveDrop_VGUI_Leaderboard_Panel::CReactiveDrop_VGUI_Leaderboard_Panel( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	m_lblTitle = new vgui::Label( this, "LblTitle", "Leaderboard" );
	m_gplLeaderboard = new BaseModUI::GenericPanelList( this, "GplLeaderboard", BaseModUI::GenericPanelList::ISM_ELEVATOR );
	m_bOverrideEntry = false;
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
	CSteamID localUserID = steamapicontext->SteamUser()->GetSteamID();

	m_gplLeaderboard->RemoveAllPanelItems();

	FOR_EACH_VEC( entries, i )
	{
		CReactiveDrop_VGUI_Leaderboard_Entry *pEntry = m_gplLeaderboard->AddPanelItem<CReactiveDrop_VGUI_Leaderboard_Entry>( "Entry" );
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
	m_gplLeaderboard->ShowScrollProgress( bScrollable );
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
}

CReactiveDrop_VGUI_Leaderboard_Entry::~CReactiveDrop_VGUI_Leaderboard_Entry()
{
}

void CReactiveDrop_VGUI_Leaderboard_Entry::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	LoadControlSettings( "resource/UI/RDLeaderboardEntry.res" );
}

void CReactiveDrop_VGUI_Leaderboard_Entry::SetEntry( const RD_LeaderboardEntry_t & entry )
{
	m_nRank = entry.entry.m_nGlobalRank;
	m_nScore = entry.entry.m_nScore;
	m_SteamID = entry.entry.m_steamIDUser;

	m_lblRank->SetText( VarArgs( "%d", entry.entry.m_nGlobalRank ) );

	CSteamID steamIDCopy = entry.entry.m_steamIDUser;
	m_imgAvatar->SetAvatarBySteamID( &steamIDCopy );

	int wide, tall;
	m_imgAvatar->GetSize( wide, tall );
	if ( ((CAvatarImage*)m_imgAvatar->GetImage()) )
	{
		((CAvatarImage*)m_imgAvatar->GetImage())->SetAvatarSize( wide, tall );
		((CAvatarImage*)m_imgAvatar->GetImage())->SetPos( -AVATAR_INDENT_X, -AVATAR_INDENT_Y );
	}

	wchar_t wszName[k_cwchPersonaNameMax];
	Q_UTF8ToUnicode( steamapicontext->SteamFriends()->GetFriendPersonaName( entry.entry.m_steamIDUser ), wszName, sizeof( wszName ) );
	m_lblName->SetText( wszName );

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

	if ( !MarineProfileList() || !ASWEquipmentList() )
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
			if ( CASW_EquipItem *pPrimaryWeapon = ASWEquipmentList()->GetRegular( entry.details.v1.m_iPrimaryWeapon ) )
			{
				if ( CASW_WeaponInfo *pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pPrimaryWeapon->m_EquipClass ) ) )
				{
					m_imgPrimaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
				}
			}
			if ( CASW_EquipItem *pSecondaryWeapon = ASWEquipmentList()->GetRegular( entry.details.v1.m_iSecondaryWeapon ) )
			{
				if ( CASW_WeaponInfo *pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pSecondaryWeapon->m_EquipClass ) ) )
				{
					m_imgSecondaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
				}
			}
			if ( CASW_EquipItem *pExtraWeapon = ASWEquipmentList()->GetExtra( entry.details.v1.m_iExtraWeapon ) )
			{
				if ( CASW_WeaponInfo *pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pExtraWeapon->m_EquipClass ) ) )
				{
					m_imgExtraWeapon->SetImage( pWeaponInfo->szEquipIcon );
				}
			}
			if ( entry.details.v1.m_iSquadSize > 1 )
			{
				m_lblSquadMembers->SetText( VarArgs( "+%d", entry.details.v1.m_iSquadSize - 1 ) );
			}
			// TODO: uint64 m_iTimestamp;
			wchar_t wszCountry[3];
			wszCountry[0] = entry.details.v1.m_CountryCode[0];
			wszCountry[1] = entry.details.v1.m_CountryCode[1];
			wszCountry[2] = 0;
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
			m_lblOnslaught->SetVisible( entry.details.v1.m_iModeFlags & 1 );
			m_lblHardcoreFF->SetVisible( entry.details.v1.m_iModeFlags & 2 );
		}
		break;

	case 2:
		{
			if ( CASW_Marine_Profile *pMarine = MarineProfileList()->GetProfile( entry.details.v2.m_iMarine ) )
			{
				m_imgMarine->SetImage( VarArgs( "briefing/face_%s", pMarine->m_PortraitName ) );
			}
			if ( CASW_EquipItem *pPrimaryWeapon = ASWEquipmentList()->GetRegular( entry.details.v2.m_iPrimaryWeapon ) )
			{
				if ( CASW_WeaponInfo *pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pPrimaryWeapon->m_EquipClass ) ) )
				{
					m_imgPrimaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
				}
			}
			if ( CASW_EquipItem *pSecondaryWeapon = ASWEquipmentList()->GetRegular( entry.details.v2.m_iSecondaryWeapon ) )
			{
				if ( CASW_WeaponInfo *pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pSecondaryWeapon->m_EquipClass ) ) )
				{
					m_imgSecondaryWeapon->SetImage( pWeaponInfo->szEquipIcon );
				}
			}
			if ( CASW_EquipItem *pExtraWeapon = ASWEquipmentList()->GetExtra( entry.details.v2.m_iExtraWeapon ) )
			{
				if ( CASW_WeaponInfo *pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pExtraWeapon->m_EquipClass ) ) )
				{
					m_imgExtraWeapon->SetImage( pWeaponInfo->szEquipIcon );
				}
			}
			if ( entry.details.v2.m_iSquadSize > 1 )
			{
				m_lblSquadMembers->SetText( VarArgs( "+%d", entry.details.v2.m_iSquadSize - 1 ) );
			}
			// TODO: uint64 m_iTimestamp;
			wchar_t wszCountry[3];
			wszCountry[0] = entry.details.v2.m_CountryCode[0];
			wszCountry[1] = entry.details.v2.m_CountryCode[1];
			wszCountry[2] = 0;
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
			m_lblOnslaught->SetVisible( entry.details.v2.m_iModeFlags & 1 );
			m_lblHardcoreFF->SetVisible( entry.details.v2.m_iModeFlags & 2 );
		}
		break;
	}
}
