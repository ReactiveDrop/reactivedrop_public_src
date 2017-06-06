#ifndef asw_item_regen_h__
#define asw_item_regen_h__

class CASW_Item_Regen : public CLogicalEntity 
{
public:
	DECLARE_CLASS( CASW_Item_Regen, CLogicalEntity );
	CASW_Item_Regen();
	~CASW_Item_Regen();

	virtual void Spawn();
	virtual void Think();
};

#endif // asw_item_regen_h__