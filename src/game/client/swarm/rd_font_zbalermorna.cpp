#include "cbase.h"
#include "rd_font_zbalermorna.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// this table maps ASCII characters to indices in the glyph table.
constexpr unsigned char s_zbalermornaIndex[128] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	51, 25, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 27, 26, 10, 0,
	0, 13, 14, 15, 16, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0,
	47, 41, 0, 0, 0, 42, 0, 0, 0, 43, 0, 0, 0, 0, 0, 44,
	0, 0, 0, 0, 0, 45, 0, 0, 0, 46, 0, 48, 0, 49, 50, 28,
	35, 29, 17, 7, 18, 30, 4, 19, 0, 31, 23, 3, 5, 8, 24, 32,
	1, 39, 21, 6, 2, 33, 20, 40, 9, 34, 22, 36, 0, 37, 38, 0,
};

// many of the glyphs share widths, so we can save space by mapping them to indices in this array
constexpr float s_zbalermornaAdvance1[] =
{
	0.0f,
	0.3f,
	0.46f,
	0.6f,
	0.6f,
	0.6f,
	0.6f,
};

// we can store a separate advance value for combining characters
constexpr float s_zbalermornaAdvance2[] =
{
	0.0f,
	0.15f,
	0.36f,
	0.3f,
	0.2f,
	0.4f,
	0.5f,
};

// similarly, for the other offsets, we can store 7-bit indices and use 98*4+51*4=596 bytes of space rather than storing 32-bit floats and using 816 bytes
// (it's actually even better because we have 4 bits to use for other things left over)
constexpr float s_zbalermornaOffset[] =
{
	-0.00025f, -0.011625f, -0.014625f, -0.01525f,
	-0.021875f, -0.0375f, -0.04f, -0.0475f,
	-0.057375f, -0.058875f, -0.059375f, -0.061f,
	-0.06109038669551109f, -0.061625f, -0.063125f, -0.066375f,
	-0.071375f, -0.0715f, -0.07325f, -0.074f,
	-0.075f, -0.0755f, -0.07725f, -0.0775f,
	-0.07975f, -0.0965f, -0.15625f, -0.171875f,
	-0.188f, -0.21925f, -0.234375f, -0.245875f,
	-0.2475f, -0.265625f, -0.266125f, -0.298875f,
	-0.34375f, -0.38975f, -0.39225f, -0.405375f,
	-0.406875f, -0.415f, -0.4225f, -0.436f,
	-0.45275f, -0.462375f, -0.480375f, -0.483f,
	-0.483375f, -0.589375f, -0.591875f, -0.594375f,
	-0.594875f, -0.596875f, -0.599375f, -0.842875f,
	-0.846875f, 0.006375f, 0.017f, 0.019125f,
	0.119375f, 0.121875f, 0.123875f, 0.124375f,
	0.125875f, 0.129375f, 0.15625f, 0.171875f,
	0.187f, 0.21825f, 0.222875f, 0.234375f,
	0.2525f, 0.265125f, 0.265625f, 0.294875f,
	0.31225f, 0.321875f, 0.3375f, 0.343125f,
	0.34375f, 0.3451596133044889f, 0.36025f, 0.362875f,
	0.36425f, 0.371875f, 0.376875f, 0.522375f,
	0.5285f, 0.652375f, 0.659375f, 0.659875f,
	0.661375f, 0.6725f, 0.6745f, 0.675f,
	0.676f, 0.6785f,
};

// the glyph table!
constexpr struct zbalermorna_glyph
{
	// distance to move cursor
	unsigned advance : 3;
	// texture coordinates
	unsigned s0 : 7;
	unsigned t0 : 7;
	unsigned s1 : 5;
	unsigned t1 : 5;
	// 0 = nothing special
	// 1 = rotate 90 degrees ccw
	// 2 = combining above
	// 3 = combining below
	unsigned special : 2;
	// 3 bits spare

	// visual glyph position
	unsigned x0 : 7;
	unsigned y0 : 7;
	unsigned x1 : 7;
	unsigned y1 : 7;
	// 4 bits spare
} s_zbalermornaGlyphs[] =
{
	{ 0, 127, 127, 0, 0, 0, 0, 0, 0, 0 }, // null glyph (for ASCII characters not associated with a glyph)
	{ 5, 0, 0, 23, 31, 0, 10, 56, 90, 61 }, // ZBALERMORNA LETTER PY "p"
	{ 5, 23, 0, 23, 31, 0, 10, 56, 90, 61 }, // ZBALERMORNA LETTER TY "t"
	{ 5, 46, 0, 23, 31, 0, 10, 56, 90, 61 }, // ZBALERMORNA LETTER KY "k"
	{ 3, 93, 31, 23, 23, 0, 8, 51, 92, 63 }, // ZBALERMORNA LETTER FY "f"
	{ 3, 0, 54, 23, 23, 0, 15, 51, 89, 63 }, // ZBALERMORNA LETTER LY "l"
	{ 4, 69, 0, 24, 31, 0, 23, 55, 93, 64 }, // ZBALERMORNA LETTER SY "s"
	{ 4, 93, 0, 23, 31, 0, 10, 56, 90, 61 }, // ZBALERMORNA LETTER CY "c"
	{ 3, 71, 54, 24, 23, 0, 21, 54, 94, 60 }, // ZBALERMORNA LETTER MY "m"
	{ 3, 95, 54, 24, 23, 0, 20, 51, 95, 63 }, // ZBALERMORNA LETTER XY "x"
	{ 1, 33, 100, 12, 12, 0, 5, 42, 78, 7 }, // ZBALERMORNA SYMBOL FOR DENPA BU "."
	{ 1, 116, 0, 11, 23, 0, 4, 51, 77, 63 }, // ZBALERMORNA SYMBOL FOR YHY "'"
	{ 2, 23, 77, 20, 23, 0, 25, 51, 88, 63 }, // ZBALERMORNA SYMBOL FOR CNIMAHO BU  ";"
	{ 0, 0, 100, 16, 13, 3, 32, 14, 72, 79 }, // ZBALERMORNA COMBINING PA TONGA "1"
	{ 0, 0, 100, 16, 13, 3, 70, 22, 31, 82 }, // ZBALERMORNA COMBINING RE TONGA "2"
	{ 0, 108, 77, 19, 13, 3, 35, 12, 75, 81 }, // ZBALERMORNA COMBINING CI TONGA "3"
	{ 0, 108, 77, 19, 13, 3, 75, 84, 35, 18 }, // ZBALERMORNA COMBINING VO TONGA "4"
	{ 6, 0, 0, 23, 31, 0, 90, 86, 10, 50 }, // ZBALERMORNA LETTER BY "b"
	{ 3, 23, 0, 23, 31, 0, 90, 86, 10, 50 }, // ZBALERMORNA LETTER DY "d"
	{ 3, 46, 0, 23, 31, 0, 90, 86, 10, 50 }, // ZBALERMORNA LETTER GY "g"
	{ 3, 93, 31, 23, 23, 0, 90, 63, 10, 51 }, // ZBALERMORNA LETTER VY "v"
	{ 3, 0, 54, 23, 23, 0, 94, 63, 21, 51 }, // ZBALERMORNA LETTER RY "r"
	{ 3, 69, 0, 24, 31, 0, 97, 85, 17, 53 }, // ZBALERMORNA LETTER ZY "z"
	{ 3, 93, 0, 23, 31, 0, 90, 86, 10, 50 }, // ZBALERMORNA LETTER JY "j"
	{ 3, 71, 54, 24, 23, 0, 94, 65, 21, 49 }, // ZBALERMORNA LETTER NY "n"
	{ 0, 116, 31, 11, 11, 3, 27, 59, 67, 83 }, // ZBALERMORNA COMBINING BAHEBU "!"
	{ 2, 84, 100, 19, 10, 0, 16, 37, 87, 22 }, // ZBALERMORNA SYMBOL FOR SMAJI BU "-"
	{ 0, 127, 127, 0, 0, 0, 0, 0, 0, 0 }, // ZBALERMORNA SYMBOL FOR SLAKA BU "," (not in the sheet)
	{ 0, 62, 100, 22, 10, 0, 36, 0, 80, 76 }, // ZBALERMORNA COMBINING TCENA BU "_"
	{ 0, 84, 77, 10, 15, 2, 26, 45, 66, 57 }, // ZBALERMORNA VOWEL SIGN ABU "a"
	{ 0, 103, 100, 15, 10, 2, 30, 38, 71, 24 }, // ZBALERMORNA VOWEL SIGN EBU "e"
	{ 0, 116, 31, 11, 11, 2, 27, 40, 67, 14 }, // ZBALERMORNA VOWEL SIGN IBU "i"
	{ 0, 45, 100, 17, 11, 2, 33, 39, 74, 13 }, // ZBALERMORNA VOWEL SIGN OBU "o"
	{ 0, 16, 100, 17, 12, 2, 34, 41, 73, 6 }, // ZBALERMORNA VOWEL SIGN UBU "u"
	{ 0, 94, 77, 14, 14, 2, 29, 44, 69, 3 }, // ZBALERMORNA VOWEL SIGN YBU "y"
	{ 0, 43, 77, 12, 16, 2, 28, 47, 68, 58 }, // ZBALERMORNA VOWEL SIGN AIBU "`"
	{ 0, 16, 100, 17, 12, 2, 73, 11, 34, 43 }, // ZBALERMORNA VOWEL SIGN EIBU "{"
	{ 0, 67, 77, 17, 15, 2, 34, 48, 73, 2 }, // ZBALERMORNA VOWEL SIGN OIBU "}"
	{ 0, 55, 77, 12, 15, 2, 28, 46, 68, 1 }, // ZBALERMORNA VOWEL SIGN AUBU "~"
	{ 3, 23, 31, 23, 23, 0, 9, 51, 91, 63 }, // ZBALERMORNA LETTER SEMIVOWEL IY "q"
	{ 3, 23, 31, 23, 23, 0, 91, 63, 9, 51 }, // ZBALERMORNA LETTER SEMIVOWEL UY "w"
	{ 3, 0, 77, 23, 23, 0, 10, 51, 90, 63 }, // ZBALERMORNA LETTER FULL ABU "A"
	{ 3, 47, 54, 24, 23, 0, 21, 51, 94, 63 }, // ZBALERMORNA LETTER FULL EBU "E"
	{ 3, 23, 54, 24, 23, 0, 21, 51, 94, 63 }, // ZBALERMORNA LETTER FULL IBU "I"
	{ 3, 0, 77, 23, 23, 0, 90, 63, 10, 51 }, // ZBALERMORNA LETTER FULL OBU "O"
	{ 3, 23, 54, 24, 23, 0, 90, 63, 10, 51 }, // ZBALERMORNA LETTER FULL UBU "U"
	{ 3, 70, 31, 23, 23, 0, 10, 51, 90, 63 }, // ZBALERMORNA LETTER FULL YBU "Y"
	{ 3, 23, 54, 24, 23, 1, 10, 51, 90, 63 }, // ZBALERMORNA LETTER FULL AIBU "@"
	{ 3, 46, 31, 24, 23, 0, 19, 52, 96, 62 }, // ZBALERMORNA LETTER FULL EIBU "["
	{ 3, 23, 54, 24, 23, 1, 90, 63, 10, 51 }, // ZBALERMORNA LETTER FULL OIBU "]"
	{ 3, 0, 31, 23, 23, 0, 9, 51, 91, 63 }, // ZBALERMORNA LETTER FULL AUBU "^"
	{ 1, 127, 127, 0, 0, 0, 0, 0, 0, 0 }, // SPACE " "
};

constexpr static float s_flSizeCorrectiveFactor = 0.8f;
constexpr static float s_flFirstLineYOffset = 1.0f;
constexpr static float s_flLineHeight = 1.177f;
constexpr static float s_flCombiningOffsetBelow = 0.05f;
constexpr static float s_flCombiningOffsetAbove = -0.4f;
constexpr static float s_flCombiningOffsetAboveBlank = -0.35f;

static vgui::HTexture s_hFontTexture = NULL;

namespace zbalermorna
{
	void PaintText( float x0, float y0, const char *szText, float flTextSize, const Color &color )
	{
		vgui::ISurface *pSurface = vgui::surface();
		Assert( pSurface );
		if ( !s_hFontTexture )
		{
			s_hFontTexture = pSurface->CreateNewTextureID();
			pSurface->DrawSetTextureFile( s_hFontTexture, "vgui/fonts/manri", 1, false );
		}

		pSurface->DrawSetColor( color );
		pSurface->DrawSetTexture( s_hFontTexture );

		flTextSize *= s_flSizeCorrectiveFactor;

		float x = x0;
		float y = y0 + s_flFirstLineYOffset * flTextSize;
		float mid = x;
		float ascenderBlank = y + s_flCombiningOffsetAboveBlank * flTextSize;
		float ascenderGlyph = y + s_flCombiningOffsetAbove * flTextSize;
		float ascender = ascenderBlank;
		float descender = y + s_flCombiningOffsetBelow * flTextSize;

		for ( const char *psz = szText; *psz; psz++ )
		{
			if ( *psz == '\n' )
			{
				x = x0;
				mid = x;
				y += s_flLineHeight * flTextSize;
				ascenderBlank = y + s_flCombiningOffsetAboveBlank * flTextSize;
				ascenderGlyph = y + s_flCombiningOffsetAbove * flTextSize;
				ascender = ascenderBlank;
				descender = y + s_flCombiningOffsetBelow * flTextSize;

				continue;
			}

			int iGlyph = s_zbalermornaIndex[*psz];
			Assert( iGlyph );

			const zbalermorna_glyph &glyph = s_zbalermornaGlyphs[iGlyph];

			if ( glyph.s1 )
			{
				float glyphX = x, glyphY = y;
				if ( glyph.special == 2 )
				{
					glyphX = mid;
					glyphY = ascender;
				}
				else if ( glyph.special == 3 )
				{
					glyphX = mid;
					glyphY = descender;
				}
				else
				{
					mid = x + s_zbalermornaAdvance2[glyph.advance] * flTextSize;
					ascender = ascenderGlyph;
				}

				float s0 = ( glyph.s0 + 0.5f ) / 128.0f;
				float t0 = ( glyph.t0 + 0.5f ) / 128.0f;
				float s1 = ( glyph.s0 + glyph.s1 + 0.5f ) / 128.0f;
				float t1 = ( glyph.t0 + glyph.t1 + 0.5f ) / 128.0f;

				float gx0 = glyphX + s_zbalermornaOffset[glyph.x0] * flTextSize;
				float gy0 = glyphY + s_zbalermornaOffset[glyph.y0] * flTextSize;
				float gx1 = glyphX + s_zbalermornaOffset[glyph.x1] * flTextSize;
				float gy1 = glyphY + s_zbalermornaOffset[glyph.y1] * flTextSize;

				vgui::Vertex_t quad[4]
				{
					{ { gx0, gy0 }, { s0, t0 } },
					{ { gx1, gy0 }, { s1, t0 } },
					{ { gx1, gy1 }, { s1, t1 } },
					{ { gx0, gy1 }, { s0, t1 } },
				};
				if ( glyph.special == 1 )
				{
					// rotate counter-clockwise 90 degrees
					quad[0].m_TexCoord.x = s1;
					quad[1].m_TexCoord.y = t1;
					quad[2].m_TexCoord.x = s0;
					quad[3].m_TexCoord.y = t0;
				}

				pSurface->DrawTexturedPolygon( NELEMS( quad ), quad );
			}
			else
			{
				mid = x + s_zbalermornaAdvance1[glyph.advance] * flTextSize;
				ascender = ascenderBlank;
			}

			x += s_zbalermornaAdvance1[glyph.advance] * flTextSize;
		}
	}

	void MeasureText( const char *szText, float flTextSize, float &flWide, float &flTall, bool bLastLineWide )
	{
		float flCurrentLineWide = 0.0f;
		flTall = s_flFirstLineYOffset;

		for ( const char *psz = szText; *psz; psz++ )
		{
			if ( *psz == '\n' )
			{
				if ( flWide < flCurrentLineWide )
				{
					flWide = flCurrentLineWide;
				}

				flTall += s_flLineHeight;
				flCurrentLineWide = 0.0f;

				continue;
			}

			int iGlyph = s_zbalermornaIndex[*psz];
			Assert( iGlyph );

			const zbalermorna_glyph &glyph = s_zbalermornaGlyphs[iGlyph];

			flCurrentLineWide += s_zbalermornaAdvance1[glyph.advance];
		}

		if ( flWide < flCurrentLineWide || bLastLineWide )
		{
			flWide = flCurrentLineWide;
		}

		flTextSize *= s_flSizeCorrectiveFactor;
		flWide *= flTextSize;
		flTall *= flTextSize;
	}

	float MeasureChar( char ch, float flTextSize )
	{
		int iGlyph = s_zbalermornaIndex[ch];
		Assert( iGlyph );

		const zbalermorna_glyph &glyph = s_zbalermornaGlyphs[iGlyph];

		flTextSize *= s_flSizeCorrectiveFactor;
		return s_zbalermornaAdvance1[glyph.advance] * flTextSize;
	}
}
