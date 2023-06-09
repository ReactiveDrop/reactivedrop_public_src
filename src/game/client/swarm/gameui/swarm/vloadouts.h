#pragma once

#include "basemodui.h"

namespace BaseModUI
{

class Loadouts : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Loadouts, CBaseModFrame );

public:
	Loadouts( vgui::Panel *parent, const char *panelName );
	~Loadouts();
};

}
