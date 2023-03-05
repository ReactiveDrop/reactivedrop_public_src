//========= Copyright Valve Corporation, All rights reserved. ==============================//
//
// Purpose: Intercepts game events for VScript and call OnGameEvent_<eventname>.
//
//==========================================================================================//

#ifndef SCRIPT_GAME_EVENT_LISTENER_H
#define SCRIPT_GAME_EVENT_LISTENER_H
#ifdef _WIN32
#pragma once
#endif

#include "gameeventlistener.h"
#include <igamesystem.h>

// Used to intercept game events for VScript and call OnGameEvent_<eventname>.
class CScriptGameEventListener : public CGameEventListener, public CBaseGameSystem
{
public:
	CScriptGameEventListener();
	virtual ~CScriptGameEventListener();

	virtual void SetVScriptEventValues( IGameEvent *event, HSCRIPT table );

public: // IGameEventListener Interface
	
	void ListenForGameEvent( const char *name )
	{
		gameeventmanager->AddListener( this, name, true );
	}

	void StopListeningForAllEvents()
	{
		gameeventmanager->RemoveListener( this );
	}

	virtual void FireGameEvent( IGameEvent * event );
	
public: // CBaseGameSystem overrides

	virtual bool Init();
};

#define NUM_SCRIPT_GAME_EVENTS 156
extern const char *const g_ScriptGameEventList[NUM_SCRIPT_GAME_EVENTS];
extern CScriptGameEventListener *g_pScriptGameEventListener;

#endif
