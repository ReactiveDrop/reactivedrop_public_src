#pragma once

#include <vgui_controls/EditablePanel.h>
#include "rd_inventory_shared.h"
#include "rd_hud_glow_helper.h"

struct LeaderboardEntry_t;
struct LeaderboardScoreDetails_Points_t;
class CAvatarImagePanel;
class StatsBar;

class CRD_VGUI_Commander_Mini_Profile : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Commander_Mini_Profile, vgui::EditablePanel );

	CRD_VGUI_Commander_Mini_Profile( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Commander_Mini_Profile();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void ApplySettings( KeyValues *pSettings ) override;
	void NavigateTo() override;
	void OnCursorEntered() override;
	void PaintBackground() override;

	void InitForLocalPlayer();
	void InitShared( CSteamID steamID );
	void SetExperienceAndPromotion( int iExperience, int iPromotion );
	void SetHoIAFData( const LeaderboardEntry_t &entry, const LeaderboardScoreDetails_Points_t &details );
	void SetHoIAFError();
	void ClearHoIAFData();

	CAvatarImagePanel *m_pImgAvatar;
	vgui::Label *m_pLblPlayerName;
	vgui::ImagePanel *m_pImgPromotionIcon;
	StatsBar *m_pExperienceBar;
	vgui::Label *m_pLblExperience;
	vgui::Label *m_pLblLevel;
	vgui::ImagePanel *m_pImgMedal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
	vgui::ImagePanel *m_pImgPredictedHoIAFMedal;

	HUDGlowHelper_t m_GlowHover;

	CSteamID m_SteamID;
	CRD_ItemInstance m_Medals[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
	CRD_ItemInstance m_PredictedHoIAFMedal;

	CPanelAnimationVar( bool, m_bShowLocalPlayer, "showLocalPlayer", "0" );
	CPanelAnimationVar( bool, m_bIsButton, "isButton", "0" );
	bool m_bEmbedded{ false };
	bool m_bHoIAFError{ false };

	STEAM_CALLBACK( CRD_VGUI_Commander_Mini_Profile, OnPersonaStateChange, PersonaStateChange_t );
	CCallResult<CRD_VGUI_Commander_Mini_Profile, LeaderboardScoresDownloaded_t> m_LeaderboardScoresDownloaded;
	void OnLeaderboardScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure );
};
