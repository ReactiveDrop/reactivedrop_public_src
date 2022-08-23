g_strAlienClassnames <- [
	//"asw_drone",	//"sk_asw_drone_damage" "600" in one_hit.txt so drones still kill doors fast
	//"asw_drone_jumper",
	//"asw_drone_uber",
	//"asw_buzzer",
	//"asw_harvester",
	//"asw_queen",
	//"asw_shaman",
	//"npc_antlionguard",
	//"npc_antlionguard_cavern",
	
	"asw_shieldbug",
	"asw_boomer",
	"asw_boomer_blob",	// when boomer dies this becomes the attacker
	"asw_mortarbug",
	"asw_mortarbug_shell",	// when mortarbug dies this becomes the attacker
	"asw_ranger",
	"asw_missile_round",	// when ranger dies this becomes the attacker
	"asw_parasite_defanged",
	"asw_parasite"
];

function OnTakeDamage_Alive_Any( victim, inflictor, attacker, weapon, damage, damageType, ammoName )
{	
	if ( !victim )
		return damage;

	if ( victim.GetClassname() == "asw_marine" && IsAlien( attacker ) )
		return 1000;
	
	return damage;
}

function IsAlien( handle_attacker )
{
	if ( !handle_attacker )
		return false;
	
	local str_attacker = handle_attacker.GetClassname();
	
	foreach ( str_classname in g_strAlienClassnames )
		if ( str_attacker == str_classname )
			return true;
			
	return false;
}