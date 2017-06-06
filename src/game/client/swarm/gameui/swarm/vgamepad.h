#ifndef __VGAMEPAD_H__
#define __VGAMEPAD_H__


#include "basemodui.h"
#include "VFlyoutMenu.h"

class CNB_Button;
class CNB_Header_Footer;

namespace BaseModUI {

	class DropDownMenu;
	class SliderControl;
	class BaseModHybridButton;

	class Gamepad : public CBaseModFrame, public FlyoutMenuListener
	{
		DECLARE_CLASS_SIMPLE( Gamepad, CBaseModFrame );

	public:
		Gamepad( vgui::Panel *parent, const char *panelName );
		~Gamepad();

		//FloutMenuListener
		virtual void OnNotifyChildFocus( vgui::Panel* child );
		virtual void OnFlyoutMenuClose( vgui::Panel* flyTo );
		virtual void OnFlyoutMenuCancelled();

		virtual void PerformLayout();

		Panel* NavigateBack();

	protected:
		virtual void Activate();
		virtual void OnThink();
		virtual void PaintBackground();
		virtual void ApplySchemeSettings( vgui::IScheme* pScheme );
		virtual void OnKeyCodePressed( vgui::KeyCode code );
		virtual void OnCommand( const char *command );

	private:
		void				UpdateFooter( bool bEnableCloud );

		DropDownMenu		*m_drpGamepadEnable;
		SliderControl		*m_sldGamepadHSensitivity;
		SliderControl		*m_sldGamepadVSensitivity;
		DropDownMenu		*m_drpGamepadYInvert;
		DropDownMenu		*m_drpGamepadSwapSticks;

		CNB_Button	*m_btnDone;

		CNB_Header_Footer *m_pHeaderFooter;
	};

};

#endif // __VGAMEPAD_H__