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
		if ( !V_strcmp( szLang, "english" ) || !V_strcmp( szLang, "german" ) || !V_strcmp( szLang, "italian" ) )
		{
			s_chAlphabet = 'L'; // Latin
		}
		else if ( !V_strcmp( szLang, "russian" ) )
		{
			s_chAlphabet = 'C'; // Cyrillic
		}
		else if ( !V_strcmp( szLang, "schinese" ) )
		{
			s_chAlphabet = 'H'; // Han
		}
		else
		{
			s_chAlphabet = 'L'; // default to Latin charset for languages that haven't yet been translated
		}
	}

	return category == s_chAlphabet;
}

PRECACHE_REGISTER_BEGIN( GLOBAL, PrecacheASWScalableText )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_a", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_b", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_d", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_e", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_f", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_g", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_h", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_i", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_l", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_m", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_n", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_o", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_p", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_r", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_s", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_t", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_u", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_z", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00c3", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00cd", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_a_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_b_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_d_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_e_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_f_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_g_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_h_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_i_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_l_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_m_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_n_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_o_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_p_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_r_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_s_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_t_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_u_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_z_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00c3_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00cd_glow", IsAlphabet( 'L' ) )
	
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0410", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0412", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0415", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0417", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0418", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041a", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041b", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041c", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041d", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041e", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041f", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0420", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0421", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0428", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_042f", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0410_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0412_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0415_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0417_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0418_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041a_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041b_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041c_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041d_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041e_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_041f_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0420_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0421_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0428_glow", IsAlphabet( 'C' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_042f_glow", IsAlphabet( 'C' ) )

	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_3002", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_4e86", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_4efb", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5229", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_52a1", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5317", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5355", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_573a", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5931", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5b8c", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5df2", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5f79", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_61be", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6210", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6218", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_672c", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6b21", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6b64", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_8d25", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_9057", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_987a", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_ff01", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_3002_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_4e86_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_4efb_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5229_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_52a1_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5317_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5355_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_573a_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5931_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5b8c_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5df2_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_5f79_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_61be_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6210_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6218_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_672c_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6b21_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_6b64_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_8d25_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_9057_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_987a_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_ff01_glow", IsAlphabet( 'H' ) )
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
	/*
	// To get letter widths:
	var ctx = document.createElement("canvas").getContext("2d");
	ctx.font = 'bold 155px "Neo Sans Pro"'; // Latin/Cyrillic
	ctx.font = 'bold 155px "Noto Sans CJK SC"'; // Han
	(ctx.measureText("A").width / 100).toFixed(3);

	// To create 256x256 textures, use fonts listed above in GIMP in white at size 224.
	// Align with 32px below top edge, flush with all other edges for Latin/Cyrillic;
	// align flush with all edges for Han. To create glow version, clone letter layer,
	// gaussian blur stddev 7.5, duplicate blurred layer twice, and merge all four layers.
	*/

	switch ( ch )
	{
	case L'A':
		return 1.037f;
	case L'B':
		return 1.037f;
	case L'C':
		return 0.869f;
	case L'D':
		return 1.057f;
	case L'E':
		return 0.899f;
	case L'F':
		return 0.901f;
	case L'G':
		return 1.000f;
	case L'H':
		return 1.130f;
	case L'I':
		return 0.482f;
	case L'L':
		return 0.804f;
	case L'M':
		return 1.265f;
	case L'N':
		return 1.130f;
	case L'O':
		return 1.082f;
	case L'P':
		return 1.009f;
	case L'R':
		return 1.054f;
	case L'S':
		return 0.911f;
	case L'T':
		return 0.910f;
	case L'U':
		return 1.102f;
	case L'Z':
		return 0.953f;
	case L'Ã':
		return 1.037f;
	case L'Í':
		return 0.482f;
	case L'А':
		return 1.012f;
	case L'В':
		return 0.986f;
	case L'Е':
		return 0.885f;
	case L'З':
		return 0.938f;
	case L'И':
		return 1.085f;
	case L'К':
		return 0.972f;
	case L'Л':
		return 1.012f;
	case L'М':
		return 1.211f;
	case L'Н':
		return 1.085f;
	case L'О':
		return 1.056f;
	case L'П':
		return 1.085f;
	case L'Р':
		return 0.942f;
	case L'С':
		return 0.876f;
	case L'Ш':
		return 1.477f;
	case L'Я':
		return 1.014f;
	default:
		if ( ch >= 0x2E80 )
			return 1.550f;
		else
			return 1.0f;
	}
}
