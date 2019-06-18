///////////////////////////////////////////////////////////////////////////////
//
// A timer system to call a function after a certain amount of time
//
///////////////////////////////////////////////////////////////////////////////

/**
 * The Timer table allows the developer to easily add synchronized callbacks.
 */
if (!("Timers" in getroottable()))
{
	::Timers <-
	{
		TimersList = {}
		TimersID = {}
		ClockList = {}
		count = 0
	}
}

/*
 * Constants
 */

// Passable constants
getconsttable()["NO_TIMER_PARAMS"] <- null; /** No timer params */

// Internal constants
const UPDATE_RATE = 0.1; /** Fastest possible update rate */

// Flags
getconsttable()["TIMER_FLAG_KEEPALIVE"] <- (1 << 1); /** Keep timer alive even after RoundEnd is called */
getconsttable()["TIMER_FLAG_COUNTDOWN"] <- (1 << 2); /** Fire the timer the specified number of times before the timer removes itself */
getconsttable()["TIMER_FLAG_DURATION"] <- (1 << 3); /** Fire the timer each interval for the specified duration */
getconsttable()["TIMER_FLAG_DURATION_VARIANT"] <- (1 << 4); /** Fire the timer each interval for the specified duration, regardless of internal function call time loss */


/**
 * Creates a named timer that will be added to the timers list. If a named timer already exists,
 * it will be replaced.
 *
 * @return Name of the created timer
 */
function Timers::AddTimerByName(strName, delay, repeat, func, paramTable = null, flags = 0, value = {})
{
	::Timers.RemoveTimerByName(strName);
	::Timers.TimersID[strName] <- ::Timers.AddTimer(delay, repeat, func, paramTable, flags, value);
	return strName;
}

/**
 * Deletes a named timer.
 */
function Timers::RemoveTimerByName(strName)
{
	if (strName in ::Timers.TimersID)
	{
		::Timers.RemoveTimer(::Timers.TimersID[strName]);
		delete ::Timers.TimersID[strName];
	}
}

/**
 * Calls a function and passes the specified table to the callback after the specified delay.
 */
function Timers::AddTimer(delay, repeat, func, paramTable = null, flags = 0, value = {})
{
	local TIMER_FLAG_COUNTDOWN = (1 << 2);
	local TIMER_FLAG_DURATION = (1 << 3);
	local TIMER_FLAG_DURATION_VARIANT = (1 << 4);
	
	delay = delay.tofloat();
	repeat = repeat.tointeger();
	
	local rep = (repeat > 0) ? true : false;
	
	if (delay < UPDATE_RATE)
	{
		printl("Timer Warning: Timer delay cannot be less than " + UPDATE_RATE + " second(s). Delay has been reset to " + UPDATE_RATE + ".");
		delay = UPDATE_RATE;
	}
	
	if (paramTable == null)
		paramTable = {};
	
	if (typeof value != "table")
	{
		printl("Timer Error: Illegal parameter: 'value' parameter needs to be a table.");
		return -1;
	}
	else if (flags & TIMER_FLAG_COUNTDOWN && !("count" in value))
	{
		printl("Timer Error: Could not create the countdown timer because the 'count' field is missing from 'value'.");
		return -1;
	}
	else if ((flags & TIMER_FLAG_DURATION || flags & TIMER_FLAG_DURATION_VARIANT) && !("duration" in value))
	{
		printl("Timer Error: Could not create the duration timer because the 'duration' field is missing from 'value'.");
		return -1;
	}
	
	// Convert the flag into countdown
	if (flags & TIMER_FLAG_DURATION)
	{
		flags = flags & ~TIMER_FLAG_DURATION;
		flags = flags | TIMER_FLAG_COUNTDOWN;
		
		value["count"] <- floor(value["duration"].tofloat() / delay);
	}
	
	++count;
	TimersList[count] <-
	{
		_delay = delay
		_func = func
		_params = paramTable
		_startTime = Time()
		_baseTime = Time()
		_repeat = rep
		_flags = flags
		_opval = value
	}
	
	return count;
}

/**
 * Removes the specified timer.
 */
function Timers::RemoveTimer(idx)
{
	if (idx in TimersList)
		delete ::Timers.TimersList[idx];
}

/**
 * Manages timers.
 */
function Timers::ManageTimer(idx, command, value = null, allowNegTimer = false)
{
	if ( idx in ::Timers.ClockList && value == null )
	{
		::Timers.ClockList[idx]._command <- command;
		::Timers.ClockList[idx]._allowNegTimer <- allowNegTimer;
	}
	else
	{
		if ( value == null )
			value = 0;
		
		::Timers.ClockList[idx] <-
		{
			_value = value
			_startTime = Time()
			_lastUpdateTime = Time()
			_command = command
			_allowNegTimer = allowNegTimer
		}
	}
}

/**
 * Returns the value of a timer.
 */
function Timers::ReadTimer(idx)
{
	if ( idx in ::Timers.ClockList )
		return ::Timers.ClockList[idx]._value;
	
	return null;
}

/**
 * Returns a timer as a displayable string --:--.
 */
function Timers::DisplayTime(idx)
{
	function TimeToDisplayString( disp_time )
	{
		local minutes = ( disp_time / 60 ).tointeger();
		local seconds_10s = ( ( disp_time % 60) / 10 ).tointeger();
		local seconds_1s = ( disp_time % 10 ).tointeger();
		return minutes + ":" + seconds_10s + seconds_1s;
	}
	
	return "0" + TimeToDisplayString(::Timers.ReadTimer(idx));
}

/**
 * Manages all timers and provides interface for custom updates.
 */
::Timers._thinkFunc <- function()
{
	local TIMER_FLAG_COUNTDOWN = (1 << 2);
	local TIMER_FLAG_DURATION_VARIANT = (1 << 4);
	
	// current time
	local curtime = Time();
	
	// Execute timers as needed
	foreach (idx, timer in ::Timers.TimersList)
	{
		if ((curtime - timer._startTime) >= timer._delay)
		{
			if (timer._flags & TIMER_FLAG_COUNTDOWN)
			{
				timer._params["TimerCount"] <- timer._opval["count"];
				
				if ((--timer._opval["count"]) <= 0)
					timer._repeat = false;
			}
			
			if (timer._flags & TIMER_FLAG_DURATION_VARIANT && (curtime - timer._baseTime) > timer._opval["duration"])
			{
				delete ::Timers.TimersList[idx];
				continue;
			}
			
			try
			{
				if (timer._func(timer._params) == false)
					timer._repeat = false;
			}
			catch (id)
			{
				printl("Timer caught exception; closing timer " + idx + ". Error was: " + id.tostring());
				local deadFunc = timer._func;
				local params = timer._params;
				delete ::Timers.TimersList[idx];
				deadFunc(params); // this will most likely throw
				continue;
			}
			
			if (timer._repeat)
				timer._startTime = curtime;
			else
				if (idx in ::Timers.TimersList) // recheck-- timer may have been removed by timer callback
					delete ::Timers.TimersList[idx];
		}
	}
	foreach (idx, timer in ::Timers.ClockList)
	{
		if ( Time() > timer._lastUpdateTime )
		{
			local newTime = Time() - timer._lastUpdateTime;
			
			if ( timer._command == 1 )
				timer._value += newTime;
			else if ( timer._command == 2 )
			{
				if ( timer._allowNegTimer )
					timer._value -= newTime;
				else
				{
					if ( timer._value > 0 )
						timer._value -= newTime;
				}
			}
			
			timer._lastUpdateTime <- Time();
		}
	}
}

/*
 * Create a think timer
 */
if (!("_thinkTimer" in ::Timers))
{
	::Timers._thinkTimer <- Entities.CreateByClassname("info_target");
	::Timers._thinkTimer.SetName("rd_vscript_timer");
	if (::Timers._thinkTimer != null)
	{
		::Timers._thinkTimer.ValidateScriptScope();
		local scrScope = ::Timers._thinkTimer.GetScriptScope();
		scrScope["ThinkTimer"] <- ::Timers._thinkFunc;
		AddThinkToEnt(::Timers._thinkTimer, "ThinkTimer");
	}
	else
		throw "Timer Error: Timer system could not be created; Could not create dummy entity";
}
