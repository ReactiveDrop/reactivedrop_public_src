#pragma once

class CRD_Briefing_Camera : public CPointEntity
{
	DECLARE_CLASS( CRD_Briefing_Camera, CPointEntity );
public:
	DECLARE_DATADESC();

	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo ) override;
};
