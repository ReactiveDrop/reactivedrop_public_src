#include "cbase.h"
#include "winlite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRD_ConVar_PrintDescription_Hack : public CAutoGameSystem
{
public:
	CRD_ConVar_PrintDescription_Hack() : CAutoGameSystem( "CRD_ConVar_PrintDescription_Hack" )
	{
	}

	void PostInit() override
	{
#ifdef GAME_DLL
		if ( !engine->IsDedicatedServer() )
			return;
#endif

		ConCommand *pHelpCommand = g_pCVar->FindCommand( "help" );
		Assert( pHelpCommand );

		const byte *pHelpThunk = *reinterpret_cast< const byte *const * >( reinterpret_cast< const byte * >( pHelpCommand ) + sizeof( ConCommandBase ) );
		// The thunk we're looking at is very simple (as most thunks are):
		// 4 bytes: copy command args pointer from stack to register
		Assert( pHelpThunk[0] == 0x8B );
		Assert( pHelpThunk[1] == 0x44 );
		Assert( pHelpThunk[2] == 0x24 );
		Assert( pHelpThunk[3] == 0x04 );
		// 6 bytes: initialize "this" pointer
		Assert( pHelpThunk[4] == 0x8B );
		Assert( pHelpThunk[5] == 0x0D );
		// 1 byte: push command args pointer to stack
		Assert( pHelpThunk[10] == 0x50 );
		// 5 bytes: call the next function we need
		Assert( pHelpThunk[11] == 0xE8 );
		// 1 byte: return
		Assert( pHelpThunk[16] == 0xC3 );

		const byte *pHelpFunction = reinterpret_cast< const byte * >( uintptr_t( pHelpThunk ) + 16 + *reinterpret_cast< const uintptr_t * >( pHelpThunk + 12 ) );
		// Now we're in the actual implementation of the help console command.
		// We only care about one thing here: the function call at the very end.
		// But just for completeness, let's document this whole function as well as it's relatively simple.
		// 4 bytes: copy args pointer from stack to register
		Assert( pHelpFunction[0] == 0x8B );
		Assert( pHelpFunction[1] == 0x44 );
		Assert( pHelpFunction[2] == 0x24 );
		Assert( pHelpFunction[3] == 0x04 );
		// 3 bytes: check to make sure we have exactly two args ("help" and the cvar name)
		Assert( pHelpFunction[4] == 0x83 );
		Assert( pHelpFunction[5] == 0x38 );
		Assert( pHelpFunction[6] == 0x02 );
		// 2 bytes: jump forward if we have the correct number of args
		Assert( pHelpFunction[7] == 0x74 );
		Assert( pHelpFunction[8] == 0x11 );
		// 5 bytes: push "Usage:  help <cvarname>\n" to the stack
		Assert( pHelpFunction[9] == 0x68 );
		// 6 bytes: call the ConMsg function
		Assert( pHelpFunction[14] == 0xFF );
		Assert( pHelpFunction[15] == 0x15 );
		// 3 bytes: pop the stack
		Assert( pHelpFunction[20] == 0x83 );
		Assert( pHelpFunction[21] == 0xC4 );
		Assert( pHelpFunction[22] == 0x04 );
		// 3 bytes: return
		Assert( pHelpFunction[23] == 0xC2 );
		Assert( pHelpFunction[24] == 0x04 );
		Assert( pHelpFunction[25] == 0x00 );
		// here's where we jumped forward to earlier:
		// 6 bytes: initialize "this" pointer (g_pCVar)
		Assert( pHelpFunction[26] == 0x8B );
		Assert( pHelpFunction[27] == 0x0D );
		// 1 byte: save ESI register
		Assert( pHelpFunction[32] == 0x56 );
		// 6 bytes: put cvar name in ESI register
		Assert( pHelpFunction[33] == 0x8B );
		Assert( pHelpFunction[34] == 0xB0 );
		Assert( pHelpFunction[35] == 0x0C );
		Assert( pHelpFunction[36] == 0x04 );
		Assert( pHelpFunction[37] == 0x00 );
		Assert( pHelpFunction[38] == 0x00 );
		// 2 bytes: get g_pCVar virtual function table
		Assert( pHelpFunction[39] == 0x8B );
		Assert( pHelpFunction[40] == 0x01 );
		// 3 bytes: get function pointer for g_pCVar->FindCommandBase
		Assert( pHelpFunction[41] == 0x8B );
		Assert( pHelpFunction[42] == 0x50 );
		Assert( pHelpFunction[43] == 0x38 );
		// 1 byte: push cvar name to stack
		Assert( pHelpFunction[44] == 0x56 );
		// 2 bytes: call g_pCVar->FindCommandBase
		Assert( pHelpFunction[45] == 0xFF );
		Assert( pHelpFunction[46] == 0xD2 );
		// 2 bytes: check if we got a ConCommandBase
		Assert( pHelpFunction[47] == 0x85 );
		Assert( pHelpFunction[48] == 0xC0 );
		// 2 bytes: if we got one, jump forward again
		Assert( pHelpFunction[49] == 0x75 );
		Assert( pHelpFunction[50] == 0x13 );
		// 1 byte: push the cvar name to the stack again
		Assert( pHelpFunction[51] == 0x56 );
		// 6 bytes: push "help:  no cvar or command named %s\n" to the stack
		Assert( pHelpFunction[52] == 0x68 );
		Assert( pHelpFunction[53] == 0xA4 );
		// 6 bytes: call ConMsg
		Assert( pHelpFunction[57] == 0xFF );
		Assert( pHelpFunction[58] == 0x15 );
		// 3 bytes: pop the stack
		Assert( pHelpFunction[63] == 0x83 );
		Assert( pHelpFunction[64] == 0xC4 );
		Assert( pHelpFunction[65] == 0x08 );
		// 1 byte: restore ESI register
		Assert( pHelpFunction[66] == 0x5E );
		// 3 bytes: return
		Assert( pHelpFunction[67] == 0xC2 );
		Assert( pHelpFunction[68] == 0x04 );
		Assert( pHelpFunction[69] == 0x00 );
		// here's where the second jump forward put us, and this is what we really care about:
		// 1 byte: push ConCommandBase to stack
		Assert( pHelpFunction[70] == 0x50 );
		// 5 bytes: call the ConVar_PrintDescription function from tier1.
		Assert( pHelpFunction[71] == 0xE8 );
		// 3 bytes: pop the stack
		Assert( pHelpFunction[76] == 0x83 );
		Assert( pHelpFunction[77] == 0xC4 );
		Assert( pHelpFunction[78] == 0x04 );
		// 1 byte: restore the ESI register
		Assert( pHelpFunction[79] == 0x5E );
		// 3 bytes: return
		Assert( pHelpFunction[80] == 0xC2 );
		Assert( pHelpFunction[81] == 0x04 );
		Assert( pHelpFunction[82] == 0x00 );

		const byte *pPrintDescriptionFunction = reinterpret_cast< const byte * >( uintptr_t( pHelpFunction ) + 76 + *reinterpret_cast< const uintptr_t * >( pHelpFunction + 72 ) );
		// The next function is fairly complicated and we only care about a string right at the end, so let's skip ahead.
		char *pszFormatString = *reinterpret_cast< char *const * >( pPrintDescriptionFunction + 693 );
		Assert( !V_strcmp( pszFormatString, "%-80s - %.80s\n" ) );
		Assert( pszFormatString[9] == '.' );

		// Remove the .80 from the format string. And we're done.
		DWORD oldProtect{};
		VirtualProtect( pszFormatString + 9, 3, PAGE_READWRITE, &oldProtect );
		pszFormatString[9] = 's';
		pszFormatString[10] = '\n';
		pszFormatString[11] = '\0';
		VirtualProtect( pszFormatString + 9, 3, oldProtect, &oldProtect );
	}
};

CRD_ConVar_PrintDescription_Hack s_RD_ConVar_PrintDescription_Hack;
