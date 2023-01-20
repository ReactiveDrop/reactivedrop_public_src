//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Combine guard gun, strider destroyer
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "grenade_ar2.h"
#include "soundent.h"
#include "explode.h"
#include "shake.h"
#include "energy_wave.h"
#include "te_particlesystem.h"
#include "ndebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Concussive explosion entity

class CTEConcussiveExplosion : public CTEParticleSystem
{
public:
	DECLARE_CLASS( CTEConcussiveExplosion, CTEParticleSystem );
	DECLARE_SERVERCLASS();

	CTEConcussiveExplosion( const char *name );
	virtual	~CTEConcussiveExplosion( void );

	CNetworkVector( m_vecNormal );
	CNetworkVar( float, m_flScale );
	CNetworkVar( int, m_nRadius );
	CNetworkVar( int, m_nMagnitude );
};

IMPLEMENT_SERVERCLASS_ST( CTEConcussiveExplosion, DT_TEConcussiveExplosion )
	SendPropVector( SENDINFO(m_vecNormal), -1, SPROP_COORD ),
	SendPropFloat( SENDINFO(m_flScale), 0, SPROP_NOSCALE ),
	SendPropInt( SENDINFO(m_nRadius), 32, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nMagnitude), 32, SPROP_UNSIGNED ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTEConcussiveExplosion::CTEConcussiveExplosion( const char *name ) : BaseClass( name )
{
	m_nRadius		= 0;
	m_nMagnitude	= 0;
	m_flScale		= 0.0f;

	m_vecNormal.Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTEConcussiveExplosion::~CTEConcussiveExplosion( void )
{
}


// Singleton to fire TEExplosion objects
static CTEConcussiveExplosion g_TEConcussiveExplosion( "ConcussiveExplosion" );

void TE_ConcussiveExplosion( IRecipientFilter& filter, float delay,
	const Vector* pos, float scale, int radius, int magnitude, const Vector* normal )
{
	g_TEConcussiveExplosion.m_vecOrigin		= *pos;
	g_TEConcussiveExplosion.m_flScale			= scale;
	g_TEConcussiveExplosion.m_nRadius			= radius;
	g_TEConcussiveExplosion.m_nMagnitude		= magnitude;

	if ( normal )
		g_TEConcussiveExplosion.m_vecNormal	= *normal;
	else 
		g_TEConcussiveExplosion.m_vecNormal	= Vector(0,0,1);

	// Send it over the wire
	g_TEConcussiveExplosion.Create( filter, delay );
}

//Temp ent for the blast

class CConcussiveBlast : public CBaseEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CConcussiveBlast, CBaseEntity );

	int		m_spriteTexture;

	CConcussiveBlast( void ) {}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output :
	//-----------------------------------------------------------------------------
	void Precache( void )
	{
		m_spriteTexture = PrecacheModel( "sprites/lgtning.vmt" );

		BaseClass::Precache();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output :
	//-----------------------------------------------------------------------------

	void Explode( float magnitude )
	{
		//Create a concussive explosion
		CPASFilter filter( GetAbsOrigin() );

		Vector vecForward;
		AngleVectors( GetAbsAngles(), &vecForward );
		TE_ConcussiveExplosion( filter, 0.0,
			&GetAbsOrigin(),//position
			1.0f,	//scale
			256*magnitude,	//radius
			175*magnitude,	//magnitude
			&vecForward );	//normal
		
		int	colorRamp = random->RandomInt( 128, 255 );

		//Shockring
		CBroadcastRecipientFilter filter2;
		te->BeamRingPoint( filter2, 0, 
			GetAbsOrigin(),	//origin
			16,			//start radius
			300*magnitude,		//end radius
			m_spriteTexture, //texture
			0,			//halo index
			0,			//start frame
			2,			//framerate
			0.3f,		//life
			128,		//width
			16,			//spread
			0,			//amplitude
			colorRamp,	//r
			colorRamp,	//g
			255,		//g
			24,			//a
			128			//speed
			);

		//Do the radius damage
		RadiusDamage( CTakeDamageInfo( this, GetOwnerEntity(), 200, DMG_BLAST|DMG_DISSOLVE ), GetAbsOrigin(), 256, CLASS_NONE, NULL );

		UTIL_Remove( this );
	}
};

LINK_ENTITY_TO_CLASS( concussiveblast, CConcussiveBlast );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CConcussiveBlast )

//	DEFINE_FIELD( m_spriteTexture,	FIELD_INTEGER ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Create a concussive blast entity and detonate it
//-----------------------------------------------------------------------------
void CreateConcussiveBlast( const Vector &origin, const Vector &surfaceNormal, CBaseEntity *pOwner, float magnitude )
{
	QAngle angles;
	VectorAngles( surfaceNormal, angles );
	CConcussiveBlast *pBlast = (CConcussiveBlast *) CBaseEntity::Create( "concussiveblast", origin, angles, pOwner );

	if ( pBlast )
	{
		pBlast->Explode( magnitude );
	}
}
