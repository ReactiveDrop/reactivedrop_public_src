#include "cbase.h"
#include "tier0/dbg.h"
#include "asrd_gns_message_registry.h"

#include <string.h>

#if defined( _WIN32 ) && !defined( _X360 )

#include "inetmessage.h"

#include <windows.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

namespace
{
	static const DWORD kRegistryMagic = 0x4152474D; // "ARGM"
	static const DWORD kRegistryVersion = 1;
	static const int kMaxEntries = 512;

#pragma pack(push, 1)
	struct SharedEntry
	{
		LONG type;
		DWORD messageAddress;
		DWORD channelAddress;
		DWORD handlerContextAddress;
	};

	struct SharedRegistry
	{
		DWORD magic;
		DWORD version;
		LONG count;
		LONG captureCount;
		LONG lookupCount;
		SharedEntry entries[ kMaxEntries ];
	};
#pragma pack(pop)

	static HANDLE s_mapping = NULL;
	static HANDLE s_mutex = NULL;
	static SharedRegistry *s_registry = NULL;

	static void LogRegistry( const char *message )
	{
		Warning( "[ASRD-REG] %s\n", message );
	}

	static void LogRegistryf( const char *format, ... )
	{
		char buffer[ 512 ];
		va_list args;
		va_start( args, format );
		_vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
		buffer[ sizeof( buffer ) - 1 ] = '\0';
		va_end( args );
		LogRegistry( buffer );
	}

	static bool MakeObjectName( const char *prefix, char *name, size_t capacity )
	{
		if ( !prefix || !name || capacity == 0 )
			return false;
		const int written = _snprintf( name, capacity, "Local\\%s_%08lX",
			prefix, (unsigned long)GetCurrentProcessId() );
		return written > 0 && (size_t)written < capacity;
	}

	static bool OpenRegistry( void )
	{
		char mappingName[ 96 ];
		char mutexName[ 96 ];
		if ( !MakeObjectName( "ASRD_GNS_MESSAGE_REGISTRY", mappingName, sizeof( mappingName ) ) ||
			!MakeObjectName( "ASRD_GNS_MESSAGE_REGISTRY_LOCK", mutexName, sizeof( mutexName ) ) )
			return false;

		if ( !s_mapping )
		{
			s_mapping = CreateFileMappingA( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
				0, (DWORD)sizeof( SharedRegistry ), mappingName );
			if ( !s_mapping )
				return false;
		}
		if ( !s_registry )
		{
			s_registry = (SharedRegistry *)MapViewOfFile( s_mapping, FILE_MAP_ALL_ACCESS, 0, 0,
				sizeof( SharedRegistry ) );
		}
		if ( !s_registry )
			return false;
		if ( !s_mutex )
			s_mutex = CreateMutexA( NULL, FALSE, mutexName );
		return s_mutex != NULL;
	}

	static bool LockRegistry( void )
	{
		if ( !OpenRegistry() )
			return false;
		const DWORD result = WaitForSingleObject( s_mutex, 10000 );
		if ( result != WAIT_OBJECT_0 && result != WAIT_ABANDONED )
			return false;

		if ( s_registry->magic != kRegistryMagic || s_registry->version != kRegistryVersion )
		{
			memset( s_registry, 0, sizeof( *s_registry ) );
			s_registry->magic = kRegistryMagic;
			s_registry->version = kRegistryVersion;
		}
		return true;
	}

	static void UnlockRegistry( void )
	{
		if ( s_mutex )
			ReleaseMutex( s_mutex );
	}

	static void FillRegistration( const SharedEntry &entry, ASRD_GNS_MessageRegistration *out )
	{
		out->type = (int)entry.type;
		out->message = (void *)(uintptr_t)entry.messageAddress;
		out->channel = (void *)(uintptr_t)entry.channelAddress;
		// Message registration supplies a concrete INetMessage handler. The channel
		// is the stable context recorded at this boundary; processing continues to
		// use the handler embedded in the message.
		out->handlerContext = (void *)(uintptr_t)entry.handlerContextAddress;
	}
}

void ASRD_GNS_MessageRegistryCapture( void *message, void *channel )
{
	if ( !message || !channel || sizeof( void * ) != 4 )
	{
		LogRegistry( "capture skipped: invalid message/channel or non-x86 process" );
		return;
	}

	const int type = static_cast<INetMessage *>( message )->GetType();
	if ( !LockRegistry() )
	{
		LogRegistry( "capture skipped: registry state/lock unavailable" );
		return;
	}

	for ( LONG i = 0; i < s_registry->count; ++i )
	{
		SharedEntry &entry = s_registry->entries[ i ];
		if ( entry.type == type && entry.channelAddress == (DWORD)(uintptr_t)channel )
		{
			// Duplicate registrations on the same channel are unexpected. Preserve the
			// latest opaque values so the anomaly remains diagnosable.
			entry.messageAddress = (DWORD)(uintptr_t)message;
			entry.handlerContextAddress = (DWORD)(uintptr_t)channel;
			InterlockedIncrement( &s_registry->captureCount );
			UnlockRegistry();
			LogRegistryf( "capture duplicate/update type=%d message=%p channel=%p count=%ld",
				type, message, channel, (long)s_registry->count );
			return;
		}
	}

	if ( s_registry->count >= kMaxEntries )
	{
		UnlockRegistry();
		LogRegistryf( "capture dropped type=%d: registry full capacity=%d", type, kMaxEntries );
		return;
	}

	SharedEntry &entry = s_registry->entries[ s_registry->count++ ];
	entry.type = type;
	entry.messageAddress = (DWORD)(uintptr_t)message;
	entry.channelAddress = (DWORD)(uintptr_t)channel;
	entry.handlerContextAddress = (DWORD)(uintptr_t)channel;
	InterlockedIncrement( &s_registry->captureCount );
	UnlockRegistry();
}

bool ASRD_GNS_MessageRegistryLookup( int type, ASRD_GNS_MessageRegistration *out )
{
	if ( !out )
		return false;
	memset( out, 0, sizeof( *out ) );
	if ( sizeof( void * ) != 4 || !LockRegistry() )
		return false;

	InterlockedIncrement( &s_registry->lookupCount );
	for ( LONG i = 0; i < s_registry->count; ++i )
	{
		const SharedEntry &entry = s_registry->entries[ i ];
		if ( entry.type == type )
		{
			FillRegistration( entry, out );
			UnlockRegistry();
			return true;
		}
	}
	UnlockRegistry();
	LogRegistryf( "lookup miss type=%d", type );
	return false;
}

unsigned int ASRD_GNS_MessageRegistryCount( void )
{
	if ( !LockRegistry() )
		return 0;
	const unsigned int count = (unsigned int)s_registry->count;
	UnlockRegistry();
	return count;
}

unsigned int ASRD_GNS_MessageRegistryCaptureCount( void )
{
	if ( !LockRegistry() )
		return 0;
	const unsigned int count = (unsigned int)s_registry->captureCount;
	UnlockRegistry();
	return count;
}

#else

void ASRD_GNS_MessageRegistryCapture( void *, void * )
{
}

bool ASRD_GNS_MessageRegistryLookup( int, ASRD_GNS_MessageRegistration *out )
{
	if ( out )
		memset( out, 0, sizeof( *out ) );
	return false;
}

unsigned int ASRD_GNS_MessageRegistryCount( void )
{
	return 0;
}

unsigned int ASRD_GNS_MessageRegistryCaptureCount( void )
{
	return 0;
}

#endif
