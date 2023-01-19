#ifndef ASW_INHABITABLE_NPC_H
#define ASW_INHABITABLE_NPC_H
#pragma once

#include "ai_playerally.h"
#include "asw_player_shared.h"

class CASW_Player;
class CASW_Weapon;
class CEnvTonemapController;
class CTonemapTrigger;

#define ASW_ALIEN_HEALTH_BITS 14

class CASW_Inhabitable_NPC : public CAI_PlayerAlly
{
public:
	DECLARE_CLASS( CASW_Inhabitable_NPC, CAI_PlayerAlly );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	CASW_Inhabitable_NPC();
	virtual ~CASW_Inhabitable_NPC();

	bool IsInhabitableNPC() const override { return true; }

	void SetCommander( CASW_Player *player );
	CASW_Player *GetCommander() const;
	HSCRIPT ScriptGetCommander() const;
	CNetworkHandle( CASW_Player, m_Commander );

	virtual bool IsInhabited() { return m_bInhabited; }
	virtual void SetInhabited( bool bInhabited ) { m_bInhabited = bInhabited; }
	virtual void InhabitedBy( CASW_Player *player ) { }
	virtual void UninhabitedBy( CASW_Player *player ) { }

	void SetInitialCommander( CASW_Player *player );
	char m_szInitialCommanderNetworkID[64]; // ASWNetworkID of the first commander for this marine in this mission
	const char *GetPlayerName() const;

	virtual void Suicide();

	CASW_Weapon *GetASWWeapon( int index ) const;
	CASW_Weapon *GetActiveASWWeapon() const;
	int m_iDamageAttributeEffects;

	virtual bool StartUsing( CBaseEntity *pEntity );
	virtual void StopUsing();
	inline CBaseEntity *GetUsingEntity() const { return m_hUsingEntity.Get(); }
	CNetworkHandle( CBaseEntity, m_hUsingEntity );
	void SetFacingPoint( const Vector &vec, float fDuration );
	CNetworkVar( Vector, m_vecFacingPointFromServer );
	float m_fStopFacingPointTime;

	virtual int TranslateSchedule( int scheduleType ) override;
	virtual float MaxSpeed();
	int m_nOldButtons;
	CNetworkVar( bool, m_bWalking );
	CNetworkVar( bool, m_bInhabited );
	ASW_Controls_t GetASWControls();
	CNetworkVar( int, m_iControlsOverride );
	void ScriptSetControls( int iControls );

	// Texture names and surface data, used by CASW_MarineGameMovement
	int				m_surfaceProps;
	surfacedata_t *m_pSurfaceData;
	float			m_surfaceFriction;
	char			m_chTextureType;
	char			m_chPreviousTextureType;	// Separate from m_chTextureType. This is cleared if the player's not on the ground.

	void ScriptSetFogController( HSCRIPT hEnt );
	CHandle<CFogController> m_hFogController;
	void ScriptSetPostProcessController( HSCRIPT hEnt );
	CHandle<CPostProcessController> m_hPostProcessController;
	void ScriptSetColorCorrection( HSCRIPT hEnt );
	CHandle<CColorCorrection> m_hColorCorrection;
	void ScriptSetTonemapController( HSCRIPT hEnt );
	CHandle<CEnvTonemapController> m_hTonemapController;
	void OnTonemapTriggerStartTouch( CTonemapTrigger *pTonemapTrigger );
	void OnTonemapTriggerEndTouch( CTonemapTrigger *pTonemapTrigger );
	CUtlVector<CHandle<CTonemapTrigger>> m_hTriggerTonemapList;

	virtual void DoImpactEffect( trace_t &tr, int nDamageType );
	virtual void DoMuzzleFlash();
	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );
	virtual void MakeUnattachedTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );
};

#endif /* ASW_INHABITABLE_NPC_H */
