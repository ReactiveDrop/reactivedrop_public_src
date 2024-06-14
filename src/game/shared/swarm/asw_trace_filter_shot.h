#ifndef _INCLUDED_ASW_TRACE_FILTER_SHOT
#define _INCLUDED_ASW_TRACE_FILTER_SHOT
#pragma once

class CASWTraceFilterShot : public CTraceFilterSimpleList
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS( CASWTraceFilterShot, CTraceFilterSimpleList );

	CASWTraceFilterShot( IHandleEntity *passentity, IHandleEntity *passentity2, int collisionGroup, Vector shotNormal );
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );

	virtual void SetSkipMarines( bool bSkip ) { m_bSkipMarines = bSkip; }
	virtual void SetSkipRollingMarines( bool bSkip ) { m_bSkipRollingMarines = bSkip; }
	virtual void SetSkipMarinesReflectingProjectiles( bool bSkip ) { m_bSkipMarinesReflectingProjectiles = bSkip; }
	virtual void SetSkipAliens( bool bSkip ) { m_bSkipAliens = bSkip; }

private:
	bool m_bSkipMarines;
	bool m_bSkipRollingMarines;
	bool m_bSkipMarinesReflectingProjectiles;
	bool m_bSkipAliens;
	Vector m_vecShotNormal;
};

#endif // _INCLUDED_ASW_TRACE_FILTER_SHOT
