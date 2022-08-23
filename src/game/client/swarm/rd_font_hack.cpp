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


#ifdef RD_USE_FONT_HACK

#define REPLACEMENT_FONT_PATH "resource/NotoSansSC-Regular.otf"
#define REPLACEMENT_FONT_NAME "Noto Sans SC"

CRD_Font_Hack g_ReactiveDropFontHack;

static bool __stdcall ShouldUseFallbackFontOnly(const char *szFont)
{
	if ( V_strcmp( szFont, "Neo Sans" ) )
		return false; // only swap out our main font

	const char *szLanguage = vgui::scheme()->GetLanguage();
	// Steam languages (https://partner.steamgames.com/doc/store/localization#supported_languages)
	// that are not using the Latin or Cyrillic character set.
	return !V_strcmp( szLanguage, "arabic" ) ||
		!V_strcmp( szLanguage, "bulgarian" ) ||
		!V_strcmp( szLanguage, "schinese" ) ||
		!V_strcmp( szLanguage, "tchinese" ) ||
		!V_strcmp( szLanguage, "greek" ) ||
		!V_strcmp( szLanguage, "japanese" ) ||
		!V_strcmp( szLanguage, "koreana" ) ||
		!V_strcmp( szLanguage, "thai" );
}

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

	// BenLubar: Future versions of Source Engine have a check for whether the language uses a Latin character set, and if it doesn't,
	// they use the fallback font exclusively. Our version of the Source Engine does not do this, but we are going to rewrite its code
	// so that it does do that. The function we are messing with is vgui::surface()->SetFontGlyphSet, roughly.

	// The specific part of the function we're looking at is:
	// 55                  PUSH EBP
	// 8b ce               MOV  ECX,ESI
	// e8 ?? ?? ?? ??      CALL FUN_10021910
	// 84 c0               TEST AL,AL
	// 0f 85 df 00 00 00   JNZ  LAB_1002224f

	Assert( pSetFontGlyphSetFunc[96] == '\x55' );
	Assert( pSetFontGlyphSetFunc[97] == '\x8b' );
	Assert( pSetFontGlyphSetFunc[98] == '\xce' );
	Assert( pSetFontGlyphSetFunc[99] == '\xe8' );
	Assert( pSetFontGlyphSetFunc[104] == '\x84' );
	Assert( pSetFontGlyphSetFunc[105] == '\xc0' );
	Assert( pSetFontGlyphSetFunc[106] == '\x0f' );
	Assert( pSetFontGlyphSetFunc[107] == '\x85' );
	Assert( pSetFontGlyphSetFunc[108] == '\xdf' );
	Assert( pSetFontGlyphSetFunc[109] == '\x00' );
	Assert( pSetFontGlyphSetFunc[110] == '\x00' );
	Assert( pSetFontGlyphSetFunc[111] == '\x00' );

	// We have 16 bytes to play with, and all we need to do is:
	// - Call ShouldUseFallbackFontOnly (defined above).
	// - If the answer is "yes", clear the EBX register.

	// We're going to do this as:
	// 55                  PUSH EBP
	// e8 ?? ?? ?? ??      CALL ShouldUseFallbackFontOnly
	// 84 c0               TEST AL,AL
	// 74 02               JZ [past the next instruction]
	// 31 db               XOR EBX,EBX
	// 90 90 90 90         NOP

	VirtualProtect( pSetFontGlyphSetFunc + 96, 16, PAGE_EXECUTE_READWRITE, &oldProtect );
	pSetFontGlyphSetFunc[96] = '\x55';
	pSetFontGlyphSetFunc[97] = '\xe8';
	*reinterpret_cast< int * >( &pSetFontGlyphSetFunc[98] ) = reinterpret_cast< const char * >( &ShouldUseFallbackFontOnly ) - ( pSetFontGlyphSetFunc + 102 );
	pSetFontGlyphSetFunc[102] = '\x84';
	pSetFontGlyphSetFunc[103] = '\xc0';
	pSetFontGlyphSetFunc[104] = '\x74';
	pSetFontGlyphSetFunc[105] = '\x02';
	pSetFontGlyphSetFunc[106] = '\x31';
	pSetFontGlyphSetFunc[107] = '\xdb';
	pSetFontGlyphSetFunc[108] = '\x90';
	pSetFontGlyphSetFunc[109] = '\x90';
	pSetFontGlyphSetFunc[110] = '\x90';
	pSetFontGlyphSetFunc[111] = '\x90';
	VirtualProtect( pSetFontGlyphSetFunc + 96, 16, oldProtect, &oldProtect );
	FlushInstructionCache( GetCurrentProcess(), pSetFontGlyphSetFunc + 96, 16 );

	// BenLubar: vgui2 uses the Steam language rather than the game language by default. Fix that.
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

#endif
