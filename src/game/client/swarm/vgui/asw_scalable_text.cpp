#include "cbase.h"
#include <vgui_controls/Panel.h>
#include "asw_scalable_text.h"
#include <vgui/isurface.h>
#include "steam/isteamapps.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static bool IsAlphabet( char category )
{
	static char s_chAlphabet = '\0';

	if ( !s_chAlphabet )
	{
		// Only compute this once. If the user changes their language during a game session, we might precache the wrong materials, but otherwise no issue.
		Assert( SteamApps() );
		if ( !SteamApps() )
		{
			s_chAlphabet = 'e'; // error
			return false;
		}

		const char *szLang = SteamApps()->GetCurrentGameLanguage();
		if ( !V_strcmp( szLang, "english" ) || !V_strcmp( szLang, "german" ) || !V_strcmp( szLang, "italian" ) || !V_strcmp( szLang, "vietnamese" ) )
		{
			s_chAlphabet = 'L'; // Latin
		}
		else if ( !V_strcmp( szLang, "russian" ) )
		{
			s_chAlphabet = 'C'; // Cyrillic
		}
		else if ( !V_strcmp( szLang, "koreana" ) || !V_strcmp( szLang, "schinese" ) )
		{
			s_chAlphabet = 'H'; // Han/Hangul
		}
		else
		{
			s_chAlphabet = 'L'; // default to Latin charset for languages that haven't yet been translated
		}
	}

	return category == s_chAlphabet;
}


PRECACHE_REGISTER_BEGIN( GLOBAL, PrecacheASWScalableText )
#define DECLARE_SCALABLE_LETTER( codepoint, c, advance, charset ) \
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_" codepoint, IsAlphabet( charset ) ) \
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_" codepoint "_glow", IsAlphabet( charset ) )
#include "asw_scalable_text.inc"
#undef DECLARE_SCALABLE_LETTER
PRECACHE_REGISTER_END()

CASW_Scalable_Text* g_pScalableText = NULL;
CASW_Scalable_Text* ASWScalableText()
{
	if ( !g_pScalableText )
	{
		g_pScalableText = new CASW_Scalable_Text();
	}
	return g_pScalableText;
}

CASW_Scalable_Text::CASW_Scalable_Text() : m_nLetterTextureID( DefLessFunc( wchar_t ) ), m_nGlowLetterTextureID( DefLessFunc( wchar_t ) )
{
}

CASW_Scalable_Text::~CASW_Scalable_Text()
{

}

int CASW_Scalable_Text::GetLetterTexture( wchar_t ch, bool bGlow )
{
	CUtlMap<wchar_t, int> &textures = bGlow ? m_nGlowLetterTextureID : m_nLetterTextureID;
	ushort index = textures.Find( ch );
	if ( index != textures.InvalidIndex() )
	{
		return textures[index];
	}

	char szMaterialName[256]{};
	if ( ch >= 'A' && ch <= 'Z' )
	{
		V_snprintf( szMaterialName, sizeof( szMaterialName ), "vgui/letters/letter_%c%s", ch - 'A' + 'a', bGlow ? "_glow" : "" );
	}
	else
	{
		V_snprintf( szMaterialName, sizeof( szMaterialName ), "vgui/letters/letter_%04x%s", ch, bGlow ? "_glow" : "" );
	}

	int textureID = vgui::surface()->CreateNewTextureID();

	textures.Insert( ch, textureID );

	vgui::surface()->DrawSetTextureFile( textureID, szMaterialName, true, false );

	return textureID;
}

float CASW_Scalable_Text::GetLetterWidth( wchar_t ch )
{
	switch ( ch )
	{
#define DECLARE_SCALABLE_LETTER( codepoint, c, advance, charset ) \
	case c: \
		return advance / 256.0f;
#include "asw_scalable_text.inc"
#undef DECLARE_SCALABLE_LETTER
	default:
		return 0.75f;
	}
}
