#ifndef ASW_INHABITABLE_NPC_H
#define ASW_INHABITABLE_NPC_H
#pragma once

#include "ai_playerally.h"

class CASW_Inhabitable_NPC : public CAI_PlayerAlly
{
public:
	DECLARE_CLASS( CASW_Inhabitable_NPC, CAI_PlayerAlly );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CASW_Inhabitable_NPC();
	virtual ~CASW_Inhabitable_NPC();
};

#endif /* ASW_INHABITABLE_NPC_H */
