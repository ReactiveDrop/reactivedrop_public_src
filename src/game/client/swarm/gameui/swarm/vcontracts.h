#pragma once

#include "basemodui.h"

namespace BaseModUI
{

class Contracts : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Contracts, CBaseModFrame );

public:
	Contracts( vgui::Panel *parent, const char *panelName );
	~Contracts();
};

}
