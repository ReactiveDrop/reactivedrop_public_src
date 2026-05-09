#include "cbase.h"
#include "rd_collections_crafting.h"
#include "rd_crafting_defs.h"
#include "rd_inventory_shared.h"
#include "nb_button.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CRD_Collection_Tab_Crafting::CRD_Collection_Tab_Crafting( TabbedGridDetails *parent, const char *szLabel )
	: BaseClass( parent, szLabel )
{
}

vgui::Panel *CRD_Collection_Tab_Crafting::CreatePanel()
{
	return new CRD_Crafting_Panel( this );
}

CRD_Crafting_Panel::CRD_Crafting_Panel( CRD_Collection_Tab_Crafting *pTab )
	: BaseClass( pTab->m_pParent, "CraftingPanel" )
{
	SetConsoleStylePanel( true );

	m_pParent = pTab;
	m_pGplRecipes = new BaseModUI::GenericPanelList( this, "GplRecipes", BaseModUI::GenericPanelList::ISM_PERITEM );
	m_pLblRecipeTitle = new vgui::Label( this, "LblRecipeTitle", "" );
	m_pLblFlavor = new vgui::Label( this, "LblFlavor", "" );
	m_pLblWarning = new vgui::Label( this, "LblWarning", "" );
	m_pBtnCraft = new CNB_Button( this, "BtnCraft", "#rd_crafting_submit_ready", this, "ConfirmCraft" );
	m_iSelectedRecipe = -1;
	m_SelectedRecipeOutput = 0;
	m_iLastFullInventoryUpdates = -1;
}

CRD_Crafting_Panel::~CRD_Crafting_Panel()
{
	m_pParent->m_pPanel = nullptr;
}

void CRD_Crafting_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	bool bWasVisible = IsVisible();

	LoadControlSettings( "Resource/UI/CraftingPanel.res" );

	SetVisible( bWasVisible );

	BaseClass::ApplySchemeSettings( pScheme );

	m_pLblRecipeTitle->SetText( "" );
	m_pLblFlavor->SetText( "" );
	m_pLblWarning->SetText( "" );
	m_pBtnCraft->SetVisible( false );

	m_iLastFullInventoryUpdates = ReactiveDropInventory::g_nFullInventoryUpdates;
	m_pGplRecipes->SetScrollBarVisible( true );
	m_pGplRecipes->RemoveAllPanelItems();
	FOR_EACH_VEC( g_RD_Crafting_Recipes, i )
	{
		bool bHideIfItem = false;
		FOR_EACH_VEC( g_RD_Crafting_Recipes[i].m_HideIfItem, j )
		{
			CUtlVector<ReactiveDropInventory::ItemInstance_t> instances;
			ReactiveDropInventory::GetItemsForDef( instances, g_RD_Crafting_Recipes[i].m_HideIfItem[j] );
			if ( instances.Count() != 0 )
			{
				bHideIfItem = true;
				break;
			}
		}
		if ( bHideIfItem )
		{
			continue;
		}

		bHideIfItem = g_RD_Crafting_Recipes[i].m_HideUnlessItem.Count() != 0;
		FOR_EACH_VEC( g_RD_Crafting_Recipes[i].m_HideUnlessItem, j )
		{
			CUtlVector<ReactiveDropInventory::ItemInstance_t> instances;
			ReactiveDropInventory::GetItemsForDef( instances, g_RD_Crafting_Recipes[i].m_HideUnlessItem[j] );
			if ( instances.Count() != 0 )
			{
				bHideIfItem = false;
				break;
			}
		}
		if ( bHideIfItem )
		{
			continue;
		}

		BaseModUI::CRD_Crafting_Recipe_Button *pButton = new BaseModUI::CRD_Crafting_Recipe_Button( this, g_RD_Crafting_Recipes[i].m_szDisplayName, i );
		ushort iButtonIndex = m_pGplRecipes->AddPanelItem( pButton, true );
		if ( i == m_iSelectedRecipe )
		{
			m_pGplRecipes->SelectPanelItem( iButtonIndex );
			pButton->DoClick();
		}
	}
}

void CRD_Crafting_Panel::OnThink()
{
	BaseClass::OnThink();

	if ( m_iLastFullInventoryUpdates != ReactiveDropInventory::g_nFullInventoryUpdates )
	{
		InvalidateLayout( true, true );
	}
}

void CRD_Crafting_Panel::OnCommand( const char *szCommand )
{
	if ( const char *szRecipeNumber = StringAfterPrefix( szCommand, "SelectRecipe" ) )
	{
		int iRecipeNumber = V_atoi( szRecipeNumber );
		if ( m_iSelectedRecipe != iRecipeNumber )
		{
			m_iSelectedRecipe = iRecipeNumber;
			m_SelectedItems.Purge();
		}

		m_FilteredVariants.Purge();
		FOR_EACH_VEC( g_RD_Crafting_Recipes[iRecipeNumber].m_Variants, i )
		{
			// we can take a reference (and later a pointer) because these vectors are immutable
			const RD_Crafting_Recipe_Variant &variant = g_RD_Crafting_Recipes[iRecipeNumber].m_Variants[i];

			// TODO: support recipes with ingredient quantities other than 1
			CUtlVector<SteamItemInstanceID_t> selectedItems;

			// simplifying assumption: the order ingredients are selected does not matter;
			// it is impossible to fill an input slot with the "wrong" item.
			// (this assumption allows us to use a greedy algorithm)
			FOR_EACH_VEC( variant.m_Inputs, j )
			{
				Assert( variant.m_Inputs[j].m_iQuantity == 1 );

				FOR_EACH_VEC( variant.m_Inputs[j].m_AllowedItem, k )
				{
					CUtlVector<ReactiveDropInventory::ItemInstance_t> instances;
					ReactiveDropInventory::GetItemsForDef( instances, variant.m_Inputs[j].m_AllowedItem[k] );
					FOR_EACH_VEC( instances, l )
					{
						int nQuantityUsed = 0;
						FOR_EACH_VEC( selectedItems, m )
						{
							if ( selectedItems[m] == instances[l].ItemID )
							{
								nQuantityUsed++;
							}
						}

						if ( instances[l].Quantity > nQuantityUsed )
						{
							selectedItems.AddToTail( instances[l].ItemID );
							break;
						}
					}

					if ( selectedItems.Count() == j + 1 )
					{
						// we found an item
						break;
					}
				}

				if ( selectedItems.Count() != j + 1 )
				{
					// we didn't find an item
					break;
				}
			}

			if ( selectedItems.Count() == variant.m_Inputs.Count() )
			{
				m_FilteredVariants.AddToTail( &variant );
			}
		}

		m_pLblRecipeTitle->SetText( g_RD_Crafting_Recipes[iRecipeNumber].m_szDisplayName );

		if ( g_RD_Crafting_Recipes[iRecipeNumber].m_szFlavorText )
		{
			m_pLblFlavor->SetText( g_RD_Crafting_Recipes[iRecipeNumber].m_szFlavorText );
			m_pLblFlavor->SetVisible( true );
		}
		else
		{
			m_pLblFlavor->SetVisible( false );
		}

		if ( g_RD_Crafting_Recipes[iRecipeNumber].m_szWarning )
		{
			m_pLblWarning->SetText( g_RD_Crafting_Recipes[iRecipeNumber].m_szWarning );
			m_pLblWarning->SetVisible( true );
		}
		else
		{
			m_pLblWarning->SetVisible( false );
		}

		m_SelectedItems.Purge();

		UpdateCraftState();
	}
	else
	{
		BaseClass::OnCommand( szCommand );
	}
}

void CRD_Crafting_Panel::UpdateCraftState()
{
	CUtlVector<const ReactiveDropInventory::ItemInstance_t *> selectedItems;
	FOR_EACH_VEC( m_SelectedItems, i )
	{
		const ReactiveDropInventory::ItemInstance_t *pCached = ReactiveDropInventory::GetLocalItemCache( m_SelectedItems[i] );
		if ( !pCached )
		{
			m_SelectedItems.Purge();
			selectedItems.Purge();
			break;
		}

		selectedItems.AddToTail( pCached );
	}

	m_AutoSelectedItems.Purge();
	m_SelectedRecipeOutput = 0;

	CUtlVector<const RD_Crafting_Recipe_Variant *> possibleAutoVariants;
	CUtlVector<const RD_Crafting_Recipe_Variant *> possibleManualVariants;
	CUtlRBTree<SteamItemInstanceID_t> possibleNextIngredients( DefLessFunc( SteamItemInstanceID_t ) );
	FOR_EACH_VEC( m_FilteredVariants, i )
	{
		CUtlVector<const ReactiveDropInventory::ItemInstance_t *> orderedItems;
		orderedItems.SetCount( m_FilteredVariants[i]->m_Inputs.Count() );
		orderedItems.FillWithValue( nullptr );
		CUtlVector<SteamItemInstanceID_t> autoItems;

		bool bValid = true;
		FOR_EACH_VEC( selectedItems, j )
		{
			bool bFound = false;
			FOR_EACH_VEC( m_FilteredVariants[i]->m_Inputs, k )
			{
				if ( orderedItems[k] != nullptr )
				{
					continue;
				}

				if ( m_FilteredVariants[i]->m_Inputs[k].m_iFlags & RD_CRAFTING_RECIPE_AUTO_SELECT )
				{
					continue;
				}

				if ( m_FilteredVariants[i]->m_Inputs[k].m_AllowedItem.Find( selectedItems[j]->ItemDefID ) != m_FilteredVariants[i]->m_Inputs[k].m_AllowedItem.InvalidIndex() )
				{
					orderedItems[k] = selectedItems[j];
					bFound = true;
					break;
				}
			}

			if ( !bFound )
			{
				// we have an item selected that is invalid for this variant; we're not crafting this variant.
				bValid = false;
				break;
			}
		}

		if ( !bValid )
		{
			continue;
		}

		bool bAnyMissing = false;
		FOR_EACH_VEC( orderedItems, j )
		{
			if ( orderedItems[j] != nullptr )
			{
				continue;
			}

			if ( m_FilteredVariants[i]->m_Inputs[j].m_iFlags & RD_CRAFTING_RECIPE_AUTO_SELECT )
			{
				FOR_EACH_VEC( m_FilteredVariants[i]->m_Inputs[j].m_AllowedItem, k )
				{
					CUtlVector<ReactiveDropInventory::ItemInstance_t> instances;
					ReactiveDropInventory::GetItemsForDef( instances, m_FilteredVariants[i]->m_Inputs[j].m_AllowedItem[k] );

					FOR_EACH_VEC( instances, l )
					{
						int nQuantityUsed = 0;
						FOR_EACH_VEC( orderedItems, m )
						{
							if ( orderedItems[m] != nullptr && orderedItems[m]->ItemID == instances[l].ItemID )
							{
								nQuantityUsed++;
							}
						}

						if ( instances[l].Quantity > nQuantityUsed )
						{
							orderedItems[j] = ReactiveDropInventory::GetLocalItemCache( instances[l].ItemID );
							autoItems.AddToTail( instances[l].ItemID );
							break;
						}
					}

					if ( orderedItems[j] != nullptr )
					{
						break;
					}
				}

				if ( orderedItems[j] == nullptr )
				{
					bValid = false;
					break;
				}
			}
			else
			{
				bAnyMissing = true;

				FOR_EACH_VEC( m_FilteredVariants[i]->m_Inputs[j].m_AllowedItem, k )
				{
					CUtlVector<ReactiveDropInventory::ItemInstance_t> instances;
					ReactiveDropInventory::GetItemsForDef( instances, m_FilteredVariants[i]->m_Inputs[j].m_AllowedItem[k] );

					FOR_EACH_VEC( instances, l )
					{
						int nQuantityUsed = 0;
						FOR_EACH_VEC( orderedItems, m )
						{
							if ( orderedItems[m] != nullptr && orderedItems[m]->ItemID == instances[l].ItemID )
							{
								nQuantityUsed++;
							}
						}

						if ( instances[l].Quantity > nQuantityUsed )
						{
							possibleNextIngredients.Insert( instances[l].ItemID );
						}
					}
				}
			}
		}

		if ( bValid )
		{
			if ( bAnyMissing )
			{
				possibleManualVariants.AddToTail( m_FilteredVariants[i] );
			}
			else
			{
				Assert( possibleAutoVariants.Count() == 0 );
				possibleAutoVariants.AddToTail( m_FilteredVariants[i] );
				m_AutoSelectedItems.AddVectorToTail( autoItems );
			}
		}
	}

	if ( m_FilteredVariants.Count() == 0 )
	{
		m_pBtnCraft->SetText( "#rd_crafting_submit_missing_ingredients" );
		m_pBtnCraft->SetEnabled( false );
		m_pBtnCraft->SetVisible( true );
	}
	else if ( possibleAutoVariants.Count() != 0 )
	{
		Assert( possibleAutoVariants.Count() == 1 );
		Assert( possibleManualVariants.Count() == 0 );

		m_SelectedRecipeOutput = possibleAutoVariants[0]->m_ExchangeItem;

		m_pBtnCraft->SetText( "#rd_crafting_submit_ready" );
		m_pBtnCraft->SetEnabled( true );
		m_pBtnCraft->SetVisible( true );
	}
	else
	{
		m_pBtnCraft->SetText( "#rd_crafting_submit_unselected_ingredients" );
		m_pBtnCraft->SetEnabled( false );
		m_pBtnCraft->SetVisible( true );
	}

	// TODO: show items
}

BaseModUI::CRD_Crafting_Recipe_Button::CRD_Crafting_Recipe_Button( CRD_Crafting_Panel *pParent, const char *szRecipeLabel, int iRecipeIndex )
	: BaseClass( NULL, "RecipeButton", szRecipeLabel, pParent, VarArgs( "SelectRecipe%d", iRecipeIndex ) )
{
}

void BaseModUI::CRD_Crafting_Recipe_Button::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetTall( YRES( 20 ) );
	SetStyle( BUTTON_FLYOUTITEM );
}
