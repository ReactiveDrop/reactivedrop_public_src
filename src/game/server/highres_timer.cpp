#include "cbase.h"
#include <windows.h>

#pragma comment(lib, "winmm.lib")

void winmm_timer_acquire_once( unsigned int ms )
{
	static UINT wTimerRes = 0;
	static TIMECAPS tc;

	// already acquired
	if ( wTimerRes )
		return;

	// getcaps might fail, but it's ok to continue if it does
	timeGetDevCaps( &tc, sizeof( TIMECAPS ) );

	wTimerRes = min( max( tc.wPeriodMin, ms ), tc.wPeriodMax );
	timeBeginPeriod( wTimerRes );
}
