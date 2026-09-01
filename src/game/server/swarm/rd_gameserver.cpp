#include "cbase.h"
#include "rd_gameserver.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

inline uint16 GetCommandLinePort(const char* type, uint16 defaultPort = 0)
{
	uint port = 0;

	if (CommandLine()->CheckParm(type)) {
		const char* sPortStr = CommandLine()->ParmValue(type, nullptr);
		port = atoi(sPortStr);
	}

	if (port < 1024) port = defaultPort;

	return port;
}

inline const char* GetGameVersion()
{
	static char version[64] = "0.0.0.0";

	FILE* f = fopen("reactivedrop/steam.inf", "r");
	if (!f)
		return version;

	char line[256];
	while (fgets(line, sizeof(line), f))
	{
		if (strncmp(line, "PatchVersion=", 13) == 0)
		{
			strncpy(version, line + 13, sizeof(version));
			version[strcspn(version, "\r\n")] = '\0';
			break;
		}
	}

	fclose(f);
	return version;
}

bool GameServerInit()
{
	// initialize the gameserver, srcds already binds to the correct ip and ports, so we can just pass the same parameters here
	// get ip
	uint32 ip = INADDR_ANY;
	const char* ipStr = CommandLine()->ParmValue("-ip", nullptr);

	if (CommandLine()->CheckParm("-ip")) {
		struct sockaddr_in sa;
		if (inet_pton(AF_INET, ipStr, &sa.sin_addr) == 1) {
			ip = ntohl(sa.sin_addr.s_addr);
		}
	}

	// get ports
	uint16 gamePort = GetCommandLinePort("-port", 0);
	uint16 clientPort = GetCommandLinePort("-clientport", 0);

	// get version
	const char* version = GetGameVersion();

	// show some debug
	ConMsg("Using ip: %s:%d (%u) [%s]\n", ip > 0 ? ipStr : "0.0.0.0", gamePort, ip, version);

	// load steam api
	// this is done in engine, but needs to be redone at exactly this timing, to reconnect all steam interfaces
	SteamAPI_Init();

	// we can spawn a gameserver immediately
	const bool result = SteamGameServer_Init(ip, gamePort, clientPort, eServerModeAuthenticationAndSecure, version);
	ConMsg("StartGameServer_Init resulted in %s\n", result ? "success" : "failure");
	return result;
}

void GameServerCallbacks()
{
	// steam callbacks
	SteamGameServer_RunCallbacks();

	// this is normally done in engine, but not anymore
	// we need to put this here
	if (SteamGameServer()) {
		bool bLoggedIn = SteamGameServer()->BLoggedOn();
		if (!bLoggedIn) {
			SteamGameServer()->LogOnAnonymous();
		}
	}
}

