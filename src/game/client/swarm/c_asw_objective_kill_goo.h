#ifndef _INCLUDED_C_ASW_OBJECTIVE_KILL_GOO_H
#define _INCLUDED_C_ASW_OBJECTIVE_KILL_GOO_H

#include "c_asw_objective.h"

class C_ASW_Objective_Kill_Goo : public C_ASW_Objective
{
public:
	DECLARE_CLASS( C_ASW_Objective_Kill_Goo, C_ASW_Objective );
	DECLARE_CLIENTCLASS();

	C_ASW_Objective_Kill_Goo();

	virtual const wchar_t* GetObjectiveTitle();
	virtual bool NeedsTitleUpdate();
	virtual float GetObjectiveProgress();
	void FindText();
	wchar_t *GetPluralText();
	wchar_t *GetSingularText();

	CNetworkVar(int, m_iTargetKills);
	CNetworkVar(int, m_iCurrentKills);
	
	wchar_t m_wszTitleBuffer[64];
	int m_iLastKills;

private:
	C_ASW_Objective_Kill_Goo( const C_ASW_Objective_Kill_Goo & ) = delete; // not defined, not accessible
};

#endif // _INCLUDED_C_ASW_OBJECTIVE_KILL_GOO_H
