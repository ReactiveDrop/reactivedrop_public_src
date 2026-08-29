#pragma once

// The pointer members are opaque at this ABI boundary; this declaration exposes
// no layout, ownership, or individual lifetime for their targets.
// They are process-local and must be accessed only while the current game
// process is running, on the engine/game thread.
struct ASRD_GNS_MessageRegistration
{
	int type;
	void *message;
	void *channel;
	void *handlerContext;
};

void ASRD_GNS_MessageRegistryCapture( void *message, void *channel );
bool ASRD_GNS_MessageRegistryLookup( int type, ASRD_GNS_MessageRegistration *out );
unsigned int ASRD_GNS_MessageRegistryCount( void );
unsigned int ASRD_GNS_MessageRegistryCaptureCount( void );
