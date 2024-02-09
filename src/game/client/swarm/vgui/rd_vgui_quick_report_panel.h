#pragma once

#include <vgui_controls/EditablePanel.h>
#include "steam/steamclientpublic.h"

class CAvatarImagePanel;
class CBitmapButton;
class CNB_Button;
class CNB_Header_Footer;

class CRD_VGUI_Quick_Report_Panel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Quick_Report_Panel, vgui::EditablePanel );
public:
	CRD_VGUI_Quick_Report_Panel( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Quick_Report_Panel();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;
	void OnCommand( const char *command ) override;
	void SetPlayer( CSteamID player );
	void ConfirmAndSendQuickReport( const char *szTitle, const char *szMessage, const char *szCategory, CSteamID player );

	CNB_Header_Footer *m_pHeaderFooter;
	CAvatarImagePanel *m_pImgPlayerAvatar;
	vgui::Label *m_pLblPlayerName;
	vgui::Label *m_pLblProgress;
	CNB_Button *m_pBtnBack;

	CBitmapButton *m_pBtnCommendFriendly;
	CBitmapButton *m_pBtnCommendLeader;
	CBitmapButton *m_pBtnCommendTeacher;
	CBitmapButton *m_pBtnReportCheating;
	CBitmapButton *m_pBtnReportGriefing;
	CBitmapButton *m_pBtnReportCommunication;

	vgui::Label *m_pLblCommendFriendly;
	vgui::Label *m_pLblCommendLeader;
	vgui::Label *m_pLblCommendTeacher;
	vgui::Label *m_pLblReportCheating;
	vgui::Label *m_pLblReportGriefing;
	vgui::Label *m_pLblReportCommunication;

	CSteamID m_PlayerID;
};
