PrecacheModel( "models/swarm/Ammo/ammopdw.mdl" );
PrecacheModel( "models/swarm/Ammo/ammovindicator.mdl" );
PrecacheModel( "models/swarm/Ammo/ammopistol.mdl" );
PrecacheModel( "models/swarm/Ammo/ammoshotgun.mdl" );
PrecacheModel( "models/swarm/ammo/ammoassaultrifle.mdl" );

RandomLoot_t <- 
[
	"asw_ammo_vindicator",
	"asw_ammo_vindicator",
	"asw_ammo_shotgun",
	"asw_ammo_shotgun",
	"asw_ammo_pdw",
	"asw_ammo_pdw",
	"asw_ammo_pistol",
	"asw_ammo_pistol",
	"asw_ammo_pistol",
	"asw_pickup_flares",
	"asw_pickup_flares",
	"asw_pickup_flares",
	"asw_ammo_rifle",
	"asw_ammo_rifle",
	"asw_ammo_rifle",
	"asw_pickup_medkit"
];

function SpawnRandomLoot()
{
	if ( !RandomInt( 0, 2 ) )
	{
		self.Destroy();
		
		return;
	}
	
	local strLoot = RandomLoot_t[ RandomInt( 0, RandomLoot_t.len() - 1 ) ];
	local hLoot = Entities.CreateByClassname( strLoot );
	hLoot.SetOrigin( self.GetOrigin() + Vector( 0.0, 0.0, 18.0 ) );
	hLoot.SetAngles( 0.0, RandomFloat( -180.0, 180.0 ), 0.0 );
	if ( strLoot == "asw_pickup_flares" )
	{
		NetProps.SetPropInt( hLoot, "m_iBulletsInGun", 8 );
	}
	hLoot.Spawn();
	hLoot.Activate();
	
	self.Destroy();
}
