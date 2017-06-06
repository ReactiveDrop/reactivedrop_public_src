#pragma once

#include <vgui_controls/EditablePanel.h>
#include <gameui/swarm/vgenericpanellist.h>
#include "c_asw_steamstats.h"
#include "vgui_avatarimage.h"

class CReactiveDrop_VGUI_Leaderboard_Panel : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CReactiveDrop_VGUI_Leaderboard_Panel, vgui::EditablePanel );

	CReactiveDrop_VGUI_Leaderboard_Panel( vgui::Panel *parent, const char *panelName );
	virtual ~CReactiveDrop_VGUI_Leaderboard_Panel();

	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void SetTitle( const char *szTitle );
	void SetTitle( const wchar_t *wszTitle );
	void SetEntries( const CUtlVector<RD_LeaderboardEntry_t> & entries );
	void OverrideEntry( const RD_LeaderboardEntry_t & entry );
	void SetScrollable( bool bScrollable );

private:
	void DoOverrideEntry();

	vgui::Label *m_lblTitle;
	BaseModUI::GenericPanelList *m_gplLeaderboard;
	bool m_bOverrideEntry;
	RD_LeaderboardEntry_t m_OverrideEntry;
};

class CReactiveDrop_VGUI_Leaderboard_Entry : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CReactiveDrop_VGUI_Leaderboard_Entry, vgui::EditablePanel );

	CReactiveDrop_VGUI_Leaderboard_Entry( vgui::Panel *parent, const char *panelName );
	virtual ~CReactiveDrop_VGUI_Leaderboard_Entry();

	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void SetEntry( const RD_LeaderboardEntry_t & entry );

	int32 m_nRank;
	int32 m_nScore;
	CSteamID m_SteamID;

private:
	vgui::Label *m_lblRank;
	CAvatarImagePanel *m_imgAvatar;
	vgui::Label *m_lblName;
	vgui::Label *m_lblScore;
	vgui::ImagePanel *m_imgMarine;
	vgui::ImagePanel *m_imgPrimaryWeapon;
	vgui::ImagePanel *m_imgSecondaryWeapon;
	vgui::ImagePanel *m_imgExtraWeapon;
	vgui::Label *m_lblSquadMembers;
	vgui::Label *m_lblCountry;
	vgui::Label *m_lblDifficulty;
	vgui::Label *m_lblOnslaught;
	vgui::Label *m_lblHardcoreFF;
};
