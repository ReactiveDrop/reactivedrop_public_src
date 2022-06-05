function OnGameplayStart()
{
	local player = null;
	
	while( ( player = Entities.FindByClassname( player, "player" ) ) != null )
		NetProps.SetPropInt( player, "m_iDefaultFOV", 90 );
}
