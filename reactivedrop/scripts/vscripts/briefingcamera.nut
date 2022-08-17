/*
	How to use this script for your map:
	
		create a rd_briefing_camera entity anywhere, the position does not matter because it will be teleported
		name it camera_briefing
		in Entity Scripts field, add: briefingcamera.nut
		
		create a logic_auto entity and execute the InitCamera function when the map spawns, you do that with a output in logic_auto entity: OnMapSpawn camera_briefing RunScriptCode InitCamera()
		
		place info_target entities in your map with the name format like this: camtarget_<number of the target>_<camera_fov>
		note: camera_fov is optional to add, by default it will be 75.
		examples of some names: camtarget_1_75, camtarget_2_80, camtarget_3, camtarget_4_110, camtarget_5. In this case camera will be teleporting between those 5 places
		
		place the info_target entities in spots where you want the camera to teleport to
		make the info_target entities have the angles (pitch, yaw, roll) that you want, the camera will swap to those angles when it is teleported there
	
	Function parameters:
	
		calling InitCamera() without parameters will make it teleport every 3 seconds.
		calling InitCamera(5) will make it teleport every 5 seconds.
		calling InitCamera(5, 10) will make it teleport at a random number of seconds between 5 and 10
		
		by default camera teleports randomly between places, it could teleport from 2nd to 1st to 5th to 3rd to 5th to 1st to 4th etc., without a pattern
		if you want the camera to teleport to places in order (from 1st to 2nd to 3rd to 4th to 5th to 1st to 2nd to 3rd etc.), then you need to add a 3rd parameter: false
		examples: InitCamera( 0, 0, false ) will make it teleport every 3 seconds 
		InitCamera( 5, 0, false ) will make it teleport every 5 seconds
		InitCamera( 5, 10, false ) will make it teleport at a random number of seconds between 5 and 10
*/
g_places_t <- [];
g_next_place <- 0;

g_min_delay <- 0.0;
g_max_delay <- 0.0;
g_random_targets <- true;

function InitCamera( min_delay = 0.0, max_delay = 0.0, random_targets = true )
{	
	if ( typeof( min_delay ) != "integer" && typeof( min_delay ) != "float" )
		return;
		
	if ( typeof( max_delay ) != "integer" && typeof( max_delay ) != "float" )
		return;
	
	local hTarget = null;
	while ( hTarget = Entities.FindByClassname( hTarget, "info_target" ) )
	{
		if ( GetPlaceNumber( hTarget ) < 0 )
			continue;
		
		g_places_t.push( hTarget );
	}
	
	if ( g_places_t.len() < 1 )
		return;
	
	if ( g_places_t.len() == 1 )
	{
		ChangePlace( g_places_t[0] );
		return;
	}
	
	// sort the places in order of their numbers
	local reference = g_places_t;
	local numplaces = g_places_t.len();
	for ( local i = 0; i < numplaces - 1; ++i )
	{
		for ( local j = i + 1; j < numplaces; ++j )
		{
			if ( GetPlaceNumber( reference[j] ) < GetPlaceNumber( reference[i] ) )
			{	
				local temp = reference[j];
				reference[j] = reference[i];
				reference[i] = temp;
			}
		}
	}
	g_places_t <- reference;
	
	if ( min_delay < 0.5 )
		min_delay = 3.0;
		
	if ( max_delay <= min_delay )
		max_delay = 0.0;
	
	g_min_delay <- min_delay;
	g_max_delay <- max_delay;
	g_random_targets <- random_targets;
	
	AddThinkToEnt( self, "OnCameraThink" );
	OnCameraThink();
}

function OnCameraThink()
{
	if ( g_random_targets )
	{
		local next_place = g_next_place;
		
		// make sure next place is not the same place as before
		while ( next_place == g_next_place )
			next_place = RandomInt( 0, g_places_t.len() - 1 );
			
		g_next_place <- next_place;
	}
	else
	{
		if ( g_next_place + 1 >= g_places_t.len() )
			g_next_place <- 0;
		else
			g_next_place++;
	}
	
	ChangePlace( g_places_t[ g_next_place ] );
	
	return g_max_delay == 0.0 ? g_min_delay : RandomFloat( g_min_delay, g_max_delay );
}

function ChangePlace( hTarget )
{
	self.SetOrigin( hTarget.GetOrigin() );
	self.SetAnglesVector( hTarget.GetAngles() );
	
	self.__KeyValueFromFloat( "fov" , GetFOV( hTarget ) );
}

function GetPlaceNumber( hTarget )
{
	local str_targetname = hTarget.GetName();
	if ( str_targetname.len() < 10 )
		return -1;
		
	local str_array = split( str_targetname, "_" );
	if ( str_array.len() < 2 )
		return -1;
			
	if ( str_array[0] != "camtarget" )
		return -1;
		
	return str_array[1].tointeger();
}

function GetFOV( hTarget )
{
	local str_array = split( hTarget.GetName(), "_" );
	
	return str_array.len() <3 ? 75.0 : str_array[2].tofloat();
}