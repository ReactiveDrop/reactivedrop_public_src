#include "cbase.h"
#include "mathlib/mathlib.h"
#include "util_shared.h"
//#include "model_types.h"
#include "convar.h"
#include "IEffects.h"
//#include "vphysics/object_hash.h"
//#include "IceKey.H"
//#include "checksum_crc.h"
#include "asw_fx_shared.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
	#include "c_asw_player.h"
	#include "asw_gamerules.h"
#else
	#include "te_effect_dispatch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void UTIL_ASW_EggGibs( const Vector &pos, int iFlags, int iEntIndex )
{
#ifdef GAME_DLL
	Vector vecOrigin = pos;
	CPASFilter filter( vecOrigin );
	UserMessageBegin( filter, "ASWEggEffects" );
	WRITE_FLOAT( vecOrigin.x );
	WRITE_FLOAT( vecOrigin.y );
	WRITE_FLOAT( vecOrigin.z );
	WRITE_SHORT( iFlags );
	WRITE_SHORT( iEntIndex );
	MessageEnd();
#endif
}

void UTIL_ASW_BuzzerDeath( const Vector &pos )
{
#ifdef GAME_DLL
	Vector vecExplosionPos = pos;
	CPASFilter filter( vecExplosionPos );
	UserMessageBegin( filter, "ASWBuzzerDeath" );
	WRITE_FLOAT( vecExplosionPos.x );
	WRITE_FLOAT( vecExplosionPos.y );
	WRITE_FLOAT( vecExplosionPos.z );
	MessageEnd();
#endif
}

void UTIL_ASW_DroneBleed( const Vector &pos, const Vector &dir, int amount )
{
	CEffectData	data;

	data.m_vOrigin = pos;
	data.m_vNormal = dir;
	//data.m_flScale = (float)amount;

	// todo: use filter?
	QAngle	vecAngles;
	VectorAngles( data.m_vNormal, vecAngles );
	DispatchParticleEffect( "drone_shot", data.m_vOrigin, vecAngles );

	//DispatchEffect( "DroneBleed", data );
}

void UTIL_ASW_BloodImpact( const Vector &pos, const Vector &dir, int color, int amount )
{
	CEffectData	data;

	data.m_vOrigin = pos;
	data.m_vNormal = dir;
	data.m_flScale = (float)amount;
	data.m_nColor = (unsigned char)color;

	DispatchEffect( "ASWBloodImpact", data );
}

void UTIL_ASW_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

//	if ( g_Language.GetInt() == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
//		color = 0;

	if ( amount > 255 )
		amount = 255;

	if (color == BLOOD_COLOR_MECH)
	{
		g_pEffects->Sparks(origin);
		if (random->RandomFloat(0, 2) >= 1)
		{
			UTIL_Smoke(origin, random->RandomInt(10, 15), 10);
		}
	}
	else
	{
		// Normal blood impact
		//UTIL_ASW_BloodImpact( origin, direction, color, amount );
		QAngle	vecAngles;
		VectorAngles( direction, vecAngles );
		if ( amount < 4 )
			DispatchParticleEffect( "marine_bloodsplat_light", origin, vecAngles );
		else
			DispatchParticleEffect( "marine_bloodsplat_heavy", origin, vecAngles );
	}
}

void UTIL_ASW_MarineTakeDamage( const Vector &origin, const Vector &direction, int color, int amount, CASW_Marine *pMarine, bool bFriendly )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

	if ( amount > 255 )
		amount = 255;

	// TODO: use amount to determine large versus small attacks taken?
	QAngle	vecAngles;
	VectorAngles( -direction, vecAngles );
	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

#ifdef CLIENT_DLL
	const char *pchEffectName = NULL;
	if ( bFriendly )
		pchEffectName = "marine_hit_blood_ff";
	else
		pchEffectName = "marine_hit_blood";

	CUtlReference< CNewParticleEffect > pEffect;
	pEffect = pMarine->ParticleProp()->Create( pchEffectName, PATTACH_CUSTOMORIGIN );

	if ( pEffect )
	{
		pMarine->ParticleProp()->AddControlPoint( pEffect, 2, pMarine, PATTACH_ABSORIGIN_FOLLOW );
		pEffect->SetControlPoint( 0, origin );//origin - pMarine->GetAbsOrigin()
		pEffect->SetControlPointOrientation( 0, vecForward, vecRight, vecUp );
	}
	else
	{
		Warning( "Could not create effect for marine hurt: %s", pchEffectName );
	}
#endif
}

void UTIL_ASW_GrenadeExplosion( const Vector &vecPos, float flRadius )
{
#ifdef GAME_DLL
	Vector vecExplosionPos = vecPos;
	CPASFilter filter( vecExplosionPos );
	UserMessageBegin( filter, "ASWGrenadeExplosion" );
	WRITE_FLOAT( vecExplosionPos.x );
	WRITE_FLOAT( vecExplosionPos.y );
	WRITE_FLOAT( vecExplosionPos.z );
	WRITE_FLOAT( flRadius );
	MessageEnd();
#endif
}

void UTIL_ASW_EnvExplosionFX( const Vector &vecPos, float flRadius, bool bOnGround )
{
#ifdef GAME_DLL
	Vector vecExplosionPos = vecPos;
	CPASFilter filter( vecExplosionPos );
	UserMessageBegin( filter, "ASWEnvExplosionFX" );
	WRITE_FLOAT( vecExplosionPos.x );
	WRITE_FLOAT( vecExplosionPos.y );
	WRITE_FLOAT( vecExplosionPos.z );
	WRITE_FLOAT( flRadius );
	WRITE_BOOL( bOnGround );
	//damage.bFriendlyFire = msg.ReadOneBit() ? true : false;
	MessageEnd();
#endif
}

#ifdef CLIENT_DLL
#define CPointToiletFlushable C_PointToiletFlushable
#endif

class CPointToiletFlushable : public CBaseEntity,
#ifdef GAME_DLL
	public IASW_Server_Usable_Entity
#else
	public IASW_Client_Usable_Entity
#endif
{
	DECLARE_CLASS( CPointToiletFlushable, CBaseEntity );
public:
	CPointToiletFlushable()
	{
		AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
#ifdef GAME_DLL
		m_iszFlushSound = MAKE_STRING( "d1_trainstation.toiletflush" );
		m_flCooldown = 10;
#endif
		m_flNextUse = 0;
	}

#ifdef GAME_DLL
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	void Precache() override
	{
		BaseClass::Precache();

		PrecacheScriptSound( STRING( m_iszFlushSound ) );
		PrecacheEffect( "watersplashquiet" );
	}
	void Spawn() override
	{
		Precache();

		BaseClass::Spawn();
	}
	int UpdateTransmitState() override
	{
		// we don't have a model, so we can't transmit based on PVS
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	CBaseEntity *GetEntity() override { return this; }
	bool IsUsable( CBaseEntity *pUser ) override { return m_flNextUse <= gpGlobals->curtime && pUser->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) < Square( 64 ); }
	bool RequirementsMet( CBaseEntity *pUser ) override { return true; }
	void ActivateUseIcon( CASW_Inhabitable_NPC *pUser, int nHoldType ) override
	{
		if ( nHoldType == ASW_USE_HOLD_START )
			return;

		if ( m_flNextUse > gpGlobals->curtime )
			return;

		m_flNextUse = gpGlobals->curtime + m_flCooldown;

		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flScale = 1;

		CBaseEntity *pHelpHelpImBeingSupressed = ( CBaseEntity * )te->GetSuppressHost();
		te->SetSuppressHost( NULL );

		DispatchEffect( "watersplashquiet", data );
		EmitSound( STRING( m_iszFlushSound ) );

		te->SetSuppressHost( pHelpHelpImBeingSupressed );

		m_OnPlayerUse.FireOutput( pUser, this );
	}
	void NPCStartedUsing( CASW_Inhabitable_NPC *pUser ) override {}
	void NPCStoppedUsing( CASW_Inhabitable_NPC *pUser ) override {}
	void NPCUsing( CASW_Inhabitable_NPC *pUser, float fDeltaTime ) override {}
	bool NeedsLOSCheck() override { return false; }
#else
	DECLARE_CLIENTCLASS();

	C_BaseEntity *GetEntity() override { return this; }
	bool IsUsable( C_BaseEntity *pUser ) override
	{
		if ( pUser->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) >= Square( 64 ) )
			return false;

		if ( m_flNextUse <= gpGlobals->curtime )
			return true;

		C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pUser );
		// sanity check
		if ( engine->IsPlayingDemo() || !pMarine || !pMarine->IsInhabited() || !pMarine->GetCommander() || !pMarine->GetCommander()->IsLocalPlayer() || !ASWGameRules() || ASWGameRules()->m_bCheated )
			return false;

		ISteamInventory *pSteamInventory = SteamInventory();
		if ( !pSteamInventory )
			return false;

		// on second thought we don't need sanity we've got this instead
		if ( pMarine->m_bOnFire && pMarine->IsInfested() )
		{
			SteamInventoryResult_t hResult;
			if ( pSteamInventory->AddPromoItem( &hResult, 28 ) )
			{
				pSteamInventory->DestroyResult( hResult );
			}
		}

		return false;
	}
	bool GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser ) override
	{
		action.wszText[0] = L'\0';
		action.bShowUseKey = false;
		action.UseTarget = this;

		return true;
	}
	void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) override {}
	bool ShouldPaintBoxAround() override { return false; }
	bool NeedsLOSCheck() override { return false; }
#endif

	CNetworkVar( float, m_flNextUse );
#ifdef GAME_DLL
	string_t m_iszFlushSound;
	float m_flCooldown;

	COutputEvent m_OnPlayerUse;
#endif
};

LINK_ENTITY_TO_CLASS( point_toilet_flushable, CPointToiletFlushable );
IMPLEMENT_NETWORKCLASS_ALIASED( PointToiletFlushable, DT_PointToiletFlushable );

#ifdef GAME_DLL
BEGIN_DATADESC( CPointToiletFlushable )
	DEFINE_KEYFIELD( m_iszFlushSound, FIELD_SOUNDNAME, "flushsound" ),
	DEFINE_KEYFIELD( m_flCooldown, FIELD_FLOAT, "cooldown" ),
	DEFINE_FIELD( m_flNextUse, FIELD_TIME ),
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
END_DATADESC()
#endif

BEGIN_NETWORK_TABLE( CPointToiletFlushable, DT_PointToiletFlushable )
#ifdef GAME_DLL
	SendPropTime( SENDINFO( m_flNextUse ) ),
#else
	RecvPropTime( RECVINFO( m_flNextUse ) ),
#endif
END_NETWORK_TABLE()


