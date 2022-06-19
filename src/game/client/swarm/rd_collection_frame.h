#pragma once

#include <vgui_controls/Frame.h>

enum class RD_COLLECTION_TYPE
{
	EQUIPMENT_REGULAR,
	EQUIPMENT_EXTRA,
	ALIENS,
	MARINES,

	INVENTORY_MEDALS,

	NUM_TYPES,
};

class CNB_Header_Footer;
class CRD_Collection_List;
class CRD_Collection_Entry;
class CRD_Collection_Details;
class vgui::PropertySheet;

class CRD_Collection_Frame : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Frame, vgui::Frame );
public:
	CRD_Collection_Frame( vgui::Panel *pParent, const char *pElementName, RD_COLLECTION_TYPE iCollectionType = RD_COLLECTION_TYPE::NUM_TYPES );
	CRD_Collection_Frame( vgui::Panel *pParent, const char *pElementName, int iBriefingSlot, int iInventorySlot );
	virtual ~CRD_Collection_Frame();

	virtual void OnCommand( const char *command );
	virtual void OnKeyCodeTyped( vgui::KeyCode keycode );
	virtual void OnKeyCodePressed( vgui::KeyCode keycode );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

	void AddTab( RD_COLLECTION_TYPE iCollectionType );

	CNB_Header_Footer *m_pHeaderFooter;
	vgui::PropertySheet *m_pSheet;
	CRD_Collection_Details *m_pDetails[int( RD_COLLECTION_TYPE::NUM_TYPES )];
	int m_iPrevActiveTab;
};
