#pragma once

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include "steam/steam_api.h"

class CRD_Collection_Details;
class CRD_Collection_Entry;
class vgui::Label;
class vgui::ScrollBar;

class CRD_Collection_List : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_List, vgui::EditablePanel );
public:
	CRD_Collection_List( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails );
	virtual ~CRD_Collection_List();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnMouseWheeled( int delta );
	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );

	CRD_Collection_Details *m_pDetails;
	vgui::Panel *m_pHolder;
	vgui::Label *m_pErrorMessage;
	vgui::ScrollBar *m_pScrollBar;
	CUtlVector<vgui::DHANDLE<CRD_Collection_Entry>> m_Entries;

	int m_nLastX, m_nLastY;
};

class CRD_Collection_List_Equipment : public CRD_Collection_List
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_List_Equipment, CRD_Collection_List );
public:
	CRD_Collection_List_Equipment( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails, bool bExtra );
	virtual ~CRD_Collection_List_Equipment();

	void SetBriefingSlot( int iBriefingSlot, int iInventorySlot );

	int m_iBriefingSlot{ -1 };
	int m_iInventorySlot{ -1 };
};

class CRD_Collection_List_Aliens : public CRD_Collection_List
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_List_Aliens, CRD_Collection_List );
public:
	CRD_Collection_List_Aliens( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails );
	virtual ~CRD_Collection_List_Aliens();
};

class CRD_Collection_List_Marines : public CRD_Collection_List
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_List_Marines, CRD_Collection_List );
public:
	CRD_Collection_List_Marines( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails );
	virtual ~CRD_Collection_List_Marines();
};

class CRD_Collection_List_Inventory : public CRD_Collection_List
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_List_Inventory, CRD_Collection_List );
public:
	CRD_Collection_List_Inventory( vgui::Panel *pParent, const char *pElementName, CRD_Collection_Details *pDetails, const char *szSlot );
	virtual ~CRD_Collection_List_Inventory();

	virtual void PerformLayout();
	virtual void OnThink();

	const char *m_szSlot;
	SteamInventoryResult_t m_hResult;
	bool m_bUnavailable;
};
