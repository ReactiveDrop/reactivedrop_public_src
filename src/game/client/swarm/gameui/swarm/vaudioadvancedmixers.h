#pragma once

#include "basemodui.h"

class CNB_Header_Footer;

namespace BaseModUI
{
	class AudioAdvancedMixers : public CBaseModFrame
	{
		DECLARE_CLASS_SIMPLE( AudioAdvancedMixers, CBaseModFrame );

	public:
		AudioAdvancedMixers( vgui::Panel *parent, const char *panelName );
		~AudioAdvancedMixers();

		virtual void OnCommand( const char *command );

		CNB_Header_Footer *m_pHeaderFooter;
	};
}
