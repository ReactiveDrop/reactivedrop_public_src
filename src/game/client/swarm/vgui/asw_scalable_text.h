#ifndef _INCLUDED_ASW_SCALABLE_TEXT_H
#define _INCLUDED_ASW_SCALABLE_TEXT_H
#ifdef _WIN32
#pragma once
#endif

#define NUM_SCALABLE_LETTERS 26

class CASW_Scalable_Text
{
public:
	CASW_Scalable_Text();
	virtual ~CASW_Scalable_Text();

	int DrawSetLetterTexture( wchar_t ch, bool bGlow );

	CUtlMap<wchar_t, int> m_nLetterTextureID;
	CUtlMap<wchar_t, int> m_nGlowLetterTextureID;
};

CASW_Scalable_Text* ASWScalableText();

#endif // _INCLUDED_ASW_SCALABLE_TEXT_H














