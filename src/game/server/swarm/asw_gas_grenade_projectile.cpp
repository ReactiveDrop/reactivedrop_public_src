#include "cbase.h"
#include "asw_gas_grenade_projectile.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "IEffects.h"
#include "weapon_flaregun.h"
#include "decals.h"
#include "asw_shareddefs.h"
#include "particle_parse.h"
#include "asw_generic_emitter_entity.h"
#include "asw_radiation_volume.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar sk_plr_dmg_asw_gas_grenades;
extern ConVar sk_npc_dmg_asw_gas_grenades;

#define GAS_GRENADE_MODEL "models/swarm/grenades/HandGrenadeProjectile.mdl"

extern ConVar asw_gas_grenade_duration;
extern ConVar asw_gas_grenade_fuse;
extern ConVar asw_gas_grenade_damage;
extern ConVar asw_gas_grenade_damage_interval;
extern ConVar asw_gas_grenade_cloud_width;

LINK_ENTITY_TO_CLASS( asw_gas_grenade_projectile, CASW_Gas_Grenade_Projectile );

BEGIN_DATADESC( CASW_Gas_Grenade_Projectile )
	DEFINE_FUNCTION( Gas_GrenadeTouch ),
	DEFINE_FUNCTION( Gas_GrenadeBurnTouch ),
	DEFINE_FUNCTION( Gas_GrenadeThink ),

	// Fields
	DEFINE_FIELD( m_flDamage, FIELD_FLOAT ),
	DEFINE_FIELD( m_flDmgInterval, FIELD_FLOAT ),
	DEFINE_FIELD( m_flDmgDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFuse, FIELD_FLOAT ),
	DEFINE_FIELD( m_nBounces, FIELD_INTEGER ),
	DEFINE_FIELD( m_flTimeBurnOut, FIELD_TIME ),
	DEFINE_FIELD( m_flScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_flDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_bFading, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bSmoke, FIELD_BOOLEAN ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Gas_Grenade_Projectile, DT_ASW_Gas_Grenade_Projectile )
	SendPropFloat( SENDINFO( m_flTimeBurnOut ), 0,	SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flScale ), 0, SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_bSmoke ), 1, SPROP_UNSIGNED ),
END_SEND_TABLE()

CASW_Gas_Grenade_Projectile::CASW_Gas_Grenade_Projectile()
{
	m_flScale		= 2.0f;
	m_nBounces		= 0;
	m_bFading		= false;
	m_bSmoke		= true;
	m_flNextDamage	= gpGlobals->curtime;
	m_lifeState		= LIFE_ALIVE;
	m_iHealth		= 100;
	//m_pRadSound	= NULL;
}

CASW_Gas_Grenade_Projectile::~CASW_Gas_Grenade_Projectile( void )
{	
}

Class_T CASW_Gas_Grenade_Projectile::Classify( void )
{
	return Class_T(CLASS_ASW_GAS_GRENADE);
}

int CASW_Gas_Grenade_Projectile::Restore( IRestore &restore )
{
	int result = BaseClass::Restore( restore );

	return result;
}

void CASW_Gas_Grenade_Projectile::Spawn( void )
{
	Precache( );

	SetModel( GAS_GRENADE_MODEL );
	m_nSkin = 2;
	UTIL_SetSize( this, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ) );
	SetSolid( SOLID_BBOX );
	//AddSolidFlags( FSOLID_NOT_SOLID );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	m_takedamage	= DAMAGE_NO;

	SetFriction( 0.6f );

	AddEffects( EF_NOSHADOW|EF_NORECEIVESHADOW );

	AddFlag( FL_OBJECT );
	
	SetCollisionGroup( ASW_COLLISION_GROUP_IGNORE_NPCS );
	//CreateVPhysics();

	// Tumble in air
	QAngle vecAngVelocity( 0, random->RandomFloat ( -100, -500 ), 0 );
	SetLocalAngularVelocity( vecAngVelocity );

	SetTouch( &CASW_Gas_Grenade_Projectile::Gas_GrenadeTouch );

	//AddSolidFlags( FSOLID_NOT_STANDABLE );	
	
	//SetThink( &CASW_Gas_Grenade_Projectile::Gas_GrenadeThink );

	if ( m_flDmgDuration > 0 )
	{
		m_flTimeBurnOut = gpGlobals->curtime + m_flDmgDuration + m_flFuse;
	}
	else
	{
		m_flTimeBurnOut = -1.0f;
	}

	//SetNextThink( gpGlobals->curtime + 0.1f );

	SetFuseLength( asw_gas_grenade_fuse.GetFloat() );
}

unsigned int CASW_Gas_Grenade_Projectile::PhysicsSolidMaskForEntity( void ) const
{
	return (MASK_NPCSOLID & ~CONTENTS_MONSTERCLIP);
}

void CASW_Gas_Grenade_Projectile::Gas_GrenadeThink( void )
{
	float deltaTime = ( m_flTimeBurnOut - gpGlobals->curtime );

	if ( m_flTimeBurnOut != -1.0f )
	{
		//Fading away
		if ( ( deltaTime <= FLARE_DECAY_TIME ) && ( m_bFading == false ) )
		{
			m_bFading = true;
		}

		//Burned out
		if ( m_flTimeBurnOut < gpGlobals->curtime )
		{
			StopSound( "ASW_GasGrenade.Explode" );
			UTIL_Remove( this );
			return;
		}
	}
	
	//Act differently underwater
	if ( GetWaterLevel() > 1 )
	{
		UTIL_Bubbles( GetEffectOrigin() + Vector( -2, -2, -2 ), GetEffectOrigin() + Vector( 2, 2, 2 ), 1 );
		m_bSmoke = false;
	}

	//Next update
	SetNextThink( gpGlobals->curtime + 0.1f );
}

const Vector& CASW_Gas_Grenade_Projectile::GetEffectOrigin()
{
	static Vector s_vecEffectPos;
	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);
	s_vecEffectPos = GetAbsOrigin() + up * 5;
	return s_vecEffectPos;
}

void CASW_Gas_Grenade_Projectile::Precache( void )
{
	PrecacheModel( GAS_GRENADE_MODEL );
	PrecacheParticleSystem( "grenade_gas_cloud" );

	PrecacheScriptSound( "ASW_GasGrenade.Explode" );
	PrecacheScriptSound( "ASW_Flare.Touch" );

	BaseClass::Precache();
}



CASW_Gas_Grenade_Projectile* CASW_Gas_Grenade_Projectile::Gas_Grenade_Projectile_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float flDamage, float flDmgInterval, float flDmgDuration, float flFuse )
{
	CASW_Gas_Grenade_Projectile *pGas_Grenade = (CASW_Gas_Grenade_Projectile *)CreateEntityByName( "asw_gas_grenade_projectile" );
	pGas_Grenade->SetAbsAngles( angles );
	pGas_Grenade->m_flDamage = flDamage;
	pGas_Grenade->m_flDmgInterval = flDmgInterval;
	pGas_Grenade->m_flDmgDuration = flDmgDuration;
	pGas_Grenade->m_flFuse = flFuse;
	pGas_Grenade->Spawn();
	pGas_Grenade->SetOwnerEntity( pOwner );
	//Msg("making pGas_Grenade with velocity %f,%f,%f\n", velocity.x, velocity.y, velocity.z);
	UTIL_SetOrigin( pGas_Grenade, position );
	pGas_Grenade->SetAbsVelocity( velocity );

	return pGas_Grenade;
}

void CASW_Gas_Grenade_Projectile::Gas_GrenadeTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() )
		return;

	return;

	//If the flare hit a person or NPC, do damage here.
	if ( pOther && pOther->m_takedamage )
	{
		/*
			The Gas_Grenade is the iRifle round right now. No damage, just ignite. (sjb)

		//Damage is a function of how fast the flare is flying.
		int iDamage = GetAbsVelocity().Length() / 50.0f;

		if ( iDamage < 5 )
		{
			//Clamp minimum damage
			iDamage = 5;
		}

		//Use m_pOwner, not GetOwnerEntity()
		pOther->TakeDamage( CTakeDamageInfo( this, m_pOwner, iDamage, (DMG_BULLET|DMG_BURN) ) );
		m_flNextDamage = gpGlobals->curtime + 1.0f;
		*/

		//CBaseAnimating *pAnim;

		//pAnim = dynamic_cast<CBaseAnimating*>(pOther);
		//if( pAnim )
		//{
			//pAnim->Ignite( 30.0f );
		//}

		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity	*= 0.1f;
		SetAbsVelocity( vecNewVelocity );

		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
		SetGravity(1.0f);

		return;
	}
	else
	{
		// hit the world, check the material type here, see if the flare should stick.
		trace_t tr;
		tr = CBaseEntity::GetTouchTrace();

		//Only do this on the first bounce
		if ( m_nBounces == 0 )
		{
			surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );	

			if ( pdata != NULL )
			{
				//Only embed into concrete and wood (jdw: too obscure for players?)
				//if ( ( pdata->gameMaterial == 'C' ) || ( pdata->gameMaterial == 'W' ) )
				{
					Vector	impactDir = ( tr.endpos - tr.startpos );
					VectorNormalize( impactDir );

					float	surfDot = tr.plane.normal.Dot( impactDir );

					//Do not stick to ceilings or on shallow impacts
					if ( ( tr.plane.normal.z > 0.3f ) && ( surfDot < -0.9f ) )
					{
						RemoveSolidFlags( FSOLID_NOT_SOLID );
						AddSolidFlags( FSOLID_TRIGGER );
						LayFlat();
						UTIL_SetOrigin( this, tr.endpos + ( tr.plane.normal * 2.0f ) );
						SetAbsVelocity( vec3_origin );
						SetMoveType( MOVETYPE_NONE );
						
						SetTouch( &CASW_Gas_Grenade_Projectile::Gas_GrenadeBurnTouch );
						
						//int index = decalsystem->GetDecalIndexForName( "SmallScorch" );
						//if ( index >= 0 )
						//{
							//CBroadcastRecipientFilter filter;
							//te->Decal( filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX( tr.m_pEnt ), tr.hitbox, index );
						//}
						
						CPASAttenuationFilter filter2( this, "ASW_Flare.Touch" );
						EmitSound( filter2, entindex(), "ASW_Flare.Touch" );

						return;
					}
				}
			}
		}

		//Scorch decal
		//if ( GetAbsVelocity().LengthSqr() > (250*250) )
		//{
			//int index = decalsystem->GetDecalIndexForName( "FadingScorch" );
			//if ( index >= 0 )
			//{
				//CBroadcastRecipientFilter filter;
				//te->Decal( filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX( tr.m_pEnt ), tr.hitbox, index );
			//}
		//}

		// Change our flight characteristics
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
		SetGravity( UTIL_ScaleForGravity( 640 ) );
		
		m_nBounces++;

		//After the first bounce, smacking into whoever fired the flare is fair game
		SetOwnerEntity( NULL );	

		// Slow down
		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity.x *= 0.8f;
		vecNewVelocity.y *= 0.8f;
		SetAbsVelocity( vecNewVelocity );

		//Stopped?
		if ( GetAbsVelocity().Length() < 64.0f )
		{
			LayFlat();
			SetAbsVelocity( vec3_origin );
			SetMoveType( MOVETYPE_NONE );
			RemoveSolidFlags( FSOLID_NOT_SOLID );
			AddSolidFlags( FSOLID_TRIGGER );
			SetTouch( &CASW_Gas_Grenade_Projectile::Gas_GrenadeBurnTouch );
		}
	}
}

void CASW_Gas_Grenade_Projectile::LayFlat()
{
	return;
	QAngle angFacing = GetAbsAngles();
	if (angFacing[PITCH] > 0 && angFacing[PITCH] < 180.0f)
		angFacing[PITCH] = 90;
	else
		angFacing[PITCH] = 270;
	SetAbsAngles(angFacing);
	//Msg("Laying flat to %f, %f, %f\n", angFacing[PITCH], angFacing[YAW], angFacing[ROLL]);
}

void CASW_Gas_Grenade_Projectile::SetFuseLength( float fSeconds )
{
	SetThink( &CASW_Gas_Grenade_Projectile::Detonate );
	SetNextThink( gpGlobals->curtime + fSeconds );
}

void CASW_Gas_Grenade_Projectile::Detonate()
{
	// Reset our angles so the particle effect renders right-side up.
	SetAbsAngles( QAngle( 0, GetAbsAngles().y, 0 ) );

	// also spawn our big cloud marking out the area of radiation
	DispatchParticleEffect( "grenade_gas_cloud", WorldSpaceCenter(), QAngle( 0, 0, 0 ), PATTACH_CUSTOMORIGIN_FOLLOW, this );
	EmitSound( "ASW_GasGrenade.Explode" );

	//StartRadLoopSound();

	// create a volume to HURT people
	m_hRadVolume = ( CASW_Radiation_Volume* ) CreateEntityByName( "asw_radiation_volume" );
	if ( m_hRadVolume )
	{
		m_hRadVolume->SetParent( this );
		m_hRadVolume->SetLocalOrigin( vec3_origin );
		m_hRadVolume->m_hCreator = GetOwnerEntity();
		m_hRadVolume->m_flDamage = m_flDamage;
		m_hRadVolume->m_flDmgInterval = m_flDmgInterval;
		m_hRadVolume->m_flBoxWidth = asw_gas_grenade_cloud_width.GetFloat();
		m_hRadVolume->m_hWeapon = this;
		m_hRadVolume->Spawn();
	}

	// send a user message marking this entity as scanner noisy (makes the player's minimap distort)	
	CRecipientFilter filter;
	filter.AddAllPlayers();
	UserMessageBegin( filter, "ASWScannerNoiseEnt" );
		WRITE_SHORT( entindex() );
		WRITE_FLOAT( 150.0f );
		WRITE_FLOAT( 400.0f );
	MessageEnd();

	SetThink( &CASW_Gas_Grenade_Projectile::Gas_GrenadeThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CASW_Gas_Grenade_Projectile::Gas_GrenadeBurnTouch( CBaseEntity *pOther )
{
	// asw no burning flares for now
	//if ( pOther && pOther->m_takedamage && ( m_flNextDamage < gpGlobals->curtime ) )
	//{
		//pOther->TakeDamage( CTakeDamageInfo( this, m_pFirer, 1, (DMG_BULLET|DMG_BURN) ) );
		//m_flNextDamage = gpGlobals->curtime + 1.0f;
	//}
}

extern ConVar asw_flare_autoaim_radius;
void CASW_Gas_Grenade_Projectile::DrawDebugGeometryOverlays()
{
	// draw arrows showing the extent of our autoaim
	for (int i=0;i<360;i+=45)
	{
		float flBaseSize = 10;
		float flHeight = asw_flare_autoaim_radius.GetFloat();

		Vector vBasePos = GetAbsOrigin() + Vector( 0, 0, 5 );
		QAngle angles( 0, 0, 0 );
		Vector vForward, vRight, vUp;
		angles[YAW] = i;
		AngleVectors( angles, &vForward, &vRight, &vUp );
		NDebugOverlay::Triangle( vBasePos+vRight*flBaseSize/2, vBasePos-vRight*flBaseSize/2, vBasePos+vForward*flHeight, 0, 255, 0, 255, false, 10 );		
	}

	BaseClass::DrawDebugGeometryOverlays();
}

void CASW_Gas_Grenade_Projectile::StopLoopingSounds()
{
	if ( m_pRadSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pRadSound );
		m_pRadSound = NULL;
	}
	BaseClass::StopLoopingSounds();
}

void CASW_Gas_Grenade_Projectile::StartRadLoopSound()
{
	if ( !m_pRadSound )
	{
		const char *pszSound = "Misc.Geiger";
		float fRadPitch = random->RandomInt( 95, 105 );

		CPASAttenuationFilter filter( this );
		m_pRadSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), CHAN_STATIC, pszSound, ATTN_NORM );

		CSoundEnvelopeController::GetController().Play( m_pRadSound, 1.0, fRadPitch );
	}
}
