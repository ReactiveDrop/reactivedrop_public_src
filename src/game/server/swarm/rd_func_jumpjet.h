#pragma once

#include "basetoggle.h"

DECLARE_AUTO_LIST( IJumpJetVolumes );

class CRD_Func_JumpJet : public CServerOnlyEntity, public IJumpJetVolumes
{
public:
	DECLARE_CLASS( CRD_Func_JumpJet, CServerOnlyEntity );
	DECLARE_DATADESC();
	IMPLEMENT_AUTO_LIST_GET();

	CRD_Func_JumpJet();
	virtual ~CRD_Func_JumpJet();

	virtual void Spawn();

	bool IsPointInBounds( const Vector & vec );

	void InputEnable( inputdata_t & data );
	void InputDisable( inputdata_t & data );

	bool m_bDisabled;
};
