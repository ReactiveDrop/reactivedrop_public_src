#include "cbase.h"
#include <vgui_controls/Panel.h>
#include "asw_scalable_text.h"
#include <vgui/isurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

PRECACHE_REGISTER_BEGIN( GLOBAL, PrecacheASWScalableText )
	PRECACHE( MATERIAL, "vgui/letters/letter_a" )
	PRECACHE( MATERIAL, "vgui/letters/letter_c" )
	PRECACHE( MATERIAL, "vgui/letters/letter_d" )
	PRECACHE( MATERIAL, "vgui/letters/letter_e" )
	PRECACHE( MATERIAL, "vgui/letters/letter_f" )
	PRECACHE( MATERIAL, "vgui/letters/letter_g" )
	PRECACHE( MATERIAL, "vgui/letters/letter_i" )
	PRECACHE( MATERIAL, "vgui/letters/letter_l" )
	PRECACHE( MATERIAL, "vgui/letters/letter_m" )
	PRECACHE( MATERIAL, "vgui/letters/letter_n" )
	PRECACHE( MATERIAL, "vgui/letters/letter_o" )
	PRECACHE( MATERIAL, "vgui/letters/letter_p" )
	PRECACHE( MATERIAL, "vgui/letters/letter_s" )
	PRECACHE( MATERIAL, "vgui/letters/letter_t" )
	PRECACHE( MATERIAL, "vgui/letters/letter_a_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_c_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_d_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_e_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_f_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_g_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_i_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_l_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_m_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_n_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_o_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_p_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_s_glow" )
	PRECACHE( MATERIAL, "vgui/letters/letter_t_glow" )
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

int CASW_Scalable_Text::DrawSetLetterTexture( wchar_t ch, bool bGlow )
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
