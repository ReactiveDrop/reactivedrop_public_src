g_strAlienClassnames <- [
	//"asw_drone",	//"sk_asw_drone_damage" "600" in one_hit.txt so drones still kill doors fast
	//"asw_drone_jumper",
	//"asw_drone_uber",
	//"asw_buzzer",	// buzzer one hit killing could be too annoying
	//"asw_shaman",
	
	"asw_queen",
	"asw_queen_grabber",	// when queen dies this becomes the attacker
	"npc_headcrab",
	"npc_headcrab_black",
	"npc_headcrab_fast",
	"npc_headcrab_poison",
	"npc_antlion",
	"npc_antlion_worker",
	"grenade_spit",			// when antlion_worker dies this becomes the attacker
	"npc_antlionguard",
	"npc_antlionguard_normal",
	"npc_antlionguard_cavern",
	"npc_zombie",
	//"npc_zombie_torso",	// doesnt attack, marines only take damage when stepping on it
	"npc_fastzombie",
	//"npc_fastzombie_torso",	// doesnt attack, marines only take damage when stepping on it
	"npc_poisonzombie",
	
	"asw_harvester",
	"asw_shieldbug",
	"asw_boomer",
	"asw_boomer_blob",		// when boomer dies this becomes the attacker
	"asw_mortarbug",
	"asw_mortarbug_shell",	// when mortarbug dies this becomes the attacker
	"asw_ranger",
	"asw_missile_round",	// when ranger dies this becomes the attacker
	"asw_parasite_defanged",
	"asw_parasite"
];

const bCorrosiveSkinKills = 0;

function OnTakeDamage_Alive_Any( victim, inflictor, attacker, weapon, damage, damageType, ammoName )
{	
	if ( !victim )
		return damage;

	if ( victim.GetClassname() == "asw_marine" )
	{
		if ( !IsAlien( attacker ) )
			return damage;
	
		if ( !bCorrosiveSkinKills )
			if ( attacker.GetClassname() == "asw_mortarbug" || attacker.GetClassname() == "asw_harvester" )
				if ( attacker == inflictor )
					return damage;
		
		return 1000;
	}
	else if ( inflictor )
	{
		// reduce damage from alien's explosions to other aliens back to normal
		if ( inflictor.GetClassname() == "asw_boomer_blob" || inflictor.GetClassname() == "asw_mortarbug_shell" )
			return damage / 12;
	}
	
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
