#pragma once

#include "cbase.h"
ConVar rd_high_resolution_timer_ms ( "rd_dedicated_high_resolution_timer_ms", "1", FCVAR_NONE, "acquire a high resolution timer with specified resolution." );

static void winmm_timer_acquire_once ();
