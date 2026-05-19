#pragma once

#include "tabbedgriddetails.h"

class CRD_Collection_Tab_Crafting_Research : public TGD_Tab_Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Crafting_Research, TGD_Tab_Panel );
public:
	CRD_Collection_Tab_Crafting_Research( TabbedGridDetails *parent, const char *szLabel );

	vgui::Panel *CreatePanel() override;
};

class CRD_Crafting_Research_Panel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Research_Panel, vgui::EditablePanel );
public:
	explicit CRD_Crafting_Research_Panel( CRD_Collection_Tab_Crafting_Research *pTab );
	~CRD_Crafting_Research_Panel() override;

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;
	void OnCommand( const char *szCommand ) override;

	CRD_Collection_Tab_Crafting_Research *m_pParent;
};
