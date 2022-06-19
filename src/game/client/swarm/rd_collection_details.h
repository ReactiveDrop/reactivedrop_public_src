#pragma once

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>

class CRD_Collection_Entry;
class vgui::ImagePanel;
class vgui::RichText;

class CRD_Collection_Details : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details, vgui::EditablePanel );
public:
	CRD_Collection_Details( vgui::Panel *pParent, const char *pElementName );
	virtual ~CRD_Collection_Details();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void HighlightEntry( CRD_Collection_Entry *pEntry );

	CRD_Collection_Entry *m_pLastEntry;
};

class CRD_Collection_Details_Equipment : public CRD_Collection_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Equipment, CRD_Collection_Details );
public:
	CRD_Collection_Details_Equipment( vgui::Panel *pParent, const char *pElementName );
	virtual ~CRD_Collection_Details_Equipment();
};

class CRD_Collection_Details_Aliens : public CRD_Collection_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Aliens, CRD_Collection_Details );
public:
	CRD_Collection_Details_Aliens( vgui::Panel *pParent, const char *pElementName );
	virtual ~CRD_Collection_Details_Aliens();
};

class CRD_Collection_Details_Marines : public CRD_Collection_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Marines, CRD_Collection_Details );
public:
	CRD_Collection_Details_Marines( vgui::Panel *pParent, const char *pElementName );
	virtual ~CRD_Collection_Details_Marines();
};

class CRD_Collection_Details_Inventory : public CRD_Collection_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Inventory, CRD_Collection_Details );
public:
	CRD_Collection_Details_Inventory( vgui::Panel *pParent, const char *pElementName, const char *szSlot );
	virtual ~CRD_Collection_Details_Inventory();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

	virtual void HighlightEntry( CRD_Collection_Entry *pEntry );

	vgui::ImagePanel *m_pIcon;
	vgui::RichText *m_pTitle;
	vgui::RichText *m_pDescription;

	const char *m_szSlot;
};
