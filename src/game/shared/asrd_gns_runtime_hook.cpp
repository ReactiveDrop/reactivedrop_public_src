#include "cbase.h"
#include "tier0/dbg.h"
#include "asrd_gns_runtime_hook.h"
#include "asrd_gns_message_registry.h"
#include "asrd_gns_client_lifecycle.h"
#include "asrd_gns_server_lifecycle.h"

#if defined( _WIN32 ) && !defined( _X360 )

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace
{
	// This fixed local PE32 engine.dll is verified before process memory is
	// changed. Task 20 records the binary evidence; every mismatch fails closed.
	static const DWORD kEngineTimeDateStamp = 0x5F363761;
	static const DWORD kEngineSizeOfImage = 0x006F2000;
	static const DWORD kConnectIntentRva = 0x000D69F0;
	static const DWORD kRegisterMessageRva = 0x00079920;
	static const DWORD kSvThinkCallRva = 0x00189517;
	static const DWORD kSvThinkOriginalTargetRva = 0x001861F0;
	static const DWORD kStateMagic = 0x41524748; // "ARGH"
	static const DWORD kStateVersion = 4;
	// The copied eight-byte spans end on x86 instruction boundaries at both
	// targets. The longer signatures remain fail-closed; see Task 20 evidence.
	static const size_t kPatchSize = 8;
	static const BYTE kConnectIntentExpectedBytes[] =
	{
		0x8B, 0x44, 0x24, 0x08, 0x8B, 0x54, 0x24, 0x04,
		0x6A, 0x01, 0x50, 0x52, 0xE8, 0xEF, 0xFE, 0xFF,
		0xFF, 0xC2, 0x08, 0x00
	};
	// The target starts push esi; push edi; mov edi,[esp+0Ch]; mov eax,[edi].
	// ECX is the channel and the message starts at [ESP+4]; see Task 20 evidence.
	static const BYTE kRegisterMessageExpectedBytes[] =
	{
		0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x8B, 0x07
	};
	static const size_t kSvThinkPatchSize = 5;
	static const BYTE kSvThinkExpectedBytes[ kSvThinkPatchSize ] =
	{
		0xE8, 0xD4, 0xCC, 0xFF, 0xFF
	};

	enum HookStateValue
	{
		HOOK_STATE_EMPTY = 0,
		HOOK_STATE_INSTALLING = 1,
		HOOK_STATE_INSTALLED = 2,
		HOOK_STATE_FAILED = 3
	};

	// This named mapping has a fixed packed layout.  The hook fields,
	// runtime role, and client callback address are process state; transport
	// activation remains owned by the corresponding game DLL.
#pragma pack(push, 1)
	struct SharedHookState
	{
		DWORD magic;
		DWORD version;
		LONG state;
		LONG installCount;
		LONG initCount;
		LONG callCount;
		DWORD targetRva;
		DWORD targetAddress;
		DWORD trampolineAddress;
		DWORD ownerPid;
		DWORD ownerTid;

		LONG registrationState;
		LONG registrationInstallCount;
		LONG registrationCallCount;
		DWORD registrationTargetRva;
		DWORD registrationTargetAddress;
		DWORD registrationTrampolineAddress;
		DWORD registrationOwnerPid;
		DWORD registrationOwnerTid;

		LONG runtimeRole;
		DWORD clientConnectIntentHandlerAddress;

		LONG serverWakeState;
		LONG serverWakeInstallCount;
		LONG serverWakeCallCount;
		DWORD serverWakeTargetRva;
		DWORD serverWakeTargetAddress;
		DWORD serverWakeOriginalAddress;
		DWORD serverWakeOwnerPid;
		DWORD serverWakeOwnerTid;
	};
#pragma pack(pop)

	static HANDLE s_stateMapping = NULL;
	static SharedHookState *s_state = NULL;
	static INIT_ONCE s_sharedStateInitOnce = INIT_ONCE_STATIC_INIT;

	// At engine+0xD69F0, ECX is retained and the two explicit arguments begin
	// at [ESP+4]. The __fastcall detour consumes ECX, ignores EDX, and preserves
	// the original ret 8 stack cleanup; see Task 20 evidence.
	typedef bool (__thiscall *EngineRegisterMessageFn)( void *channel, void *message );
	typedef void (__cdecl *EngineSvThinkFn)( bool finalTick );

	static void LogHook( const char *message )
	{
		Warning( "[ASRD-HOOK] %s\n", message );
	}

	static void LogHookf( const char *format, ... )
	{
		char buffer[ 512 ];
		va_list args;
		va_start( args, format );
		_vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
		buffer[ sizeof( buffer ) - 1 ] = '\0';
		va_end( args );
		LogHook( buffer );
	}

	static bool MakeObjectName( const char *prefix, char *name, size_t capacity )
	{
		if ( !prefix || !name || capacity == 0 )
			return false;
		const int written = _snprintf( name, capacity, "Local\\%s_%08lX",
			prefix, (unsigned long)GetCurrentProcessId() );
		return written > 0 && (size_t)written < capacity;
	}

	static BOOL CALLBACK InitializeSharedState( PINIT_ONCE, PVOID, PVOID * )
	{
		char mappingName[ 96 ];
		if ( !MakeObjectName( "ASRD_GNS_RUNTIME_HOOK_STATE", mappingName,
			sizeof( mappingName ) ) )
			return FALSE;

		s_stateMapping = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
			0, (DWORD)sizeof( SharedHookState ), mappingName );
		if ( !s_stateMapping )
			return FALSE;

		s_state = (SharedHookState *)MapViewOfFile( s_stateMapping, FILE_MAP_ALL_ACCESS, 0, 0,
			sizeof( SharedHookState ) );
		if ( s_state )
			return TRUE;

		CloseHandle( s_stateMapping );
		s_stateMapping = NULL;
		return FALSE;
	}

	static bool OpenSharedState( HANDLE *pMutex )
	{
		if ( !pMutex )
			return false;

		char mutexName[ 96 ];
		if ( !MakeObjectName( "ASRD_GNS_RUNTIME_HOOK_LOCK", mutexName, sizeof( mutexName ) ) )
			return false;

		if ( !InitOnceExecuteOnce( &s_sharedStateInitOnce, InitializeSharedState,
			NULL, NULL ) )
			return false;

		HANDLE mutex = CreateMutexA( NULL, FALSE, mutexName );
		if ( !mutex )
			return false;
		*pMutex = mutex;
		return true;
	}

	static bool LockSharedState( HANDLE *pMutex )
	{
		if ( !OpenSharedState( pMutex ) )
			return false;

		const DWORD waitResult = WaitForSingleObject( *pMutex, 10000 );
		if ( waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED )
		{
			CloseHandle( *pMutex );
			*pMutex = NULL;
			return false;
		}

		if ( s_state->magic != kStateMagic || s_state->version != kStateVersion )
		{
			memset( s_state, 0, sizeof( *s_state ) );
			s_state->magic = kStateMagic;
			s_state->version = kStateVersion;
		}
		return true;
	}

	static void UnlockSharedState( HANDLE mutex )
	{
		if ( mutex )
		{
			ReleaseMutex( mutex );
			CloseHandle( mutex );
		}
	}

	static const char *RuntimeRoleName( ASRD_GNS_RuntimeRole role )
	{
		switch ( role )
		{
		case ASRD_GNS_RUNTIME_CLIENT:
			return "client";
		case ASRD_GNS_RUNTIME_DEDICATED_SERVER:
			return "dedicated_server";
		case ASRD_GNS_RUNTIME_LISTEN_SERVER:
			return "listen_server";
		default:
			return "uninitialized";
		}
	}

	static bool IsExpectedEngineModule( HMODULE engine, DWORD targetRva,
		const BYTE *expectedBytes, size_t expectedSize, BYTE **targetAddress )
	{
		if ( !engine || !expectedBytes || expectedSize == 0 || !targetAddress )
			return false;

		const BYTE *base = (const BYTE *)engine;
		const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)base;
		if ( dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 || dos->e_lfanew > 0x100000 )
			return false;

		const IMAGE_NT_HEADERS32 *nt = (const IMAGE_NT_HEADERS32 *)(base + dos->e_lfanew);
		if ( nt->Signature != IMAGE_NT_SIGNATURE ||
			nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
			nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
			nt->FileHeader.TimeDateStamp != kEngineTimeDateStamp ||
			nt->OptionalHeader.SizeOfImage != kEngineSizeOfImage ||
			nt->OptionalHeader.ImageBase != 0x10000000 )
			return false;

		if ( targetRva + expectedSize > nt->OptionalHeader.SizeOfImage )
			return false;

		BYTE *target = const_cast<BYTE *>( base + targetRva );
		if ( memcmp( target, expectedBytes, expectedSize ) != 0 )
			return false;

		// The target must be in an executable section of the current image.
		const IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION( nt );
		bool executable = false;
		for ( unsigned int i = 0; i < nt->FileHeader.NumberOfSections; ++i )
		{
			const DWORD sectionStart = section[ i ].VirtualAddress;
			const DWORD sectionSize = section[ i ].Misc.VirtualSize > section[ i ].SizeOfRawData
				? section[ i ].Misc.VirtualSize : section[ i ].SizeOfRawData;
			if ( targetRva >= sectionStart && targetRva + expectedSize <= sectionStart + sectionSize )
			{
				executable = ( section[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
				break;
			}
		}
		if ( !executable )
			return false;

		*targetAddress = target;
		return true;
	}

	static bool IsExecutableEngineRange( const IMAGE_NT_HEADERS32 *nt,
		DWORD rva, size_t bytes )
	{
		if ( !nt || bytes == 0 || rva >= nt->OptionalHeader.SizeOfImage ||
			bytes > (size_t)nt->OptionalHeader.SizeOfImage - rva )
			return false;

		const IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION( nt );
		for ( unsigned int i = 0; i < nt->FileHeader.NumberOfSections; ++i )
		{
			const DWORD sectionStart = section[ i ].VirtualAddress;
			const DWORD sectionSize = section[ i ].Misc.VirtualSize > section[ i ].SizeOfRawData
				? section[ i ].Misc.VirtualSize : section[ i ].SizeOfRawData;
			if ( rva < sectionStart || rva - sectionStart > sectionSize ||
			bytes > (size_t)sectionSize - ( rva - sectionStart ) )
				continue;
			return ( section[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
		}
		return false;
	}

	static bool IsExpectedSvThinkCall( HMODULE engine, BYTE **callAddress,
		DWORD *originalTargetAddress )
	{
		if ( !callAddress || !originalTargetAddress )
			return false;

		BYTE *call = NULL;
		if ( !IsExpectedEngineModule( engine, kSvThinkCallRva,
			kSvThinkExpectedBytes, sizeof( kSvThinkExpectedBytes ), &call ) )
			return false;

		const BYTE *base = (const BYTE *)engine;
		const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)base;
		const IMAGE_NT_HEADERS32 *nt =
			(const IMAGE_NT_HEADERS32 *)( base + dos->e_lfanew );
		int32_t displacement = 0;
		memcpy( &displacement, call + 1, sizeof( displacement ) );
		const uintptr_t decodedAddress =
			(uintptr_t)( (intptr_t)(uintptr_t)( call + kSvThinkPatchSize ) +
				(intptr_t)displacement );
		const BYTE *expectedTarget = base + kSvThinkOriginalTargetRva;
		if ( decodedAddress != (uintptr_t)expectedTarget ||
			!IsExecutableEngineRange( nt, kSvThinkOriginalTargetRva, 1 ) )
			return false;

		*callAddress = call;
		*originalTargetAddress = (DWORD)decodedAddress;
		return true;
	}

	enum PatchRestoreStatus
	{
		PATCH_RESTORE_FAILED = 0,
		PATCH_RESTORE_COMPLETE = 1,
		PATCH_RESTORE_DIAGNOSTIC = 2
	};

	static PatchRestoreStatus RestoreOriginalBytes( BYTE *source, const BYTE *originalBytes,
		DWORD originalProtection, bool sourceIsWritable, const char *operation,
		size_t patchSize = kPatchSize )
	{
		if ( !source || !originalBytes || !operation || patchSize == 0 )
			return PATCH_RESTORE_FAILED;

		if ( !sourceIsWritable )
		{
			DWORD ignoredProtection = 0;
			if ( !VirtualProtect( source, patchSize, PAGE_EXECUTE_READWRITE, &ignoredProtection ) )
			{
				const DWORD error = GetLastError();
				LogHookf( "%s restore protect failed target=%p error=%lu", operation, source,
					(unsigned long)error );
				return PATCH_RESTORE_FAILED;
			}
		}

		memcpy( source, originalBytes, patchSize );
		const bool bytesRestored = memcmp( source, originalBytes, patchSize ) == 0;
		const BOOL flushed = FlushInstructionCache( GetCurrentProcess(), source, patchSize );
		const DWORD flushError = flushed ? ERROR_SUCCESS : GetLastError();

		DWORD ignoredProtection = 0;
		const BOOL protectionRestored = VirtualProtect( source, patchSize,
			originalProtection, &ignoredProtection );
		const DWORD protectionError = protectionRestored ? ERROR_SUCCESS : GetLastError();
		if ( !protectionRestored )
		{
			LogHookf( "%s protection restore failed target=%p error=%lu", operation, source,
				(unsigned long)protectionError );
		}
		if ( !flushed )
		{
			LogHookf( "%s cache flush failed target=%p error=%lu", operation, source,
				(unsigned long)flushError );
		}
		if ( !bytesRestored )
		{
			LogHookf( "%s byte restore verification failed; target may remain patched target=%p",
				operation, source );
			return PATCH_RESTORE_FAILED;
		}
		if ( !flushed || !protectionRestored )
			return PATCH_RESTORE_DIAGNOSTIC;
		return PATCH_RESTORE_COMPLETE;
	}

	static bool WriteRelativeJump( BYTE *source, const BYTE *destination,
		bool *originalBytesRestored )
	{
		if ( originalBytesRestored )
			*originalBytesRestored = true;
		if ( !source || !destination )
			return false;
		if ( !originalBytesRestored )
			return false;

		// E9 rel32 is five bytes. The eight-byte patch adds three NOPs, and its
		// displacement is measured from the end of E9; see Task 20 evidence.
		const intptr_t relative = (intptr_t)destination - ( (intptr_t)source + 5 );
		if ( relative < (intptr_t)0x80000000LL || relative > (intptr_t)0x7FFFFFFFLL )
			return false;

		BYTE jump[ kPatchSize ] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90, 0x90 };
		const int32_t displacement = (int32_t)relative;
		memcpy( jump + 1, &displacement, sizeof( displacement ) );
		BYTE originalBytes[ kPatchSize ];
		memcpy( originalBytes, source, sizeof( originalBytes ) );

		DWORD oldProtection = 0;
		if ( !VirtualProtect( source, kPatchSize, PAGE_EXECUTE_READWRITE, &oldProtection ) )
			return false;
		memcpy( source, jump, sizeof( jump ) );
		*originalBytesRestored = false;
		const BOOL flushed = FlushInstructionCache( GetCurrentProcess(), source, kPatchSize );
		const DWORD flushError = flushed ? ERROR_SUCCESS : GetLastError();
		DWORD ignoredProtection = 0;
		const BOOL protectionRestored = VirtualProtect( source, kPatchSize, oldProtection,
			&ignoredProtection );
		const DWORD protectionError = protectionRestored ? ERROR_SUCCESS : GetLastError();
		if ( flushed && protectionRestored )
			return true;

		if ( !protectionRestored )
		{
			LogHookf( "patch protection restore failed target=%p error=%lu", source,
				(unsigned long)protectionError );
		}
		if ( !flushed )
		{
			LogHookf( "patch cache flush failed target=%p error=%lu", source,
				(unsigned long)flushError );
		}

		const PatchRestoreStatus restoreStatus = RestoreOriginalBytes( source, originalBytes,
			oldProtection, false, "patch rollback" );
		*originalBytesRestored = restoreStatus != PATCH_RESTORE_FAILED;
		if ( restoreStatus == PATCH_RESTORE_FAILED )
		{
			LogHookf( "patch rollback failed; target may remain patched target=%p", source );
		}
		else if ( restoreStatus == PATCH_RESTORE_DIAGNOSTIC )
		{
			LogHookf( "patch rollback restored target bytes with diagnostics target=%p", source );
		}
		return false;
	}

	static bool WriteRelativeCall( BYTE *source, const BYTE *destination,
		const BYTE *expectedBytes, size_t patchSize, bool *originalBytesRestored )
	{
		if ( originalBytesRestored )
			*originalBytesRestored = true;
		if ( !source || !destination || !expectedBytes ||
			patchSize != kSvThinkPatchSize || !originalBytesRestored )
			return false;
		if ( memcmp( source, expectedBytes, patchSize ) != 0 )
			return false;

		const intptr_t relative = (intptr_t)destination -
			( (intptr_t)source + (intptr_t)patchSize );
		if ( relative < (intptr_t)0x80000000LL ||
			relative > (intptr_t)0x7FFFFFFFLL )
			return false;

		BYTE call[ kSvThinkPatchSize ] = { 0xE8, 0, 0, 0, 0 };
		const int32_t displacement = (int32_t)relative;
		memcpy( call + 1, &displacement, sizeof( displacement ) );
		BYTE originalBytes[ kSvThinkPatchSize ];
		memcpy( originalBytes, source, patchSize );

		DWORD oldProtection = 0;
		if ( !VirtualProtect( source, patchSize, PAGE_EXECUTE_READWRITE, &oldProtection ) )
			return false;
		memcpy( source, call, patchSize );
		*originalBytesRestored = false;
		const BOOL flushed = FlushInstructionCache( GetCurrentProcess(), source, patchSize );
		const DWORD flushError = flushed ? ERROR_SUCCESS : GetLastError();
		DWORD ignoredProtection = 0;
		const BOOL protectionRestored = VirtualProtect( source, patchSize, oldProtection,
			&ignoredProtection );
		const DWORD protectionError = protectionRestored ? ERROR_SUCCESS : GetLastError();
		if ( flushed && protectionRestored )
			return true;

		if ( !protectionRestored )
		{
			LogHookf( "SV_Think call patch protection restore failed target=%p error=%lu",
				source, (unsigned long)protectionError );
		}
		if ( !flushed )
		{
			LogHookf( "SV_Think call patch cache flush failed target=%p error=%lu",
				source, (unsigned long)flushError );
		}

		const PatchRestoreStatus restoreStatus = RestoreOriginalBytes( source,
			originalBytes, oldProtection, false, "SV_Think call patch rollback", patchSize );
		// A diagnostic restore may have copied the bytes back but failed to
		// flush the instruction cache or restore protection. Keep the original
		// target published until the rollback is fully complete.
		*originalBytesRestored = restoreStatus == PATCH_RESTORE_COMPLETE;
		if ( restoreStatus == PATCH_RESTORE_FAILED )
			LogHook( "SV_Think call patch rollback failed; target may remain patched" );
		else if ( restoreStatus == PATCH_RESTORE_DIAGNOSTIC )
			LogHook( "SV_Think call patch rollback incomplete; original retained" );
		return false;
	}

	static int __fastcall RuntimeHookThunk( void *self, void *, const char *endpoint,
		const char *secondary )
	{
		SharedHookState *state = s_state;
		if ( state )
			InterlockedIncrement( &state->callCount );

		// A valid direct-IP intent is always owned by GNS. The original engine
		// function is intentionally not called, including on wrapper failure;
		// this prevents a silent legacy gameplay fallback.  The callback is
		// registered by client.dll independently of detour installation, so a
		// server.dll-first load order cannot redirect client state into server.dll.
		const DWORD handlerAddress = state
			? (DWORD)InterlockedCompareExchange(
				(volatile LONG *)&state->clientConnectIntentHandlerAddress, 0, 0 )
			: 0;
		if ( handlerAddress == 0 )
		{
			if ( state && InterlockedCompareExchange( &state->callCount, 0, 0 ) == 1 )
				LogHook( "connect callback unavailable; request blocked" );
			return 0;
		}

		ASRD_GNS_ClientConnectIntentHandler handler =
			(ASRD_GNS_ClientConnectIntentHandler)(uintptr_t)handlerAddress;
		const bool takeover = handler( self, endpoint, secondary );
		if ( takeover )
			return 1;

		// The saved trampoline is installation evidence only; connect takeover
		// never invokes the original target as a fallback.
		if ( !state || state->trampolineAddress == 0 )
			return 0;

		return 0;
	}

	static bool __fastcall RuntimeRegisterMessageThunk( void *channel, void *, void *message )
	{
		SharedHookState *state = s_state;
		if ( state )
			InterlockedIncrement( &state->registrationCallCount );

		if ( !state || state->registrationTrampolineAddress == 0 )
		{
			if ( state && InterlockedCompareExchange( &state->registrationCallCount, 0, 0 ) == 1 )
				LogHook( "registration original=unavailable" );
			return false;
		}

		// __thiscall reloads ECX=channel and places message at [ESP+4]. The
		// trampoline executes the copied prologue then jumps to target+8.
		EngineRegisterMessageFn original =
			(EngineRegisterMessageFn)(uintptr_t)state->registrationTrampolineAddress;
		const bool result = original( channel, message );
		if ( result )
			ASRD_GNS_MessageRegistryCapture( message, channel );
		return result;
	}

	static void __cdecl RuntimeServerWakeHookThunk( bool finalTick )
	{
		SharedHookState *state = s_state;
		if ( state )
			InterlockedIncrement( &state->serverWakeCallCount );

		if ( ASRD_GNS_GetRuntimeRole() == ASRD_GNS_RUNTIME_DEDICATED_SERVER &&
			ASRD_GNS_ServerIsInitialized() && ASRD_GNS_ServerIsHibernating() )
		{
			ASRD_GNS_ServerWakeControlFrame();
		}

		const DWORD originalAddress = state
			? (DWORD)InterlockedCompareExchange(
				(volatile LONG *)&state->serverWakeOriginalAddress, 0, 0 )
			: 0;
		if ( originalAddress == 0 )
		{
			// A live patched call-site publishes this address before the rewrite
			// and retains it across any incomplete rollback. This is therefore an
			// invariant violation, never a normal no-op path.
			LogHook( "SV_Think original=unavailable; wrapper invariant violated" );
			return;
		}

		// The call-site patch preserves SV_Think's cdecl argument and this
		// wrapper invokes the original target exactly once on every entry.
		EngineSvThinkFn original = (EngineSvThinkFn)(uintptr_t)originalAddress;
		original( finalTick );
	}

	static bool InstallHook( BYTE *target, const BYTE *detour, DWORD *trampolineAddress,
		bool *targetBytesRestored )
	{
		if ( targetBytesRestored )
			*targetBytesRestored = true;
		if ( !target || !detour || !trampolineAddress || !targetBytesRestored ||
			sizeof( void * ) != 4 )
			return false;
		*trampolineAddress = 0;

		// The detour intentionally remains in place until process exit. Pin the
		// module containing the thunk so engine cannot call an unloaded DLL.
		HMODULE ownerModule = NULL;
		if ( !GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCSTR)&RuntimeHookThunk, &ownerModule ) )
			return false;

		BYTE *trampoline = (BYTE *)VirtualAlloc( NULL, kPatchSize + kPatchSize,
			MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
		if ( !trampoline )
			return false;
		*trampolineAddress = (DWORD)(uintptr_t)trampoline;

		// Copy complete instructions, then return to the first unmodified target
		// instruction at target+8. The original function completes its own return.
		memcpy( trampoline, target, kPatchSize );
		bool trampolineBytesRestored = true;
		if ( !WriteRelativeJump( trampoline + kPatchSize, target + kPatchSize,
			&trampolineBytesRestored ) )
		{
			// The engine target is not patched yet, so releasing this trampoline is safe.
			if ( VirtualFree( trampoline, 0, MEM_RELEASE ) )
			{
				*trampolineAddress = 0;
			}
			else
			{
				const DWORD error = GetLastError();
				LogHookf( "trampoline setup release failed target=%p trampoline=%p error=%lu",
					target, trampoline, (unsigned long)error );
			}
			return false;
		}

		bool targetOriginalBytesRestored = true;
		if ( !WriteRelativeJump( target, detour, &targetOriginalBytesRestored ) )
		{
			*targetBytesRestored = targetOriginalBytesRestored;
			if ( !targetOriginalBytesRestored )
			{
				LogHookf( "detour patch rollback failed; target remains patched; trampoline retained "
					"target=%p trampoline=%p", target, trampoline );
				return false;
			}
			if ( VirtualFree( trampoline, 0, MEM_RELEASE ) )
			{
				*trampolineAddress = 0;
			}
			else
			{
				const DWORD error = GetLastError();
				LogHookf( "trampoline release after target rollback failed target=%p trampoline=%p "
					"error=%lu", target, trampoline, (unsigned long)error );
			}
			return false;
		}

		*targetBytesRestored = false;
		return true;
	}

	static bool InstallServerWakeHook( BYTE *target, DWORD originalTargetAddress,
		bool *targetBytesRestored )
	{
		if ( targetBytesRestored )
			*targetBytesRestored = true;
		if ( !target || originalTargetAddress == 0 || !targetBytesRestored ||
			sizeof( void * ) != 4 )
			return false;

		// Pin the server.dll containing this thunk before the engine call site is
		// changed.  The detour remains installed until process exit.
		HMODULE ownerModule = NULL;
		if ( !GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCSTR)&RuntimeServerWakeHookThunk,
			&ownerModule ) )
			return false;

		return WriteRelativeCall( target,
			(const BYTE *)&RuntimeServerWakeHookThunk, kSvThinkExpectedBytes,
			kSvThinkPatchSize, targetBytesRestored );
	}

	static bool RollbackHook( BYTE *target, const BYTE *originalBytes, DWORD trampolineAddress )
	{
		if ( !target || !originalBytes || trampolineAddress == 0 )
			return false;

		DWORD oldProtection = 0;
		if ( !VirtualProtect( target, kPatchSize, PAGE_EXECUTE_READWRITE, &oldProtection ) )
		{
			const DWORD error = GetLastError();
			LogHookf( "hook rollback protect failed target=%p error=%lu", target,
				(unsigned long)error );
			return false;
		}

		const PatchRestoreStatus restoreStatus = RestoreOriginalBytes( target, originalBytes,
			oldProtection, true, "hook rollback" );
		if ( restoreStatus == PATCH_RESTORE_FAILED )
		{
			LogHookf( "hook rollback failed; target may remain patched target=%p trampoline=%p",
				target, (LPVOID)(uintptr_t)trampolineAddress );
			return false;
		}
		if ( restoreStatus == PATCH_RESTORE_DIAGNOSTIC )
		{
			LogHookf( "hook rollback restored target bytes with diagnostics; releasing trampoline "
				"target=%p", target );
		}

		if ( !VirtualFree( (LPVOID)(uintptr_t)trampolineAddress, 0, MEM_RELEASE ) )
		{
			const DWORD error = GetLastError();
			LogHookf( "hook rollback trampoline release failed target=%p error=%lu", target,
				(unsigned long)error );
			return false;
		}
		return true;
	}

	static void ClearConnectInstallationState( SharedHookState *state )
	{
		if ( !state )
			return;
		state->targetRva = 0;
		state->targetAddress = 0;
		state->trampolineAddress = 0;
		state->ownerPid = 0;
		state->ownerTid = 0;
		InterlockedExchange( &state->installCount, 0 );
		InterlockedExchange( &state->state, HOOK_STATE_EMPTY );
	}

	static void ClearRegistrationInstallationState( SharedHookState *state )
	{
		if ( !state )
			return;
		state->registrationTargetRva = 0;
		state->registrationTargetAddress = 0;
		state->registrationTrampolineAddress = 0;
		state->registrationOwnerPid = 0;
		state->registrationOwnerTid = 0;
		InterlockedExchange( &state->registrationInstallCount, 0 );
		InterlockedExchange( &state->registrationState, HOOK_STATE_EMPTY );
	}

	enum RuntimeRoleOperation
	{
		RUNTIME_ROLE_SET_CLIENT = 0,
		RUNTIME_ROLE_SET_DEDICATED_SERVER,
		RUNTIME_ROLE_ACTIVATE_LISTEN_SERVER,
		RUNTIME_ROLE_DEACTIVATE_LISTEN_SERVER,
	};

	static bool ChangeRuntimeRole( RuntimeRoleOperation operation )
	{
		HANDLE mutex = NULL;
		if ( !LockSharedState( &mutex ) )
		{
			LogHook( "runtime role state setup failed" );
			return false;
		}

		const ASRD_GNS_RuntimeRole current = (ASRD_GNS_RuntimeRole)
			InterlockedCompareExchange( &s_state->runtimeRole, 0, 0 );
		ASRD_GNS_RuntimeRole next = current;
		bool accepted = false;
		const char *operationName = "unknown";

		switch ( operation )
		{
		case RUNTIME_ROLE_SET_CLIENT:
			operationName = "set_client";
			// Dedicated-server identity is persistent.  A listen-server role
			// is also preserved if client.dll finishes initialization after the
			// server-side GameInit callback.
			if ( current != ASRD_GNS_RUNTIME_DEDICATED_SERVER )
			{
				if ( current == ASRD_GNS_RUNTIME_UNINITIALIZED )
					next = ASRD_GNS_RUNTIME_CLIENT;
				accepted = true;
			}
			break;

		case RUNTIME_ROLE_SET_DEDICATED_SERVER:
			operationName = "set_dedicated_server";
			// Dedicated identity is authoritative once the caller has supplied
			// independent engine evidence.  The public setter enforces that
			// evidence before reaching this shared, locked state transition.
			if ( current != ASRD_GNS_RUNTIME_DEDICATED_SERVER )
			{
				next = ASRD_GNS_RUNTIME_DEDICATED_SERVER;
			}
			accepted = true;
			break;

		case RUNTIME_ROLE_ACTIVATE_LISTEN_SERVER:
			operationName = "activate_listen_server";
			if ( current != ASRD_GNS_RUNTIME_DEDICATED_SERVER )
			{
				next = ASRD_GNS_RUNTIME_LISTEN_SERVER;
				accepted = true;
			}
			break;

		case RUNTIME_ROLE_DEACTIVATE_LISTEN_SERVER:
			operationName = "deactivate_listen_server";
			if ( current != ASRD_GNS_RUNTIME_DEDICATED_SERVER )
			{
				next = current == ASRD_GNS_RUNTIME_LISTEN_SERVER
					? ASRD_GNS_RUNTIME_CLIENT : current;
				accepted = true;
			}
			break;
		}

		if ( accepted && next != current )
			InterlockedExchange( &s_state->runtimeRole, (LONG)next );

		LogHookf( "runtime_role operation=%s current=%s next=%s result=%s",
			operationName, RuntimeRoleName( current ), RuntimeRoleName( next ),
			accepted ? "accepted" : "rejected" );
		UnlockSharedState( mutex );
		return accepted;
	}
}

bool ASRD_GNS_EnsureRuntimeHookInstalled( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
	{
		LogHook( "state setup failed; hook disabled" );
		return false;
	}
	const LONG attempt = InterlockedIncrement( &s_state->initCount );

	bool connectInstalledHere = false;
	bool registrationInstalledHere = false;
	bool connectTargetBytesRestored = true;
	bool registrationTargetBytesRestored = true;
	BYTE *connectTarget = NULL;
	BYTE *registrationTarget = NULL;
	DWORD connectTrampolineAddress = 0;
	DWORD registrationTrampolineAddress = 0;
	bool connectSuccess = s_state->state == HOOK_STATE_INSTALLED;
	bool registrationSuccess = s_state->registrationState == HOOK_STATE_INSTALLED;

	if ( connectSuccess )
	{
		LogHookf( "attempt=%ld result=already_installed installCount=%ld targetRva=0x%08lX",
			(long)attempt, (long)s_state->installCount,
			(unsigned long)s_state->targetRva );
	}
	else if ( s_state->state == HOOK_STATE_INSTALLING )
	{
		LogHook( "concurrent installation state unresolved; hook disabled" );
		InterlockedExchange( &s_state->state, HOOK_STATE_FAILED );
	}
	else if ( s_state->state != HOOK_STATE_FAILED )
	{
		HMODULE engine = GetModuleHandleA( "engine.dll" );
		BYTE *target = NULL;
		if ( !engine )
		{
			LogHook( "engine.dll is not loaded; hook disabled" );
		}
		else if ( !IsExpectedEngineModule( engine, kConnectIntentRva,
			kConnectIntentExpectedBytes, sizeof( kConnectIntentExpectedBytes ), &target ) )
		{
			LogHook( "engine build/target sanity mismatch; hook disabled" );
		}
		else
		{
			InterlockedExchange( &s_state->state, HOOK_STATE_INSTALLING );
			connectSuccess = InstallHook( target, (const BYTE *)&RuntimeHookThunk,
				&s_state->trampolineAddress, &connectTargetBytesRestored );
			if ( connectSuccess )
			{
				s_state->targetRva = kConnectIntentRva;
				s_state->targetAddress = (DWORD)(uintptr_t)target;
				s_state->ownerPid = GetCurrentProcessId();
				s_state->ownerTid = GetCurrentThreadId();
				InterlockedExchange( &s_state->installCount, 1 );
				InterlockedExchange( &s_state->state, HOOK_STATE_INSTALLED );
				connectInstalledHere = true;
				connectTarget = target;
				connectTrampolineAddress = s_state->trampolineAddress;
				LogHookf( "attempt=%ld result=installed installCount=%ld targetRva=0x%08lX target=%p",
					(long)attempt, (long)s_state->installCount,
					(unsigned long)kConnectIntentRva, target );
			}
			else
			{
				InterlockedExchange( &s_state->state, HOOK_STATE_FAILED );
				if ( !connectTargetBytesRestored )
					LogHook( "detour installation failed; target remains patched and trampoline retained" );
				else
					LogHook( "detour installation failed; hook disabled" );
			}
		}
	}

	if ( s_state->registrationState == HOOK_STATE_INSTALLED )
	{
		registrationSuccess = true;
		LogHookf( "attempt=%ld registration=result=already_installed installCount=%ld targetRva=0x%08lX",
			(long)attempt, (long)s_state->registrationInstallCount,
			(unsigned long)s_state->registrationTargetRva );
	}
	else if ( s_state->registrationState == HOOK_STATE_INSTALLING )
	{
		LogHook( "registration concurrent installation state unresolved; hook disabled" );
		InterlockedExchange( &s_state->registrationState, HOOK_STATE_FAILED );
	}
	else if ( s_state->registrationState != HOOK_STATE_FAILED )
	{
		HMODULE engine = GetModuleHandleA( "engine.dll" );
		BYTE *target = NULL;
		if ( !engine )
		{
			LogHook( "engine.dll is not loaded; registration hook disabled" );
		}
		else if ( !IsExpectedEngineModule( engine, kRegisterMessageRva,
			kRegisterMessageExpectedBytes, sizeof( kRegisterMessageExpectedBytes ), &target ) )
		{
			LogHook( "engine build/registration target sanity mismatch; registration hook disabled" );
		}
		else
		{
			InterlockedExchange( &s_state->registrationState, HOOK_STATE_INSTALLING );
			registrationSuccess = InstallHook( target,
				(const BYTE *)&RuntimeRegisterMessageThunk,
				&s_state->registrationTrampolineAddress, &registrationTargetBytesRestored );
			if ( registrationSuccess )
			{
				s_state->registrationTargetRva = kRegisterMessageRva;
				s_state->registrationTargetAddress = (DWORD)(uintptr_t)target;
				s_state->registrationOwnerPid = GetCurrentProcessId();
				s_state->registrationOwnerTid = GetCurrentThreadId();
				InterlockedExchange( &s_state->registrationInstallCount, 1 );
				InterlockedExchange( &s_state->registrationState, HOOK_STATE_INSTALLED );
				registrationInstalledHere = true;
				registrationTarget = target;
				registrationTrampolineAddress = s_state->registrationTrampolineAddress;
				LogHookf( "attempt=%ld registration=result=installed installCount=%ld targetRva=0x%08lX target=%p",
					(long)attempt, (long)s_state->registrationInstallCount,
					(unsigned long)kRegisterMessageRva, target );
			}
			else
			{
				InterlockedExchange( &s_state->registrationState, HOOK_STATE_FAILED );
				if ( !registrationTargetBytesRestored )
					LogHook( "registration installation failed; target remains patched and trampoline retained" );
				else
					LogHook( "registration detour installation failed; hook disabled" );
			}
		}
	}

	if ( !connectSuccess || !registrationSuccess )
	{
		if ( registrationInstalledHere )
		{
			if ( RollbackHook( registrationTarget, kRegisterMessageExpectedBytes,
				registrationTrampolineAddress ) )
			{
				ClearRegistrationInstallationState( s_state );
				registrationSuccess = false;
				LogHookf( "attempt=%ld result=rolled_back hook=registration target=%p",
					(long)attempt, registrationTarget );
			}
			else
			{
				InterlockedExchange( &s_state->registrationState, HOOK_STATE_FAILED );
				registrationSuccess = false;
				LogHookf( "attempt=%ld result=rollback_incomplete hook=registration target=%p",
					(long)attempt, registrationTarget );
			}
		}
		if ( connectInstalledHere )
		{
			if ( RollbackHook( connectTarget, kConnectIntentExpectedBytes,
				connectTrampolineAddress ) )
			{
				ClearConnectInstallationState( s_state );
				connectSuccess = false;
				LogHookf( "attempt=%ld result=rolled_back hook=connect target=%p",
					(long)attempt, connectTarget );
			}
			else
			{
				InterlockedExchange( &s_state->state, HOOK_STATE_FAILED );
				connectSuccess = false;
				LogHookf( "attempt=%ld result=rollback_incomplete hook=connect target=%p",
					(long)attempt, connectTarget );
			}
		}
	}

	UnlockSharedState( mutex );
	return connectSuccess && registrationSuccess;
}

bool ASRD_GNS_EnsureServerWakeHookInstalled( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
	{
		LogHook( "server wake hook state setup failed; hook disabled" );
		return false;
	}

	const ASRD_GNS_RuntimeRole role = (ASRD_GNS_RuntimeRole)
		InterlockedCompareExchange( &s_state->runtimeRole, 0, 0 );
	if ( role != ASRD_GNS_RUNTIME_DEDICATED_SERVER )
	{
		LogHookf( "server wake hook rejected role=%s", RuntimeRoleName( role ) );
		UnlockSharedState( mutex );
		return false;
	}

	bool success = s_state->serverWakeState == HOOK_STATE_INSTALLED;
	if ( success )
	{
		LogHookf( "server wake hook result=already_installed targetRva=0x%08lX",
			(unsigned long)s_state->serverWakeTargetRva );
	}
	else if ( s_state->serverWakeState == HOOK_STATE_INSTALLING )
	{
		LogHook( "server wake hook concurrent installation state unresolved; hook disabled" );
		InterlockedExchange( &s_state->serverWakeState, HOOK_STATE_FAILED );
	}
	else if ( s_state->serverWakeState != HOOK_STATE_FAILED )
	{
		HMODULE engine = GetModuleHandleA( "engine.dll" );
		BYTE *target = NULL;
		DWORD originalTargetAddress = 0;
		if ( !engine )
		{
			LogHook( "engine.dll is not loaded; server wake hook disabled" );
		}
		else if ( !IsExpectedSvThinkCall( engine, &target, &originalTargetAddress ) )
		{
			LogHook( "engine build/SV_Think call-site sanity mismatch; server wake hook disabled" );
		}
		else
		{
			InterlockedExchange( &s_state->serverWakeState, HOOK_STATE_INSTALLING );
			// Publish the independently validated SV_Think target before the
			// call-site can become live. A thunk entered during the patch or an
			// incomplete rollback must always have an original to invoke.
			InterlockedExchange( (volatile LONG *)&s_state->serverWakeOriginalAddress,
				(LONG)originalTargetAddress );
			bool targetBytesRestored = true;
			success = InstallServerWakeHook( target, originalTargetAddress,
				&targetBytesRestored );
			if ( success )
			{
				s_state->serverWakeTargetRva = kSvThinkCallRva;
				s_state->serverWakeTargetAddress = (DWORD)(uintptr_t)target;
				s_state->serverWakeOwnerPid = GetCurrentProcessId();
				s_state->serverWakeOwnerTid = GetCurrentThreadId();
				InterlockedExchange( &s_state->serverWakeInstallCount, 1 );
				InterlockedExchange( &s_state->serverWakeState, HOOK_STATE_INSTALLED );
				LogHookf( "server wake hook result=installed targetRva=0x%08lX target=%p original=%p",
					(unsigned long)kSvThinkCallRva, target,
					(void *)(uintptr_t)originalTargetAddress );
			}
			else
			{
				if ( targetBytesRestored )
					InterlockedExchange(
						(volatile LONG *)&s_state->serverWakeOriginalAddress, 0 );
				InterlockedExchange( &s_state->serverWakeState, HOOK_STATE_FAILED );
				if ( !targetBytesRestored )
					LogHook( "server wake detour installation failed; target may remain patched; "
						"original retained" );
				else
					LogHook( "server wake detour installation failed; hook disabled" );
			}
		}
	}

	UnlockSharedState( mutex );
	return success;
}

bool ASRD_GNS_RuntimeHookRegisterClientConnectHandler(
	ASRD_GNS_ClientConnectIntentHandler handler )
{
	if ( !handler || sizeof( void * ) != 4 )
	{
		LogHook( "client connect callback registration rejected" );
		return false;
	}

	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
	{
		LogHook( "client connect callback registration state unavailable" );
		return false;
	}

	const DWORD handlerAddress = (DWORD)(uintptr_t)handler;
	HMODULE handlerModule = NULL;
	if ( !GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCSTR)(uintptr_t)handler, &handlerModule ) )
	{
		const DWORD error = GetLastError();
		LogHookf( "client connect callback pin failed address=%p error=%lu", handler,
			(unsigned long)error );
		UnlockSharedState( mutex );
		return false;
	}
	InterlockedExchange( (volatile LONG *)&s_state->clientConnectIntentHandlerAddress,
		(LONG)handlerAddress );
	LogHookf( "client connect callback registered address=%p", handler );
	UnlockSharedState( mutex );
	return true;
}

void ASRD_GNS_RuntimeHookUnregisterClientConnectHandler(
	ASRD_GNS_ClientConnectIntentHandler handler )
{
	if ( !handler || sizeof( void * ) != 4 )
		return;

	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
		return;

	const DWORD handlerAddress = (DWORD)(uintptr_t)handler;
	const DWORD registeredAddress = (DWORD)InterlockedCompareExchange(
		(volatile LONG *)&s_state->clientConnectIntentHandlerAddress, 0, 0 );
	if ( registeredAddress == handlerAddress )
	{
		InterlockedExchange( (volatile LONG *)&s_state->clientConnectIntentHandlerAddress, 0 );
		LogHook( "client connect callback unregistered" );
	}
	UnlockSharedState( mutex );
}

ASRD_GNS_RuntimeRole ASRD_GNS_GetRuntimeRole( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
		return ASRD_GNS_RUNTIME_UNINITIALIZED;

	const ASRD_GNS_RuntimeRole role = (ASRD_GNS_RuntimeRole)
		InterlockedCompareExchange( &s_state->runtimeRole, 0, 0 );
	UnlockSharedState( mutex );
	return role;
}

bool ASRD_GNS_SetClientRuntimeRole( void )
{
	return ChangeRuntimeRole( RUNTIME_ROLE_SET_CLIENT );
}

bool ASRD_GNS_SetDedicatedServerRuntimeRole( bool verifiedDedicated )
{
	if ( !verifiedDedicated )
	{
		LogHook( "dedicated runtime role rejected: missing engine evidence" );
		return false;
	}
	return ChangeRuntimeRole( RUNTIME_ROLE_SET_DEDICATED_SERVER );
}

bool ASRD_GNS_ActivateListenServerRuntimeRole( void )
{
	return ChangeRuntimeRole( RUNTIME_ROLE_ACTIVATE_LISTEN_SERVER );
}

bool ASRD_GNS_DeactivateListenServerRuntimeRole( void )
{
	return ChangeRuntimeRole( RUNTIME_ROLE_DEACTIVATE_LISTEN_SERVER );
}

void ASRD_GNS_RuntimeHookShutdown( void )
{
	// The detours remain installed for the life of the process. Removing them
	// during DLL shutdown could leave engine calls targeting an unloaded DLL.
}

bool ASRD_GNS_RuntimeHookInstalled( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
		return false;
	const bool installed = s_state->state == HOOK_STATE_INSTALLED;
	UnlockSharedState( mutex );
	return installed;
}

unsigned int ASRD_GNS_RuntimeHookInstallCount( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
		return 0;
	const unsigned int count = (unsigned int)s_state->installCount;
	UnlockSharedState( mutex );
	return count;
}

bool ASRD_GNS_MessageRegistrationHookInstalled( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
		return false;
	const bool installed = s_state->registrationState == HOOK_STATE_INSTALLED;
	UnlockSharedState( mutex );
	return installed;
}

unsigned int ASRD_GNS_MessageRegistrationHookInstallCount( void )
{
	HANDLE mutex = NULL;
	if ( !LockSharedState( &mutex ) )
		return 0;
	const unsigned int count = (unsigned int)s_state->registrationInstallCount;
	UnlockSharedState( mutex );
	return count;
}

#else

bool ASRD_GNS_EnsureRuntimeHookInstalled( void )
{
	return false;
}

bool ASRD_GNS_EnsureServerWakeHookInstalled( void )
{
	return false;
}

void ASRD_GNS_RuntimeHookShutdown( void )
{
}

bool ASRD_GNS_RuntimeHookInstalled( void )
{
	return false;
}

unsigned int ASRD_GNS_RuntimeHookInstallCount( void )
{
	return 0;
}

bool ASRD_GNS_RuntimeHookRegisterClientConnectHandler(
	ASRD_GNS_ClientConnectIntentHandler )
{
	return false;
}

void ASRD_GNS_RuntimeHookUnregisterClientConnectHandler(
	ASRD_GNS_ClientConnectIntentHandler )
{
}

ASRD_GNS_RuntimeRole ASRD_GNS_GetRuntimeRole( void )
{
	return ASRD_GNS_RUNTIME_UNINITIALIZED;
}

bool ASRD_GNS_SetClientRuntimeRole( void )
{
	return false;
}

bool ASRD_GNS_SetDedicatedServerRuntimeRole( bool )
{
	return false;
}

bool ASRD_GNS_ActivateListenServerRuntimeRole( void )
{
	return false;
}

bool ASRD_GNS_DeactivateListenServerRuntimeRole( void )
{
	return false;
}

bool ASRD_GNS_MessageRegistrationHookInstalled( void )
{
	return false;
}

unsigned int ASRD_GNS_MessageRegistrationHookInstallCount( void )
{
	return 0;
}

#endif
