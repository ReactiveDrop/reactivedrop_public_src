#pragma once

#include <vgui_controls/EditablePanel.h>

struct LeaderboardEntry_t;
struct LeaderboardScoreDetails_Points_t;

class CRD_VGUI_Commander_Mini_Profile : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Commander_Mini_Profile, vgui::EditablePanel );

	CRD_VGUI_Commander_Mini_Profile( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Commander_Mini_Profile();

	void SetHoIAFData( const LeaderboardEntry_t &entry,	const LeaderboardScoreDetails_Points_t &details );
	void ClearHoIAFData();

	bool m_bEmbedded{ false };
};
