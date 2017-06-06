#ifndef logicentities_h__
#define logicentities_h__

#include "baseentity.h"

class CTimerEntity : public CLogicalEntity
{
public:
	DECLARE_CLASS( CTimerEntity, CLogicalEntity );

	void Spawn( void );
	void Think( void );

	void Toggle( void );
	void Enable( void );
	void Disable( void );
	void FireTimer( void );

	int DrawDebugTextOverlays(void);

	// outputs
	COutputEvent m_OnTimer;
	COutputEvent m_OnTimerHigh;
	COutputEvent m_OnTimerLow;

	// inputs
	void InputToggle( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputFireTimer( inputdata_t &inputdata );
	void InputRefireTime( inputdata_t &inputdata );
	void InputResetTimer( inputdata_t &inputdata );
	void InputAddToTimer( inputdata_t &inputdata );
	void InputSubtractFromTimer( inputdata_t &inputdata );

	int m_iDisabled;
	float m_flRefireTime;
	bool m_bUpDownState;
	int m_iUseRandomTime;
	float m_flLowerRandomBound;
	float m_flUpperRandomBound;

	// methods
	void ResetTimer( void );

	DECLARE_DATADESC();
};


#endif // logicentities_h__