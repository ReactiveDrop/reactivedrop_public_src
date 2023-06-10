#pragma once

#include "basemodui.h"

class CNB_Header_Footer;
class CRD_VGUI_Main_Menu_Top_Bar;

namespace BaseModUI
{

class Loadouts : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Loadouts, CBaseModFrame );

public:
	Loadouts( vgui::Panel *parent, const char *panelName );
	~Loadouts();

	CNB_Header_Footer *m_pHeaderFooter;
	CRD_VGUI_Main_Menu_Top_Bar *m_pTopBar;
};

}
