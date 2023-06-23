#include "cbase.h"
#include "asw_gamerules.h"
#include "c_asw_player.h"
#include "asw_util_shared.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static HSCRIPT Script_GetLocalPlayer()
{
	C_ASW_Player *pLocal = C_ASW_Player::GetLocalASWPlayer();
	Assert( pLocal );

	return ToHScript( pLocal );
}

static float Script_XRES( float x )
{
	return XRES( x );
}

static float Script_YRES( float y )
{
	return YRES( y );
}

static ScriptVariant_t Script_TryLocalize( const char *szToken )
{
	wchar_t wsz[2048];

	TryLocalize( szToken, wsz, sizeof( wsz ) );

	char sz[4096];

	V_UnicodeToUTF8( wsz, sz, sizeof( sz ) );

	return ScriptVariant_t( sz, true );
}

void C_AlienSwarm::RegisterScriptFunctions()
{
	if ( !g_pScriptVM ) return;

	ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetLocalPlayer, "GetLocalPlayer", "Returns the current NPC being played or spectated" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_XRES, "XRes", "Returns the X position relative to a 640 pixel wide screen" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_YRES, "YRes", "Returns the Y position relative to a 480 pixel tall screen" );
	ScriptRegisterFunction( g_pScriptVM, ScreenWidth, "Returns the game screen width in pixels" );
	ScriptRegisterFunction( g_pScriptVM, ScreenHeight, "Returns the game screen height in pixels" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_TryLocalize, "TryLocalize", "Returns the translated text for a translation string" );
}
