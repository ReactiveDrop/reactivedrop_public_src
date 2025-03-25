#pragma once

#include "tabbedgriddetails.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "gameui/swarm/vhybridbutton.h"
#include "rd_inventory_shared.h"
#include "rd_collections.h"

class CRD_Collection_Tab_Crafting : public TGD_Tab_Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Crafting, TGD_Tab_Panel );
public:
	CRD_Collection_Tab_Crafting( TabbedGridDetails *parent, const char *szLabel );

	vgui::Panel *CreatePanel() override;
};

class CRD_Crafting_Panel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Panel, vgui::EditablePanel );
public:
	explicit CRD_Crafting_Panel( CRD_Collection_Tab_Crafting *pTab );
	virtual ~CRD_Crafting_Panel();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;
	void OnCommand( const char *szCommand ) override;
	void UpdateCraftState();

	CRD_Collection_Tab_Crafting *m_pParent;
	BaseModUI::GenericPanelList *m_pGplRecipes;
	vgui::Label *m_pLblRecipeTitle;
	vgui::Label *m_pLblFlavor;
	vgui::Label *m_pLblWarning;
	CNB_Button *m_pBtnCraft;
	vgui::Label *m_pLblIngredients;
	class CRD_Crafting_Item_Grid *m_pGridIngredients;
	class CRD_Crafting_Item_Grid *m_pGridIngredientsTall;
	vgui::Label *m_pLblOutputs;
	class CRD_Crafting_Item_Grid *m_pGridOutputs;
	CUtlVector<SteamItemInstanceID_t> m_SelectedItems;
	CUtlVector<SteamItemInstanceID_t> m_AutoSelectedItems;
	CUtlVector<const struct RD_Crafting_Recipe_Variant *> m_FilteredVariants;
	CUtlVector<ReactiveDropInventory::ItemInstance_t> m_PossibleNextIngredients;
	SteamItemDef_t m_SelectedRecipeOutput;
	int m_iSelectedRecipe;
	int m_iLastFullInventoryUpdates;
};

namespace BaseModUI
{
	class CRD_Crafting_Recipe_Button : public BaseModHybridButton
	{
		DECLARE_CLASS_SIMPLE( CRD_Crafting_Recipe_Button, BaseModHybridButton );

	public:
		CRD_Crafting_Recipe_Button( CRD_Crafting_Panel *pParent, const char *szRecipeLabel, int iRecipeIndex );

		void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	};
}

class CRD_Crafting_Item_Grid : public TGD_Grid
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Item_Grid, TGD_Grid );

public:
	explicit CRD_Crafting_Item_Grid( CRD_Crafting_Panel *pParent, const char *szPanelName );
};

class CRD_Crafting_Item_Entry : public CRD_Collection_Entry_Inventory
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Item_Entry, CRD_Collection_Entry_Inventory );

public:
	CRD_Crafting_Item_Entry( TGD_Grid *pGrid, CRD_Crafting_Panel *pParent, int index, bool bRemovable, const ReactiveDropInventory::ItemInstance_t &details );

	void ApplyEntry() override;

	CRD_Crafting_Panel *m_pCrafting;
	bool m_bRemovable;
};

class CRD_Crafting_Item_Entry_Add : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Item_Entry_Add, TGD_Entry );

public:
	CRD_Crafting_Item_Entry_Add( TGD_Grid *pGrid, CRD_Crafting_Panel *pParent );

	void ApplyEntry() override;

	CRD_Crafting_Panel *m_pCrafting;
};
