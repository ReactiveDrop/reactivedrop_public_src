#include "cbase.h"
#include "rd_font_hack.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "winlite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CRD_Font_Hack g_ReactiveDropFontHack;

constexpr struct
{
	const char *szFontName;
	int nRangeStart;
	int nRangeEnd;
} s_InsertFallbackFonts[] =
{
	{ "ManriASRD", 0xEC44, 0xEDBF },
};

// And now we do some memory tricks.

struct Font_t
{
	// only document the parts we need to use
	unsigned short	m_opaque1[11];
	short			m_iTall;
	unsigned short	m_iWeight;
	unsigned short	m_iFlags;
	unsigned short	m_iScanLines;
	unsigned short	m_iBlur;
};

struct FontRange_t
{
	int m_iLow;
	int m_iHigh;
	Font_t *m_pFont;
};

struct FontAmalgam_t
{
	CUtlVector<FontRange_t> m_Ranges;
	int m_opaque1[2];
};

struct FontManager_t
{
	CUtlVector<FontAmalgam_t> m_FontAmalgams;
	CUtlVector<Font_t *> m_Fonts;
	// more fields after here, but we don't use them

	bool SetFontGlyphSetWrapper( vgui::HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin, int nRangeMax );
};

static FontManager_t *s_pFontManager = NULL;
static bool( FontManager_t:: *s_pSetFontGlyphSet )( vgui::HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin, int nRangeMax );
static Font_t *( FontManager_t:: *s_pFindOrCreateFont )( const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags );

bool FontManager_t::SetFontGlyphSetWrapper( vgui::HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin, int nRangeMax )
{
	Assert( this == s_pFontManager );

	bool bRet = ( s_pFontManager->*s_pSetFontGlyphSet )( font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax );

	g_ReactiveDropFontHack.HackFont( font );

	return bRet;
}

bool CRD_Font_Hack::Init()
{
	bool bOK = vgui::surface()->AddCustomFontFile( "resource/manri.ttf" );
	Assert( bOK );
	if ( !bOK )
	{
		Error( "failed to load font resource/manri.ttf\n" );
		return false;
	}

	auto pVTable = *reinterpret_cast< char *const *const * >( vgui::surface() );
	auto pSetFontGlyphSet = pVTable[68];
	auto pGetFontManager = reinterpret_cast< FontManager_t * ( __stdcall * )( void ) >( pSetFontGlyphSet + 50 + *reinterpret_cast< const ptrdiff_t * >( pSetFontGlyphSet + 46 ) );
	auto pCallDiff = reinterpret_cast< ptrdiff_t * >( pSetFontGlyphSet + 53 );

	auto pInnerSetFontGlyphSet = pSetFontGlyphSet + 57 + *pCallDiff;
	reinterpret_cast< const void *& >( s_pSetFontGlyphSet ) = pInnerSetFontGlyphSet;
	reinterpret_cast< const void *& >( s_pFindOrCreateFont ) = pInnerSetFontGlyphSet + 86 + *reinterpret_cast< const ptrdiff_t * >( pInnerSetFontGlyphSet + 82 );

	DWORD oldProtect;
	VirtualProtect( pCallDiff, sizeof( ptrdiff_t ), PAGE_EXECUTE_READWRITE, &oldProtect );
	auto wrapper = &FontManager_t::SetFontGlyphSetWrapper;
	COMPILE_TIME_ASSERT( sizeof( wrapper ) == sizeof( ptrdiff_t ) );
	*pCallDiff = *reinterpret_cast< ptrdiff_t * >( &wrapper ) - ptrdiff_t( pSetFontGlyphSet + 57 );
	VirtualProtect( pCallDiff, sizeof( ptrdiff_t ), oldProtect, &oldProtect );
	FlushInstructionCache( GetCurrentProcess(), pSetFontGlyphSet, 57 );

	s_pFontManager = pGetFontManager();

	for ( int i = 1; i < s_pFontManager->m_FontAmalgams.Count(); i++ )
	{
		HackFont( i );
	}

	return true;
}

static Font_t *FindOrCreateFont( const char *szName, Font_t *pTemplate )
{
	return ( s_pFontManager->*s_pFindOrCreateFont )( szName, pTemplate->m_iTall, pTemplate->m_iWeight, pTemplate->m_iBlur, pTemplate->m_iScanLines, pTemplate->m_iFlags );
}

void CRD_Font_Hack::HackFont( int hFont )
{
	FontAmalgam_t &a = s_pFontManager->m_FontAmalgams[hFont];
	for ( int i = 0; i < NELEMS( s_InsertFallbackFonts ); i++ )
	{
		FOR_EACH_VEC( a.m_Ranges, j )
		{
			if ( a.m_Ranges[j].m_iLow < s_InsertFallbackFonts[i].nRangeStart && a.m_Ranges[j].m_iHigh > s_InsertFallbackFonts[i].nRangeEnd )
			{
				a.m_Ranges.InsertMultipleAfter( j, 2 );
				a.m_Ranges[j + 2] = a.m_Ranges[j];
				a.m_Ranges[j].m_iHigh = s_InsertFallbackFonts[i].nRangeStart - 1;
				a.m_Ranges[j + 2].m_iLow = s_InsertFallbackFonts[i].nRangeEnd + 1;
				a.m_Ranges[j + 1].m_pFont = FindOrCreateFont( s_InsertFallbackFonts[i].szFontName, a.m_Ranges[j].m_pFont );
				a.m_Ranges[j + 1].m_iLow = s_InsertFallbackFonts[i].nRangeStart;
				a.m_Ranges[j + 1].m_iHigh = s_InsertFallbackFonts[i].nRangeEnd;
				break;
			}
		}
	}
}
