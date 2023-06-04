#pragma once

#include <vgui_controls/Frame.h>

class TGD_Tab;
class TGD_Grid;
class TGD_Entry;
class TGD_Details;
class vgui::Label;
class vgui::ScrollBar;
class CNB_Button;
class CNB_Header_Footer;

class TabbedGridDetails : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( TabbedGridDetails, vgui::Frame );
public:
	explicit TabbedGridDetails();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void OnCommand( const char *command ) override;
	virtual void OnKeyCodeTyped( vgui::KeyCode keycode ) override;
	virtual void OnKeyCodePressed( vgui::KeyCode keycode ) override;
	virtual void SetTitle( const char *title, bool surfaceTitle ) override;
	virtual void SetTitle( const wchar_t *title, bool surfaceTitle ) override;

	void ShowFullScreen();
	void RememberTabIndex( ConVar *pCVar );
	void AddTab( TGD_Tab *pTab );
	void RemoveTab( TGD_Tab *pTab );
	void ActivateTab( TGD_Tab *pTab );
	void SetOverridePanel( vgui::Panel *pPanel );

	CNB_Header_Footer *m_pHeaderFooter;
	CNB_Button *m_pBackButton;
	vgui::Panel *m_pTabStrip;
	vgui::Label *m_pTabLeftHint;
	vgui::Label *m_pTabRightHint;
	vgui::Dar<TGD_Tab *> m_Tabs;
	vgui::DHANDLE<TGD_Tab> m_hCurrentTab;
	vgui::PHandle m_hOverridePanel;

	ConVar *m_pLastTabConVar;
};

abstract_class TGD_Tab : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( TGD_Tab, vgui::EditablePanel );
	explicit TGD_Tab( TabbedGridDetails *parent );
public:
	TGD_Tab( TabbedGridDetails *parent, const char *szLabel );
	TGD_Tab( TabbedGridDetails *parent, const wchar_t *wszLabel );
	virtual ~TGD_Tab();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void OnKeyFocusTicked() override;

	virtual TGD_Grid *CreateGrid();
	virtual TGD_Details *CreateDetails() = 0;

	virtual void ActivateTab();
	virtual void DeactivateTab();

	TabbedGridDetails *m_pParent;
	vgui::Label *m_pLabel;
	vgui::Label *m_pLabelHighlight;
	vgui::Panel *m_pHighlight;
	TGD_Grid *m_pGrid;
	TGD_Details *m_pDetails;
	int m_iMinWidth;
};

class TGD_Grid : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( TGD_Grid, vgui::EditablePanel );
public:
	explicit TGD_Grid( TGD_Tab *pTab );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void OnMouseWheeled( int delta ) override;
	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );
	virtual void OnKeyFocusTicked() override;

	void DeleteAllEntries();
	void AddEntry( TGD_Entry *pEntry );
	void RemoveEntry( TGD_Entry *pEntry );
	void DisplayEntry( TGD_Entry *pEntry );

	void SetMessage( const char *szMessage );
	void SetMessage( const wchar_t *wszMessage );

	TGD_Tab *m_pParent;
	vgui::Dar<TGD_Entry *> m_Entries;
	vgui::DHANDLE<TGD_Entry> m_hCurrentEntry;
	vgui::Label *m_pMessage;
	vgui::ScrollBar *m_pScrollBar;
	int m_iLastFocus;
};

abstract_class TGD_Entry : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( TGD_Entry, vgui::EditablePanel );
public:
	TGD_Entry( TGD_Grid *parent, const char *panelName );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnKeyCodePressed( vgui::KeyCode keycode ) override;

	virtual void ApplyEntry() = 0;

	TGD_Grid *m_pParent;
	vgui::Panel *m_pFocusHolder;
	vgui::Panel *m_pHighlight;
};

abstract_class TGD_Details : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( TGD_Details, vgui::EditablePanel );
public:
	explicit TGD_Details( TGD_Tab *pTab );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;

	TGD_Entry *GetCurrentEntry();

	virtual void DisplayEntry( TGD_Entry *pEntry ) = 0;

	TGD_Tab *m_pParent;
};
