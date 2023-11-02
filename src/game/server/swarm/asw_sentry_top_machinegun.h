#ifndef _DEFINED_ASW_SENTRY_TOP_MACHINEGUN_H
#define _DEFINED_ASW_SENTRY_TOP_MACHINEGUN_H
#pragma once

#include "asw_sentry_top.h"

class CASW_Sentry_Top_Machinegun : public CASW_Sentry_Top
{
public:
	DECLARE_CLASS( CASW_Sentry_Top_Machinegun, CASW_Sentry_Top );
	DECLARE_DATADESC();

	void Spawn( void ) override;

	void SetTopModel() override;
	bool HasHysteresis() override;
	void Fire() override;

	// Classification
	Class_T Classify( void ) override { return (Class_T) CLASS_ASW_SENTRY_GUN; }

protected:
	float m_flFireHysteresisTime; // some turrets have a mechanism to continue shooting without an enemy
	 
};


#endif /* _DEFINED_ASW_SENTRY_TOP_H */