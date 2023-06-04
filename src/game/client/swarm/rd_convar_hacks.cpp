#include "cbase.h"
#include "winlite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static class CRD_Convar_Hacks final : public CAutoGameSystem
{
public:
	CRD_Convar_Hacks() : CAutoGameSystem( "CRD_Convar_Hacks" )
	{
	}

	constexpr static const char *s_szNonCheat[] =
	{
		// These were marked as cheats to avoid them being used as a kind
		// of primitive wallhack. As a top-down game, we are not concerned
		// with players seeing behind a wall, and due to the nature of the
		// game's spawning, it is often required to have active players
		// on a map in order to accurately measure its performance. We are
		// removing FCVAR_CHEAT from these commands and variables to allow
		// mappers and programmers to debug performance while spectating
		// or playing.
		"+showbudget",
		"+showbudget_texture",
		"+showbudget_texture_global",
		"-showbudget",
		"-showbudget_texture",
		"-showbudget_texture_global",
		"showbudget_texture",
	};

	constexpr static const char *s_szCheat[] =
	{
		"buildcubemaps",
	};

	constexpr static const char *s_szAddArchive[] =
	{
		// Depth blur strength is a setting (0 or -1), so we need it to be saved.
		"mat_depth_blur_strength_override",
	};

	virtual bool Init() override
	{
		Assert( g_pCVar );
		if ( !g_pCVar )
			return false;

		for ( int i = 0; i < NELEMS( s_szNonCheat ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szNonCheat[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->RemoveFlags( FCVAR_CHEAT );
		}

		ApplyShowBudgetHack();

		for ( int i = 0; i < NELEMS( s_szCheat ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szCheat[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->AddFlags( FCVAR_CHEAT );
		}

		for ( int i = 0; i < NELEMS( s_szAddArchive ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szAddArchive[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->AddFlags( FCVAR_ARCHIVE );
		}

		return true;
	}

	void ApplyShowBudgetHack()
	{
		// In addition to marking the convar as non-cheat, we have to disable the
		// code that closes the panel automatically when cheats are disabled.
		ConCommand *pCmd = g_pCVar->FindCommand( "+showbudget" );
		const byte *pCallback = *reinterpret_cast< const byte *const * >( reinterpret_cast< const byte * >( pCmd ) + sizeof( ConCommandBase ) );
		const byte *pPanel = **reinterpret_cast< const byte *const * const * >( pCallback + 3 );
		Assert( pPanel );
		byte *pThink = ( *reinterpret_cast< byte *const *const * >( pPanel ) )[28];
		Assert( pThink[21] == 0x75 );
		Assert( pThink[22] == 0x33 );

		DWORD oldProtect{};
		VirtualProtect( pThink + 21, 1, PAGE_EXECUTE_READWRITE, &oldProtect );
		pThink[21] = 0xEB;
		VirtualProtect( pThink + 21, 1, oldProtect, &oldProtect );
		FlushInstructionCache( GetCurrentProcess(), pThink, 22 );
	}
} s_RD_Convar_Hacks;
