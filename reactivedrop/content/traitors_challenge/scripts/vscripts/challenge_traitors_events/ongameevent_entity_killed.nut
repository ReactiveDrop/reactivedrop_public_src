function OnGameEvent_entity_killed(params) {
	if (!g_bool_ClhallengeEnable) {
		return;
	}
	if (g_bool_Initialized) {
		// 获取死亡实体句柄
		local victim = EntIndexToHScript(params["entindex_killed"]);
		// 如果是士兵 且 ( 玩家控制 或 ( 机器人控制 且 开启机器人杀死更新 ) )
		if (victim != null && victim.GetClassname() == "asw_marine") {
			local attacker = null;
			local inflictor = null;
			if ("entindex_attacker" in params) {
				attacker = EntIndexToHScript(params["entindex_attacker"]);
			}
			if ("entindex_inflictor" in params) {
				inflictor = EntIndexToHScript(params["entindex_inflictor"]);
			}

			//检查是否是sniper，并发动技能
			if (victim == g_marine_Sniper && victim != g_marine_SilencedMarine && attacker != null && attacker.GetClassname() == "asw_marine" && victim != attacker) {
				attacker.TakeDamage(99999, 64, victim);
			}

			//检查是否是demo，并发动技能
			if (victim == g_marine_Demo && victim != g_marine_SilencedMarine && attacker != null && attacker.GetClassname() == "asw_marine" && victim != attacker) {
				local deathPos = victim.GetOrigin();

				local explosion = Entities.CreateByClassname("env_explosion"); // 创建爆炸效果
				explosion.SetOrigin(deathPos); // 设置爆炸位置
				explosion.__KeyValueFromInt("iMagnitude", 0); // 设置伤害为 0（由自定义函数处理）
				explosion.__KeyValueFromInt("fireballsprite", 1); // 设置爆炸效果
				EntFireByHandle(explosion, "Explode", "", 0, null, null); // 触发爆炸
				EntFireByHandle(explosion, "Kill", "", 3, null, null); // 销毁爆炸实体
				victim.PrecacheSoundScript("Traitors.Boomer_Explode");
				victim.EmitSound("Traitors.Boomer_Explode");
				ApplyExplosionDamageToMarines(victim, deathPos, 150, RandomHQNormalDistribution(0.95, 0.05));
			}

			local tempList = [];
			foreach(hMarine in g_marine_TraitorAlive) {
				if (hMarine != null && hMarine.IsValid() && hMarine != victim) {
					tempList.append(hMarine);
				}
			}
			g_marine_TraitorAlive = tempList; //更新存活的内鬼列表
			g_int_TraitorAliveCount = g_marine_TraitorAlive.len(); // 更新存活的内鬼数量

			tempList = [];
			foreach(hMarine in g_marine_IafAlive) {
				if (hMarine != null && hMarine.IsValid() && hMarine != victim) {
					tempList.append(hMarine);
				}
			}
			g_marine_IafAlive = tempList; //更新存活的IAF队员列表
			g_int_IafAliveCount = g_marine_IafAlive.len(); // 更新存活的IAF队员数量

			// 如果所有IAF队员死亡且内鬼存活数量大于0，则内鬼胜利
			if (g_int_IafAliveCount == 0 && g_int_TraitorAliveCount > 0) {
				g_bool_TraitorWin = true;
				Director.MissionComplete(true);
			}
		}
	}
}

function ApplyExplosionDamageToMarines(hAttacker, explosionPos, radius, damageRatio) {
	// 查找所有类名为 asw_marine 的实体
	local hMarine = null
	local ratio = 1.0;
	while (hMarine = Entities.FindByClassnameWithin(hMarine, "asw_marine", explosionPos, 4 * radius)) {
		// 检查实体是否有效且存活
		if (hMarine.IsValid() && hMarine != hAttacker && hMarine.GetHealth() > 0) {
			local targetPos = hMarine.GetOrigin();
			local hitCount = GetHitCount(explosionPos, targetPos);
			if (hitCount >= 115) {
				continue;
			}
			local distance = (targetPos - explosionPos).Length();
			local maxHealth = hMarine.GetMaxHealth();
			local currentHealth = hMarine.GetHealth();
			if (distance > 4 * radius) {
				ratio = 0.0;
			} else if (distance > radius && distance <= 4 * radius) {
				ratio = -pow((distance / radius - 1), 2.0) / 9.0 + 1.0;
			}
			// 应用伤害
			local finalDamage = maxHealth * ratio * damageRatio;
			finalDamage = ApplyShieldProtection(finalDamage, hMarine);
			finalDamage = ApplyExtraHealthProtection(finalDamage, hMarine);
			if (currentHealth - finalDamage >= 1) {
				hMarine.SetHealth(currentHealth - finalDamage);
			} else {
				hMarine.SetHealth(1);
				hMarine.TakeDamage(999, DAMAGE_TYPE.DMG_BLAST, hMarine);
			}
		}
	}
}