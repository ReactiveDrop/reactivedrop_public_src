#pragma once

#include <vgui_controls/EditablePanel.h>

class CRD_VGUI_Player_Reporting : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Player_Reporting, vgui::EditablePanel );
public:
	CRD_VGUI_Player_Reporting( vgui::Panel *parent, const char *panelName );
};
