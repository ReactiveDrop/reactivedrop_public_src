function TryCreateDefanged()
{	
	local difficulty = Convars.GetFloat( "asw_skill" );
	
	if ( difficulty <3 )
		return;
	
	if ( RandomInt( 0, ( difficulty - 2 ).tointeger() ) )
		Director.SpawnAlienAt( "asw_parasite_defanged", self.GetOrigin(), self.GetAngles() );
	
	self.Destroy();
}