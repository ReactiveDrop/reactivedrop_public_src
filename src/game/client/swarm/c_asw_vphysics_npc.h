#ifndef _INCLUDED_C_ASW_VPHYSICS_NPC_H
#define _INCLUDED_C_ASW_VPHYSICS_NPC_H
#include "c_asw_inhabitable_npc.h"
#ifdef _WIN32
#pragma once
#endif

class C_ASW_VPhysics_NPC : public C_ASW_Inhabitable_NPC
{
public:
	DECLARE_CLASS( C_ASW_VPhysics_NPC, C_ASW_Inhabitable_NPC );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_ASW_VPhysics_NPC();
	virtual			~C_ASW_VPhysics_NPC();

private:
	C_ASW_VPhysics_NPC( const C_ASW_VPhysics_NPC & ); // not defined, not accessible
};
#endif // _INCLUDED_C_ASW_VPHYSICS_NPC_H
