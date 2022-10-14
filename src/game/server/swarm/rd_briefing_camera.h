#pragma once

class CRD_Briefing_Camera : public CBaseEntity
{
	DECLARE_CLASS( CRD_Briefing_Camera, CBaseEntity );
public:
	CRD_Briefing_Camera();

	DECLARE_DATADESC();

	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo ) override;
};
