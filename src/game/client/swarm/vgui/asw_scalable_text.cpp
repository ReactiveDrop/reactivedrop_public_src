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
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_v", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_z", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0020", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00c0", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00c3", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00cd", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0110", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ea0", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ea4", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ebe", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ec6", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1eca", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ee4", IsAlphabet( 'L' ) )
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
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_v_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_z_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0020_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00c0_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00c3_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_00cd_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_0110_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ea0_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ea4_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ebe_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ec6_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1eca_glow", IsAlphabet( 'L' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_1ee4_glow", IsAlphabet( 'L' ) )
	
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
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_b8cc", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_bb34", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c2e4", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c644", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c784", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c791", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c804", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_d328", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_d568", IsAlphabet( 'H' ) )
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
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_b8cc_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_bb34_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c2e4_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c644_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c784_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c791_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_c804_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_d328_glow", IsAlphabet( 'H' ) )
	PRECACHE_CONDITIONAL( MATERIAL, "vgui/letters/letter_d568_glow", IsAlphabet( 'H' ) )
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
	ctx.font = '900 224px "Noto Sans CJK SC"';
	(ctx.measureText("A").width / 144).toFixed(3);

	// To create textures and materials, run create_scalable_letter.py.
	*/

	switch ( ch )
	{
	case L' ':
		return 0.356f;
	case L'A':
		return 1.027f;
	case L'B':
		return 1.081f;
	case L'C':
		return 1.037f;
	case L'D':
		return 1.134f;
	case L'E':
		return 0.980f;
	case L'F':
		return 0.940f;
	case L'G':
		return 1.140f;
	case L'H':
		return 1.204f;
	case L'I':
		return 0.544f;
	case L'L':
		return 0.929f;
	case L'M':
		return 1.364f;
	case L'N':
		return 1.188f;
	case L'O':
		return 1.223f;
	case L'P':
		return 1.069f;
	case L'R':
		return 1.101f;
	case L'S':
		return 0.994f;
	case L'T':
		return 0.996f;
	case L'U':
		return 1.187f;
	case L'V':
		return 1.000f;
	case L'Z':
		return 0.963f;
	case L'À':
		return 1.027f;
	case L'Ã':
		return 1.027f;
	case L'Í':
		return 0.544f;
	case L'Đ':
		return 1.179f;
	case L'А':
		return 1.027f;
	case L'В':
		return 1.081f;
	case L'Е':
		return 0.980f;
	case L'З':
		return 1.006f;
	case L'И':
		return 1.209f;
	case L'К':
		return 1.117f;
	case L'Л':
		return 1.176f;
	case L'М':
		return 1.364f;
	case L'Н':
		return 1.204f;
	case L'О':
		return 1.223f;
	case L'П':
		return 1.184f;
	case L'Р':
		return 1.069f;
	case L'С':
		return 1.037f;
	case L'Ш':
		return 1.630f;
	case L'Я':
		return 1.104f;
	case L'Ạ':
		return 1.027f;
	case L'Ấ':
		return 1.027f;
	case L'Ế':
		return 0.980f;
	case L'Ệ':
		return 0.980f;
	case L'Ị':
		return 0.544f;
	case L'Ụ':
		return 1.187f;
	default:
		if ( ch >= 0x2E80 )
			return 1.556f;
		else
			return 1.0f;
	}
}
