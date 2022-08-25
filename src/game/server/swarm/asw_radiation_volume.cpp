#include "cbase.h"
#include "asw_radiation_volume.h"
#include "asw_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_radiation_volume, CASW_Radiation_Volume );

#define RAD_DAMAGE_INTERVAL 1.0f
#define ASW_RAD_DAMAGE 20

BEGIN_DATADESC( CASW_Radiation_Volume )
	DEFINE_FUNCTION(RadTouch),
	DEFINE_THINKFUNC(RadThink),
	DEFINE_FIELD(m_hCreator, FIELD_EHANDLE),
	DEFINE_FIELD(m_hWeapon, FIELD_EHANDLE),
	DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
	DEFINE_FIELD(m_flDmgInterval, FIELD_FLOAT),
	DEFINE_FIELD(m_flBoxWidth, FIELD_FLOAT),
END_DATADESC()

CASW_Radiation_Volume::CASW_Radiation_Volume()
{
	m_flDamage = ASW_RAD_DAMAGE;
	m_flDmgInterval = RAD_DAMAGE_INTERVAL;
	m_flBoxWidth = 100.0f;
}

void CASW_Radiation_Volume::Spawn( void )
{
	BaseClass::Spawn();

	// make us invisible, a cube, non solid, but still firing touch triggers
	AddEffects(EF_NODRAW);
	SetSolid( SOLID_BBOX );
	float boxWidth = m_flBoxWidth;
	UTIL_SetSize(this, Vector(-boxWidth,-boxWidth,0),Vector(boxWidth,boxWidth,boxWidth * 2));
	SetCollisionGroup(ASW_COLLISION_GROUP_PASSABLE);	
	AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	SetTouch( &CASW_Radiation_Volume::RadTouch );

	SetThink( &CASW_Radiation_Volume::RadThink );
	SetNextThink( TICK_NEVER_THINK );
}

bool CASW_Radiation_Volume::IsValidRadTarget( CBaseEntity *pOther )
{
	if (!pOther)
		return false;

	return pOther->IsNPC();
}

void CASW_Radiation_Volume::RadTouch( CBaseEntity *pOther )
{
	// if other is a valid entity to radiate, add it to our list
	if (IsValidRadTarget(pOther) && m_hRadTouching.Find(pOther) == m_hRadTouching.InvalidIndex())
	{
		m_hRadTouching.AddToTail(pOther);
		if (GetNextThink() == TICK_NEVER_THINK)
			SetNextThink( gpGlobals->curtime );
	}
}

bool CASW_Radiation_Volume::RadTouching(CBaseEntity *pEnt)
{
	if (!pEnt || !pEnt->CollisionProp() || !CollisionProp())
		return false;

	Vector vecNearest;
	pEnt->CollisionProp()->CalcNearestPoint( GetAbsOrigin(), &vecNearest );
	return CollisionProp()->IsPointInBounds(vecNearest);
}

void CASW_Radiation_Volume::RadHurt(CBaseEntity *pEnt)
{
	if (!pEnt)
		return;

	int iDamageType = DMG_RADIATION;

	CBaseEntity *pAttacker = this;
	if (m_hCreator.Get() && pEnt->Classify() != CLASS_ASW_MARINE)	// don't deal friendly fire damage from rad barrels
		pAttacker = m_hCreator.Get();

	CBaseEntity *pWeapon = NULL;
	if (m_hWeapon.Get())
	{
		pWeapon = m_hWeapon.Get();
		if ( pWeapon->Classify() == CLASS_ASW_GAS_GRENADE )
		{
			pAttacker = m_hCreator.Get();
			iDamageType = DMG_NERVEGAS;
		}
	}

	float fDamage = m_flDamage;
	if (pEnt->Classify() == CLASS_ASW_MARINE)
		fDamage *= 0.5f;
	pEnt->TakeDamage( CTakeDamageInfo( this, pAttacker, pWeapon, vec3_origin, WorldSpaceCenter(), fDamage, iDamageType ) );
}

void CASW_Radiation_Volume::RadThink()
{
	// remove people who aren't in our volume
	for (int i = 0; i < m_hRadTouching.Count();)
	{
		if (!m_hRadTouching[i] || m_hRadTouching[i]->GetHealth()<=0
				|| !RadTouching(m_hRadTouching[i]))
		{
			m_hRadTouching.Remove( i );
		}
		else
		{
			i++;
		}
	}
	// go through our list, hurting people
	for (int i = 0; i < m_hRadTouching.Count(); i++)
	{
		RadHurt(m_hRadTouching[i]);
	}

	if (m_hRadTouching.Count() > 0)
	{
		SetNextThink( gpGlobals->curtime + m_flDmgInterval );
	}
}