//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "vgui_avatarimage.h"
#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif
#include "steam/steam_api.h"
#include "hud.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


DECLARE_BUILD_FACTORY( CAvatarImagePanel );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAvatarImage::CAvatarImage( void )
{
	m_iTextureID = -1;
	ClearAvatarSteamID();
	m_SourceArtSize = k_EAvatarSize32x32;
	m_pFriendIcon = NULL;
	m_nX = 0;
	m_nY = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::ClearAvatarSteamID( void ) 
{ 
	m_bValid = false; 
	m_bFriend = false;
	m_SteamID.Clear();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAvatarImage::SetAvatarSteamID( CSteamID steamIDUser, bool bFetch )
{
	if ( m_steamIDUser == steamIDUser && m_bValid )
		return true;

	ClearAvatarSteamID();
	m_steamIDUser = steamIDUser;

	if ( SteamFriends() && SteamUtils() )
	{
		m_SteamID = steamIDUser;

		// if this goes through we'll get a callback
		if ( bFetch )
			SteamFriends()->RequestUserInformation( m_SteamID, false );

		int iAvatar;

		switch ( m_SourceArtSize )
		{
		case k_EAvatarSize32x32:
			iAvatar = SteamFriends()->GetSmallFriendAvatar( steamIDUser );
			break;
		case k_EAvatarSize64x64:
			iAvatar = SteamFriends()->GetMediumFriendAvatar( steamIDUser );
			break;
		case k_EAvatarSize184x184:
		default:
			iAvatar = SteamFriends()->GetLargeFriendAvatar( steamIDUser );
			break;
		}

		/*
		// See if it's in our list already
		*/

		uint32 wide, tall;
		if ( SteamUtils()->GetImageSize( iAvatar, &wide, &tall ) )
		{
			int cubImage = wide * tall * 4;
			byte *rgubDest = ( byte * )_alloca( cubImage );
			SteamUtils()->GetImageRGBA( iAvatar, rgubDest, cubImage );
			InitFromRGBA( rgubDest, wide, tall );
		}

		UpdateFriendStatus();
	}

	return m_bValid;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::UpdateFriendStatus( void )
{
	if ( !m_SteamID.IsValid() )
		return;

	if ( SteamFriends() && SteamUtils() )
	{
		m_bFriend = SteamFriends()->HasFriend( m_SteamID, k_EFriendFlagImmediate );
		if ( m_bFriend && !m_pFriendIcon )
		{
			m_pFriendIcon = HudIcons().GetIcon( "ico_friend_indicator_avatar" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::InitFromRGBA( const byte *rgba, int width, int height )
{
	if ( m_iTextureID == -1 )
	{
		m_iTextureID = vgui::surface()->CreateNewTextureID( true );
	}

	vgui::surface()->DrawSetTextureRGBA( m_iTextureID, rgba, width, height );
	m_nWide = YRES( width );
	m_nTall = YRES( height );
	m_Color = Color( 255, 255, 255, 255 );

	m_bValid = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImage::Paint( void )
{
	if ( m_bValid )
	{
		if ( m_bFriend && m_pFriendIcon && m_bLegacyPadding )
		{
			m_pFriendIcon->DrawSelf( m_nX, m_nY, m_nWide, m_nTall, m_Color );
		}

		vgui::surface()->DrawSetColor( m_Color );
		vgui::surface()->DrawSetTexture( m_iTextureID );
		if ( m_bLegacyPadding )
			vgui::surface()->DrawTexturedRect( m_nX + AVATAR_INDENT_X, m_nY + AVATAR_INDENT_Y, m_nX + AVATAR_INDENT_X + m_iAvatarWidth, m_nY + AVATAR_INDENT_Y + m_iAvatarHeight );
		else
			vgui::surface()->DrawTexturedRect( m_nX, m_nY, m_nX + m_iAvatarWidth, m_nY + m_iAvatarHeight );
	}
}

void CAvatarImage::OnPersonaStateChange( PersonaStateChange_t *pParam )
{
	if ( m_steamIDUser == pParam->m_ulSteamID )
	{
		// re-init
		m_bValid = false;
		SetAvatarSteamID( m_steamIDUser, false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAvatarImagePanel::CAvatarImagePanel( vgui::Panel *parent, const char *name ) : vgui::ImagePanel( parent, name )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImagePanel::SetPlayer( C_BasePlayer *pPlayer )
{
	if ( pPlayer )
	{
		int iIndex = pPlayer->entindex();
		SetPlayerByIndex( iIndex );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImagePanel::SetPlayerByIndex( int iIndex )
{
	if ( iIndex && SteamUtils() )
	{
		player_info_t pi;
		if ( engine->GetPlayerInfo(iIndex, &pi) )
		{
			if ( pi.friendsID )
			{
				CSteamID steamIDForPlayer( pi.friendsID, 1, SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );
				SetAvatarBySteamID( &steamIDForPlayer );
			}
		}
	}
}

void CAvatarImagePanel::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( CAvatarImage *pImage = assert_cast< CAvatarImage * >( GetImage() ) )
	{
		int iIndent = 2;
		pImage->SetPos( iIndent, iIndent );
		int wide = GetWide() - ( iIndent * 2 );
		pImage->SetAvatarSize( ( wide > 32 ) ? k_EAvatarSize64x64 : k_EAvatarSize32x32 );
		pImage->SetAvatarSize( wide, GetTall() - ( iIndent * 2 ), m_bLegacyPadding );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAvatarImagePanel::PaintBackground( void )
{
	vgui::IImage *pImage = GetImage();
	if ( pImage )
	{
		pImage->SetColor( GetDrawColor() );
		pImage->Paint();
	}
}

void CAvatarImagePanel::SetAvatarBySteamID( const CSteamID *friendsID )
{
	CAvatarImage *pImage = assert_cast< CAvatarImage * > ( GetImage() );
	if ( !pImage )
	{
		pImage = new CAvatarImage();
		SetImage( pImage );
	}

	// Indent the image. These are deliberately non-resolution-scaling.
	int iIndent = 2;
	pImage->SetPos( iIndent, iIndent );
	int wide = GetWide() - ( iIndent * 2 );

	// fix aspect ratio
	if ( m_bLegacyPadding )
		wide += AVATAR_INDENT_Y - AVATAR_INDENT_X;

	pImage->SetAvatarSize( ( wide > 32 ) ? k_EAvatarSize64x64 : k_EAvatarSize32x32 );
	if ( friendsID )
		pImage->SetAvatarSteamID( *friendsID );
	else
		pImage->ClearAvatarSteamID();

	pImage->SetAvatarSize( wide, GetTall() - ( iIndent * 2 ), m_bLegacyPadding );
}
