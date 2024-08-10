//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "cdll_bounded_cvars.h"
#include "convar_serverbounded.h"
#include "tier0/icommandline.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


bool g_bForceCLPredictOff = false;

// ------------------------------------------------------------------------------------------ //
// cl_predict.
// ------------------------------------------------------------------------------------------ //

class CBoundedCvar_Predict : public ConVar_ServerBounded
{
public:
	CBoundedCvar_Predict() :
		ConVar_ServerBounded( "cl_predict",
			"1.0",
			FCVAR_USERINFO | FCVAR_CHEAT,
			"Perform client side prediction. WARNING: Turning this off will cause extreme lag and break certain interactions like melee animations." )
	{
	}

	virtual float GetFloat() const
	{
		// Used temporarily for CS kill cam.
		if ( g_bForceCLPredictOff )
			return 0;

		static const ConVar *pClientPredict = dynamic_cast< const ConVar * >( g_pCVar->FindCommandBase( "sv_client_predict" ) );
		if ( pClientPredict && pClientPredict->GetInt() != -1 )
		{
			// Ok, the server wants to control this value.
			return pClientPredict->GetFloat();
		}
		else
		{
			return GetBaseFloatValue();
		}
	}
};

static CBoundedCvar_Predict cl_predict_var;
ConVar_ServerBounded *cl_predict = &cl_predict_var;



// ------------------------------------------------------------------------------------------ //
// cl_interp_ratio.
// ------------------------------------------------------------------------------------------ //
static void WarnIfInterpRatioIsLessThan2( IConVar *pConVar, const char *szOldValue, float flOldValue )
{
	ConVarRef ref{ pConVar };
	if ( ref.GetFloat() < 2.0f )
	{
		Warning( "WARNING: Setting cl_interp_ratio to a value less than 2.0 (currently %f) may cause very bad stuttering if your connection with the server is not perfect or during slow motion.\n", ref.GetFloat() );
	}
}

class CBoundedCvar_InterpRatio : public ConVar_ServerBounded
{
public:
	CBoundedCvar_InterpRatio() :
		ConVar_ServerBounded( "cl_interp_ratio",
			"2.0",
			FCVAR_USERINFO,
			"Sets the interpolation amount (final amount is cl_interp_ratio / cl_updaterate).",
			WarnIfInterpRatioIsLessThan2 )
	{
	}

	virtual float GetFloat() const
	{
		static const ConVar *pMin = dynamic_cast< const ConVar * >( g_pCVar->FindCommandBase( "sv_client_min_interp_ratio" ) );
		static const ConVar *pMax = dynamic_cast< const ConVar * >( g_pCVar->FindCommandBase( "sv_client_max_interp_ratio" ) );
		if ( pMin && pMax && pMin->GetFloat() != -1 )
		{
			return clamp( GetBaseFloatValue(), pMin->GetFloat(), pMax->GetFloat() );
		}
		else
		{
			return GetBaseFloatValue();
		}
	}
};

static CBoundedCvar_InterpRatio cl_interp_ratio_var;
ConVar_ServerBounded *cl_interp_ratio = &cl_interp_ratio_var;


#ifdef INFESTED_DLL
CON_COMMAND_F( cl_interp, "This variable was removed and is now always set to 0.0.", FCVAR_HIDDEN )
{
	Warning( "As of September 2024, cl_interp has been removed and now always has a value of 0.0. If you need to change network interpolation for some reason, use cl_update_ratio.\n" );
}
#else
// ------------------------------------------------------------------------------------------ //
// cl_interp
// ------------------------------------------------------------------------------------------ //

class CBoundedCvar_Interp : public ConVar_ServerBounded
{
public:
	CBoundedCvar_Interp() :
		ConVar_ServerBounded( "cl_interp",
			"0.1",
			FCVAR_USERINFO,
			"Sets the interpolation amount (bounded on low side by server interp ratio settings and cl_interp_ratio).", true, 0.0f, true, 0.5f )
	{
	}

	virtual float GetFloat() const
	{
		static const ConVar_ServerBounded *pUpdateRate = dynamic_cast< const ConVar_ServerBounded * >( g_pCVar->FindCommandBase( "cl_updaterate" ) );
		static const ConVar *pMin = dynamic_cast< const ConVar * >( g_pCVar->FindCommandBase( "sv_client_min_interp_ratio" ) );
		if ( pUpdateRate && pMin && pMin->GetFloat() != -1 )
		{
			return MAX( GetBaseFloatValue(), pMin->GetFloat() / pUpdateRate->GetFloat() );
		}
		else
		{
			return GetBaseFloatValue();
		}
	}
};

static CBoundedCvar_Interp cl_interp_var;
ConVar_ServerBounded *cl_interp = &cl_interp_var;
#endif

float GetClientInterpAmount()
{
	static const ConVar_ServerBounded *pUpdateRate = dynamic_cast< const ConVar_ServerBounded * >( g_pCVar->FindCommandBase( "cl_updaterate" ) );
	if ( pUpdateRate )
	{
		// #define FIXME_INTERP_RATIO
#ifdef INFESTED_DLL
		return MAX( 0.0f, cl_interp_ratio->GetFloat() / pUpdateRate->GetFloat() );
#else
		return MAX( cl_interp->GetFloat(), cl_interp_ratio->GetFloat() / pUpdateRate->GetFloat() );
#endif
	}
	else
	{
		if ( !CommandLine()->FindParm( "-hushasserts" ) )
		{
			AssertMsgOnce( false, "GetInterpolationAmount: can't get cl_updaterate cvar." );
		}

		return 0.1;
	}
}
