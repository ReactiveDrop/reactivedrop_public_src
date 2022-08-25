#ifndef _INCLUDED_C_ASW_OBJECTIVE_TRIGGERED_H
#define _INCLUDED_C_ASW_OBJECTIVE_TRIGGERED_H

#include "c_asw_objective.h"

class C_ASW_Objective_Triggered : public C_ASW_Objective
{
public:
	DECLARE_CLASS( C_ASW_Objective_Triggered, C_ASW_Objective );
	DECLARE_CLIENTCLASS();

	C_ASW_Objective_Triggered();

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual const wchar_t *GetObjectiveTitle();
	virtual bool NeedsTitleUpdate();
	virtual float GetObjectiveProgress();

	CNetworkVar( int, m_nProgress );
	CNetworkVar( int, m_nMaxProgress );
	int m_nPrevProgress;
	bool m_bNeedsUpdate;

	wchar_t m_wszObjectiveTitle[256];

private:
	C_ASW_Objective_Triggered( const C_ASW_Objective_Triggered & ) = delete;
};


#endif // _INCLUDED_C_ASW_OBJECTIVE_TRIGGERED_H
