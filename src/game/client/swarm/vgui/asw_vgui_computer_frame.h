#ifndef _INCLUDED_ASW_VGUI_COMPUTER_FRAME_H
#define _INCLUDED_ASW_VGUI_COMPUTER_FRAME_H

#include "asw_vgui_ingame_panel.h"
#include "asw_vgui_frame.h"

class C_ASW_Hack_Computer;
class CASW_VGUI_Computer_Menu;
class CASW_VGUI_Computer_Splash;
class ImageButton;

// frame holding the computer vgui panels
class CASW_VGUI_Computer_Frame : public vgui::Panel, public CASW_VGUI_Ingame_Panel
{
	DECLARE_CLASS_SIMPLE( CASW_VGUI_Computer_Frame, vgui::Panel );

	CASW_VGUI_Computer_Frame( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackDoor );
	virtual ~CASW_VGUI_Computer_Frame();

	void SetBackdrop( int iBackdropType );
	int m_iBackdropType;

	void SplashFinished();
	void ASWInit();

	bool IsPDA();
	void RecordComputerContents();

	void PerformLayout() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override; // called every frame before painting, but only if panel is visible
	void OnCommand( const char *pCommand ) override;
	bool MouseClick( int x, int y, bool bRightClick, bool bDown ) override;
	void SetTitleHidden( bool bHidden );
	void SetHackOption( int iOption );

	// current computer hack
	CHandle<C_ASW_Hack_Computer> m_hHackComputer;
	CASW_VGUI_Computer_Splash *m_pSplash;
	CASW_VGUI_Computer_Menu *m_pMenuPanel;
	ImageButton *m_pLogoffLabel;

	vgui::Panel *m_pCurrentPanel;
	vgui::ImagePanel *m_pScan[3];
	vgui::ImagePanel *m_pBackdropImage;
	int m_iScanHeight;

	// overall scale of this window
	float m_fScale;

	float m_fLastThinkTime;
	bool m_bPlayingSplash;
	bool m_bSetAlpha;
	bool m_bHideLogoffButton;
};

class CASW_VGUI_Computer_Container : public CASW_VGUI_Frame
{
	DECLARE_CLASS_SIMPLE( CASW_VGUI_Computer_Container, CASW_VGUI_Frame );

	CASW_VGUI_Computer_Container( Panel *parent, const char *panelName, const char *pszTitle );
	virtual ~CASW_VGUI_Computer_Container();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	virtual void OnThink();
};

#endif /* _INCLUDED_ASW_VGUI_COMPUTER_FRAME_H */
