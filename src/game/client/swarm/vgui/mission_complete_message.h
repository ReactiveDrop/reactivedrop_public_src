#ifndef _INCLUDED_MISSION_COMPLETE_MESSAGE_H
#define _INCLUDED_MISSION_COMPLETE_MESSAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>

class vgui::Label;
class vgui::ImagePanel;
class vgui::TextEntry;

struct CAnimating_Letter
{
	float m_flStartTime;
	float m_flEndTime;
	float m_flStartX;
	float m_flStartY;
	float m_flEndX;
	float m_flEndY;
	wchar_t m_chLetter;
};

class CMission_Complete_Message : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMission_Complete_Message, vgui::EditablePanel );
public:
	CMission_Complete_Message( vgui::Panel *parent, const char *name );
	virtual ~CMission_Complete_Message();
	
	virtual void PerformLayout();
	virtual void Paint();

	void PaintMessageBackground();
	void StartMessage( bool bSuccess );
	void AddWord( const wchar_t *wszWord, int row_middle_x, int row_middle_y, float & flStartTime, float flLetterTimeInterval );
	void AddLetter( wchar_t letter, int x, int y, float letter_offset, float flStartTime );
	void PaintLetters();
	void PaintLetter( CAnimating_Letter *pLetter, bool bGlow );
	bool m_bSuccess;
	float m_flMessageBackgroundStartTime;
	float m_flMessageBackgroundFadeDuration;
	float m_flStartWidth;
	float m_flStartHeight;
	float m_flEndWidth;
	float m_flEndHeight;
	
	CUtlVector<CAnimating_Letter*> m_aAnimatingLetters;
};

#endif // _INCLUDED_MISSION_COMPLETE_MESSAGE_H














