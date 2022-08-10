#include "cbase.h"
#include "rd_font_hack.h"
#include "filesystem.h"
#include <vgui_controls/Controls.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include "steam/steam_api.h"
#include "winlite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define REPLACEMENT_FONT_PATH "resource/NotoSansSC-Regular.otf"
#define REPLACEMENT_FONT_NAME "Noto Sans SC"

CRD_Font_Hack g_ReactiveDropFontHack;

bool CRD_Font_Hack::Init()
{
	char szFullPath[MAX_PATH];
	g_pFullFileSystem->RelativePathToFullPath( REPLACEMENT_FONT_PATH, "GAME", szFullPath, sizeof( szFullPath ) );
	int nFontsLoaded = AddFontResourceExA( szFullPath, FR_PRIVATE, NULL );
	if ( !nFontsLoaded )
	{
		Error( "Failed to load fallback font." );
		return false;
	}

	// BenLubar: We're going to go into the engine's memory and mess with its list of font names.
	const char **ppszAllowedFont = reinterpret_cast< const char ** >( vgui::input() ) - 0x77B;
	AssertValidStringPtr( ppszAllowedFont[0] );
	AssertValidStringPtr( ppszAllowedFont[10] );
	AssertValidStringPtr( ppszAllowedFont[13] );
	AssertValidStringPtr( ppszAllowedFont[21] );
	Assert( !V_strcmp( ppszAllowedFont[0], "Marlett" ) );
	Assert( !ppszAllowedFont[1] );
	Assert( !V_strcmp( ppszAllowedFont[10], "Tahoma" ) );
	Assert( !ppszAllowedFont[11] );
	Assert( !ppszAllowedFont[12] );
	Assert( !V_strcmp( ppszAllowedFont[13], "Tahoma" ) );
	Assert( !ppszAllowedFont[20] );
	Assert( !V_strcmp( ppszAllowedFont[21], "Tahoma" ) );
	ppszAllowedFont[10] = REPLACEMENT_FONT_NAME;
	ppszAllowedFont[13] = REPLACEMENT_FONT_NAME;
	ppszAllowedFont[21] = REPLACEMENT_FONT_NAME;

	// BenLubar: There's one last font name reference, and it's in a function's implementation, so this one is a big sticky mess:
	char *pSetFontGlyphSetWrapper = ( *reinterpret_cast< char *** >( vgui::surface() ) )[68];
	char *pSetFontGlyphSetFunc = pSetFontGlyphSetWrapper + 57 + *reinterpret_cast< const int * >( &pSetFontGlyphSetWrapper[53] );
	ppszAllowedFont = reinterpret_cast< const char ** >( pSetFontGlyphSetFunc + 129 + *reinterpret_cast< const int * >( &pSetFontGlyphSetFunc[115] ) );
	AssertValidStringPtr( *ppszAllowedFont );
	Assert( !V_strcmp( *ppszAllowedFont, "Tahoma" ) );
	DWORD oldProtect{};
	VirtualProtect( reinterpret_cast< char * >( ppszAllowedFont ) - 1, 5, PAGE_EXECUTE_READWRITE, &oldProtect );
	*ppszAllowedFont = REPLACEMENT_FONT_NAME;
	VirtualProtect( reinterpret_cast< char * >( ppszAllowedFont ) - 1, 5, oldProtect, &oldProtect );
	FlushInstructionCache( GetCurrentProcess(), reinterpret_cast< const char * >( ppszAllowedFont ) - 1, 5 );

	// BenLubar: vgui2 uses the Steam language rather than the game language by default. Fix that.
	// This causes languages that do not use Latin or Cyrillic characters to use the fallback font exclusively.
	if ( SteamApps() )
	{
		vgui::scheme()->SetLanguage( SteamApps()->GetCurrentGameLanguage() );
	}

	// BenLubar: Our final hurdle is that ReloadFonts doesn't actually do anything if the screen size hasn't changed. Luckily, we can fake it.
	vgui::surface()->ForceScreenSizeOverride( true, 1, 1 );
	vgui::scheme()->ReloadFonts();
	vgui::surface()->ForceScreenSizeOverride( false, 0, 0 );
	vgui::scheme()->ReloadFonts();

	return true;
}
