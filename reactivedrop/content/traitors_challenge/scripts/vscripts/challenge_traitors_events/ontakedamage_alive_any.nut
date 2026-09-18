function OnTakeDamage_Alive_Any(victim, inflictor, attacker, weapon, damage, damageType, ammoName) {
	if (!g_bool_ClhallengeEnable || attacker == null || attacker.IsAlien() || inflictor == null || inflictor.IsAlien() || victim == null || victim.GetClassname() != "asw_marine") {
		return damage;
	}

	local strInflictorName = inflictor.GetName();
	if (g_enum_CurrentMap == MAP.TILA_9) {
		if (strInflictorName == "ship_turrets") {
			return 0;
		} else if (strInflictorName == "ship_turrets_2") {
			return damage * 0.1;
		}
	} else if (g_enum_CurrentMap == MAP.BIO_1) {
		if (strInflictorName == "turrets") {
			return 0;
		}
	} else if (g_enum_CurrentMap == MAP.JAC_3 || g_enum_CurrentMap == MAP.JAC_7) {
		if (strInflictorName == "ship_turrets") {
			return 0;
		}
	}

	if (g_int_Counter <= g_int_ImmuneCounter && (attacker.GetClassname() == "asw_marine" || attacker.GetClassname() == "asw_radiation_volume" || attacker.GetClassname() == "env_fire" || attacker.GetClassname() == "asw_burning" || attacker.GetClassname() == "asw_grenade_cluster" || attacker.GetClassname() == "asw_t75") && attacker != victim) // 开局一定时间内玩家无法互相造成伤害，但自己攻击自己造成正常伤害
	{
		return 0;
	} else if (g_int_Counter <= g_int_ImmuneCounter + 20 && (attacker.GetClassname() == "asw_marine" || attacker.GetClassname() == "asw_radiation_volume" || attacker.GetClassname() == "env_fire" || attacker.GetClassname() == "asw_burning" || attacker.GetClassname() == "asw_grenade_cluster" || attacker.GetClassname() == "asw_t75") && attacker != victim) // 此后10s伤害减半
	{
		damage *= 0.5;
	}

	local factor1 = 1.00;
	local factor2 = 1.00;
	local factor3 = 1.00;
	local victimMaxHealth = victim.GetMaxHealth();
	local victimCurrentHealth = victim.GetHealth();
	if (attacker.GetClassname() == "asw_marine" && weapon != null && weapon.IsValid()) {
		//536875010 : 0010 0000 0000 0000 0001 0000 0000 0010
		//			= (1 << 29)		| (1 << 12)		| (1 << 1)
		//          = DMG_BUCKSHOT  | DMG_NEVERGIB	| DMG_BULLET
		//Why the original author used damageType == 536875010 and it works?
		if (damageType == 536875010 || weapon.GetClassname() == "asw_weapon_chainsaw") {
			switch (weapon.GetClassname()) {
				case "asw_weapon_combat_rifle": //战斗步枪霰弹
					factor1 = 0.08;
					factor2 = 0.06;
					factor3 = 0.10;
					break;
				case "asw_weapon_devastator": //毁灭者霰弹
					factor1 = 0.31;
					factor2 = 0.21;
					factor3 = 0.41;
					break;
				case "asw_weapon_vindicator": //复仇者霰弹
					factor1 = 0.20;
					factor2 = 0.15;
					factor3 = 0.25;
					break;
				default:
					factor1 = 0.08;
					factor2 = 0.06;
					factor3 = 0.10;
			}
		} else if (damageType == DAMAGE_TYPE.DMG_CLUB && inflictor.GetClassname() == "asw_marine") {
			damage = 0;
			PunchKnockdown(attacker, victim);

		} else {
			switch (weapon.GetClassname()) {
				case "asw_weapon_rifle": //突击步枪
					factor1 = 0.95;
					factor2 = 0.85;
					factor3 = 1.05;
					if (inflictor.GetClassname() == "asw_rifle_grenade") {
						damage = damage > 100 ? 100 : damage;
						factor1 = 0.35;
						factor2 = 0.3;
						factor3 = 0.4;
					}
					break;
				case "asw_weapon_prifle": //原型突击步枪
					local distance = (attacker.GetOrigin() - victim.GetOrigin()).Length();
					local temp = distance < 250 ? 1.0 : 300.0 / distance;
					factor1 = temp * RandomHQUniformFloatDistribution(0.25, 0.54);
					factor2 = temp * RandomHQUniformFloatDistribution(0.23, 0.44);
					factor3 = temp * RandomHQUniformFloatDistribution(0.27, 0.64);
					break;
				case "asw_weapon_autogun": //自动机枪
					factor1 = 0.47;
					factor2 = 0.37;
					factor3 = 0.57;
					break;
				case "asw_weapon_pistol": //双手枪
					factor1 = 0.30;
					factor2 = 0.25;
					factor3 = 0.35;
					break;
				case "asw_weapon_shotgun": //泵动式霰弹枪
					factor1 = 0.60;
					factor2 = 0.50;
					factor3 = 0.70;
					break;
				case "asw_weapon_pdw": //单兵防御武器
					factor1 = 0.35;
					factor2 = 0.30;
					factor3 = 0.40;
					break;
				case "asw_weapon_minigun": //迷你机枪
					factor1 = 0.24;
					factor2 = 0.18;
					factor3 = 0.30;
					break;
				case "asw_weapon_deagle": //斗牛犬
					factor1 = 0.26;
					factor2 = 0.20;
					factor3 = 0.32;
					break;
				case "asw_weapon_devastator": //毁灭者的霰弹伤害在上面调整
					break;
				case "asw_weapon_combat_rifle": //战斗步枪，辅助开火的霰弹伤害在上面调整
					factor1 = 0.67;
					factor2 = 0.46;
					factor3 = 0.90;
					break;
				case "asw_weapon_heavy_rifle": //重型突击步枪
					factor1 = 0.40;
					factor2 = 0.33;
					factor3 = 0.48;
					break;
				case "asw_weapon_railgun": //导轨步枪
					factor1 = 0.60;
					factor2 = 0.50;
					factor3 = 0.70;
					break;
				case "asw_weapon_sniper_rifle": //神射手
					factor1 = 0.095;
					factor2 = 0.045;
					factor3 = 0.145;
					break;
				case "asw_weapon_medrifle": //医疗冲锋枪
					factor1 = 0.50;
					factor2 = 0.40;
					factor3 = 0.60;
					break;
				case "asw_weapon_laser_mines": //激光地雷
					damage = 6;
					factor1 = 1.00;
					factor2 = 1.00;
					factor3 = 1.00;
					break;
				case "asw_weapon_grenades": //手雷
					damage = RandomHQNormalDistribution(2000, 200);
					factor1 = 0.001;
					factor2 = 0.005;
					factor3 = 0.015;
					break;
				default:
					if (attacker == weapon) {
						switch (attacker.GetMarineName()) {
							case "Vegas":
							case "Jaeger":
								damage = victimMaxHealth * 0.2;
								break;
							case "Wildcat":
							case "Faith":
								damage = victimMaxHealth * 0.18;
								break;
							default:
								damage = victimMaxHealth * 0.19;
								break;
						}
						factor1 = 1.00;
						factor2 = 1.00;
						factor3 = 1.00;
					} else {
						factor1 = 0.18;
						factor2 = 0.16;
						factor3 = 0.20;
					}
			}
		}
		if (victimCurrentHealth <= victimMaxHealth * 0.5) {
			damage = damage * factor1;
		} else if (victimCurrentHealth <= victimMaxHealth * 0.25) {
			damage = damage * factor2;
		} else {
			damage = damage * factor3;
		}
		damage *= 0.8;
	}

	if (attacker.IsValid() && attacker.GetClassname() == "asw_marine") {
		local attackerOrigin = attacker.GetOrigin();
		local victimOrigin = victim.GetOrigin();
		local distanceRatio = 1.0;
		local horizontalDistance = Vector((attackerOrigin - victimOrigin).x, (attackerOrigin - victimOrigin).y, 0).Length();
		local verticalDistance = abs(attackerOrigin.z - victimOrigin.z);
		if (horizontalDistance > 1500 || verticalDistance > 300) {
			distanceRatio = 0.0;
		} else if (horizontalDistance < 1100 && verticalDistance < 200) {
			distanceRatio = 1.0;
		} else {
			distanceRatio = (1 - pow((horizontalDistance - 1100), 2) / 160000.0) * (1 - pow((verticalDistance - 200), 2) / 10000.0);
		}
		damage *= distanceRatio;

		attacker.ValidateScriptScope();
		local hHud = Entities.FindByName(null, attacker.GetScriptScope().strFlashbangHudName);
		local ratio = 1.0;
		local currentIntensity;
		if (Time() < hHud.GetFloat(1)) {
			ratio = (Time() - hHud.GetFloat(0)) / (hHud.GetFloat(1) - hHud.GetFloat(0));
			currentIntensity = hHud.GetFloat(2) * (1 - ratio * ratio);
			if (weapon != null && (weapon.GetClassname() == "asw_weapon_railgun" || weapon.GetClassname() == "asw_weapon_sniper_rifle")) {
				// 被闪光弹影响时，狙击武器基本失去作用，这很合理
				//  1 - currentIntensity / 1.25
				// -> 0.80		~ damage * 0.0001
				// -> 0.85		~ damage * 0.0015
				// -> 0.90		~ damage * 0.015
				// -> 0.95		~ damage * 0.129
				// -> 0.97		~ damage * 0.296
				// -> 0.99		~ damage * 0.669
				damage = damage * pow((1 - currentIntensity / 1.25), 40);
			} else {
				damage = damage * (1 - currentIntensity / 1.25);
			}
		}
	}
	//计算护盾减伤
	damage = ApplyShieldProtection(damage, victim);
	damage = ApplyExtraHealthProtection(damage, victim);

	//逃兵相关计算
	if (g_marine_Deserter != null && g_marine_Deserter.IsValid() && attacker != victim) {
		local fleeTime = g_marine_Deserter.GetScriptScope().RevealTime;
		local tempTime = Time();
		if ((victim == g_marine_Deserter || attacker == g_marine_Deserter) && tempTime > fleeTime && tempTime <= fleeTime + 30.0) {
			damage = 0;
		}
		if (attacker == g_marine_Deserter && tempTime > fleeTime + 30.0 && tempTime <= fleeTime + 120.0) {
			damage *= 0.2;
		}
		if (attacker == g_marine_Deserter && tempTime > fleeTime + 120.0) {
			damage *= 0.8;
		}

		if (victim == g_marine_Deserter && attacker.IsValid() && attacker.GetClassname() == "asw_marine" && Time() < g_marine_Deserter.GetScriptScope().RevealTime && damage >= victimCurrentHealth - 1) {
			victim.SetHealth(victimMaxHealth);
			damage = 0;
			victim.GetScriptScope().Role += ROLE.DESERTER;
			victim.GetScriptScope().RevealTime = Time();
			ResetHudAndChatForDeserterPlayer(g_marine_Deserter);
		}
	}
	victim.ValidateScriptScope();
	return victim.GetScriptScope().DamageMapModifier * damage;
}

function ResetHudAndChatForDeserterPlayer(hMarine) {
	if (hMarine == null || !hMarine.IsValid() || !hMarine.IsInhabited()) {
		return;
	}
	local hPlayer = hMarine.GetCommander();
	hPlayer.ValidateScriptScope();
	local role = hMarine.GetScriptScope().Role;
	local strRole = GetRoleString(role);

	LocalizedClientPrint(null, 3, hPlayer.GetPlayerName() + "%s1", "#challenge_traitors_deserter_flee_notify1");

	local strTextColor = role < ROLE.MAX_IAF_TEAM ? TextColor(255, 255, 210) : TextColor(255, 0, 0);

	LocalizedClientPrint(hPlayer, 3, " ");
	LocalizedClientPrint(hPlayer, 3, strTextColor + "%s1", "#challenge_traitors_hud_on_game_start", null, null, null, role);
	LocalizedClientPrint(hPlayer, 3, strTextColor + "%s1%s2" + GetExtraHealthString(role), "#challenge_traitors_role_chat_role_description_" + strRole, "#challenge_traitors_role_chat_skill_description_" + strRole, null, null, role);

	local strLanguage = GetClientLanguage(hPlayer.entindex());
	local hHud1 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName1);
	hHud1.SetInt(0, role);
	hHud1.SetString(0, GetLocalizedString("#challenge_traitors_hud_on_game_start", strLanguage, role));
	hHud1.SetFloat(30, Time());
	local hHud2 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName2);
	hHud2.SetInt(0, role);
	hHud2.SetString(0, GetLocalizedString("#challenge_traitors_role_hud_role_description_" + strRole.tolower(), strLanguage, role) + GetExtraHealthString(role));
	hHud2.SetFloat(30, Time());
}

function ApplyShieldProtection(damage, victim) {
	local entIndex = victim.entindex();
	if ((entIndex in g_tbl_Shield) && (g_tbl_Shield[entIndex] > 0)) {
		local shield = g_tbl_Shield[entIndex];
		g_tbl_Shield[entIndex] -= damage;
		damage = damage * (1.0 - shield / 300.0);
	}
	return damage
}

function ApplyExtraHealthProtection(damage, victim) {
	if (victim == g_marine_SilencedMarine) {
		return damage;
	}
	local entIndex = victim.entindex();
	local victimCurrentHealth = victim.GetHealth();
	if (victimCurrentHealth < damage + 1.01 && g_tbl_ExtraHealth[entIndex] > 0) {
		local delta = damage - victimCurrentHealth + 1.0;
		g_tbl_ExtraHealth[entIndex] = g_tbl_ExtraHealth[entIndex] - delta;
		damage = g_tbl_ExtraHealth[entIndex] > 0 ? damage - delta : damage - delta - g_tbl_ExtraHealth[entIndex];
	}
	return damage;
}

function PunchKnockdown(attacker, victim) {
	if (attacker == null || !attacker.IsValid() || victim == null || !victim.IsValid()) {
		return;
	}
	local vecAttackerToVictim = attacker.GetOrigin() - victim.GetOrigin();
	vecAttackerToVictim.z = 0;
	vecAttackerToVictim.Norm();
	local vecVictimFacing = EulerToVectors(victim.GetAngles());
	vecVictimFacing.z = 0;
	vecVictimFacing.Norm();
	local cosTheta = vecAttackerToVictim.Dot(vecVictimFacing);
	local item = NetProps.GetPropEntityArray(attacker, "m_hMyWeapons", 2);
	local hasPowerFist = false;
	if (item != null && item.IsValid() && item.GetClassname == "asw_weapon_fist:") {
		hasPowerFist = true;
	}
	local thresh = 1.0;
	if (cosTheta > 0.866) {
		thresh = hasPowerFist ? 0.6 : 0.3;
	} else if (cosTheta >= 0 && cosTheta < 0.866) {
		thresh = hasPowerFist ? 0.7 : 0.45;
	} else if (cosTheta >= -0.866 && cosTheta < 0) {
		thresh = hasPowerFist ? 0.8 : 0.6;
	} else {
		thresh = hasPowerFist ? 0.9 : 0.75;
	}
	if (thresh >= RandomHQUniformFloatDistribution(0.0, 1.0)) {
		local temp = RandomHQUniformFloatDistribution(1.0, 1.5) * -200;
		vecAttackerToVictim.x *= temp;
		vecAttackerToVictim.y *= temp;
		vecAttackerToVictim.z = 70 * temp;
		victim.Knockdown(vecAttackerToVictim);
	}
}