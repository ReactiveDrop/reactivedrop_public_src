#ifndef ASW_PLAYER_H
#define ASW_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_shareddefs.h"
#include "asw_player_shared.h"
#include "player.h"
#include "asw_playerlocaldata.h"
#include "server_class.h"
#include "asw_playeranimstate.h"
#include "asw_game_resource.h"
#include "asw_info_message_shared.h"
#include "basemultiplayerplayer.h"
#include "steam/steam_api.h"
#include "rd_inventory_shared.h"

class CASW_Inhabitable_NPC;
class CASW_Marine;
class CRagdollProp;

//=============================================================================
// >> Swarm player
//=============================================================================
class CASW_Player : public CBaseMultiplayerPlayer, public IASWPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( CASW_Player, CBaseMultiplayerPlayer );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	CASW_Player();
	virtual ~CASW_Player();

	virtual int UpdateTransmitState();

	static CASW_Player *CreatePlayer( const char *className, edict_t *ed );
	static CASW_Player *Instance( int iEnt );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event );

	virtual void PreThink();
	virtual void PostThink();
	virtual void Spawn();
	virtual void Precache();
	virtual void UpdateBattery( void ) { }
	void DriveNPCMovement( CUserCmd *ucmd, IMoveHelper *moveHelper );
	void PushawayThink();
	virtual void AvoidPhysicsProps( CUserCmd *pCmd );
	virtual bool ClientCommand( const CCommand &args );
	virtual void UpdateTonemapController( void ) override;
	virtual void UpdateFXVolume( void ) override;

	void EmitPrivateSound( const char *soundName, bool bFromNPC = false );

	// eye position is above the marine we're remote controlling
	virtual Vector EyePosition(void);
	virtual Vector	EarPosition( void );
	virtual CBaseEntity		*GetSoundscapeListener();
	Vector	m_vecLastMarineOrigin;	// remember last known position to keep camera there when gibbing
	Vector m_vecFreeCamOrigin;	// if the player is using freecam cheat to look around, he'll tell us about that position
	bool m_bUsedFreeCam;		// has the player ever sent a freecam message (if he has, we started adding the freecam's pos to PVS)

	const QAngle& EyeAngles();
	virtual const QAngle& EyeAnglesWithCursorRoll();
	const Vector& GetCrosshairTracePos();
	void SetCrosshairTracePos( const Vector &vecPos ) { m_vecCrosshairTracePos = vecPos; }
	virtual void SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );
	void SetupVisibilityForNPC( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize, CASW_Inhabitable_NPC *pNPC );

	virtual void  HandleSpeedChanges( void );
	virtual bool CanBeSeenBy( CAI_BaseNPC *pNPC ) { return false; } // Players are never seen by NPCs

	CNetworkHandle( CASW_Inhabitable_NPC, m_hInhabiting );

	bool ShouldAutoReload() { return m_bAutoReload; }
	bool m_bAutoReload;
	
	// Resurrection
	HSCRIPT ResurrectMarine( const Vector position, bool bEffect = true );

	// anim state helper
	virtual CBaseCombatWeapon* ASWAnim_GetActiveWeapon();
	virtual bool ASWAnim_CanMove();

	// spectating
	void SpectateNextMarine();
	void SetSpectatingNPC( CASW_Inhabitable_NPC *pSpectating );
	CASW_Inhabitable_NPC *GetSpectatingNPC() const;
	HSCRIPT ScriptGetSpectatingNPC() const;
	CNetworkHandle( CASW_Inhabitable_NPC, m_hSpectating );
	bool m_bLastAttackButton;	// used to detect left clicks for cycling through marines
	bool m_bLastAttack2Button;	// used to detect right clicks for cycling through marines
	bool m_bRequestedSpectator;	// this player requested to be a spectator since the start of a match (won't be considered for leader, campaign votes, etc.)
	float m_fLastControlledMarineTime;
	CNetworkVar( float, m_fMarineDeathTime );	// same as above but optimized for networking
	bool IsSpectatorOnly();	// for players who can *only* spectate, i.e. not able to control characters
	CNetworkVar( bool, m_bWantsSpectatorOnly );

	void BecomeNonSolid();
	void OnNPCCommanded( CASW_Inhabitable_NPC *pNPC );
	void SetNPC( CASW_Inhabitable_NPC *pNPC );
	CASW_Inhabitable_NPC *GetNPC() const;
	HSCRIPT ScriptGetNPC() const;
	void SelectNextMarine( bool bReverse );
	bool CanSwitchToMarine( int num );
	// BenLubar(deathmatch-improvements)
	void SwitchInhabiting( CASW_Inhabitable_NPC *pNPC );
	void SwitchMarine( CASW_Marine_Resource *pMR, bool set_squad_leader = true );
	void SwitchMarine( int num, bool set_squad_leader = true );
	void OrderMarineFace( int iMarine, float fYaw, Vector &vecOrderPos );
	void LeaveMarines();
	bool HasLiveMarines();
	virtual bool IsAlive( void );
	HSCRIPT ScriptGetMarine();

	CNetworkHandle( CASW_Marine, m_hOrderingMarine );

	CASW_Game_Resource* GetGNI();

	// This player's Infested specific data that should only be replicated to 
	//  the player and not to other players.
	CNetworkVarEmbedded( CASWPlayerLocalData, m_ASWLocal );

	// gets the firing direction when the player is firing through a marine
	Vector GetAutoaimVectorForMarine(CASW_Marine* marine, float flDelta, float flNearMissDelta);
	QAngle MarineAutoaimDeflection( Vector &vecSrc, float flDist, float flDelta, float flNearMissDelta);
	QAngle m_angMarineAutoAim;
	CNetworkQAngle( m_angMarineAutoAimFromClient );

	void				FlashlightTurnOn( void );
	void				FlashlightTurnOff( void );

	// Marine position prediction (used by AI to compensate for player lag times)
	void	MoveMarineToPredictedPosition();
	void	RestoreMarinePosition();
	Vector	m_vecStoredPosition;

	virtual	CBaseCombatCharacter *ActivePlayerCombatCharacter( void );

	// shared code
	CASW_Inhabitable_NPC *GetViewNPC() const;
	HSCRIPT ScriptGetViewNPC() const;
	void ItemPostFrame();
	void ASWSelectWeapon(CBaseCombatWeapon* pWeapon, int subtype);	// for switching weapons on the current marine
	virtual bool Weapon_CanUse( CBaseCombatWeapon *pWeapon );
	virtual CBaseCombatWeapon*	Weapon_OwnsThisType( const char *pszWeapon, int iSubType = 0 ) const;  // True if already owns a weapon of this class
	virtual int Weapon_GetSlot( const char *pszWeapon, int iSubType = 0 ) const;  // Returns -1 if they don't have one
	ASW_Controls_t GetASWControls();

	// searches for nearby entities that we can use (pickups, buttons, etc)
	virtual void PlayerUse();
	virtual void FindUseEntities();
	void SortUsePair( CBaseEntity **pEnt1, CBaseEntity **pEnt2, int *pnFirstPriority, int *pnSecondPriority );
	int GetUsePriority(CBaseEntity* pEnt);
	void ActivateUseIcon( int iUseEntityIndex, int nHoldType );
	CBaseEntity* GetUseEntity( int i );
	int GetNumUseEntities() { return m_iUseEntities; }
	EHANDLE m_hUseEntities[ ASW_PLAYER_MAX_USE_ENTS ];
	int m_iUseEntities;
	CNetworkVar( float, m_flUseKeyDownTime );
	CNetworkVar( EHANDLE, m_hUseKeyDownEnt );

	/**
		In Reactive Drop players can rotate the camera. The marine's movement 
		must be aligned to the camera axis. So we store this pitch value 
		inside an ASW_Player class and use it in movement code to determine the
		movement direction. The value is in degrees an must be in range 0..359.
		For now only fixed camera angles are used 0, 90, 180, 270. So this 
		variable will hold only one of these these 4 values. 90 is default val
	*/
	CNetworkVar( float, m_flMovementAxisYaw );

	// looking at an info panel
	void ShowInfoMessage(CASW_Info_Message* pMessage);
	void HideInfoMessage();
	CNetworkHandle (CASW_Info_Message, m_pCurrentInfoMessage);

	virtual void SetAnimation( PLAYER_ANIM playerAnim );

	// ragdoll blend test
	void RagdollBlendTest();
	CRagdollProp* m_pBlendRagdoll;
	float m_fBlendAmount;

	// AI perf debugging
	float m_fLastAICountTime;

	// changing name
	bool CanChangeName() { return true; }
	void ChangeName(const char *pszNewName);
	bool HasFullyJoined() { return m_bSentJoinedMessage; }
	CNetworkVar(bool, m_bSentJoinedMessage);	// has this player told everyone that he's fully joined yet
	void OnFullyJoined( bool bSendGameEvent );
	bool IsAnyBot();

	void WelcomeMessageThink();

	// voting
	int m_iKLVotesStarted;	// how many kick or leader votes this player has started, if he starts too many, flood control will be applied to the announcements
	float m_fLastKLVoteTime;	// last time we started a kick or leader vote
	CNetworkVar(int, m_iLeaderVoteIndex);	// entindex of the player we want to be leader
	CNetworkVar(int, m_iKickVoteIndex);		// entindex of the player we want to be kicked
	const char* GetASWNetworkID();
	CNetworkVar( int, m_iMapVoted );	// 0 = didn't vote, 1 = "no" vote, 2 = "yes" vote
	bool CanVote();
	bool CanBeKicked();
	bool CanBeLeader();

	float m_flLastActiveTime;
	CNetworkVar( float, m_flInactiveKickWarning );

	// client stat counts (these are numbers each client stores and provides to the server on player creation, so server can decide to award medals)
	int m_iClientKills;
	int m_iClientMissionsCompleted;
	int m_iClientCampaignsCompleted;

	// char array of medals awarded (used for stats upload)
	CUtlVector<unsigned char> m_CharMedals;

	// have we notified other players that we're impatiently waiting on them?
	bool m_bPrintedWantStartMessage;
	bool m_bPrintedWantsContinueMessage;

	virtual void			CreateViewModel( int viewmodelindex = 0 ) { return; }	// ASW players don't have viewmodels
	virtual void SetPunchAngle( const QAngle &punchAngle ) { return; }				// ASW players never use punch angle

	void StartWalking( void );
	void StopWalking( void );
	bool IsWalking( void ) { return m_fIsWalking; }

	// random map generation
	CNetworkVar( float, m_fMapGenerationProgress );			// how far this player is through generating their local copy of the next random map

	virtual CBaseEntity* FindPickerEntity();
	HSCRIPT ScriptFindPickerEntity();

	// entity this player is highlighting with his mouse cursor
	void SetHighlightEntity( CBaseEntity* pEnt ) { m_hHighlightEntity = pEnt; }
	CBaseEntity* GetHighlightEntity() const { return m_hHighlightEntity.Get(); }
	EHANDLE m_hHighlightEntity;

	// status of selecting marine/weapons in the briefing
	CNetworkVar( int, m_nChangingMR );
	CNetworkVar( int, m_nChangingSlot );

	// experience
	int GetLevel();
	int GetLevelBeforeDebrief();
	int GetExperience();
	int GetExperienceBeforeDebrief() { return m_iExperienceBeforeDebrief; }
	void SetNetworkedExperience( int iExperience ) { m_iNetworkedXP = iExperience; }
	void SetNetworkedPromotion( int iPromotion ) { m_iNetworkedPromotion = iPromotion; }
	int GetPromotion();
	int GetEarnedXP( CASW_Earned_XP_t nType ) { return m_iEarnedXP[ nType ]; }	
	void CalculateEarnedXP();

	void AwardExperience();					// calculates earned XP.  Steam stat setting actually happens on the client.
	void RequestExperience();				// asks Steam for your current XP

	int32 m_iExperience;
	int32 m_iExperienceBeforeDebrief;
	int32 m_iPromotion;
	int32 m_iEarnedXP[ ASW_NUM_XP_TYPES ];
	int32 m_iStatNumXP[ ASW_NUM_XP_TYPES ];
	CNetworkVar( int, m_iNetworkedXP );
	CNetworkVar( int, m_iNetworkedPromotion );
	SteamInventoryResult_t m_EquippedItems[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS];
	CNetworkVarEmbedded( CRD_ItemInstance, m_EquippedMedal );
	void ClearEquippedItemForSlot( const char *szSlot );
	void SetEquippedItemForSlot( const char *szSlot, const ReactiveDropInventory::ItemInstance_t &instance );

	bool m_bHasAwardedXP;
	bool m_bPendingSteamStats;
	float m_flPendingSteamStatsStart;
	bool m_bSentPromotedMessage;

	static CBaseEntity *spawn_point;
	bool m_bWelcomed;
	float m_fLastFragTime; 
	int   m_iKillingSpree;

#if !defined(NO_STEAM)
	CCallResult< CASW_Player, UserStatsReceived_t > m_CallbackUserStatsReceived;
	void Steam_OnUserStatsReceived( UserStatsReceived_t *pUserStatsReceived, bool bError );
#endif
	// BenLubar(spectator-mouse)
	CNetworkVar( short, m_iScreenWidth );
	CNetworkVar( short, m_iScreenHeight );
	CNetworkVar( short, m_iMouseX );
	CNetworkVar( short, m_iMouseY );

	bool m_bLeaderboardReady;

private:

	// Copyed from EyeAngles() so we can send it to the client.
	CNetworkQAngle( m_angEyeAngles );	
	Vector m_vecCrosshairTracePos;			// the world location directly beneath the player's crosshair
	CNetworkVarForDerived( bool, m_fIsWalking );

	IPlayerAnimState *m_PlayerAnimState;
	bool m_bFirstInhabit;
};

extern void TE_MarineAnimEvent( CASW_Marine *pMarine, PlayerAnimEvent_t event );
extern void TE_MarineAnimEventExceptCommander( CASW_Marine *pMarine, PlayerAnimEvent_t event );
extern void TE_MarineAnimEventJustCommander( CASW_Marine *pMarine, PlayerAnimEvent_t event );

void OrderNearbyMarines(CASW_Player *pPlayer, ASW_Orders NewOrders, bool bAcknowledge = true );

inline CASW_Player* ToASW_Player( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CASW_Player*>( pEntity ) != 0 );
#endif
	return static_cast< CASW_Player* >( pEntity );
}

inline CASW_Player* ToASW_Player( CBasePlayer* pPlayer )
{
	if ( !pPlayer )
		return NULL;

#ifdef _DEBUG
	Assert(dynamic_cast<CASW_Player*>(pPlayer) != 0);
#endif
	return static_cast<CASW_Player*>( pPlayer );
}


#endif	// ASW_PLAYER_H
