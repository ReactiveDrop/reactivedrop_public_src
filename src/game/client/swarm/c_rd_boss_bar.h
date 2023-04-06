#pragma once

#include "rd_boss_bar_shared.h"
#include "c_baseentity.h"

class C_RD_Boss_Bar : public C_BaseEntity
{
	DECLARE_CLASS( C_RD_Boss_Bar, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	RD_Boss_Bar_Mode m_BarMode;
	float m_flBarValue;
	float m_flBarMaxValue;
	char m_iszBarID[256];
	char m_iszBarTitle[256];
	int m_iBarColumn;
	int m_iBarRow;
	float m_flBarHeightScale;
	Color m_BarFGColor;
	Color m_BarBGColor;
	Color m_BarBorderColor;
	Color m_BarFlashColor;
	float m_flBarFlashSustain;
	float m_flBarFlashInterpolate;
	bool m_bEnabled;
	float m_flBarRadius;
	bool m_bBarTooFarAway;

	float m_flBarValuePrev;
	float m_flBarValueTruePrev;
	float m_flBarValueLastChanged;

	C_RD_Boss_Bar();

	bool IsTooFarAway();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void ClientThink();
	void SendHudUpdate( bool bCreated );
};
