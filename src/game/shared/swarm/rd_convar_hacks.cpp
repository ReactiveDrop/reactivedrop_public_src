#include "cbase.h"

#ifndef CLIENT_DLL
#include "highres_timer.h"
#include "gameinterface.h"
extern CServerGameDLL g_ServerGameDLL;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_use_new_client_defaults("rd_use_new_defaults", "1", FCVAR_NONE, "Use newer (optimized) defaults", true, 0, true, 1);

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

		for ( int i = 0; i < NELEMS( s_szAddArchive ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szAddArchive[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->AddFlags( FCVAR_ARCHIVE );
		}

		applyDefaults();

		return true;
	}

	void applyDefaults()
	{
#ifdef CLIENT_DLL
		// if the rate somehow reverted to a minimum, set it to the minimum value CS:GO accepts
		const int iMinimumRate = 62500;

		ConVarRef rate("rate");
		if (rate.GetInt() < iMinimumRate) rate.SetValue(iMinimumRate);

		// if cpu and gpu can handle it (read: have enough power to not drop fps), request more data than the old defaults
		ConVarRef gpu_level("gpu_level");
		ConVarRef cpu_level("cpu_level");
		if (gpu_level.GetInt() >= 3 && cpu_level.GetInt() >= 3) {

			// cmdrate can be unlocked, as it is capped by the client fps and clamped by server tickrate
			// however, only touch it, if the user hasn't increased it already. It can't pass 100 without issues.
			ConVarRef cl_cmdrate("cl_cmdrate");
			if (cl_cmdrate.GetInt() <= 30) cl_cmdrate.SetValue(100);

			// updaterate can be raised as well, if the user hasn't touched it
			// we default it to the value that is used by the new server defaults, a server can clamp this
			ConVarRef cl_updaterate("cl_updaterate");
			if (cl_updaterate.GetInt() <= 30) cl_updaterate.SetValue(1000 / DEFAULT_TICK_INTERVAL * 0.999f);
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
			ConVarRef cl_interp_ratio("cl_interp_ratio");
			cl_interp_ratio.SetValue(0);
			cl_interp.SetValue(DEFAULT_TICK_INTERVAL * 2);
		}
#else

		// fixes a memory leak on dedicated server where model vertex data
		// is not freed on map transition and remains locked, leading to increased
		// memory usage and cache trashing over time
		ConVarRef rd_adjust_mod_dont_load_vertices("rd_adjust_mod_dont_load_vertices");
		if (rd_adjust_mod_dont_load_vertices.GetBool())
		{
			ConVarRef mod_dont_load_vertices("mod_dont_load_vertices");
			mod_dont_load_vertices.SetValue(1);
		}

		// claim a high resolution timer
		// without this timer, the game would be stuck at 64 fps on newer windows versions
		ConVarRef rd_dedicated_high_resolution_timer_ms("rd_dedicated_high_resolution_timer_ms");
		highres_timer_set(rd_dedicated_high_resolution_timer_ms.GetFloat());

		// apply newer server settings, to provide sane defaults for most cases

		// avoid disk stutter for hosts on hdd
		ConVarRef sv_forcepreload("sv_forcepreload");
		sv_forcepreload.SetValue(1);

		// block rcon hacking faster
		ConVarRef sv_rcon_banpenalty("sv_rcon_banpenalty");
		sv_rcon_banpenalty.SetValue(86400);

		ConVarRef sv_rcon_minfailures("sv_rcon_minfailures");
		sv_rcon_minfailures.SetValue(2);

		ConVarRef sv_rcon_maxfailures("sv_rcon_maxfailures");
		sv_rcon_maxfailures.SetValue(3);

		ConVarRef sv_rcon_minfailuretime("sv_rcon_minfailuretime");
		sv_rcon_minfailuretime.SetValue(600);

		// rates, takes from cs:go update 
		/*
		.5 Mbps   rate 62500
		1.0 Mbps  rate 125000
		1.5 Mbps  rate 187500
		1.57 Mbps rate 196608 (New Default)
		2.0 Mbps  rate 250000
		2.5 Mbps  rate 312500
		3.0 Mbps  rate 375000
		3.5 Mbps  rate 437500
		4.0 Mbps  rate 500000
		4.5 Mbps  rate 562500
		5.0 Mbps  rate 625000
		5.5 Mbps  rate 687500
		6.0 Mbps  rate 750000
		6.2 Mbps  rate 786432 (New Max)
		*/

		ConVarRef sv_minrate("sv_minrate");
		sv_minrate.SetValue(62500);
		
		ConVarRef sv_maxrate("sv_maxrate");
		sv_maxrate.SetValue(786432);

		// client upload rate is almost never limited, the amount of information sent is minimal
		// this should however not exceed tickrate (the amount of ticks the server processes
		// setConvarValue("sv_mincmdrate", 10);
		ConVarRef sv_maxcmdrate("sv_maxcmdrate");
		const float fMaxCmdRate = 1 / g_ServerGameDLL.GetTickInterval();
		sv_maxcmdrate.SetValue(fMaxCmdRate);

		// client download should be determined by the client, however it should not
		// exceed tickrate * 0.999, otherwise we get choke
		ConVarRef sv_minupdaterate("sv_minupdaterate");
		sv_minupdaterate.SetValue(20);

		const float fMaxRate = 1 / g_ServerGameDLL.GetTickInterval() * 0.999f;
		ConVarRef sv_maxupdaterate("sv_maxupdaterate");
		sv_maxupdaterate.SetValue(fMaxRate);

		// interp ratio is a tricky one. In pracice, when a client has perfect conditions
		// (low ping, no loss, no framedrops), this can be zero. However, almost no client
		// reaches this condition. A lot of users set this to zero, but have to deal with lag
		// especially during slowdown with this.
		//
		// we leave it to the user, however as cs:go recommends, this should never be below one frame.
		ConVarRef sv_client_min_interp_ratio("sv_client_min_interp_ratio");
		sv_client_min_interp_ratio.SetValue(0);

		ConVarRef sv_client_max_interp_ratio("sv_client_max_interp_ratio");
		sv_client_max_interp_ratio.SetValue(3);

		ConVarRef sv_client_predict("sv_client_predict");
		sv_client_predict.SetValue(1);

#endif

		// maxcleartime should be lowered, but not too low, taken from cs:go esl
		ConVarRef net_maxcleartime("net_maxcleartime");
		net_maxcleartime.SetValue(0.1f);


		// splitrate should be higher than 1 to prevent lags during events like adrenaline of high
		// burst of new created entities, however clients can be sensitive to this. A value of 2
		// is fine, but we should not split further, not all clients have good connections.
		const int iSplitrate = 2;
		ConVarRef net_splitrate("net_splitrate");
		net_splitrate.SetValue(iSplitrate);

		// splitpacket_rate * splitrate should be equal to maxrate
		ConVarRef net_splitpacket_maxrate("net_splitpacket_maxrate");
		const float fSplitMaxRate = floor(fMaxRate / iSplitrate);
		net_splitpacket_maxrate.SetValue(fSplitMaxRate);

		Msg("Settings applied.\n");
	}

} s_RD_Convar_Hacks;
