#include "cbase.h"
#include "asw_burning.h"
#include "asw_marine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_BurnInfo::CASW_BurnInfo(CBaseEntity *pEntity, CBaseEntity *pAttacker, float fDieTime, float fBurnInterval, float fDamagePerInterval, CBaseEntity *pDamagingWeapon ) :	
	m_fDieTime(fDieTime),
	m_fBurnInterval(fBurnInterval),
	m_fDamagePerInterval(fDamagePerInterval)
{
	m_fNextBurnTime = gpGlobals->curtime + fBurnInterval;
	m_hEntity = pEntity;
	m_hAttacker = pAttacker;
	m_hDamagingWeapon = pDamagingWeapon;
}

LINK_ENTITY_TO_CLASS( asw_burning, CASW_Burning );

BEGIN_DATADESC( CASW_Burning )
	DEFINE_THINKFUNC( FireThink ),
END_DATADESC()

CASW_Burning* g_pASW_Burning = NULL;
CASW_Burning* ASWBurning() { return g_pASW_Burning; }

CASW_Burning::CASW_Burning()
{
	Assert(!g_pASW_Burning);
	g_pASW_Burning = this;
	Reset();
}

CASW_Burning::~CASW_Burning()
{
	g_pASW_Burning = NULL;
}

void CASW_Burning::Reset()
{
	m_Burning.PurgeAndDeleteElements();
}

void CASW_Burning::Spawn()
{
	BaseClass::Spawn();
	Precache();
	SetThink(&CASW_Burning::FireThink);
	SetNextThink(gpGlobals->curtime + 0.01f);
}

void CASW_Burning::Precache()
{
	PrecacheScriptSound("ASWFire.BurningFlesh");
	PrecacheScriptSound("ASWFire.BurningObject");
	PrecacheScriptSound("ASWFire.StopBurning");
}

void CASW_Burning::FireThink()
{
	int c = m_Burning.Count();
	CBaseEntity* pEnt;
	for ( int i = c - 1; i >= 0; i-- )	// go backwards in case we remove one
	{
		pEnt = m_Burning[i]->m_hEntity.Get();
		if ( m_Burning[i]->m_fDieTime <= gpGlobals->curtime || !pEnt )
		{
			delete m_Burning[i];
			m_Burning.Remove( i );
			CBaseAnimating *pAnim = pEnt ? pEnt->GetBaseAnimating() : NULL;
			if ( pAnim )
			{
				//OnEntityExtinguished(pEnt);
				if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pAnim ) )
					pMarine->Extinguish( NULL, NULL );
				else
					pAnim->Extinguish();	// this makes the entity remove its FL_ONFIRE flag, its m_bOnFire bool and then calls back to us to check its not in the list anymore
			}
		}
		else
		{
			if ( m_Burning[i]->m_fNextBurnTime <= gpGlobals->curtime + 0.01f )
			{
				// hurt the entity!
				CBaseEntity *pAttacker = m_Burning[i]->m_hAttacker.Get();
				if ( !pAttacker )
					pAttacker = this;
				CTakeDamageInfo info( this, pAttacker, m_Burning[i]->m_hDamagingWeapon.Get(), vec3_origin, pEnt->WorldSpaceCenter(), m_Burning[i]->m_fDamagePerInterval, DMG_BURN | DMG_DIRECT );
				pEnt->TakeDamage( info );

				m_Burning[i]->m_fNextBurnTime += m_Burning[i]->m_fBurnInterval;
			}
		}
	}

	SetNextThinkTime();
}


void CASW_Burning::BurnEntity( CBaseEntity *pEntity, CBaseEntity *pAttacker, float fFireDuration, float fBurnInterval, float fDamagePerInterval, CBaseEntity *pDamagingWeapon /*= NULL */ )
{
	if (!pEntity)
		return;

	int c = m_Burning.Count();
	for (int i=0;i<c;i++)
	{
		if (m_Burning[i]->m_hEntity.Get() == pEntity)	// already burning this ent
		{
			m_Burning[i]->m_fDieTime = MAX(m_Burning[i]->m_fDieTime, gpGlobals->curtime + fFireDuration);
			return;
		}
	}

	//Msg("BurnEntity: %d:%s for %f seconds, interval %f damageper %f\n", pEntity->entindex(), pEntity->GetClassName(),
		//fFireDuration, fBurnInterval, fDamagePerInterval);

	// add a new one
	CASW_BurnInfo *pBurn = new CASW_BurnInfo(pEntity, pAttacker, gpGlobals->curtime + fFireDuration, fBurnInterval, fDamagePerInterval, pDamagingWeapon );
	if (pBurn)
	{
		m_Burning.AddToTail(pBurn);
		SetNextThinkTime();
	}
}

void CASW_Burning::SetNextThinkTime()
{
	int next = -1;
	float next_time = 0;
	int c = m_Burning.Count();
	for (int i=0;i<c;i++)
	{
		if (next_time == 0 || m_Burning[i]->m_fNextBurnTime < next_time)
		{
			next_time = m_Burning[i]->m_fNextBurnTime;
			next = i;
		}
	}

	if (next != -1)
	{
		SetThink(&CASW_Burning::FireThink);
		SetNextThink(next_time);
		return;
	}
	SetThink(&CASW_Burning::FireThink);
	SetNextThink(gpGlobals->curtime + 5.0f);
}

void CASW_Burning::Extinguish(CBaseEntity *pEntity)
{
	if (!pEntity)
		return;

	int c = m_Burning.Count();
	for (int i=c-1;i>=0;i--)
	{
		if (m_Burning[i]->m_hEntity.Get() == pEntity)
		{
			delete m_Burning[i];
			m_Burning.Remove(i);
			//OnEntityExtinguished(pEntity);
		}
	}
}

//reactivedrop 
void CASW_Burning::ExtendBurning(CBaseEntity *pEntity, float fFireDuration)
{
	if (!pEntity)
		return;

	int c = m_Burning.Count();
	for (int i = c - 1; i >= 0; i--)
	{
		if (m_Burning[i]->m_hEntity.Get() == pEntity)
		{
			m_Burning[i]->m_fDieTime = gpGlobals->curtime + fFireDuration;
		}
	}
}
