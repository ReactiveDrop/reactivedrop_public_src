#include "cbase.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ImagePanel.h>
#include "c_asw_marine_resource.h"
#include "asw_marine_profile.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_campaign_save.h"
#include "c_asw_game_resource.h"
#include "PlayerListPanel.h"
#include "PlayerListLine.h"
#include "c_playerresource.h"
#include <vgui/ILocalize.h>
#include "voice_status.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

extern ConVar asw_vote_kick_fraction;
extern ConVar asw_vote_leader_fraction;

#define MUTE_BUTTON_ICON "voice/voice_icon_hud"

ConVar rd_muted_color( "rd_muted_color", "66 66 66", FCVAR_ARCHIVE, "Color of the speaker icon in Player List window (F9) while muted" );
ConVar rd_speaking_color( "rd_speaking_color", "0 240 240", FCVAR_ARCHIVE, "Color of the speaker icon in Player List window (F9) while talking" );
ConVar rd_unmuted_color( "rd_unmuted_color", "190 190 190", FCVAR_ARCHIVE, "Color of the speaker icon in Player List window (F9) while silent" );

PlayerListLine::PlayerListLine(vgui::Panel *parent, const char *name) :
	vgui::Panel( parent, name )
{
	m_iPlayerIndex = -1;
	m_pMuteButton = new CBitmapButton( this, "MuteButton", " " );
	m_pMuteButton->AddActionSignalTarget( this );
	m_pMuteButton->SetCommand( "MuteButton" );
	m_pPlayerLabel = new vgui::Button( this, "PlayerLabel", " " );
	m_pPlayerLabel->AddActionSignalTarget( this );
	m_pPlayerLabel->SetCommand( "PlayerLabel" );
	m_pMarinesLabel = new vgui::Label( this, "MarinesLabel", " " );
	m_pFragsLabel = new vgui::Label( this, "FragsLabel", " " );
	m_pDeathsLabel = new vgui::Label( this, "DeathsLabel", " " );
	m_pPingLabel = new vgui::Label( this, "PingLabel", " " );
	m_pKickCheck = new VoteCheck( this, "KickCheck", "#asw_player_list_kick_check" );
	m_pKickCheck->AddActionSignalTarget( this );
	m_pLeaderCheck = new VoteCheck( this, "KickCheck", "#asw_player_list_leader_check" );
	m_pLeaderCheck->AddActionSignalTarget( this );
	m_wszPlayerName[0] = L'\0';
	m_wszFragsString[0] = L'\0';
	m_wszDeathsString[0] = L'\0';
	m_wszPingString[0] = L'\0';
	m_wszMarineNames[0] = L'\0';
	for ( int i = 0; i < MAX_VOTE_ICONS; i++ )
	{
		m_pKickVoteIcon[i] = new vgui::ImagePanel( this, "BootIcon" );
		m_pKickVoteIcon[i]->SetVisible( false );
		m_pKickVoteIcon[i]->SetShouldScaleImage( true );
		m_pKickVoteIcon[i]->SetImage( "swarm/PlayerList/BootIcon" );
		m_pLeaderVoteIcon[i] = new vgui::ImagePanel( this, "LeaderIcon" );
		m_pLeaderVoteIcon[i]->SetVisible( false );
		m_pLeaderVoteIcon[i]->SetShouldScaleImage( true );
		m_pLeaderVoteIcon[i]->SetImage( "swarm/PlayerList/LeaderIcon" );
		m_iKickIconState[i] = 0;
		m_iLeaderIconState[i] = 0;
	}
	m_bKickChecked = false;
	m_bLeaderChecked = false;
}

void PlayerListLine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	color32 white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;

	color32 grey;
	grey.r = 190;
	grey.g = 190;
	grey.b = 190;
	grey.a = 255;

	m_pMuteButton->SetImage( CBitmapButton::BUTTON_ENABLED, MUTE_BUTTON_ICON, grey );
	m_pMuteButton->SetImage( CBitmapButton::BUTTON_DISABLED, MUTE_BUTTON_ICON, grey );
	m_pMuteButton->SetImage( CBitmapButton::BUTTON_PRESSED, MUTE_BUTTON_ICON, white );
	m_pMuteButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, MUTE_BUTTON_ICON, white );

	vgui::HFont DefaultFont = pScheme->GetFont( "Default", IsProportional() );
	m_pPlayerLabel->SetFont( DefaultFont );

	m_pPlayerLabel->SetPaintBackgroundEnabled( false );
	m_pMarinesLabel->SetFont( DefaultFont );
	m_pMarinesLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
	m_pFragsLabel->SetFont( DefaultFont );
	m_pFragsLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
	m_pDeathsLabel->SetFont( DefaultFont );
	m_pDeathsLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
	m_pPingLabel->SetFont( DefaultFont );
	m_pPingLabel->SetFgColor( Color( 255, 255, 255, 255 ) );
	m_pKickCheck->SetFont( DefaultFont );
	m_pLeaderCheck->SetFont( DefaultFont );

	SetPaintBackgroundEnabled( true );
	SetPaintBackgroundType( 0 );
	SetBgColor( Color( 0, 0, 0, 128 ) );
}

void PlayerListLine::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "MuteButton" ) )
	{
		C_PlayerResource *p_GR = dynamic_cast< C_PlayerResource* >( GameResources() );

		if ( p_GR )
			p_GR->TogglePlayerMuteState( m_iPlayerIndex, false );
	}
	else if ( !Q_stricmp( command, "PlayerLabel" )  )
	{
		player_info_t pi;
		if ( engine->GetPlayerInfo( m_iPlayerIndex, &pi ) )
		{
			if ( pi.friendsID )
			{
				CSteamID steamIDForPlayer( pi.friendsID, 1, SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );
				uint64 id = steamIDForPlayer.ConvertToUint64();
				char steamCmd[64];
				Q_snprintf( steamCmd, sizeof( steamCmd ), "steamid/%I64u", id );
				BaseModUI::CUIGameData::Get()->ExecuteOverlayCommand( steamCmd );
			}
		}
	}
	BaseClass::OnCommand( command );
}

void PlayerListLine::PerformLayout()
{
	float fScale = (ScreenHeight() / 768.0f);
	int top = 6.0f * fScale;
	int top_line_height = 16.0f * fScale;
	int bottom_line_top = top_line_height + top;
	int bottom_line_height = 16.0f * fScale;	

	m_pMuteButton->SetBounds( PLAYER_LIST_PLAYER_X * fScale - PLAYER_LIST_KICK_ICON_W * fScale - 5, top, PLAYER_LIST_KICK_ICON_W * fScale, bottom_line_height );

	m_pPlayerLabel-> SetBounds(PLAYER_LIST_PLAYER_X * fScale,		top,			 PLAYER_LIST_PLAYER_W * fScale,		  top_line_height);
	m_pMarinesLabel->SetBounds(PLAYER_LIST_MARINES_X * fScale,		top,			 PLAYER_LIST_MARINES_W * fScale,	  top_line_height);
	m_pFragsLabel->	 SetBounds(PLAYER_LIST_FRAGS_X * fScale,		top,			 PLAYER_LIST_FRAGS_W * fScale,		  top_line_height);
	m_pDeathsLabel-> SetBounds(PLAYER_LIST_DEATHS_X * fScale,		top,			 PLAYER_LIST_DEATHS_W * fScale,		  top_line_height);
	m_pPingLabel->	 SetBounds(PLAYER_LIST_PING_X * fScale,			top,			 PLAYER_LIST_PING_W * fScale,		  top_line_height);
	m_pLeaderCheck-> SetBounds(PLAYER_LIST_LEADER_CHECK_X * fScale, bottom_line_top, PLAYER_LIST_LEADER_CHECK_W * fScale, bottom_line_height);
	m_pKickCheck->   SetBounds(PLAYER_LIST_KICK_CHECK_X * fScale,	bottom_line_top, PLAYER_LIST_KICK_CHECK_W * fScale,   bottom_line_height);
	//m_pMuteCheck->SetBounds(PLAYER_LIST_MUTE_CHECK_X * fScale, bottom_line_top, PLAYER_LIST_MUTE_CHECK_W * fScale, bottom_line_height);

	for (int i=0;i<MAX_VOTE_ICONS;i++)
	{
		m_pKickVoteIcon[i]->SetBounds((PLAYER_LIST_KICK_ICON_X + (PLAYER_LIST_KICK_ICON_W * i)) * fScale, bottom_line_top,
							PLAYER_LIST_KICK_ICON_W * fScale, bottom_line_height);
		m_pLeaderVoteIcon[i]->SetBounds((PLAYER_LIST_LEADER_ICON_X + (PLAYER_LIST_LEADER_ICON_W * i)) * fScale, bottom_line_top,
							PLAYER_LIST_LEADER_ICON_W * fScale, bottom_line_height);
	}	
}

void PlayerListLine::OnThink()
{
	if (m_iPlayerIndex != -1 && ASWGameResource())
	{
		const char *szName = g_PR->GetPlayerName( m_iPlayerIndex );
		wchar_t wszName[k_cwchPersonaNameMax];
		V_UTF8ToUnicode( szName, wszName, sizeof( wszName ) );

		const wchar_t *wszNameFormat = L"%s1";
		if (m_iPlayerIndex == ASWGameResource()->GetLeaderEntIndex())
		{
			wszNameFormat = g_pVGuiLocalize->FindSafe( "#asw_player_list_name_leader" );
		}

		// check name is the same, update label if not
		wchar_t wszPlayerName[NELEMS( m_wszPlayerName )];
		g_pVGuiLocalize->ConstructString( wszPlayerName, sizeof( wszPlayerName ), wszNameFormat, 1, wszName );

		if ( V_wcscmp( wszPlayerName, m_wszPlayerName ) )
		{
			V_wcsncpy( m_wszPlayerName, wszPlayerName, sizeof( m_wszPlayerName ) );
			m_pPlayerLabel->SetText( wszPlayerName );
		}

		const wchar_t *frags = GetFragsString();
		if ( V_wcscmp( frags, m_wszFragsString ) )
		{
			V_wcsncpy( m_wszFragsString, frags, sizeof( m_wszFragsString ) );
			m_pFragsLabel->SetText( frags );
		}
		
		const wchar_t *deaths = GetDeathsString();
		if ( V_wcscmp( deaths, m_wszDeathsString ) )
		{
			V_wcsncpy( m_wszDeathsString, deaths, sizeof( m_wszDeathsString ) );
			m_pDeathsLabel->SetText( deaths );
		}

		// check ping (todo: only every so often?)
		const wchar_t *ping = GetPingString();
		if ( V_wcscmp( ping, m_wszPingString ) )
		{
			V_wcsncpy( m_wszPingString, ping, sizeof( m_wszPingString ) );
			m_pPingLabel->SetText( ping );
		}
		
		// check marines
		const wchar_t *marines = GetMarineNames();
		if ( V_wcscmp( marines, m_wszMarineNames ) )
		{
			V_wcsncpy( m_wszMarineNames, marines, sizeof( m_wszMarineNames ) );
			m_pMarinesLabel->SetText( marines );
		}

		CVoiceStatus* pVoiceMgr = GetClientVoiceMgr();
		if ( pVoiceMgr )
		{
			C_ASW_Player *local = C_ASW_Player::GetLocalASWPlayer();
			bool bTalking = false;
			if ( local )
			{
				if ( local->entindex() == m_iPlayerIndex )
				{
					bTalking = pVoiceMgr->IsLocalPlayerSpeakingAboveThreshold( FirstValidSplitScreenSlot() );
				}
				else
				{
					bTalking = pVoiceMgr->IsPlayerSpeaking( m_iPlayerIndex );
				}
			}
			bool bMuted = pVoiceMgr->IsPlayerBlocked( m_iPlayerIndex );
			if ( bMuted )
			{
				m_pMuteButton->SetImageColor( CBitmapButton::BUTTON_ENABLED, rd_muted_color.GetColor().ToColor32() );
			}
			else if ( bTalking )
			{
				m_pMuteButton->SetImageColor( CBitmapButton::BUTTON_ENABLED, rd_speaking_color.GetColor().ToColor32() );
			}
			else
			{
				m_pMuteButton->SetImageColor( CBitmapButton::BUTTON_ENABLED, rd_unmuted_color.GetColor().ToColor32() );
			}
		}
	}
	UpdateCheckBoxes();
	UpdateVoteIcons();
}

const wchar_t *PlayerListLine::GetMarineNames()
{
	static wchar_t marines[32 * ASW_MAX_MARINE_RESOURCES];
	wchar_t buffer[32 * ASW_MAX_MARINE_RESOURCES];
	marines[0] = '\0';

	if ( !ASWGameResource() )
		return marines;

	int iMarines = 0;
	for ( int i = 0; i < ASWGameResource()->GetMaxMarineResources(); i++ )
	{
		C_ASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR && pMR->GetCommanderIndex() == m_iPlayerIndex && pMR->GetProfile() )
		{
			if ( iMarines == 0 )
			{
				V_snwprintf( marines, ARRAYSIZE( marines ), L"%s", g_pVGuiLocalize->FindSafe( pMR->GetProfile()->m_ShortName ) );
			}
			else
			{
				V_snwprintf( buffer, ARRAYSIZE( buffer ), L"%s, %s", marines, g_pVGuiLocalize->FindSafe( pMR->GetProfile()->m_ShortName ) );
				V_snwprintf( marines, ARRAYSIZE( marines ), L"%s", buffer );
			}
			iMarines++;
		}
	}

	return marines;
}

void PlayerListLine::UpdateCheckBoxes()
{
	HACK_GETLOCALPLAYER_GUARD( "need local player to see if we can vote" );
	C_ASW_Player *pLocal = C_ASW_Player::GetLocalASWPlayer();
	if ( pLocal && !pLocal->CanVote() )
	{
		m_pKickCheck->SetVisible( false );
		m_pLeaderCheck->SetVisible( false );
		return;
	}

	C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( m_iPlayerIndex ) );
	m_pKickCheck->SetVisible( pPlayer && pPlayer->CanBeKicked() );
	m_pLeaderCheck->SetVisible( pPlayer && pPlayer->CanBeLeader() );

	// make sure our selected/unselected status matches the selected index from our parent
	PlayerListPanel *pPanel = assert_cast< PlayerListPanel * >( GetParent()->GetParent()->GetParent() );
	if ( pPanel )
	{
		bool bKick = ( pPanel->m_iKickVoteIndex == m_iPlayerIndex ) && m_iPlayerIndex != -1;
		bool bLeader = ( pPanel->m_iLeaderVoteIndex == m_iPlayerIndex ) && m_iPlayerIndex != -1;

		if ( m_pKickCheck->IsSelected() != bKick )
			m_pKickCheck->SetSelected( bKick );
		if ( m_pLeaderCheck->IsSelected() != bLeader )
			m_pLeaderCheck->SetSelected( bLeader );
	}
}

void PlayerListLine::UpdateVoteIcons()
{
	// count how many players are online
	int iPlayers = 0;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		if ( g_PR->IsConnected( i ) )
		{
			C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
			if ( pPlayer && !pPlayer->CanVote() )
			{
				continue;
			}

			iPlayers++;
		}
	}

	int iMaxLeaderVotes = Ceil2Int( asw_vote_leader_fraction.GetFloat() * iPlayers );
	// make sure we're not rounding down the number of needed players
	if ( iMaxLeaderVotes < 2 )
		iMaxLeaderVotes = 2;

	int iMaxKickVotes = Ceil2Int( asw_vote_kick_fraction.GetFloat() * iPlayers );
	// make sure we're not rounding down the number of needed players
	if ( iMaxKickVotes < 2 )
		iMaxKickVotes = 2;

	int iKickVotes = 0;
	if ( ASWGameResource() && m_iPlayerIndex < ASW_MAX_READY_PLAYERS )
		iKickVotes = ASWGameResource()->m_iKickVotes[m_iPlayerIndex - 1];

	int iLeaderVotes = 0;
	if ( ASWGameResource() && m_iPlayerIndex < ASW_MAX_READY_PLAYERS )
		iLeaderVotes = ASWGameResource()->m_iLeaderVotes[m_iPlayerIndex - 1];

	// position the checkbox immediately to the right of our number of votes
	float fScale = ScreenHeight() / 768.0f;
	int top = 6.0f * fScale;
	int top_line_height = 16.0f * fScale;
	m_pLeaderCheck->SetPos( ( PLAYER_LIST_LEADER_ICON_X + PLAYER_LIST_LEADER_ICON_W * iMaxLeaderVotes ) * fScale, top + top_line_height );
	m_pKickCheck->SetPos( ( PLAYER_LIST_KICK_ICON_X + PLAYER_LIST_KICK_ICON_W * iMaxKickVotes ) * fScale, top + top_line_height );

	C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( m_iPlayerIndex ) );
	bool bCanKick = pPlayer && pPlayer->CanBeKicked();
	bool bCanBeLeader = pPlayer && pPlayer->CanBeLeader();

	// decide upon states for the icons
	int iKickIconState[MAX_VOTE_ICONS];
	int iLeaderIconState[MAX_VOTE_ICONS];
	for ( int i = 0; i < MAX_VOTE_ICONS; i++ )
	{
		if ( i >= iMaxKickVotes || !bCanKick )
		{
			iKickIconState[i] = 0;
		}
		else
		{
			if ( i >= iKickVotes )
			{
				iKickIconState[i] = 1;
			}
			else
			{
				iKickIconState[i] = 2;
			}
		}

		if ( i >= iMaxLeaderVotes || !bCanBeLeader )
		{
			iLeaderIconState[i] = 0;
		}
		else
		{
			if ( i >= iLeaderVotes )
			{
				iLeaderIconState[i] = 1;
			}
			else
			{
				iLeaderIconState[i] = 2;
			}
		}
	}

	// make sure visibility and images match up with the decided upon states for each icon
	for ( int i = 0; i < MAX_VOTE_ICONS; i++ )
	{
		if ( m_iKickIconState[i] != iKickIconState[i] )
		{
			m_iKickIconState[i] = iKickIconState[i];
			if ( m_iKickIconState[i] == 0 )
			{
				m_pKickVoteIcon[i]->SetVisible( false );
			}
			else if ( m_iKickIconState[i] == 1 )
			{
				m_pKickVoteIcon[i]->SetVisible( true );
				m_pKickVoteIcon[i]->SetDrawColor( Color( 65, 74, 96, 255 ) );
			}
			else if ( m_iKickIconState[i] == 2 )
			{
				m_pKickVoteIcon[i]->SetVisible( true );
				m_pKickVoteIcon[i]->SetDrawColor( Color( 255, 255, 255, 255 ) );
			}
		}
		if ( m_iLeaderIconState[i] != iLeaderIconState[i] )
		{
			m_iLeaderIconState[i] = iLeaderIconState[i];
			if ( m_iLeaderIconState[i] == 0 )
			{
				m_pLeaderVoteIcon[i]->SetVisible( false );
			}
			else if ( m_iLeaderIconState[i] == 1 )
			{
				m_pLeaderVoteIcon[i]->SetVisible( true );
				m_pLeaderVoteIcon[i]->SetDrawColor( Color( 65, 74, 96, 255 ) );
			}
			else if ( m_iLeaderIconState[i] == 2 )
			{
				m_pLeaderVoteIcon[i]->SetVisible( true );
				m_pLeaderVoteIcon[i]->SetDrawColor( Color( 255, 255, 255, 255 ) );
			}
		}
	}
}

bool PlayerListLine::SetPlayerIndex( int i )
{
	if ( i != m_iPlayerIndex )
	{
		m_iPlayerIndex = i;
		// todo: update with his marines etc?
		return true;
	}
	return false;
}

const wchar_t *PlayerListLine::GetFragsString()
{
	static wchar_t buffer[12];
	Q_snwprintf( buffer, sizeof( buffer ), L"%d", g_PR->GetPlayerScore( m_iPlayerIndex ) );
	return buffer;
}

const wchar_t *PlayerListLine::GetDeathsString()
{
	static wchar_t buffer[12];
	Q_snwprintf( buffer, sizeof( buffer ), L"%d", g_PR->GetDeaths( m_iPlayerIndex ) );
	return buffer;
}

const wchar_t *PlayerListLine::GetPingString()
{
	static wchar_t buffer[12];
	if ( g_PR->GetPing( m_iPlayerIndex ) < 1 )
	{
		C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( m_iPlayerIndex ) );
		if ( pPlayer && pPlayer->IsAnyBot() )
		{
			return g_pVGuiLocalize->FindSafe( "#asw_player_list_ping_bot" );
		}

		return L"";
	}

	Q_snwprintf( buffer, sizeof( buffer ), L"%d", g_PR->GetPing( m_iPlayerIndex ) );
	return buffer;
}

//======================================================

VoteCheck::VoteCheck(Panel *parent, const char *panelName, const char *text) :
	vgui::CheckButton(parent, panelName, text)
{

}

void VoteCheck::DoClick()
{
	if ( GetParent() && GetParent()->GetParent() &&
		GetParent()->GetParent()->GetParent() &&
		GetParent()->GetParent()->GetParent()->GetParent() )	// lol? 
	{
		PlayerListLine *pLine = assert_cast< PlayerListLine * >( GetParent() );
		PlayerListPanel *pPanel = assert_cast< PlayerListPanel * >( GetParent()->GetParent()->GetParent()->GetParent() );
		if ( pLine && pPanel )
		{
			if ( pLine->m_pKickCheck == this )
				pPanel->KickClicked( pLine );
			else if ( pLine->m_pLeaderCheck == this )
				pPanel->LeaderClicked( pLine );
		}
	}
}
