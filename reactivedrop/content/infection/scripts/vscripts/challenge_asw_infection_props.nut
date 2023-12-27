g_mapName <- GetMapName().tolower();

g_propLibrary <- {
	platform = "models/swarmprops/floor/minebridgeplatformmesh.mdl"
	pipe_short = "models/props_reduction/custom/xfuncpipe32_128.mdl"
	pipe_medium = "models/props_reduction/custom/xfuncpipe32_256.mdl"
	pipe_long = "models/props_reduction/custom/xfuncpipe32_512.mdl"
	fence = "models/props_c17/fence01a.mdl"
	fence_long = "models/props_c17/fence03a.mdl"
	ladder = "models/props/de_nuke/hr_nuke/metal_ladder_001/metal_ladder_001_256.mdl"
	crate_small = "models/swarm/crates/crate002.mdl"
	crate = "models/swarm/crates/crate001.mdl"
	crate_large = "models/swarm/crates/crate003.mdl"
	crate_metal = "models/swarmprops/barrelsandcrates/metalcrate2mesh.mdl"
}

foreach (prop, model in g_propLibrary)
{
	self.PrecacheModel(model);
}

function PropInit()
{
	PropCleanUp();

	if (g_mapName == "dm_desert")
	{
		DoEntFire( "relay_night", "Trigger", "", 0.0, null, null );

		if (Convars.GetFloat( "asw_controls" ) == 1)
		{
			SpawnProp("fence_long", Vector(463, 4114, 124), Vector(0, 90, 0));
		}

		SpawnProp("platform", Vector(570, 2654, 172), Vector(0, 0, 0));

		SpawnProp("pipe_long", Vector(286, 2366, 210), Vector(90, 0, 0));
		SpawnProp("pipe_long", Vector(286, 2400, 210), Vector(90, 0, 0));
		SpawnProp("pipe_long", Vector(286, 2434, 210), Vector(90, 0, 0));

		SpawnProp("platform", Vector(-708, 4692, 218), Vector(0, 90, 0));

		SpawnProp("fence", Vector(-84, 4002, 118), Vector(0, 146.5, 0));
		SpawnProp("fence", Vector(-74, 4286, 118), Vector(0, 146.5, 0));

		SpawnProp("fence", Vector(1600, 4516, 118), Vector(0, 90, 0));
		SpawnProp("fence", Vector(1600, 4516, 228), Vector(0, 90, 0));
		SpawnProp("fence", Vector(1664, 4906, 118), Vector(0, 90, 0));
		SpawnProp("fence", Vector(1444, 4776, 118), Vector(0, 0, 0));
		SpawnProp("fence", Vector(1444, 4776, 228), Vector(0, 0, 0));
	}

	if (g_mapName == "dm_deima")
	{
		g_killAltitude = -400;

		SpawnProp("pipe_long", Vector(304, 2176, -112), Vector(-60, -45, 0));

		SpawnProp("pipe_long", Vector(8, 1616, 72), Vector(60, 0, 0));
		SpawnProp("platform", Vector(512, 1616, 336), Vector(0, 90, 0));

		SpawnProp("platform", Vector(-1304, 1568, 120), Vector(-10, 60, 0));

		SpawnProp("crate_small", Vector(-200, 1264, 112), Vector(0, 0, 90));

		SpawnProp("ladder", Vector(-688, 1144, 48), Vector(-81, 90, 0));
		SpawnProp("ladder", Vector(-688, 624, 96), Vector(90, 90, 0));

		SpawnProp("pipe_long", Vector(-952, 304, 72), Vector(0, 0, 90));
		SpawnProp("platform", Vector(-976, -192, 88), Vector(0, 90, 0));

		SpawnProp("crate_small", Vector(32, 312, 104), Vector(0, -90, 90));

		SpawnProp("crate_large", Vector(-264, -256, 88), Vector(0, 0, 0));

		SpawnProp("crate", Vector(-968, -1076, -64), Vector(0, 0, 0));
		SpawnProp("crate_small", Vector(-992, -1188, 8), Vector(0, 90, 0));

		SpawnProp("platform", Vector(-1952, 308, 64), Vector(15, 90, 0));

		SpawnProp("fence_long", Vector(-1424, -272, 144), Vector(0, 90, 0));
		SpawnProp("fence_long", Vector(-1296, -400, 144), Vector(0, 0, 0));

		SpawnProp("fence_long", Vector(-1900, 1808, 148), Vector(0, 90, 0));

		SpawnProp("crate_large", Vector(1168, 1160, 40), Vector(90, 315, 0));

		SpawnProp("pipe_medium", Vector(1888, 2848, -88), Vector(0, 0, 45));
		SpawnProp("pipe_medium", Vector(1776, 2248, -64), Vector(0, 0, -45));
		SpawnProp("crate_large", Vector(1856, 1880, 24), Vector(-90, 0, 0));
		SpawnProp("crate_large", Vector(1760, 1848, -40), Vector(0, 90, 0));
		SpawnProp("pipe_medium", Vector(1888, 1064, -80), Vector(0, 0, -45));
	}

	if (g_mapName == "dm_residential")
	{
		g_killAltitude = -1000;

		Director.RestartMission();
	}
}

function SpawnProp(prop, vecPos, vecAng)
{
	local model = prop;
	if (prop in g_propLibrary)
	{
		model = g_propLibrary[prop];
	}
	local hProp = Entities.CreateByClassname( "prop_physics_override" );
	hProp.SetModel( model );
	hProp.SetOrigin( vecPos );
	hProp.SetAnglesVector( vecAng );
	NetProps.SetPropInt( hProp, "m_spawnflags", 11 );
	hProp.__KeyValueFromString("disableshadows", "1");
	hProp.SetName("asw_infection_prop");
	hProp.Spawn();
	hProp.Activate();
}

function PropCleanUp()
{
	local hProp = null;
	while((hProp = Entities.FindByName(hProp, "asw_infection_prop")) != null)
	{
		hProp.Destroy();
	}
}
