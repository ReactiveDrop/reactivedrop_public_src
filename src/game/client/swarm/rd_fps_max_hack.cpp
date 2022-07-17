#include "cbase.h"
#ifdef WIN32
#undef INVALID_HANDLE_VALUE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Change the default value for fps_max to be the current monitor's refresh rate at startup.
//
// The default value of 100 is very uncommon - most monitors are 60 Hz, and higher refresh rates
// tend to be 120/144/240, so in the base case we are either wasting 40 frames or artificially
// limiting our best-case refresh speed.
//
// There's no way to change a ConVar's default value through the API, so we do some memory hacking,
// with assertions in debug mode.
//
// This code runs before rd_override_fps_max and before config files are executed, so users who have
// already modified fps_max before this hack's introduction are unaffected.
static class CRD_FPS_Max_Hack final : public CAutoGameSystem
{
public:
	CRD_FPS_Max_Hack() : CAutoGameSystem( "CRD_FPS_Max_Hack" )
	{
	}

	virtual bool Init() override
	{
		ConVar *fps_max = g_pCVar->FindVar( "fps_max" );
		Assert( fps_max );
		if ( !fps_max )
			return true;

		int iRefreshRate;

#ifdef WIN32
		DEVMODE devMode{};
		bool bSuccess = EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &devMode );
		Assert( bSuccess );
		if ( !bSuccess )
			return true;

		iRefreshRate = devMode.dmDisplayFrequency;
#else
#error fps_max hack needs to be ported
#endif

		static_assert( sizeof( ConCommandBase ) == 24, "unexpected ConCommandBase size" );
		static_assert( sizeof( ConVar ) == 88, "unexpected ConVar size" );

		// Increase max by 1 to avoid limiting 60 fps monitors to 59 fps due to rounding error.
		iRefreshRate++;

		// Convert refresh rate to string, which will become the new default value.
		static char szRefreshRate[30]{};
		V_snprintf( szRefreshRate, sizeof( szRefreshRate ), "%d", iRefreshRate );

		// ConVar *ConVar::m_pParent
		ConVar *fps_max_parent = reinterpret_cast<ConVar **>( fps_max )[7];
		Assert( fps_max_parent == fps_max );

		// const char *ConVar::m_pszDefaultValue
		const char **fps_max_default = &reinterpret_cast<const char **>( fps_max_parent )[8];
		Assert( fps_max_parent->GetDefault() == *fps_max_default );

		// Set new default and new value.
		*fps_max_default = szRefreshRate;
		fps_max->SetValue( iRefreshRate );

		return true;
	}
} s_RD_FPS_Max_Hack;
