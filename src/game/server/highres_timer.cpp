#include "cbase.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib")

static UINT wTimerRes = 0;
static TIMECAPS tc;

void winmm_timer_acquire_once ()
{
	// already acquired
	if ( wTimerRes ) return;

	// fetch desired timer resolution
	const ConVarRef rd_high_resolution_timer_ms ( "rd_high_resolution_timer_ms" );

	uint resolution = 1;
	if ( rd_high_resolution_timer_ms.IsValid () && rd_high_resolution_timer_ms.GetInt () > 0 )
	{
		resolution = rd_high_resolution_timer_ms.GetInt ();
	}

	if ( resolution > 0 )
	{
		// getcaps might fail, but it's ok to continue if it does
		timeGetDevCaps ( &tc, sizeof ( TIMECAPS ) );

		wTimerRes = min ( max ( tc.wPeriodMin, resolution ), tc.wPeriodMax );
		timeBeginPeriod ( wTimerRes );
	}
}
