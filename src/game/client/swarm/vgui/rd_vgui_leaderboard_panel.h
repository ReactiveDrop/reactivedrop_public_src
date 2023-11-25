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
	void SetDisplayType( ELeaderboardDisplayType e ) { m_eDisplayType = e; }

private:
	void DoOverrideEntry();

	vgui::Label *m_lblTitle;
	BaseModUI::GenericPanelList *m_gplLeaderboard;
	bool m_bOverrideEntry;
	RD_LeaderboardEntry_t m_OverrideEntry;
	ELeaderboardDisplayType m_eDisplayType;
};

class CRD_VGUI_CountryCodeImage : public vgui::IImage
{
public:
	void Paint() override;
	void SetPos( int x, int y ) override { m_x = x; m_y = y; }
	void GetContentSize( int &wide, int &tall ) override { wide = 16; tall = 11; }
	void GetSize( int &wide, int &tall ) override { wide = m_wide; tall = m_tall; }
	void SetSize( int wide, int tall ) override { m_wide = wide; m_tall = tall; }
	void SetColor( Color col ) override { m_color = col; }
	bool Evict() override { return false; }
	int GetNumFrames() override { return 1; }
	void SetFrame( int nFrame ) override {}
	vgui::HTexture GetID() override;
	void SetRotation( int iRotation ) override {}

	int m_x{ 0 }, m_y{ 0 };
	int m_wide{ 0 }, m_tall{ 0 };
	Color m_color{ 255, 255, 255, 255 };
	float s0, t0, s1, t1;
};

class CReactiveDrop_VGUI_Leaderboard_Entry : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CReactiveDrop_VGUI_Leaderboard_Entry, vgui::EditablePanel );

	CReactiveDrop_VGUI_Leaderboard_Entry( vgui::Panel *parent, const char *panelName );
	virtual ~CReactiveDrop_VGUI_Leaderboard_Entry();

	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void SetEntry( const RD_LeaderboardEntry_t & entry );
	void SetDisplayType( ELeaderboardDisplayType e ) { m_eDisplayType = e; }

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
	vgui::ImagePanel *m_imgCountry;
	vgui::ImagePanel *m_imgDifficulty;
	vgui::ImagePanel *m_imgOnslaught;
	vgui::ImagePanel *m_imgHardcoreFF;
	ELeaderboardDisplayType m_eDisplayType;
	CRD_VGUI_CountryCodeImage m_CountryCodeImage;
};
