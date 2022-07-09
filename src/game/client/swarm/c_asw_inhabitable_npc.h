#ifndef _INCLUDED_C_ASW_INHABITABLE_NPC_H
#define _INCLUDED_C_ASW_INHABITABLE_NPC_H
#include "c_ai_basenpc.h"
#ifdef _WIN32
#pragma once
#endif

class C_ASW_Inhabitable_NPC : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_ASW_Inhabitable_NPC, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_ASW_Inhabitable_NPC();
	virtual			~C_ASW_Inhabitable_NPC();

private:
	C_ASW_Inhabitable_NPC( const C_ASW_Inhabitable_NPC & ) = delete; // not defined, not accessible
};
#endif // _INCLUDED_C_ASW_INHABITABLE_NPC_H
