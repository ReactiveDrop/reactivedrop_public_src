#pragma once

#include "basemodframe.h"
#include "steam/steamclientpublic.h"

class CAvatarImagePanel;
class CBitmapButton;
class CNB_Button;
class CNB_Header_Footer;
class CRD_VGUI_Option;

namespace vgui
{
	class MultiFontRichText;
}

namespace BaseModUI
{
	class GenericPanelList;

	class ReportProblem : public CBaseModFrame
	{
		DECLARE_CLASS_SIMPLE( ReportProblem, CBaseModFrame );
	public:
		ReportProblem( vgui::Panel *parent, const char *panelName );
		~ReportProblem();

		// VGUI callbacks
		void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
		void OnCommand( const char *command ) override;
		void OnThink() override;
		MESSAGE_FUNC( OnTextChanged, "TextChanged" );
		MESSAGE_FUNC( OnCheckButtonChecked, "CheckButtonChecked" );
		MESSAGE_FUNC( OnCurrentOptionChanged, "CurrentOptionChanged" );

		// sub-screen setup functions
		void HideAllControls();
		void ShowWaitScreen();
		void ShowCategorySelectionScreen();
		void ShowRecentlyPlayedWith();
		void ResumeInProgressReport();
		void StartNewReport( const char *szCategory, const char *const *SubCategories = NULL, CSteamID Player = k_steamIDNil, bool bRequireDedicatedServer = false );

		// helpers
		void UpdateReportContents();

		// global
		CNB_Button *m_pBtnBack;

		// category selection screen
		CNB_Header_Footer *m_pHeaderFooter_Category;
		CBitmapButton *m_pBtnMyAccount;
		CBitmapButton *m_pBtnServer;
		CBitmapButton *m_pBtnPlayer;
		CBitmapButton *m_pBtnGameBug;
		CBitmapButton *m_pBtnOther;
		vgui::Label *m_pLblMyAccount;
		vgui::Label *m_pLblServer;
		vgui::Label *m_pLblPlayer;
		vgui::Label *m_pLblGameBug;
		vgui::Label *m_pLblOther;
		CNB_Button *m_pBtnResume;
		vgui::Label *m_pLblLastProgress;

		// wait screen
		CNB_Header_Footer *m_pHeaderFooter_Wait;
		vgui::Label *m_pLblWait;
		vgui::Label *m_pLblDontWait;
		vgui::ImagePanel *m_pImgSpinner;

		// player select
		CNB_Header_Footer *m_pHeaderFooter_Player;
		GenericPanelList *m_pGplPlayerChoices;
		CNB_Button *m_pBtnSelectPlayer;
		CSteamID m_SelectedPlayer;

		// report in progress
		CNB_Header_Footer *m_pHeaderFooter_Report;
		vgui::Label *m_pLblReportingPlayer;
		CAvatarImagePanel *m_pImgPlayerAvatar;
		vgui::Label *m_pLblPlayerName;
		CRD_VGUI_Option *m_pSettingSubCategory;
		vgui::Label *m_pLblInstructions;
		vgui::TextEntry *m_pTxtDescription;
		vgui::ImagePanel *m_pImgScreenshot[1];
		vgui::CheckButton *m_pChkScreenshot[1];
		vgui::MultiFontRichText *m_pLblReportContents;
		CNB_Button *m_pBtnSubmit;
	};
}
