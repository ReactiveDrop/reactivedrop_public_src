#pragma once

// The hook is process-wide and role-neutral.  Either game DLL may request the
// installation; the request is idempotent and has no transport side effects.
bool ASRD_GNS_EnsureRuntimeHookInstalled( void );
// Installs the dedicated-server-only SV_Think call-site hook.  The caller
// must have independently established dedicated-server identity.
bool ASRD_GNS_EnsureServerWakeHookInstalled( void );
void ASRD_GNS_RuntimeHookShutdown( void );
bool ASRD_GNS_RuntimeHookInstalled( void );
unsigned int ASRD_GNS_RuntimeHookInstallCount( void );

// The connect detour dispatches to the client DLL's module-local lifecycle
// state through this explicitly registered callback.  Registration is separate
// from installing the process-wide detour so server.dll cannot become the
// accidental owner of client transport state.
typedef bool (*ASRD_GNS_ClientConnectIntentHandler)( void *clientState,
	const char *endpoint, const char *secondary );
bool ASRD_GNS_RuntimeHookRegisterClientConnectHandler(
	ASRD_GNS_ClientConnectIntentHandler handler );
void ASRD_GNS_RuntimeHookUnregisterClientConnectHandler(
	ASRD_GNS_ClientConnectIntentHandler handler );

enum ASRD_GNS_RuntimeRole
{
	ASRD_GNS_RUNTIME_UNINITIALIZED = 0,
	ASRD_GNS_RUNTIME_CLIENT = 1,
	ASRD_GNS_RUNTIME_DEDICATED_SERVER = 2,
	ASRD_GNS_RUNTIME_LISTEN_SERVER = 3,
};

// Runtime role is explicit process state, independent of DLL load order and
// independent of whether the GNS transport has been activated.
ASRD_GNS_RuntimeRole ASRD_GNS_GetRuntimeRole( void );
bool ASRD_GNS_SetClientRuntimeRole( void );
// The caller must pass the result of engine->IsDedicatedServer().  A verified
// dedicated identity is authoritative over an earlier client/listen claim.
bool ASRD_GNS_SetDedicatedServerRuntimeRole( bool verifiedDedicated );
bool ASRD_GNS_ActivateListenServerRuntimeRole( void );
bool ASRD_GNS_DeactivateListenServerRuntimeRole( void );

// Message registration is a separate logical hook from the runtime hook, while
// sharing the same process-wide owner and state. Its status and install-count
// queries remain separate from the runtime hook's lifecycle.
bool ASRD_GNS_MessageRegistrationHookInstalled( void );
unsigned int ASRD_GNS_MessageRegistrationHookInstallCount( void );
