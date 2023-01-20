//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		The default shared conditions
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	SQUADSLOT_H
#define	SQUADSLOT_H

#define	MAX_SQUADSLOTS 64

//=========================================================
// These are the default shared squad slots
//
// NOTE: If you add a new squad slot here, make sure you
// update GetSquadSlotDebugName()
//=========================================================
enum SQUAD_SLOT_t {

	// Currently there are no shared squad slots
	SQUAD_SLOT_NONE = -1,

	SQUAD_SLOT_ATTACK1 = 0,		// reserve 10 attack slots for most squads (limited by convar)
	SQUAD_SLOT_ATTACK2,
	SQUAD_SLOT_ATTACK3,
	SQUAD_SLOT_ATTACK4,
	SQUAD_SLOT_ATTACK5,
	SQUAD_SLOT_ATTACK6,
	SQUAD_SLOT_ATTACK7,
	SQUAD_SLOT_ATTACK8,
	SQUAD_SLOT_ATTACK9,
	SQUAD_SLOT_ATTACK10,

	SQUAD_SLOT_INVESTIGATE_SOUND,

	SQUAD_SLOT_EXCLUSIVE_HANDSIGN,	// only one person in a squad should do this!
	SQUAD_SLOT_EXCLUSIVE_RELOAD,	

	SQUAD_SLOT_PICKUP_WEAPON1,
	SQUAD_SLOT_PICKUP_WEAPON2,

	SQUAD_SLOT_SPECIAL_ATTACK,	// Combine Elite using the combine ball attack, for instance.

	// ======================================
	// IMPORTANT: This must be the last enum
	// ======================================
	LAST_SHARED_SQUADSLOT,		
};

SQUAD_SLOT_t GetMaxAttackSquadSlot();

#endif	//SQUADSLOT_H
