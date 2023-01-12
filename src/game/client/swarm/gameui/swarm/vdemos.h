#pragma once

#include "basemodui.h"

class CNB_Button;
class CNB_Header_Footer;

namespace BaseModUI
{
class DropDownMenu;
class GenericPanelList;

class Demos : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Demos, CBaseModFrame );
public:
	Demos( vgui::Panel *parent, const char *panelName );
	~Demos();

	void Activate() override;
	void UpdateWarnings();
	void OnKeyCodePressed( vgui::KeyCode code ) override;
	void OnCommand( const char *command ) override;
	void OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel ) override;
	void OnThink() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	CNB_Header_Footer *m_pHeaderFooter;
	vgui::Label *m_LblNoRecordings;
	GenericPanelList *m_GplRecordingList;
	DropDownMenu *m_DrpAutoRecord;
	vgui::Label *m_LblAutoRecordWarning;
	CNB_Button *m_BtnWatch;
	CNB_Button *m_BtnCancel;
	CNB_Button *m_BtnDelete;
	CNB_Button *m_BtnRename;
};
}
