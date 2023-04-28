#include "cbase.h"
#include <windows.h>

#pragma comment(lib, "ntdll.lib")
extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution( ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution );

static float prev = 0.0f;

void highres_timer_set( float ms )
{
	if ( ms != prev ) {

		DevMsg( "[timer] requested: %f ms\n", ms );

		ULONG currentRes;
		ULONG desiredRes = 15.0f; // 15 ms os default
		bool apply = true;

		// if zero or negative, restore default os timer resolution
		if ( ms > 0.00001f ) {
			desiredRes = round( ms * 1000 * 100 );
			DevMsg( "[timer] resolution requested: %d\n", desiredRes );
		}
		else
		{
			DevMsg( "[timer] restoring default os settings.\n" );
			apply = false; // ignore desiredRes and reset to os default
		}

		// specifies the amount of time that should elapse between each timer interrupt, in 100-nanosecond units
		NtSetTimerResolution( desiredRes, apply, &currentRes );
	}

	prev = ms;
}
