#pragma once

#include "basemodframe.h"

class CBitmapButton;
class CNB_Button;
class CNB_Header_Footer;

namespace BaseModUI
{
	class ReportProblem : public CBaseModFrame
	{
		DECLARE_CLASS_SIMPLE( ReportProblem, CBaseModFrame );
	public:
		ReportProblem( vgui::Panel *parent, const char *panelName );
		~ReportProblem();

		void OnCommand( const char *command ) override;

		CNB_Header_Footer *m_pHeaderFooter;

		CBitmapButton *m_pBtnMyAccount;
		CBitmapButton *m_pBtnServer;
		CBitmapButton *m_pBtnPlayer;
		CBitmapButton *m_pBtnBug;
		CBitmapButton *m_pBtnOther;
		vgui::Label *m_pLblMyAccount;
		vgui::Label *m_pLblServer;
		vgui::Label *m_pLblPlayer;
		vgui::Label *m_pLblBug;
		vgui::Label *m_pLblOther;
		CNB_Button *m_pBtnResume;
		CNB_Button *m_pBtnBack;
	};
}
