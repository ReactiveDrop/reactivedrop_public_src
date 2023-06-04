#pragma once

#include "basemodui.h"
#include "rd_inventory_shared.h"

class CNB_Button;
class CNB_Gradient_Bar;
class CASW_Model_Panel;

namespace BaseModUI
{

class ItemShowcase : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( ItemShowcase, CBaseModFrame );

public:
	ItemShowcase( vgui::Panel *parent, const char *panelName );
	~ItemShowcase();

	void OnOpen() override;
	void OnTick() override;
	void OnThink() override;
	void OnCommand( const char *command ) override;
	void OnKeyCodePressed( vgui::KeyCode keycode ) override;
	void OnKeyCodeTyped( vgui::KeyCode code ) override;
	void PerformLayout() override;
	void PaintBackground() override;
	void PostChildPaint() override;

	enum Mode_t
	{
		MODE_INSPECT,
		MODE_ITEM_DROP,
		MODE_UNLOCK_REGULAR_WEAPON,
		MODE_UNLOCK_EXTRA_WEAPON,
	};

	static void ShowItems( SteamInventoryResult_t hResult, int iStart, int iCount, Mode_t mode );
	static void ShowItems( int iWeapon, Mode_t mode );
	static bool ShowWeaponByClass( const char *szWeaponClass );

	CUtlVectorAutoPurge<ReactiveDropInventory::ItemInstance_t *> m_Queue;
	CUtlVector<int> m_QueueExtra;
	CUtlVector<Mode_t> m_QueueType;

	vgui::Label *m_pTitle;
	vgui::Label *m_pWeaponLabel;
	vgui::Label *m_pSubTitle;
	CNB_Gradient_Bar *m_pBanner;
	CNB_Button *m_pBackButton;
	CASW_Model_Panel *m_pItemModelPanel;
	vgui::RichText *m_pDescriptionArea;
	int m_iWeaponLabelX;
	int m_iDescriptionAreaY;
	int m_iRepositionDescription;
	bool m_bNeedsMoveToFront;
	bool m_bShowWeaponOnNextFrame;
};

}
