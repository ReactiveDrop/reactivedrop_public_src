#pragma once

#include "tabbedgriddetails.h"
#include "steam/steam_api.h"

class CRD_Collection_Tab_Inventory : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Inventory, TGD_Tab );
public:
	CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot );
	virtual ~CRD_Collection_Tab_Inventory();

	virtual TGD_Details *CreateDetails() override;
	virtual void OnThink() override;

	void UpdateErrorMessage( TGD_Grid *pGrid );

	const char *m_szSlot;
	SteamInventoryResult_t m_hResult;
	bool m_bUnavailable;
	bool m_bForceUpdateMessage;
};

class CRD_Collection_Details_Inventory : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Inventory, TGD_Details );
public:
	CRD_Collection_Details_Inventory( CRD_Collection_Tab_Inventory *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnThink() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;

	vgui::ImagePanel *m_pIcon;
	vgui::RichText *m_pTitle;
	vgui::RichText *m_pDescription;
};

class CRD_Collection_Entry_Inventory : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Inventory, TGD_Entry );
public:
	CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, int index, SteamItemDetails_t details );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	CRD_Collection_Tab_Inventory *GetTab();

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pEquippedMarker;

	int m_Index;
	SteamItemDetails_t m_Details;
};

void LaunchCollectionsFrame();
