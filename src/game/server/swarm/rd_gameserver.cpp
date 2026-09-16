#include "cbase.h"
#include "rd_gameserver.h"
#include "ws2tcpip.h"

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

inline void DisplaySteamError(const SteamErrMsg& err)
{
	const char* raw = err;                     // SteamErrMsg is a char buffer
	size_t len = strnlen(raw, sizeof(err));    // stop at first '\0'

	char clean[1024];
	size_t o = 0;

	for (size_t i = 0; i < len && o + 1 < sizeof(clean); ++i)
	{
		unsigned char c = raw[i];

		if (c >= 32 && c <= 126)               // printable ASCII only
			clean[o++] = c;
		else
			clean[o++] = ' ';                  // replace garbage
	}

	clean[o] = '\0';

	ConMsg("Steam error: %s\n", clean);
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
	ConMsg("Host IP: %s:%d (%u) [%s]\n", ip > 0 ? ipStr : "0.0.0.0", gamePort, ip, version);

	// we can spawn a gameserver immediately
	SteamErrMsg err;
	const ESteamAPIInitResult result = SteamGameServer_InitEx(ip, gamePort, clientPort, eServerModeAuthenticationAndSecure, version, &err);
	if (result != k_ESteamAPIInitResult_OK) {
		DisplaySteamError(err);
	}
	else {
		ConMsg("SteamGameServerInit resulted in success\n");
	}

	return result;
}

void GameServerCallbacks()
{
	// steam callbacks
	SteamGameServer_RunCallbacks();

	if (SteamGameServer()) {

		// login
		static bool m_bSteamApiInited = false;

		bool bLoggedIn = SteamGameServer()->BLoggedOn();
		if (!bLoggedIn) {
			SteamGameServer()->LogOnAnonymous();
		}
		else if (!m_bSteamApiInited) {

			// engine toggles this wrongfully at some point
			// because it believes the connection failed
			ConVarRef sv_lan("sv_lan");
			
			if (sv_lan.GetBool() && SteamGameServer()->BSecure())
			{
				// bSecure is set, so the only thing we need to do, is toggle sv lan back
				// engine won't touch it anymore after the initial load
				// and the game implementation of GameServerInit handles everything else
				sv_lan.SetValue(0);

				engine->ServerCommand("heartbeat\n");
				engine->ServerExecute();

				ConMsg("************************************************\n");
				ConMsg("* Connection to Steam restored.                *\n");
				ConMsg("* Server is operating in normal mode.          *\n");
				ConMsg("************************************************\n");

				m_bSteamApiInited = true;
			}
		}
	}
}

