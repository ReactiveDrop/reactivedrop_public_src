#pragma once

#include "tabbedgriddetails.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "gameui/swarm/vhybridbutton.h"

namespace ReactiveDropInventory
{
	struct ItemDef_t;
	struct ItemInstance_t;
}

class CRD_Collection_Tab_Crafting : public TGD_Tab_Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Crafting, TGD_Tab_Panel );
public:
	CRD_Collection_Tab_Crafting( TabbedGridDetails *parent, const char *szLabel );

	vgui::Panel *CreatePanel() override;
};

class CRD_Crafting_Item_Grid_Item : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Item_Grid_Item, vgui::EditablePanel );
public:
	CRD_Crafting_Item_Grid_Item( vgui::Panel *pParent, const char *szPanelName );

	void NavigateTo() override;
	void OnSetFocus() override;
	void OnKillFocus() override;
	void OnCursorMoved( int x, int y ) override;
	void OnMousePressed( vgui::MouseCode code ) override;
	void OnMouseReleased( vgui::MouseCode code ) override;
	void OnKeyCodePressed( vgui::KeyCode code ) override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	void Setup( const ReactiveDropInventory::ItemDef_t *pDef, const ReactiveDropInventory::ItemInstance_t *pInstance );

	SteamItemDef_t m_iDef = 0;
	SteamItemInstanceID_t m_iInstanceID = k_SteamItemInstanceIDInvalid;
	int32_t m_nQuantity = 0;
	bool m_bClickable = false;
	bool m_bMousePressed = false;
	bool m_bNoScrollOnFocus = false;

	vgui::ImagePanel *m_pIcon;
	vgui::Label *m_pLblName;
	vgui::Label *m_pLblDisambiguator;
	vgui::Label *m_pLblQuantity;
};

class CRD_Crafting_Item_Grid : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Item_Grid, vgui::EditablePanel );
public:
	CRD_Crafting_Item_Grid( vgui::Panel *pParent, const char *szPanelName );
	~CRD_Crafting_Item_Grid() override;

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	void OnMouseWheeled( int delta ) override;
	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );

	void AddItem( SteamItemInstanceID_t iInstanceID, int32_t nQuantity, bool bClickable );
	void AddItemDef( SteamItemDef_t iItemDef, int32_t nQuantity, bool bClickable );
	void RemoveAll();

	vgui::ScrollBar *m_pScrollBar;
	vgui::Dar<CRD_Crafting_Item_Grid_Item *> m_Items;
};

class CRD_Crafting_Panel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Panel, vgui::EditablePanel );
public:
	explicit CRD_Crafting_Panel( CRD_Collection_Tab_Crafting *pTab );
	~CRD_Crafting_Panel() override;

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;
	void OnCommand( const char *szCommand ) override;
	void UpdateCraftState();

	CRD_Collection_Tab_Crafting *m_pParent;
	BaseModUI::GenericPanelList *m_pGplRecipes;
	vgui::Label *m_pLblRecipeTitle;
	vgui::Label *m_pLblFlavor;
	vgui::Label *m_pLblIngredients;
	CRD_Crafting_Item_Grid *m_pGridIngredients;
	CRD_Crafting_Item_Grid *m_pGridIngredientsTall;
	vgui::Label *m_pLblOutputs;
	CRD_Crafting_Item_Grid *m_pGridOutputs;
	vgui::Label *m_pLblWarning;
	CNB_Button *m_pBtnCraft;
	CUtlVector<SteamItemInstanceID_t> m_SelectedItems;
	CUtlVector<SteamItemInstanceID_t> m_AutoSelectedItems;
	CUtlVector<const struct RD_Crafting_Recipe_Variant *> m_FilteredVariants;
	SteamItemDef_t m_SelectedRecipeOutput;
	int m_iSelectedRecipe;
	int m_iLastFullInventoryUpdates;

#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	STEAM_CALLBACK( CRD_Crafting_Panel, OnSteamServersConnected, SteamServersConnected_t )
	{
		// Automatically transition from "you can't craft because you're offline" to "crafting is ready", but don't care about the other way around (the crafting attempt will simply result in an error).
		UpdateCraftState();
	}
#else
	STEAM_CALLBACK( CRD_Crafting_Panel, OnSteamServersConnected, SteamServersConnected_t )
	{
		// Automatically transition from "you can't craft because you're offline" to "crafting is ready", but don't care about the other way around (the crafting attempt will simply result in an error).
		UpdateCraftState();
	}
#endif
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
