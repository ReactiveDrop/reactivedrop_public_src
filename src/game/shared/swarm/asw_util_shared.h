#ifndef _INCLUDE_ASW_UTIL_SHARED_H
#define _INCLUDE_ASW_UTIL_SHARED_H
// Misc functions used by other bits of ASW

#include "util_shared.h"
#include "asw_shareddefs.h"
#include "steam/steamclientpublic.h"

#ifdef CLIENT_DLL
#define CPointCamera C_PointCamera
#endif

#define STEAM_LEADERBOARD_HOIAF_CURRENT_SEASON 7581028ULL

class CPhysicsProp;
class CASW_Player;
class CASW_Inhabitable_NPC;
class CASW_Marine;
class CASW_Marine_Resource;
class CPointCamera;

#ifdef CLIENT_DLL
class CNewParticleEffect;
#endif

int UTIL_ASW_GetNumPlayers();

// rotates one angle towards another, with a fixed turning rate over the time
float ASW_ClampYaw( float yawSpeedPerSec, float current, float target, float time );

// time independent movement of one angle to a fraction of the desired
float ASW_ClampYaw_Fraction( float fraction, float current, float target, float time );

float ASW_Linear_Approach( float current, float target, float delta);

bool ASW_LineCircleIntersection(
	const Vector2D &center,
	const float radius,
	const Vector2D &vLinePt,
	const Vector2D &vLineDir,
	float *fIntersection1,
	float *fIntersection2);

// is a marine nearby this spot?  i.e. can a player controlling this marine see this spot (bCorpseCanSee is set to true if a marine corpse can see this spot)
CASW_Marine* UTIL_ASW_MarineCanSee(CASW_Marine_Resource* pMR, const Vector &pos, const int padding, bool &bCorpseCanSee, const int forward_limit = -1);
CASW_Marine* UTIL_ASW_AnyMarineCanSee(const Vector &pos, const int padding, bool &bCorpseCanSee, const int forward_limit = -1);
// is a marine looking at this spot?
bool UTIL_ASW_MarineViewCone(const Vector &pos);
// default camera and dot values for the above function
#define MARINE_NEARBY_DOT_768   0.6322133
#define MARINE_NEARBY_DOT_1024  0.5335417
#define ASW_DEFAULT_CAMERA_DIR		 Vector(0,0.500000f,-0.866025f)
#define ASW_DEFAULT_CAMERA_OFFSET    Vector(0.000009f,-202.499985f,350.740306f)

//void UTIL_ASW_ValidateSoundName( string_t &name, const char *defaultStr );
void UTIL_ASW_ValidateSoundName( char *szString, int stringlength, const char *defaultStr );

/// get a parabola that goes from source to destination in specified time
Vector UTIL_LaunchVector( const Vector &src, const Vector &dest, float gravity, float flightTime = 0.0f );

// returns the first collision point for a thrown entity (MOVETYPE_FLYGRAVITY)
Vector UTIL_Check_Throw( const Vector &vecSrc, const Vector &vecThrowVelocity, float flGravity, const Vector &vecHullMins, const Vector &vecHullMaxs,
						int iCollisionMask = MASK_NPCSOLID, int iCollisionGroup = COLLISION_GROUP_PROJECTILE, CBaseEntity *pIgnoreEnt = NULL, bool bDrawArc = false );

void UTIL_Bound_Velocity( Vector &vec );

char* ASW_AllocString( const char *szString );

float UTIL_ASW_CalcFastDoorHackTime(int iNumRows, int iNumColumns, int iNumWires, int iHackLevel, float fSpeedScale);

#ifdef GAME_DLL
	void UTIL_ASW_ScreenShake( const Vector &center, float amplitude, float frequency, float duration, float radius, ShakeCommand_t eCommand, bool bAirShake = false, CASW_Marine *pOnlyMarine = NULL );
	void UTIL_ASW_ScreenPunch( const Vector &center, const Vector &direction, float amplitude, float frequency, float duration, float radius );
	void UTIL_ASW_ScreenPunch( const Vector &center, float radius, const ScreenShake_t &shake );
	void UTIL_ASW_PoisonBlur( CASW_Marine *pMarine, float duration );
	CASW_Marine* UTIL_ASW_NearestMarine( const Vector &pos, float &marine_distance, ASW_Marine_Class marineClass = MARINE_CLASS_UNDEFINED, bool bAIOnly = false );	// returns the nearest marine to this point
	CASW_Marine* UTIL_ASW_NearestMarine( const CASW_Marine *pMarine, float &marine_distance );	// returns the nearest marine to this marine
	int UTIL_ASW_NumCommandedMarines( const CASW_Player *pPlayer );	// returns the number of marines commanded by this player
	bool UTIL_ASW_BlockingMarine( CBaseEntity *pEntity );
	CASW_Marine* UTIL_ASW_Marine_Can_Chatter_Spot( CBaseEntity *pEntity, float fDist = 500.0f );

	class CASW_ViewNPCRecipientFilter : public CRecipientFilter
	{
	public:
		CASW_ViewNPCRecipientFilter();
		CASW_ViewNPCRecipientFilter( CASW_Inhabitable_NPC *pNPC, bool bSendToRecorders = true );

		void AddRecipientsByViewNPC( CASW_Inhabitable_NPC *pNPC, bool bSendToRecorders = true );
	};

	void UTIL_RD_HitConfirm( CBaseEntity *pTarget, int iHealthBefore, const CTakeDamageInfo &info );
#else
	bool UTIL_ASW_ClientsideGib(C_BaseAnimating* pEnt);
	CNewParticleEffect *UTIL_ASW_CreateFireEffect( C_BaseEntity *pEntity );
	void UTIL_ASW_ClientFloatingDamageNumber( const CTakeDamageInfo &info );
	HPARTICLEFFECT UTIL_ASW_ParticleDamageNumber( C_BaseEntity *pEnt, Vector vecPos, int iDamage, int iDmgCustom, float flScale, bool bRandomVelocity, bool bSkipRampUp );
	void UTIL_RD_DecideMainMenuBackground( const char *&szImage, const char *&szVideo, const char *&szAudio, bool bAllowChange );
	const char *UTIL_RD_RandomBriefingMovie( const char *szMapName, int iSeed, const char *szType = "briefing" );
#endif

void TryLocalize( const char *token, wchar_t *unicode, int unicodeBufferSizeInBytes );

void ASW_TransmitShakeEvent( CASW_Inhabitable_NPC *pNPC, float localAmplitude, float frequency, float duration, ShakeCommand_t eCommand, const Vector &direction = Vector(0,0,0) );
void ASW_TransmitShakeEvent( CASW_Inhabitable_NPC *pNPC, const ScreenShake_t &shake );

/// this is a convenience function for rapidly iterating on a screenshake. (see .cpp for details)
ScreenShake_t ASW_DefaultScreenShake( void );

/// based on the mapname, this reports if a map should show the briefing or not
bool UTIL_ASW_MissionHasBriefing(const char* mapname);

class CTraceFilterAliensEggsGoo : public CTraceFilterSimple
{
public:
	CTraceFilterAliensEggsGoo( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask );
};

bool ASW_IsSecurityCam(CPointCamera *pCameraEnt);

#ifdef CLIENT_DLL
class C_ASW_Player;
bool UTIL_ASW_CommanderLevelAtLeast( C_ASW_Player *pPlayer, int iLevel, int iPromotion = 0 );
#else
bool UTIL_ASW_CommanderLevelAtLeast( CASW_Player *pPlayer, int iLevel, int iPromotion = 0 );
#endif

bool UTIL_RD_LoadKeyValues( KeyValues *pKV, const char *resourceName, const CUtlBuffer &buf );
bool UTIL_RD_LoadKeyValuesFromFile( KeyValues *pKV, IFileSystem *pFileSystem, const char *szFileName, const char *szPath = NULL );
typedef void (*UTIL_RD_LoadAllKeyValuesCallback)( const char *pszPath, KeyValues *pKV, void *pUserData );
void UTIL_RD_LoadAllKeyValues( const char *fileName, const char *pPathID, const char *pKVName, UTIL_RD_LoadAllKeyValuesCallback callback, void *pUserData );

bool UTIL_RD_AddLocalizeFile( const char *fileName, const char *pPathID = NULL, bool bIncludeFallbackSearchPaths = false, bool bIsCaptions = false );

void UTIL_RD_ReloadLocalizeFiles();
CRC32_t UTIL_RD_CaptionToHash( const char *szToken );
const char *UTIL_RD_HashToCaption( CRC32_t hash );

const char *UTIL_RD_EResultToString( EResult eResult );

const wchar_t *UTIL_RD_CommaNumber( int64_t num );

int UTIL_RD_IndexToBit( unsigned bits, int n );
int UTIL_RD_BitToIndex( unsigned bits, int n );

int UTIL_RD_GetCurrentHoIAFSeason( int *pDaysRemaining = NULL, int *pHoursRemaining = NULL );

void CmdMsg( _Printf_format_string_ const char *pszFormat, ... );

#endif // _INCLUDE_ASW_UTIL_SHARED_H
