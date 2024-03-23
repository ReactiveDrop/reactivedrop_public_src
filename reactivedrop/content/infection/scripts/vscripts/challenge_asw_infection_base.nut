IncludeScript("challenge_asw_infection_props");

g_enabled <- false;
g_marineTracker <- {};
g_hud <- {};
g_teamHuman <- {};
g_teamZombie <- {};
g_primeZombies <- {};
g_prime <- {};
g_lastStand <- false;
g_lastHuman <- {};
g_matchLength <- 1200;
g_matchTimer <- -1;
g_idleTimer <- 0;
g_previous <- {};
g_victoryStatus <- -1;
g_killAltitude <- false;

g_statTrack <- {};

g_alienClassnames <- [
	"asw_drone",
	"asw_buzzer",
	"asw_parasite",
	"asw_shieldbug",
	"asw_drone_jumper",
	"asw_harvester",
	"asw_parasite_defanged",
	"asw_queen",
	"asw_boomer",
	"asw_ranger",
	"asw_mortarbug",
	"asw_shaman",
	"asw_drone_uber",
	"npc_antlionguard_normal",
	"npc_antlionguard_cavern"
];

g_zombieSound <- {
	pain = [
		"npc/zombie/zombie_pain1.wav",
		"npc/zombie/zombie_pain2.wav",
		"npc/zombie/zombie_pain3.wav",
		"npc/zombie/zombie_pain4.wav",
		"npc/zombie/zombie_pain5.wav"
	]
	die = [
		"npc/zombie/zombie_die1.wav",
		"npc/zombie/zombie_die3.wav"
	]
	attack = [
		"npc/zombie/zo_attack1.wav",
		"npc/zombie/zo_attack2.wav"
	]
	claw = [
		"npc/zombie/claw_strike1.wav",
		"npc/zombie/claw_strike2.wav",
		"npc/zombie/claw_strike3.wav"
	]
	alert = [
		"npc/zombie/zombie_alert1.wav",
		"npc/zombie/zombie_alert2.wav",
		"npc/zombie/zombie_alert3.wav"
	]
	hit = [
		"npc/zombie/zombie_hit.wav"
	]
};

foreach (category, soundList in g_zombieSound)
{
	foreach (soundScript in soundList)
	{
		self.PrecacheSoundScript(soundScript);
	}
}
self.PrecacheModel("models/swarm/marine/infected_marine.mdl");
self.PrecacheModel("models/items/shield_bubble/shield_bubble.mdl");

function Credit()
{
	ClientPrint(null, 2, " ");
	ClientPrint(null, 2, "==========");
	ClientPrint(null, 2, "#asw_infection_credit_script");
	ClientPrint(null, 2, "#asw_infection_credit_model");
	ClientPrint(null, 2, "==========");
	ClientPrint(null, 2, " ");
}

function OnEnable()
{
	Credit();
	PropInit();
	local hHud = null;
	while((hHud = Entities.FindByClassname(hHud, "rd_hud_vscript")) != null)
	{
		hHud.Destroy();
	}
}

function Update()
{
	if (!(g_enabled))
	{
		OnEnable();
		g_enabled = true;
	}
	Convars.ExecuteConCommand( "rd_TeamDeathmatch_enable" );
	foreach (hPlayer, stat in g_statTrack)
	{
		if (!(hPlayer.IsValid()))
		{
			g_statTrack.rawdelete(hPlayer);
		}
	}
	CleanUp();
	if (g_matchTimer > 0 && g_matchTimer < g_matchLength)
	{
		Convars.SetValue( "rd_respawn_time", 5.0 );
	}
	else
	{
		Convars.SetValue( "rd_respawn_time", 1.0 );
	}
	if (g_lastStand)
	{
		Convars.SetValue( "rd_paint_marine_blips", 3 );
	}
	else
	{
		Convars.SetValue( "rd_paint_marine_blips", 2 );
	}
	foreach (hPlayer, hMarine in g_primeZombies)
	{
		if (!(hPlayer.IsValid()))
		{
			g_primeZombies.rawdelete(hPlayer);
		}
		else
		{
			g_primeZombies[hPlayer] = hPlayer.GetMarine();
			if (!(g_primeZombies[hPlayer] in g_prime))
			{
				BecomePrime(g_primeZombies[hPlayer]);
			}
		}
	}
	foreach (hMarine, reduction in g_prime)
	{
		if (hMarine.IsValid())
		{
			if (reduction[0] <= 0)
			{
				g_prime[hMarine][0] = 0;
				if (g_prime[hMarine][3])
				{
					g_prime[hMarine][3].Destroy();
					g_prime[hMarine][3] = null;
				}
			}
			else
			{
				if (!(g_prime[hMarine][3]))
				{
					local hBubble = Entities.CreateByClassname( "prop_dynamic" );
					hBubble.SetModel( "models/items/shield_bubble/shield_bubble.mdl" );
					hBubble.SetOrigin( hMarine.GetOrigin() + Vector(0, 0, 30) );
					hBubble.SetParent( hMarine );
					hBubble.SetModelScale(0.3, 0);
					hBubble.Spawn();
					hBubble.Activate();
					g_prime[hMarine][3] = hBubble;
				}
			}
			if (reduction[1] > 0)
			{
				g_prime[hMarine][1] = g_prime[hMarine][1] - 1;
			}
			else
			{
				if (reduction[0] < reduction[2])
				{
					g_prime[hMarine][0] = g_prime[hMarine][0] + reduction[2]*0.05;
					if (g_prime[hMarine][0] > reduction[2])
					{
						g_prime[hMarine][0] = reduction[2];
					}
				}
			}
		}
		else
		{
			g_prime.rawdelete(hMarine);
		}
	}
	local hPlayer = null;
	while((hPlayer = Entities.FindByClassname(hPlayer, "player")) != null)
	{
		if (!(hPlayer in g_marineTracker))
		{
			g_marineTracker[hPlayer] <- null;
		}
		if (!(hPlayer in g_hud))
		{
			local hHud = Entities.CreateByClassname( "rd_hud_vscript" );
			hHud.__KeyValueFromString( "client_vscript", "challenge_asw_infection_hud.nut" );
			hHud.Spawn();
			hHud.Activate();
			hHud.SetEntity(0, hPlayer);
			g_hud[hPlayer] <- hHud;
		}
		if (g_lastStand)
		{
			local hMarine = hPlayer.GetMarine();
			if (hMarine && hMarine in g_teamHuman && !(hMarine in g_lastHuman))
			{
				local nButtons = NetProps.GetPropInt( hPlayer, "m_nButtons" );
				if (nButtons & 16384)
				{
					UseLastStand(hMarine);
				}
				else if (g_matchTimer > 0 && g_matchTimer < g_matchLength)
				{
					ClientPrint(hPlayer, 4, "#asw_infection_lastStand_key");
				}
				else
				{
					ClientPrint(hPlayer, 4, " ");
				}
			}
			else
			{
				ClientPrint(hPlayer, 4, " ");
			}
		}
		UpdateStatTrack(hPlayer);
	}
	foreach (hPlayer, hHud in g_hud)
	{
		if (!(hPlayer.IsValid()))
		{
			hHud.Destroy();
			g_hud.rawdelete(hPlayer);
		}
		else
		{
			local time = UnitToTime(g_matchTimer);
			hHud.SetInt(0, time[2]);
			hHud.SetInt(1, time[1]);
			if (g_matchTimer <= 0 && g_idleTimer > 0)
			{
				if (g_victoryStatus == 1)
				{
					hHud.SetInt(2, 2);
				}
				else
				{
					hHud.SetInt(2, 1);
				}
			}
			else
			{
				hHud.SetInt(2, 0);
			}
			hHud.SetInt(3, g_teamHuman.len());
			hHud.SetInt(4, g_teamZombie.len());
			if (g_matchTimer > g_matchLength)
			{
				hHud.SetInt(5, (((g_matchTimer-g_matchLength)/10).tointeger()+1));
			}
			else
			{
				hHud.SetInt(5, 0);
			}
			local hMarine = hPlayer.GetMarine();
			if (!(hMarine))
			{
				hMarine = hPlayer.GetViewNPC();
			}
			if (hMarine)
			{
				if (hMarine in g_teamHuman)
				{
					hHud.SetInt(6, 2);
					hHud.SetFloat(0, GetRage());
				}
				else if (hMarine in g_teamZombie)
				{
					hHud.SetInt(6, 1);
					if (hMarine in g_prime)
					{
						hHud.SetFloat(0, g_prime[hMarine][0]);
						hHud.SetFloat(1, g_prime[hMarine][2]);
						local nearest = GetNearestHuman(hMarine.GetOrigin());
						if (nearest)
						{
							local dist = (nearest.GetOrigin() - hMarine.GetOrigin()).Length().tointeger();
							hHud.SetInt(7, dist);
						}
					}
					else
					{
						hHud.SetInt(7, -1);
						hHud.SetFloat(0, 0);
						hHud.SetFloat(1, 0);
					}
				}
				else
				{
					hHud.SetInt(6, 0);
				}
			}
			else
			{
				hHud.SetInt(6, 0);
			}
		}
	}
	foreach (hPlayer, hMarine in g_marineTracker)
	{
		if (!(hPlayer.IsValid()))
		{
			g_marineTracker.rawdelete(hPlayer);
		}
		else
		{
			local hCheck = hPlayer.GetMarine();
			if (hMarine != hCheck)
			{
				g_marineTracker[hPlayer] = hCheck;
				if (hCheck)
				{
					Respawned(hCheck);
				}
			}
		}
	}
	if (g_matchTimer < 0)
	{
		if (CountMarine() >= 2)
		{
			StartNewRound();
			ClientPrint(null, HUD_PRINTTALK, " ");
			ClientPrint(null, HUD_PRINTTALK, " ");
			ClientPrint(null, HUD_PRINTTALK, " ");
			ClientPrint(null, HUD_PRINTTALK, " ");
			Credit();
			ClientPrint(null, HUD_PRINTTALK, "#asw_infection_newRound");
		}
	}
	if (g_matchTimer > 0)
	{
		if (g_matchTimer < g_matchLength)
		{
			if (g_teamHuman.len() <= 0)
			{
				StopRound();
			}
			else
			{
				if (!(g_lastStand) && (CountMarine() > 4) && (g_teamHuman.len() <= g_prime.len() || g_teamHuman.len() <= g_primeZombies.len()))
				{
					LastStand();
				}
			}
		}
	}
	else
	{
		if (g_idleTimer <= 0)
		{
			ClientPrint(null, HUD_PRINTCENTER, "#asw_infection_waiting");
		}
	}
	local hMarine = null;
	while((hMarine = Entities.FindByClassname(hMarine, "asw_marine")) != null)
	{
		if (hMarine in g_teamHuman)
		{
			hMarine.SetTeam(TEAM_BETA);
			if (g_lastStand && !(hMarine.IsInhabited()))
			{
				UseLastStand(hMarine);
			}
			if (g_matchTimer >= g_matchLength - 50)
			{
				Heal(hMarine, hMarine.GetMaxHealth() - hMarine.GetHealth());
			}
		}
		else if (hMarine in g_teamZombie)
		{
			hMarine.SetTeam(TEAM_ALPHA);
			if (GetSlotWeapon(hMarine, 0) != g_teamZombie[hMarine][1])
			{
				hMarine.DropWeapon(0);
				if (g_teamZombie[hMarine][1].IsValid())
				{
					g_teamZombie[hMarine][1].Destroy();
				}
				hMarine.GiveWeapon("asw_weapon_chainsaw", 0);
				g_teamZombie[hMarine][1] = GetSlotWeapon(hMarine, 0);
			}
			g_teamZombie[hMarine][1].__KeyValueFromInt( "rendermode", 10 );
			hMarine.DropWeapon(1);
			if (GetSlotWeapon(hMarine, 2) != g_teamZombie[hMarine][2])
			{
				hMarine.DropWeapon(2);
				if (g_teamZombie[hMarine][2].IsValid())
				{
					hMarine.GiveWeapon("asw_weapon_grenades", 2);
					GetSlotWeapon(hMarine, 2).SetClip1(g_teamZombie[hMarine][2].Clip1());
					g_teamZombie[hMarine][2].Destroy();
				}
				g_teamZombie[hMarine][2] = GetSlotWeapon(hMarine, 2);
			}
			local recharge = 200;
			if (hMarine in g_prime)
			{
				recharge = 100;
			}
			if ((g_teamZombie[hMarine][2] == null || (g_teamZombie[hMarine][2] && g_teamZombie[hMarine][2].Clip1() < 2)))
			{
				g_teamZombie[hMarine][3] = g_teamZombie[hMarine][3] - 1;
				if (g_teamZombie[hMarine][3] <= 0)
				{
					if (g_teamZombie[hMarine][2])
					{
						g_teamZombie[hMarine][2].SetClip1(g_teamZombie[hMarine][2].Clip1()+1);
					}
					else
					{
						hMarine.GiveWeapon("asw_weapon_grenades", 2);
						GetSlotWeapon(hMarine, 2).SetClip1(1);
						g_teamZombie[hMarine][2] = GetSlotWeapon(hMarine, 2);
					}
					g_teamZombie[hMarine][3] = recharge;
				}
			}
			else
			{
				g_teamZombie[hMarine][3] = recharge;
			}
			if (g_teamZombie[hMarine][4] > 0)
			{
				g_teamZombie[hMarine][4] = g_teamZombie[hMarine][4] - 1;
			}
			else
			{
				Heal(hMarine, hMarine.GetMaxHealth()*0.01);
			}
		}
		else
		{
			if (g_matchTimer >= g_matchLength)
			{
				JoinHuman(hMarine);
			}
			else
			{
				JoinZombie(hMarine);
			}
		}
		local hWeapon = NetProps.GetPropEntity(hMarine, "m_hActiveWeapon");
		if (hWeapon)
		{
			if (hWeapon.GetMaxClips() > 0)
			{
				hWeapon.SetClips(1);
			}
			if (hWeapon.GetClassname() == "asw_weapon_medrifle")
			{
				hWeapon.SetClip2(0);
			}
		}
		if (g_killAltitude != false)
		{
			if (hMarine.GetOrigin().z < g_killAltitude)
			{
				hMarine.TakeDamage(hMarine.GetMaxHealth()*5, 0, null);
			}
		}
	}
	local hGrenade = null;
	while((hGrenade = Entities.FindByClassname(hGrenade, "asw_grenade_cluster")) != null)
	{
		local hOwner = hGrenade.GetOwner();
		if (hOwner && hOwner in g_teamZombie && hGrenade.GetName() != "asw_infection_zgrenade")
		{
			hGrenade.SetName("asw_infection_zgrenade");
			local parasiteProp = Entities.CreateByClassname("prop_dynamic");
			parasiteProp.__KeyValueFromString("model", "models/aliens/parasite/parasite.mdl");
			EntFireByHandle(parasiteProp, "SetBodyGroup", "1", 0, self, self);
			parasiteProp.__KeyValueFromString("DefaultAnim", "Infest_Attack");
			parasiteProp.__KeyValueFromInt("DisableBoneFollowers", 1);
			parasiteProp.__KeyValueFromInt("solid", 0);
			parasiteProp.__KeyValueFromInt("disableshadows", 1);
			parasiteProp.Spawn();
			parasiteProp.SetOrigin(hGrenade.GetOrigin() + Vector(0, 0, -8));
			parasiteProp.SetOwner(hGrenade);
			parasiteProp.SetParent(hGrenade);
			hGrenade.__KeyValueFromInt("rendermode", 10);
		}
	}
	local hSentry = null;
	while ((hSentry = Entities.FindByClassname(hSentry, "asw_sentry_base")) != null)
	{
		hSentry.Destroy();
	}
	foreach (name in g_alienClassnames)
	{
		local hAlien = null;
		while ((hAlien = Entities.FindByClassname(hAlien, name)) != null)
		{
			hAlien.Destroy();
		}
	}
	if (g_teamHuman.len() > CountMarine())
	{
		foreach (hMarine, weapons in g_teamHuman)
		{
			if (g_teamHuman[hMarine][0].IsValid())
			{
				g_teamHuman[hMarine][0].Destroy();
			}
		}
		g_teamHuman.clear();
	}
	if (g_teamZombie.len() > CountMarine() - g_teamHuman.len())
	{
		foreach (hMarine, weapons in g_teamZombie)
		{
			if (g_teamZombie[hMarine][0].IsValid())
			{
				g_teamZombie[hMarine][0].Destroy();
			}
		}
		g_teamZombie.clear();
	}
	foreach (hMarine, weapons in g_teamHuman)
	{
		if (!(hMarine && hMarine.IsValid() && hMarine.GetHealth() > 0))
		{
			g_teamHuman.rawdelete(hMarine);
		}
	}
	foreach (hMarine, weapons in g_teamZombie)
	{
		if (!(hMarine && hMarine.IsValid() && hMarine.GetHealth() > 0))
		{
			g_teamZombie.rawdelete(hMarine);
		}
	}
	foreach (hMarine, hStatus in g_lastHuman)
	{
		if (!(hMarine.IsValid()))
		{
			g_lastHuman.rawdelete(hMarine);
		}
		else
		{
			for (local i = 0; i < 2; i++)
			{
				local hWeapon = GetSlotWeapon(hMarine, i);
				if (hWeapon)
				{
					if (hWeapon.GetClassname() != "asw_weapon_chainsaw")
					{
						hMarine.DropWeapon(i);
					}
				}
			}
			if (hStatus[0] > 0 || !(ZombieNearby(hMarine)))
			{
				hMarine.CureInfestation();
				if (ZombieNearby(hMarine))
				{
					g_lastHuman[hMarine][0] = hStatus[0]-1;
				}
				else
				{
					g_lastHuman[hMarine][0] = 70;
				}
			}
			else
			{
				if (g_matchTimer > 0)
				{
					hMarine.BecomeInfested();
					hMarine.TakeDamage(hMarine.GetMaxHealth()*0.005, 33554432, null);
				}
			}
		}
	}
	if (g_matchTimer > 0)
	{
		g_matchTimer = g_matchTimer - 1;
		if (g_matchTimer == 0)
		{
			StopRound();
		}
		if (g_matchTimer == g_matchLength)
		{
			local l = [];
			local lt = [];
			foreach (hMarine, weapons in g_teamHuman)
			{
				lt.append(hMarine);
				if (!(hMarine.IsInhabited() && hMarine.GetCommander() in g_previous))
				{
					l.append(hMarine);
				}
			}
			g_previous.clear();
			local count = 1;
			if (CountMarine() >= 6)
			{
				count = 2;
			}
			for (local i = 0; i < count; i++)
			{
				local picker = l;
				if (picker.len() <= 1)
				{
					picker = lt;
				}
				local random = RandomInt(0, picker.len()-1);
				JoinZombie(picker[random]);
				g_teamZombie[picker[random]][2].SetClip1(2);
				BecomePrime(picker[random]);
				if (picker[random].IsInhabited())
				{
					local hPlayer = picker[random].GetCommander();
					g_previous[hPlayer] <- 0;
				}
				PlayZombieSound(picker[random], "alert");
			}
		}
	}
	if (g_idleTimer > 0)
	{
		g_idleTimer = g_idleTimer - 1;
		if (g_idleTimer == 0)
		{
			ResetGame();
		}
	}
	return 0.1;
}

function OnTakeDamage_Alive_Any( victim, inflictor, attacker, weapon, damage, damageType, ammoName )
{
	if ( !victim )
	{
		return damage;
	}
	if ( g_matchTimer <= 0 && attacker && attacker != victim )
	{
		return 0;
	}
	if ( victim in g_teamZombie && attacker && attacker in g_teamHuman )
	{
		if (attacker in g_lastHuman && g_lastHuman[attacker][0] < 40)
		{
			g_lastHuman[attacker][0] = 40;
		}
		g_teamZombie[victim][4] = GetRegenCD(victim);
		damage = damage * (1.0 + GetRage());
		if (NetProps.GetPropBool(victim, "m_bElectroStunned"))
		{
			damage = damage * 2;
		}
		if (inflictor)
		{
			if (attacker in g_lastHuman && inflictor == attacker && damageType && damageType == 128)
			{
				damage = victim.GetHealth()*5;
			}
			local dir = victim.GetOrigin() - inflictor.GetOrigin();
			local kb = dir * (damage/dir.Length()*20);
			if (inflictor.GetClassname() == "asw_rocket")
			{
				kb = kb * 50;
			}
			if (weapon && weapon.GetClassname() == "asw_weapon_railgun")
			{
				kb = kb * 5;
			}
			if (IsOnGround(victim))
			{
				kb = kb + Vector(0, 0, 10);
			}
			victim.SetVelocity(victim.GetVelocity() + (kb - Vector(0, 0, victim.GetVelocity().z))*GetResistMod(victim));
			if (inflictor.GetClassname() == "asw_grenade_cluster" && !(weapon && weapon.GetClassname() == "asw_weapon_grenade_launcher"))
			{
				damage = damage * 10;
			}
			if (inflictor.GetClassname() == "asw_rifle_grenade")
			{
				damage = damage * 5;
			}
			if (inflictor.GetClassname() == "asw_laser_mine")
			{
				damage = damage * 10;
			}
			if (inflictor.GetClassname() == "asw_radiation_volume")
			{
				damage = damage * 10;
			}
			if (inflictor.GetClassname() == "asw_burning")
			{
				damage = damage * 10;
			}
			if (inflictor.GetClassname() == "asw_rocket")
			{
				damage = damage * 5;
			}
		}
		if (weapon && weapon.GetClassname() == "asw_weapon_chainsaw")
		{
			damage = 300 * (damage / 30.5);
			if (!(attacker in g_lastHuman))
			{
				damage = damage*2;
			}
		}
	}
	if ( victim in g_teamHuman && attacker && attacker in g_teamZombie )
	{
		if (!(victim in g_lastHuman) && weapon && weapon.GetClassname() == "asw_weapon_chainsaw")
		{
			damage = 0;
			JoinZombie(victim);
			ClientPrint(null, HUD_PRINTTALK, "#asw_infection_infected", NameFeed(victim), NameFeed(attacker));
			PlayZombieSound(attacker, "claw");
			PlayZombieSound(attacker, "attack");
			Deathmatch.SetKills(attacker, Deathmatch.GetKills(attacker)+1);
			Deathmatch.IncreaseKillingSpree(attacker);
			if (victim.IsInhabited())
			{
				UpdateStatMarine(attacker, 0);
			}
		}
		else if (inflictor && inflictor == attacker && damageType && damageType == 128)
		{
			if (victim in g_lastHuman)
			{
				damage = damage*5;
			}
			else
			{
				damage = victim.GetHealth()*5;
			}
		}
	}
	if ( attacker && attacker in g_teamZombie && (victim in g_teamHuman || victim == attacker) && inflictor && inflictor.GetClassname() == "asw_grenade_cluster" )
	{
		local dir = victim.GetOrigin() - inflictor.GetOrigin();
		local m = 10;
		if (victim == attacker)
		{
			m = 15;
		}
		local kb = dir * (damage/dir.Length()*m);
		if (IsOnGround(victim))
		{
			kb = kb + Vector(0, 0, 500);
		}
		victim.SetVelocity(victim.GetVelocity() - Vector(0, 0, victim.GetVelocity().z) + kb);
	}
	if ( victim in g_prime && attacker && attacker in g_teamHuman )
	{
		g_prime[victim][1] = 30;
		if (g_prime[victim][0] > 0)
		{
			PlayZombieSound(victim, "hit");
			if (damage < g_prime[victim][0])
			{
				g_prime[victim][0] = g_prime[victim][0] - damage;
			}
			else
			{
				g_prime[victim][0] = 0;
			}
			damage = 0;
		}
	}
	if ( victim in g_teamZombie )
	{
		if (damage > 0)
		{
			PlayZombieSound(victim, "pain");
		}
	}
	return damage;
}

function OnGameEvent_entity_killed( params )
{
	local victim = EntIndexToHScript( params["entindex_killed"] );
	local attacker = null;
	if ( "entindex_attacker" in params )
	{
		attacker = EntIndexToHScript( params["entindex_attacker"] );
	}
	if ( !victim )
	{
		return;
	}
	if (victim in g_teamHuman)
	{
		if (g_teamHuman[victim][0].IsValid())
		{
			g_teamHuman[victim][0].Destroy();
		}
		if (victim.IsInhabited() && attacker && attacker in g_teamZombie && attacker != victim)
		{
			UpdateStatMarine(attacker, 0);
		}
	}
	if (victim in g_teamZombie)
	{
		if (attacker && attacker in g_lastHuman)
		{
			g_lastHuman[attacker][0] = 70;
		}
		if (g_teamZombie[victim][0].IsValid())
		{
			g_teamZombie[victim][0].Destroy();
		}
		PlayZombieSound(victim, "die");
		if (victim.IsInhabited() && attacker && attacker in g_teamHuman)
		{
			UpdateStatMarine(attacker, 1);
		}
	}
}

function Respawned(hMarine)
{
	if (g_matchTimer >= g_matchLength)
	{
		JoinHuman(hMarine);
	}
	else
	{
		JoinZombie(hMarine);
		PlayZombieSound(hMarine, "alert");
	}
}

function StartNewRound()
{
	ClientPrint(null, HUD_PRINTCENTER, " ");
	g_matchTimer = g_matchLength + 200;
	foreach (hMarine, weapons in g_teamZombie)
	{
		if (weapons[0].IsValid())
		{
			weapons[0].Destroy();
		}
		if (weapons[1].IsValid())
		{
			weapons[1].Destroy();
		}
		if (weapons[2].IsValid())
		{
			weapons[2].Destroy();
		}
	}
	foreach (hMarine, weapons in g_teamZombie)
	{
		if (hMarine)
		{
			Deathmatch.SetKills(hMarine, Deathmatch.GetKills(hMarine)+1);
			hMarine.TakeDamage(hMarine.GetMaxHealth()*5, 0, null);
		}
	}
	g_teamZombie.clear();
}

function StopRound()
{
	g_matchTimer = 0;
	g_idleTimer = 50;
	if (g_teamHuman.len() > 0)
	{
		g_victoryStatus = 1;
		ClientPrint(null, HUD_PRINTTALK, "#asw_infection_win_human");
		local survivorList = "";
		local l = [];
		local pts = g_teamZombie.len();
		foreach (hMarine, weapons in g_teamHuman)
		{
			Deathmatch.SetKills(hMarine, Deathmatch.GetKills(hMarine)+pts);
			l.append(hMarine);
		}
		for (local i = 0; i < l.len(); i++)
		{
			survivorList = survivorList + NameFeed(l[i]);
			if (i < l.len()-1)
			{
				survivorList = survivorList + ", ";
			}
		}
		ClientPrint(null, HUD_PRINTTALK, "#asw_infection_survivors", survivorList);
	}
	else
	{
		g_victoryStatus = 0;
		ClientPrint(null, HUD_PRINTTALK, "#asw_infection_win_zombie");
	}
}

function ResetGame()
{
	ClientPrint(null, HUD_PRINTCENTER, " ");
	local hMine = null;
	while((hMine = Entities.FindByClassname(hMine, "asw_laser_mine")) != null)
	{
		hMine.Destroy();
	}
	foreach (hMarine, weapons in g_teamZombie)
	{
		if (weapons[0].IsValid())
		{
			weapons[0].Destroy();
		}
		if (weapons[1].IsValid())
		{
			weapons[1].Destroy();
		}
		if (weapons[2].IsValid())
		{
			weapons[2].Destroy();
		}
	}
	local hMarine = null;
	while((hMarine = Entities.FindByClassname(hMarine, "asw_marine")) != null)
	{
		Deathmatch.SetKills(hMarine, Deathmatch.GetKills(hMarine)+1);
		hMarine.TakeDamage(hMarine.GetMaxHealth()*5, 0, null);
	}
	g_teamZombie.clear();
	g_primeZombies.clear();
	g_matchTimer = -1;
	g_lastStand = false;
	ResetStatTrackGlobal();
}

function CountMarine()
{
	local count = 0;
	local hMarine = null;
	while((hMarine = Entities.FindByClassname(hMarine, "asw_marine")) != null)
	{
		count = count + 1;
	}
	return count;
}

function CleanUp()
{
	RemoveEntitiesByClassName( "asw_ammo_drop" );
	RemoveEntitiesByClassName( "asw_weapon_ammo_satchel" );
	RemoveEntitiesByClassName( "asw_weapon_blink" );
	RemoveEntitiesByClassName( "asw_weapon_jump_jet" );
	RemoveEntitiesByClassName( "asw_weapon_heal_grenade" );
	RemoveEntitiesByClassName( "asw_weapon_medkit" );
	RemoveEntitiesByClassName( "asw_weapon_stim" );
	RemoveEntitiesByClassName( "asw_weapon_welder" );
	RemoveEntitiesByClassName( "asw_weapon_flares" );
	RemoveEntitiesByClassName( "asw_weapon_freeze_grenades" );
	RemoveEntitiesByClassName( "asw_weapon_sentry" );
	RemoveEntitiesByClassName( "asw_weapon_sentry_freeze" );
	RemoveEntitiesByClassName( "asw_weapon_sentry_cannon" );
	RemoveEntitiesByClassName( "asw_weapon_sentry_flamer" );
	RemoveEntitiesByClassName( "asw_weapon_heal_gun" );
	RemoveEntitiesByClassName( "asw_weapon_healamp_gun" );
	RemoveEntitiesByClassName( "asw_pickup_ammo_satchel" );
	RemoveEntitiesByClassName( "asw_pickup_blink" );
	RemoveEntitiesByClassName( "asw_pickup_jump_jet" );
	RemoveEntitiesByClassName( "asw_pickup_heal_grenade" );
	RemoveEntitiesByClassName( "asw_pickup_medkit" );
	RemoveEntitiesByClassName( "asw_pickup_stim" );
	RemoveEntitiesByClassName( "asw_pickup_welder" );
	RemoveEntitiesByClassName( "asw_pickup_flares" );
	RemoveEntitiesByClassName( "asw_pickup_freeze_grenades" );
	RemoveEntitiesByClassName( "asw_pickup_sentry" );
	RemoveEntitiesByClassName( "asw_pickup_sentry_freeze" );
	RemoveEntitiesByClassName( "asw_pickup_sentry_cannon" );
	RemoveEntitiesByClassName( "asw_pickup_sentry_flamer" );
	RemoveEntitiesByClassName( "asw_pickup_heal_gun" );
	RemoveEntitiesByClassName( "asw_pickup_healamp_gun" );
}

function RemoveEntitiesByClassName(strClassname)
{
	local hEntity = null;
	while ( ( hEntity = Entities.FindByClassname(hEntity, strClassname) ) != null )
	{
		hEntity.Destroy();
	}
}

function JoinHuman(hMarine)
{
	if (hMarine in g_teamHuman)
	{
		return;
	}
	if (hMarine in g_teamZombie)
	{
		if (g_teamZombie[hMarine][0].IsValid())
		{
			g_teamZombie[hMarine][0].Destroy();
		}
		g_teamZombie.rawdelete(hMarine);
	}
 	local hSprite = Entities.CreateByClassname( "env_sprite" );
	hSprite.__KeyValueFromInt( "spawnflags", 1 );
	hSprite.__KeyValueFromString( "Model", "materials/Sprites/light_glow03.vmt" );
	hSprite.__KeyValueFromString( "rendercolor", "0 255 255" );
	hSprite.__KeyValueFromFloat( "GlowProxySize", 24 );
	hSprite.__KeyValueFromInt( "renderamt", 192 );
	hSprite.__KeyValueFromInt( "rendermode", 9 );
	hSprite.__KeyValueFromFloat( "scale", 1.0 );
	hSprite.SetOrigin( hMarine.GetOrigin() + Vector(0, 0, 50) );
	hSprite.SetParent( hMarine );
	hSprite.Spawn();
	hSprite.Activate();
	g_teamHuman[hMarine] <- [hSprite];
	hMarine.SetTeam(TEAM_BETA);
	local newHealth = GetNewHealth(hMarine);
	if (hMarine.GetMaxHealth() != newHealth)
	{
		hMarine.SetMaxHealth(newHealth);
		hMarine.SetHealth(newHealth);
	}
	return;
}

function JoinZombie(hMarine)
{
	if (hMarine in g_teamZombie)
	{
		return;
	}
	if (hMarine in g_teamHuman)
	{
		if (g_teamHuman[hMarine][0].IsValid())
		{
			g_teamHuman[hMarine][0].Destroy();
		}
		g_teamHuman.rawdelete(hMarine);
	}
	hMarine.RemoveWeapon(0);
	hMarine.RemoveWeapon(1);
	hMarine.GiveWeapon("asw_weapon_chainsaw", 0);
	local newHealth = GetNewHealth(hMarine)*2;
	if (hMarine.GetMaxHealth() != newHealth)
	{
		hMarine.RemoveWeapon(2);
		hMarine.GiveWeapon("asw_weapon_grenades", 2);
		GetSlotWeapon(hMarine, 2).SetClip1(0);
	}
 	local hSprite = Entities.CreateByClassname( "env_sprite" );
	hSprite.__KeyValueFromInt( "spawnflags", 1 );
	hSprite.__KeyValueFromString( "Model", "materials/Sprites/light_glow03.vmt" );
	local color = "255 0 0";
	if (hMarine in g_prime)
	{
		color = "255 0 255";
	}
	hSprite.__KeyValueFromString( "rendercolor", color );
	hSprite.__KeyValueFromFloat( "GlowProxySize", 24 );
	hSprite.__KeyValueFromInt( "renderamt", 192 );
	hSprite.__KeyValueFromInt( "rendermode", 9 );
	hSprite.__KeyValueFromFloat( "scale", 1.0 );
	hSprite.SetOrigin( hMarine.GetOrigin() + Vector(0, 0, 50) );
	hSprite.SetParent( hMarine );
	hSprite.Spawn();
	hSprite.Activate();
	g_teamZombie[hMarine] <- [hSprite, GetSlotWeapon(hMarine, 0), GetSlotWeapon(hMarine, 2), 200, 0];
	hMarine.SetTeam(TEAM_ALPHA);
	if (hMarine.GetMaxHealth() != newHealth)
	{
		hMarine.SetMaxHealth(newHealth);
		hMarine.SetHealth(newHealth);
		hMarine.SetModel("models/swarm/marine/infected_marine.mdl");
		hMarine.SetOrigin(hMarine.GetOrigin() + Vector(0, 0, 32));
	}
	return;
}

function BecomePrime(hMarine)
{
	if (hMarine in g_prime)
	{
		return;
	}
	if (!(hMarine in g_teamZombie))
	{
		return;
	}
	local newHealth = GetNewHealth(hMarine)*4;
	local shield = newHealth*0.1;
	if (g_classic)
	{
		shield = 0;
	}
	g_prime[hMarine] <- [shield, 0, shield, null];
	if (g_teamZombie[hMarine][0].IsValid())
	{
		g_teamZombie[hMarine][0].Destroy();
	}
 	local hSprite = Entities.CreateByClassname( "env_sprite" );
	hSprite.__KeyValueFromInt( "spawnflags", 1 );
	hSprite.__KeyValueFromString( "Model", "materials/Sprites/light_glow03.vmt" );
	hSprite.__KeyValueFromString( "rendercolor", "255 0 255" );
	hSprite.__KeyValueFromFloat( "GlowProxySize", 24 );
	hSprite.__KeyValueFromInt( "renderamt", 192 );
	hSprite.__KeyValueFromInt( "rendermode", 9 );
	hSprite.__KeyValueFromFloat( "scale", 1.0 );
	hSprite.SetOrigin( hMarine.GetOrigin() + Vector(0, 0, 50) );
	hSprite.SetParent( hMarine );
	hSprite.Spawn();
	hSprite.Activate();
	hMarine.SetMaxHealth(newHealth);
	hMarine.SetHealth(newHealth);
	g_teamZombie[hMarine][0] = hSprite;
	g_teamZombie[hMarine][3] = 100;
	if (hMarine.IsInhabited())
	{
		local hPlayer = hMarine.GetCommander();
		if (!(hPlayer in g_primeZombies))
		{
			g_primeZombies[hPlayer] <- hMarine;
		}
	}
}

function LastStand()
{
	if (g_classic)
	{
		return;
	}
	g_lastStand = true;
	ClientPrint(null, HUD_PRINTTALK, "#asw_infection_lastStand_on");
}

function UseLastStand(hMarine)
{
	if (!(g_matchTimer > 0 && g_matchTimer < g_matchLength))
	{
		return;
	}
	if (!(hMarine in g_teamHuman))
	{
		return;
	}
	if (hMarine in g_lastHuman)
	{
		return;
	}
	AddTime(60);
	hMarine.DropWeapon(0);
	hMarine.DropWeapon(1);
	hMarine.GiveWeapon("asw_weapon_chainsaw", 0);
	local hBubble = Entities.CreateByClassname( "prop_dynamic" );
	hBubble.SetModel( "models/items/shield_bubble/shield_bubble.mdl" );
	hBubble.SetOrigin( hMarine.GetOrigin() + Vector(0, 0, 30) );
	hBubble.SetParent( hMarine );
	hBubble.SetModelScale(0.3, 0);
	hBubble.Spawn();
	hBubble.Activate();
	g_lastHuman[hMarine] <- [70, hBubble];
	local mod = 0.4*g_teamZombie.len();
	if (mod < 1)
	{
		mod = 1.0;
	}
	local newHealth = GetNewHealth(hMarine)*mod;
	hMarine.SetMaxHealth(newHealth);
	hMarine.SetHealth(newHealth);
	ClientPrint(null, HUD_PRINTTALK, "#asw_infection_lastStand_used", NameFeed(hMarine));
	ClientPrint(null, HUD_PRINTTALK, "#asw_infection_lastStand_timeAdd");
}

function OnGameEvent_player_say(params)
{
	if (!("text" in params) || params["text"] == null || !("userid" in params) || params["userid"] == null)
		return;
	
	local text = params["text"].tolower();
	local hPlayer = GetPlayerFromUserID(params["userid"]);

	switch (text){
		case "!suicide":
		case "!s":
			local hMarine = hPlayer.GetMarine();
			if (hMarine)
			{
				hMarine.TakeDamage(hMarine.GetMaxHealth()*5, 0, null);
			}
			return;
		default: return;
	}
	return;
}

function GetRage()
{
	local total = g_teamZombie.len() + g_teamHuman.len();
	local d = (total - g_prime.len()*2).tofloat();
	if (d <= 0)
	{
		return 0.0;
	}
	local n = (g_teamZombie.len() - g_prime.len()).tofloat();
	local result = n/d;
	if (total < 6)
	{
		result = result * (total.tofloat() / 6.0);
	}
	if (result > 1)
	{
		return 1.0;
	}
	return result;
}

function GetResistMod(hMarine)
{
	local r = 0.0;
	r = r + 0.1 * (GetNewHealth(hMarine)/800 - 1);
	if (hMarine in g_prime)
	{
		r = r + 0.3 * (GetNewHealth(hMarine)/800);
		if (g_prime[hMarine][0] > 0)
		{
			r = r + 0.5 + 0.2 * (g_prime[hMarine][0] / g_prime[hMarine][2]);
		}
	}
	if (r > 1)
	{
		r = 1.0;
	}
	return (1.0 - r);
}

function GetNearestHuman(origin)
{
	local result = null;
	foreach (hMarine, weapons in g_teamHuman)
	{
		if (hMarine.IsValid())
		{
			if (result)
			{
				if ((hMarine.GetOrigin() - origin).Length() < (result.GetOrigin() - origin).Length())
				{
					result = hMarine;
				}
			}
			else
			{
				result = hMarine;
			}
		}
	}
	return result;
}

function GetNewHealth(hMarine)
{
	if (hMarine)
	{
		switch(hMarine.GetMarineProfile())
		{
			case ASW_MARINE_PROFILE_SARGE:
				return 1400;
			case ASW_MARINE_PROFILE_VEGAS:
			case ASW_MARINE_PROFILE_JAEGER:
			case ASW_MARINE_PROFILE_WOLFE:
				return 1250;
			default:
				return 800;
		}
	}
	return 0;
}

function GetMedicID(hMarine)
{
	if (hMarine)
	{
		switch(hMarine.GetMarineProfile())
		{
			case ASW_MARINE_PROFILE_FAITH:
				return 1;
			case ASW_MARINE_PROFILE_BASTILLE:
				return 2;
			default:
				return 0;
		}
	}
	return 0;
}

function GetRegenCD(hMarine)
{
	if (hMarine)
	{
		switch(GetMedicID(hMarine))
		{
			case 1:
				return 40;
			case 2:
				return 60;
			default:
				return 70;
		}
	}
	return 70;
}

function Heal(ent, amt)
{
	if (!(ent))
	{
		return;
	}
	if (!(ent.IsValid()))
	{
		return;
	}
	if (ent.GetHealth() <= 0)
	{
		return;
	}
	ent.Extinguish();
	if (amt <= 0)
	{
		return;
	}
	local newHealth = ent.GetHealth() + amt;
	if (newHealth > ent.GetMaxHealth())
	{
		ent.SetHealth(ent.GetMaxHealth());
	}
	else
	{
		ent.SetHealth(newHealth);
	}
}

function AddTime(sec)
{
	local newTime = g_matchTimer + sec*10;
	if (newTime >= g_matchLength)
	{
		g_matchTimer = g_matchLength-1;
		return;
	}
	g_matchTimer = newTime;
	return;
}

function GetSlotWeapon(hMarine, slot)
{
	local slotName = "slot" + slot.tostring();
	local invTable = {};
	hMarine.GetInventoryTable(invTable);
	if (slotName in invTable)
	{
		return invTable[slotName];
	}
	return null;
}

function ZombieNearby(hMarine, dist=600)
{
	local hTarget = null;
	while ((hTarget = Entities.FindByClassnameWithin(hTarget, "asw_marine", hMarine.GetOrigin(), dist)) != null)
	{
		if (hTarget != hMarine && hTarget in g_teamZombie)
		{
			return true;
		}
	}
	return false;
}

function UnitToTime(unit, countHour=false, decimal=false, unitPerSecond=10)
{
	local time = unit/unitPerSecond;
	local sec = time%60;
	if (!(decimal))
	{
		sec = sec.tointeger();
	}
	time = time - time%60;
	if (time <= 0)
	{
		return [0, 0, sec];
	}
	if (countHour)
	{
		local min = time%3600/60;
		time = time - time%3600;
		if (time <= 0)
		{
			return [0, min, sec];
		}
		local hour = time/3600;
		return [hour, min, sec];
	}
	local min = time/60;
	return [0, min, sec];
}

function PlayZombieSound(hEmitter, sound)
{
	if (!hEmitter)
	{
		return;
	}
	if (!(sound in g_zombieSound))
	{
		return;
	}
	hEmitter.EmitSound(g_zombieSound[sound][RandomInt(0, g_zombieSound[sound].len()-1)]);
}

function NameFeed(hMarine)
{
	if (hMarine.IsInhabited())
	{
		return hMarine.GetCommander().GetPlayerName();
	}
	return hMarine.GetMarineName();
}

function IsOnGround(ent)
{
	if (TraceLine(ent.GetOrigin(), ent.GetOrigin() + Vector(0, 0, -0.1), null) == 1)
	{
		return false;
	}
	return true;
}

function UpdateStatTrack(hPlayer)
{
	if (!(hPlayer in g_statTrack))
	{
		g_statTrack[hPlayer] <- [0, 0, 0];
	}
	if (!(hPlayer in g_hud))
	{
		return;
	}
	g_hud[hPlayer].SetInt(8, g_statTrack[hPlayer][0]);
	g_hud[hPlayer].SetInt(9, g_statTrack[hPlayer][1]);
	g_statTrack[hPlayer][2] = 0;
	local hMarine = hPlayer.GetMarine();
	if (hMarine)
	{
		if (hMarine in g_teamZombie)
		{
			if (hMarine in g_prime)
			{
				g_statTrack[hPlayer][2] = 1;
			}
			else
			{
				g_statTrack[hPlayer][2] = 2;
			}
		}
		if (hMarine in g_teamHuman)
		{
			if (hMarine in g_lastHuman)
			{
				g_statTrack[hPlayer][2] = 4;
			}
			else
			{
				g_statTrack[hPlayer][2] = 3;
			}
		}
	}
	g_hud[hPlayer].SetInt(10, g_statTrack[hPlayer][2]);
}

function UpdateStat(hPlayer, index, delta=1)
{
	if (hPlayer in g_statTrack)
	{
		g_statTrack[hPlayer][index] = g_statTrack[hPlayer][index] + delta;
	}
}

function UpdateStatMarine(hMarine, index, delta=1)
{
	if (!hMarine)
	{
		return;
	}
	if (!(hMarine.IsValid()))
	{
		return;
	}
	if (hMarine.GetClassname() != "asw_marine")
	{
		return;
	}
	if (hMarine.IsInhabited())
	{
		UpdateStat(hMarine.GetCommander(), index, delta);
	}
}

function ResetStatTrack(hPlayer)
{
	if (hPlayer in g_statTrack)
	{
		for (local i = 0; i < g_statTrack[hPlayer].len(); i++)
		{
			g_statTrack[hPlayer][i] = 0;
		}
	}
}

function ResetStatTrackGlobal()
{
	local hPlayer = null;
	while((hPlayer = Entities.FindByClassname(hPlayer, "player")) != null)
	{
		ResetStatTrack(hPlayer);
	}
}
