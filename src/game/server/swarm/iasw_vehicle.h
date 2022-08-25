#ifndef _INCLUDED_ASW_VEHICLE_H
#define _INCLUDED_ASW_VEHICLE_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_shareddefs.h"
#include "iasw_server_usable_entity.h"

class CASW_Marine;
class CASW_Player;
class CBasePlayer;
class CUserCmd;
class IMoveHelper;
class CMoveData;

abstract_class IASW_Vehicle : public IASW_Server_Usable_Entity
{
public:
	virtual int ASWGetNumPassengers() = 0;
	virtual void ASWSetDriver(CASW_Marine* pDriver) = 0;
	virtual CASW_Marine* ASWGetDriver() = 0;
	virtual CASW_Marine* ASWGetPassenger(int i) = 0;
	virtual void ASWStartEngine() = 0;
	virtual void ASWStopEngine() = 0;
	virtual void SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move ) = 0;
	virtual void ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData ) = 0;

	// IASW_Server_Usable_Entity
	virtual bool RequirementsMet( CBaseEntity *pUser ) { return true; }
	virtual bool NeedsLOSCheck() { return false; }
};

#endif // _INCLUDED_ASW_VEHICLE_H
