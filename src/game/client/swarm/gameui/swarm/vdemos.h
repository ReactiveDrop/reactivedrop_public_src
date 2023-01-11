#pragma once

#include "basemodui.h"

class CNB_Header_Footer;

namespace BaseModUI
{
class GenericPanelList;

class Demos : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Demos, CBaseModFrame );
public:
	Demos( vgui::Panel *parent, const char *panelName );
	~Demos();

	void Activate() override;
	void OnCommand( const char *command ) override;
	void OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel ) override;
	void OnThink() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	CNB_Header_Footer *m_pHeaderFooter;
	vgui::Label *m_LblNoRecordings;
	GenericPanelList *m_GplRecordingList;
};
}
