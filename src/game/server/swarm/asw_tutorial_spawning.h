#ifndef _INCLUDED_ASW_TUTORIAL_SPAWNING_H
#define _INCLUDED_ASW_TUTORIAL_SPAWNING_H
#ifdef _WIN32
#pragma once
#endif

#include "info_player_start.h"

class CASW_TutorialStartPoint : public CBaseStart
{
public:
	DECLARE_DATADESC();
	DECLARE_CLASS( CASW_TutorialStartPoint, CPointEntity );

private:
	int m_iMarineSlot;
	int m_iSaveStage;

public:
	static CASW_TutorialStartPoint* GetTutorialStartPoint(int iMarineSlot);
	static int GetTutorialSaveStage();
};

#endif // _INCLUDED_ASW_TUTORIAL_SPAWNING_H