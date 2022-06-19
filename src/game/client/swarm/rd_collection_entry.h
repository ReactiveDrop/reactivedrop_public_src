#pragma once

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>
#include "steam/steam_api.h"

class CRD_Collection_List;
class CRD_Collection_List_Equipment;
class CRD_Collection_List_Marines;
class CRD_Collection_List_Inventory;
class vgui::ImagePanel;
class vgui::Label;
class CASW_WeaponInfo;
class CASW_Marine_Profile;

class CRD_Collection_Entry : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry, vgui::EditablePanel );
public:
	CRD_Collection_Entry( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List *pList );
	virtual ~CRD_Collection_Entry();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Accept();

	CRD_Collection_List *m_pList;
	vgui::EditablePanel *m_pFocusHolder;
	vgui::Panel *m_pHighlight;
};

class CRD_Collection_Entry_Equipment : public CRD_Collection_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Equipment, CRD_Collection_Entry );
public:
	CRD_Collection_Entry_Equipment( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Equipment *pList, int index, CASW_WeaponInfo *pWeaponInfo );
	virtual ~CRD_Collection_Entry_Equipment();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pClassIcon;
	vgui::Label *m_pClassLabel;
	vgui::ImagePanel *m_pLockedIcon;
	vgui::Label *m_pLockedOverlay;
	vgui::Label *m_pLockedLabel;

	int m_Index;
	CASW_WeaponInfo *m_pWeaponInfo;
	int m_nLevelRequirement;
};

class CRD_Collection_Entry_Marines : public CRD_Collection_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Marines, CRD_Collection_Entry );
public:
	CRD_Collection_Entry_Marines( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Marines *pList, int index, CASW_Marine_Profile *pProfile );
	virtual ~CRD_Collection_Entry_Marines();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	vgui::ImagePanel *m_pPortrait;
	vgui::ImagePanel *m_pHighlightPortrait;

	int m_Index;
	CASW_Marine_Profile *m_pProfile;
};

class CRD_Collection_Entry_Inventory : public CRD_Collection_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Inventory, CRD_Collection_Entry );
public:
	CRD_Collection_Entry_Inventory( vgui::Panel *pParent, const char *pElementName, CRD_Collection_List_Inventory *pList, int index, SteamItemDetails_t details );
	virtual ~CRD_Collection_Entry_Inventory();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void Accept();

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pEquippedMarker;

	int m_Index;
	SteamItemDetails_t m_Details;
};
