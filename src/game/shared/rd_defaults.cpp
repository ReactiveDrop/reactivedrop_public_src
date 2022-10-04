#include "cbase.h"
#include "rd_defaults.h"

// allow these changes applied when toggling
inline void ConvarChangeCallback(IConVar* pConVar, const char* pOldString, float flOldValue)
{
#ifdef CLIENT_DLL
	rd_apply_new_client_defaults();
#else
	rd_apply_new_server_defaults();
#endif
}

#ifndef CLIENT_DLL
ConVar rd_use_new_server_defaults("rd_use_new_server_defaults", "1", FCVAR_NONE, "Use newer optimized defaults for hosting games", true, 0, true, 1, ConvarChangeCallback);
#include "highres_timer.h"
#include "gameinterface.h"
extern CServerGameDLL g_ServerGameDLL;

ConVar rd_adjust_mod_dont_load_vertices("rd_adjust_mod_dont_load_vertices", "1", FCVAR_NONE, "Automatically disables loading of vertex data.", true, 0, true, 1, ConvarChangeCallback);
ConVar rd_dedicated_high_resolution_timer_ms("rd_dedicated_high_resolution_timer_ms", "0.01", FCVAR_NONE, "Acquire timer with specified resolution in ms", true, 0, true, 1, ConvarChangeCallback);

#else
ConVar rd_use_new_client_defaults("rd_use_new_client_defaults", "1", FCVAR_NONE, "Use newer optimized defaults for client", true, 0, true, 1);
#endif


inline void setConvarValue(const char* name, int value)
{
	ConVarRef convar(name);
	if (convar.IsValid()) convar.SetValue(value);
}

inline void setConvarValueFloat(const char* name, float value)
{
	ConVarRef convar(name);
	if (convar.IsValid()) convar.SetValue(value);
}

inline int getConVarValue(const char* name)
{
	ConVarRef convar(name);
	if (convar.IsValid()) return convar.GetInt();

	return -1;
}


#ifndef CLIENT_DLL

void rd_apply_new_server_defaults()
{
	// fixes a memory leak on dedicated server where model vertex data
	// is not freed on map transition and remains locked, leading to increased
	// memory usage and cache trashing over time
	if( rd_adjust_mod_dont_load_vertices.GetBool() )
	{
		ConVarRef mod_dont_load_vertices("mod_dont_load_vertices");
		mod_dont_load_vertices.SetValue(1);
	}

	// claim a high resolution timer
	// without this timer, the game would be stuck at 64 fps on newer windows versions
	highres_timer_set( rd_dedicated_high_resolution_timer_ms.GetFloat() );

	// apply newer server settings, to provide sane defaults for most cases

	// avoid disk stutter for hosts on hdd
	setConvarValue("sv_forcepreload", 1);

	// block rcon hacking
	setConvarValue("sv_rcon_banpenalty", 86400);
	setConvarValue("sv_rcon_minfailures", 2);
	setConvarValue("sv_rcon_maxfailures", 3);
	setConvarValue("sv_rcon_minfailuretime", 300);

	// ignore two player requirement
	setConvarValue("rd_override_allow_rotate_camera", 1);
	setConvarValue("rd_ready_mark_override", 1);

	// fps
	setConvarValue("rd_override_fps_max", 1);

	// rates, takes from cs:go update 
	/*
	.5 Mbps – rate 62500
	1.0 Mbps – rate 125000
	1.5 Mbps – rate 187500
	1.57 Mbps – rate 196608 (New Default)
	2.0 Mbps – rate 250000
	2.5 Mbps – rate 312500
	3.0 Mbps – rate 375000
	3.5 Mbps – rate 437500
	4.0 Mbps – rate 500000
	4.5 Mbps – rate 562500
	5.0 Mbps – rate 625000
	5.5 Mbps – rate 687500
	6.0 Mbps – rate 750000
	6.2 Mbps – rate 786432 (New Max)
	*/

	setConvarValue("sv_minrate", 62500);
	setConvarValue("sv_maxrate", 786432);
	
	// client upload rate is almost never limited, the amount of information sent is minimal
	// this should however not exceed tickrate (the amount of ticks the server processes
	setConvarValue("sv_mincmdrate", 10);
	setConvarValue("sv_maxcmdrate", round(1 / g_ServerGameDLL.GetTickInterval()));

	// client download should be determined by the client, however it should not
	// exceed tickrate * 0.999, otherwise we get choke
	setConvarValue("sv_minupdaterate", 20);

	ConMsg("tick ival: %f\n", g_ServerGameDLL.GetTickInterval());

	const float fMaxRate = 1 / g_ServerGameDLL.GetTickInterval() * 0.999f;
	setConvarValueFloat("sv_maxupdaterate", fMaxRate);

	// rates don't need to be adjusted anymore with this
	setConvarValue("rd_adjust_sv_maxrate", 0);

	// maxcleartime should be lowered, but not too low, taken from cs:go esl
	setConvarValue("net_maxcleartime", 0.1);

	// splitrate should be higher than 1 to prevent lags during events like adrenaline of high
	// burst of new created entities, however clients can be sensitive to this. A value of 2
	// is fine, but we should split further.
	const int iSplitrate = 2;
	setConvarValue("net_splitrate", iSplitrate);

	// splitpacket_rate * splitrate should be equal to maxrate
	setConvarValue("net_splitpacket_maxrate", floor(fMaxRate / iSplitrate));

	// interp ratio is a tricky one. In pracice, when a client has perfect conditions
	// (low ping, no loss, no framedrops), this can be zero. However, almost no client
	// reaches this condition. A lot of users set this to zero, but have to deal with lag
	// especially during slowdown with this.
	//
	// we leave it to the user, however as cs:go recommends, this should never be below one frame.
	setConvarValue("sv_client_min_interp_ratio", 0);
	setConvarValue("sv_client_max_interp_ratio", 5);
	setConvarValue("sv_client_predict", 1);

	// disable the restart countdown
	setConvarValue("rd_restart_mission_countdown", 0);
}

#else
void rd_apply_new_client_defaults()
{
	// if the rate somehow reverted to a minimum, set it to the minimum value cs:go accepts
	const int iMinimumRate = 62500;
	if (getConVarValue("rate") < iMinimumRate) setConvarValue("rate", iMinimumRate);

	// if cpu and gpu can handle it (read: have enough power to not drop fps), request more data than the old defaults
	if (getConVarValue("gpu_level") >= 3 && getConVarValue("cpu_level") >= 3) {

		// cmdrate can be unlocked, as it is capped by the client fps and clamped by server tickrate
		// however, only touch it, if the user hasn't increased it already. It can't pass 100 without issues.
		if (getConVarValue("cl_cmdrate") <= 30) {
			setConvarValue("cl_cmdrate", 100);
		}

		// updaterate can be raised as well, if the user hasn't touched it
		// we default it to the value that is used by the new server defaults, a server can clamp this
		if (getConVarValue("cl_updaterate") <= 30) {
			setConvarValueFloat("cl_updaterate", 1000 / DEFAULT_TICK_INTERVAL * 0.999f);
		}
	}

	// the default interp ratio makes no sense in a modern era
	// cs:go esl recommends the ratio is 2 (or under perfect conditions 1), but should never be 0.
	// zero can introduce lag under non-perfect conditions, noticable mostly under slowdown or when
	// a lot of events happen.
	// 
	// lets only touch this, if this value is default
	ConVarRef cl_interp("cl_interp");
	const bool iInterpIsDefault = round(cl_interp.GetFloat() * 10) == 1;
	if (iInterpIsDefault) {

		// because ratio is tied to updaterate, and not tickrate, we have to take this into account
		// we set ratio to zero and use interp, in case the user has lowered update rates
		setConvarValue("cl_interp_ratio", 0);
		setConvarValue("cl_interp", DEFAULT_TICK_INTERVAL * 2);
	}
}
#endif