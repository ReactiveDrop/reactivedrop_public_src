#ifndef _INCLUDED_ASW_SCALABLE_TEXT_H
#define _INCLUDED_ASW_SCALABLE_TEXT_H
#ifdef _WIN32
#pragma once
#endif

class CASW_Scalable_Text
{
public:
	CASW_Scalable_Text();
	virtual ~CASW_Scalable_Text();

	int GetLetterTexture( wchar_t ch, bool bGlow );
	float GetLetterWidth( wchar_t ch );

	CUtlMap<wchar_t, int> m_nLetterTextureID;
	CUtlMap<wchar_t, int> m_nGlowLetterTextureID;
};

CASW_Scalable_Text* ASWScalableText();

#endif // _INCLUDED_ASW_SCALABLE_TEXT_H
