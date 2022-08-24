function Init()
{
	AddThinkToEnt( self, "_Think" );
	_Think();
}

function _Think()
{
	if ( Entities.FindByClassnameNearest( "asw_marine", self.GetOrigin(), 1400.0 ) == null )
		DoEntFire( "counter_health", "Subtract", "1", 0, null, null );
	
	return 1.0;
}