#include "cbase.h"
#include <windows.h>

#pragma comment(lib, "ntdll.lib")
extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution( ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution );

void winmm_timer_acquire_once( float ms )
{
	static ULONG newRes = NULL;

	if ( !newRes ) 
	{
		ULONG requestRes = ms * 1000;
		ULONG currentRes;

		newRes = NtSetTimerResolution( round(ms * 1000), TRUE, &currentRes );
		DevMsg( "timer resolution changed from %d to %d ns\n", currentRes, newRes );
	}
}
