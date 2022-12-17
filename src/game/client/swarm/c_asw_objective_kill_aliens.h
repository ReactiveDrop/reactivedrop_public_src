#ifndef _INCLUDED_C_ASW_OBJECTIVE_KILL_ALIENS_H
#define _INCLUDED_C_ASW_OBJECTIVE_KILL_ALIENS_H

#include "c_asw_objective.h"

class C_ASW_Objective_Kill_Aliens : public C_ASW_Objective
{
public:
	DECLARE_CLASS( C_ASW_Objective_Kill_Aliens, C_ASW_Objective );
	DECLARE_CLIENTCLASS();

	C_ASW_Objective_Kill_Aliens();

	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual const wchar_t *GetObjectiveTitle();
	virtual bool NeedsTitleUpdate();
	virtual float GetObjectiveProgress();
	void FindText();
	const wchar_t *GetPluralText();
	const wchar_t *GetSingularText();

	CNetworkVar(int, m_iTargetKills);
	CNetworkVar(int, m_iCurrentKills);
	CNetworkVar(int, m_AlienClassNum);

	const wchar_t *m_pKillText;
	const wchar_t *m_pAlienPluralText;
	const wchar_t *m_pAlienSingularText;
	bool m_bFoundText;

	wchar_t m_wszTitleBuffer[64];
	int m_iLastKills;

private:
	C_ASW_Objective_Kill_Aliens( const C_ASW_Objective_Kill_Aliens & ) = delete; // not defined, not accessible
};

#endif // _INCLUDED_C_ASW_OBJECTIVE_KILL_ALIENS_H
