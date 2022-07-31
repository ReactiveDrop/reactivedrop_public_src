#ifndef _INCLUDED_C_ASW_INHABITABLE_NPC_H
#define _INCLUDED_C_ASW_INHABITABLE_NPC_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"

class C_ASW_Player;
class C_ASW_Weapon;

class C_ASW_Inhabitable_NPC : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_ASW_Inhabitable_NPC, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_ASW_Inhabitable_NPC();
	virtual ~C_ASW_Inhabitable_NPC();

	virtual bool IsInhabited();
	C_ASW_Player *GetCommander() const;
	CNetworkHandle( C_ASW_Player, m_Commander );
	const char *GetPlayerName() const;

	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual bool ShouldPredict( void );
	virtual C_BasePlayer *GetPredictionOwner();
	virtual void InitPredictable( C_BasePlayer *pOwner );

	// using entities over time
	C_BaseEntity *GetUsingEntity() { return m_hUsingEntity.Get(); }
	CNetworkHandle( C_BaseEntity, m_hUsingEntity );	// if set, marine will face this object
	const Vector &GetFacingPoint();
	void SetFacingPoint( const Vector &vec, float fDuration );
	Vector m_vecFacingPoint, m_vecFacingPointFromServer;
	float m_fStopFacingPointTime;

	C_ASW_Weapon *GetActiveASWWeapon( void ) const;
	C_ASW_Weapon *GetASWWeapon( int index ) const;

	void TickRedName( float delta );
	float m_fRedNamePulse;	// from 0 to 1, how red the marine's name should appear on the HUD for medics
	bool m_bRedNamePulseUp;

	bool m_bUseLastRenderedEyePosition;
	float m_fLastTurningYaw;

	virtual float MaxSpeed();
	virtual float GetBasePlayerYawRate();
	int m_nOldButtons;
	CNetworkVar( bool, m_bWalking );
	CNetworkVar( bool, m_bInhabited );

	// Texture names and surface data, used by CASW_MarineGameMovement
	int				m_surfaceProps;
	surfacedata_t *m_pSurfaceData;
	float			m_surfaceFriction;
	char			m_chTextureType;
	char			m_chPreviousTextureType;	// Separate from m_chTextureType. This is cleared if the player's not on the ground.

private:
	C_ASW_Inhabitable_NPC( const C_ASW_Inhabitable_NPC & ) = delete; // not defined, not accessible
};

#endif // _INCLUDED_C_ASW_INHABITABLE_NPC_H
