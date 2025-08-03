DEBUG <- true; // Debug flags 调试标识
INT_MAX <- 2147483647; // Constant 常量
isServer <- true; // If the code is running server side or client side 指示代码是在服务端运行的还是在客户端运行的
IsPaused <- false; // Not actually being used 没有实际用处
g_bool_Initialized <- false; // If all parameters and functions are initialized 是否已经初始化
g_bool_ClhallengeEnable <- true; // Enable / disable challenge 启用/禁用挑战
g_bool_TechMap <- false; // Not actually being used 没有实际用处
g_bool_TechKill <- false; // Not actually being used 没有实际用处

IncludeScript("challenge_traitors_enums.nut"); // Enum types 枚举类型
IncludeScript("challenge_traitors_random.nut"); // Random number lib 随机数相关函数
IncludeScript("challenge_traitors_localize.nut"); // Localization related 本地化函数
IncludeScript("challenge_traitors_translations_all.nut"); // Localization strings 翻译文本
IncludeScript("challenge_traitors_client_shared.nut"); // Shared functions for client side UI UI公用函数
IncludeScript("challenge_traitors_map_handler.nut"); // Handles edge cases of different maps处理地图的特殊情况
//Events 事件
IncludeScript("challenge_traitors_events/ongameevent_heal_beacon_placed.nut");
IncludeScript("challenge_traitors_events/ongameevent_entity_killed.nut");
IncludeScript("challenge_traitors_events/ongameevent_flare_placed.nut");
IncludeScript("challenge_traitors_events/ongameevent_marine_selected.nut");
IncludeScript("challenge_traitors_events/ongameevent_mission_success_or_fail.nut");
IncludeScript("challenge_traitors_events/ongameevent_player_fullyjoined.nut");
IncludeScript("challenge_traitors_events/ongameevent_weapon_fire.nut");
IncludeScript("challenge_traitors_events/ongameevent_weapon_reload.nut");
IncludeScript("challenge_traitors_events/OnReceivedTextMessage.nut");
IncludeScript("challenge_traitors_events/OnTakeDamage_Alive_Any.nut");


//Role variables / arrays / tables 角色相关的变量等
g_marine_Total_Unshuffled <- [];
g_marine_Total <- [];
g_marine_Traitor <- [];
g_player_Traitor <- [];
g_player_TraitorHistory <- [];
g_marine_Iaf <- [];
g_marine_IafAlive <- [];
g_marine_TraitorAlive <- [];
g_str_TraitorNameList <- "";
g_str_TraitorNameListHUD <- "";
g_marine_Scanner <- null;
g_marine_Biochemist <- null;
g_marine_IafLeader <- null;
g_marine_Shield <- null;
g_marine_Sniper <- null;
g_marine_Demo <- null;
g_marine_Deserter <- null;
g_marine_TraitorLeader <- null;
g_marine_Infector <- null;
g_marine_Infected <- null;
g_marine_Boomer <- null;
g_marine_Silencer <- null;
g_marine_Mimic <- null;

//Role count / flags 角色数量相关
g_int_MarineCount <- 0;
g_int_TraitorCount <- 0;
g_int_IafAliveCount <- 0;
g_int_TraitorAliveCount <- 0;
g_bool_HasScanner <- false;
g_bool_HasBiochemist <- false;
g_bool_HasIafLeader <- false;
g_bool_HasShield <- false;
g_bool_HasSniper <- false;
g_bool_HasDemo <- false;
g_bool_HasDeserter <- false;
g_bool_HasTraitorLeader <- false;
g_bool_HasInfector <- false;
g_bool_HasBoomer <- false;
g_bool_HasSilencer <- false;
g_bool_HasMimic <- false;

//Skill targets 技能目标
g_marine_SilencedMarine <- null;
g_marine_HealedMarine <- null;
g_marine_KilledMarine <- null;
g_marine_AbetedMarine <- null;
g_marine_ShieldedMarine <- null;

g_tbl_RoleInfo <- { // Role names for UI displaying UI显示用到的角色名称
	Scanner = "",
	Biochemist = "",
	Iaf_Leader = "",
	Shield = "",
	Sniper = "",
	Demo = "",
	Deserter = "",
	Traitor_Leader = "",
	Infector = "",
	Boomer = "",
	Silencer = "",
	Mimic = "",
};

g_tbl_Shield <- {}; // Shield for different marine entity 额外护甲
g_tbl_ExtraHealth <- {}; // Extra health for different marine entity 额外血量

g_tbl_surrenderVotes <- { // Surrender votes 投降的投票
	VoteStartTime = -100.0,
	Duration = 20,
	CurrentTickets = {},
	TotalTickets = {},
};

g_tbl_MenuSkillInfo <- { // Info / Properties / Status for each skill, used to update UI UI使用的技能信息
	gameStartTime = Time(),
	scannerRandomScanCounter = 3,

	biochemistLastSkillTime = 0.0,
	infectorLastSkillTime = 0.0,
	scannerLastSkillTime = 0.0,
	shieldLastSkillTime = 0.0,
	silencerLastSkillTime = 0.0,

	biochemistSkillCD = 20,
	infectorSkillCD = 20,
	scannerSkillCD = 30,
	shieldSkillCD = 10,
	silencerSkillCD = 10,

	biochemistIsHealUsed = false,
	biochemistIsKillUsed = false,
	infectorIsSkillActive = false,
	infectorIsSkillUsed = false,
	shieldIsSkillUsed = false,
	silencerIsSkillUsed = false,
};
g_lst_MenuProps <- []; // Info / Properties / Status for each marine, used to update UI UI使用的状态信息

g_tbl_Flashbang <- {}; // Variable that used to handle flashbang effect 处理闪光弹用到的变量

g_int_Counter <- 0; // Current tick for this challenge, 10 ticks per sencond 当前计数器
g_int_ImmuneCounter <- 0; // Marines are immune if g_int_Counter <= g_int_ImmuneCounter 无敌时间计数器
g_int_JumpHeight <- 50; // Adjust marine jump height 跳跃高度

g_enum_CurrentMap <- MAP.OTHER; // Current map 当前地图

g_bool_IafWin <- false; // IAF wins
g_bool_TraitorWin <- false; // Traitor wins
g_bool_TraitorSurrender <- false; // Traitors surrender and IAF wins
g_bool_IafWinDelayFlag <- false; // Delay displaying winner msg
g_bool_TraitorWinDelyFlag <- false; //Delay displaying winner msg

g_ent_HudAndVGui <- []; // Entindex of all UI elements that have ever been created, used to release them at end of each mission. 存储所有挑战使用到的UI实体索引，用于游戏结束时释放资源

function Update() {
	if (!g_bool_ClhallengeEnable) { //挑战没有启用，无限期等待(65535秒)
		return 65535;
	}
	if (!g_bool_Initialized) { // 没有初始化，等待0.1s
		return 0.1;
	}

	//DebugKillAliens(20);

	SetTraitorIcon();

	DropWeapon();
	RefreshSkillMenu();
	DetectAndApplySkill(5);
	FixT75(20);
	Flashbang(5); // 处理闪光弹效果
	//RemoveAmmoOnMap(10);		// 移除地图上的弹药箱
	SetPlayerAmmo(20); // 设置玩家弹药量
	UpdateTraitorHud(10); // 更新内鬼hud
	ProcessMapSpecialCases(); // 处理地图特殊情况
	RemoveBot(20); // 移除机器人
	SetConVars(199); // 每20s随机一下部分数值
	CheckSurrenderVotes(10);

	g_int_Counter++; // 计数器+1

	return CheckAndShowWinner(); // 检查游戏是否结束，结束时显示结果，否则等待0.1s
}

function DebugKillAliens(interval = 1) {
	if (g_int_Counter % interval != 0) {
		return;
	}
	printl(g_marine_Total[0].GetOrigin());
	local hEntitiy = null;
	while (hEntitiy = Entities.FindInSphere(hEntitiy, g_marine_Total[0].GetOrigin(), 1000)) {
		if (hEntitiy.IsAlien()) {
			hEntitiy.SetHealth(1);
			hEntitiy.TakeDamage(99, DAMAGE_TYPE.DMG_FALL, null);
		}
	}
}

function SetTraitorIcon(interval = 10) {
	if (g_int_Counter % interval != 0) {
		return;
	}
	foreach(hMarine in g_marine_Total) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		hMarine.ValidateScriptScope();
		switch (hMarine.GetScriptScope().Role) {
			case ROLE.TRAITOR:
			case ROLE.INFECTED_IAF:
			case ROLE.INFECTED_SCANNER:
			case ROLE.INFECTED_BIOCHEMIST:
			case ROLE.INFECTED_IAF_LEADER:
			case ROLE.INFECTED_SHIELD:
			case ROLE.INFECTED_SNIPER:
			case ROLE.INFECTED_DEMO:
			case ROLE.INFECTED_DESERTER:
				NetProps.SetPropInt(hMarine, "m_iEmote", NetProps.GetPropInt(hMarine, "m_iEmote") | (1 << 8));
				break;
			case ROLE.TRAITOR_LEADER:
				NetProps.SetPropInt(hMarine, "m_iEmote", NetProps.GetPropInt(hMarine, "m_iEmote") | (1 << 9));
				break;
			case ROLE.INFECTOR:
				NetProps.SetPropInt(hMarine, "m_iEmote", NetProps.GetPropInt(hMarine, "m_iEmote") | (1 << 10));
				break;
			case ROLE.BOOMER:
				NetProps.SetPropInt(hMarine, "m_iEmote", NetProps.GetPropInt(hMarine, "m_iEmote") | (1 << 11));
				break;
			case ROLE.SILENCER:
				NetProps.SetPropInt(hMarine, "m_iEmote", NetProps.GetPropInt(hMarine, "m_iEmote") | (1 << 12));
				break;
			case ROLE.MIMIC:
				NetProps.SetPropInt(hMarine, "m_iEmote", NetProps.GetPropInt(hMarine, "m_iEmote") | (1 << 13));
				break;
		}
	}
	foreach(hPlayer in g_player_TraitorHistory) {
		if (hPlayer == null || !hPlayer.IsValid()) {
			continue;
		}
		NetProps.SetPropInt(hPlayer, "m_iFrags", 99);
	}
}

function DropWeapon() {
	local idx_end = g_int_Counter % 31;
	for (local i = 0; i * 31 + idx_end < g_int_MarineCount; i++) {
		local hMarine = g_marine_Total[i * 10 + idx_end];
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		local tbl = {};
		local weapon1 = null;
		local weapon2 = null;
		local veryHighDamageWeaponCount = 0;
		local highFiringRateWeaponCount = 0;
		local accurateWeaponCount = 0;
		NetProps.GetTable(hMarine, 1, tbl);
		weapon1 = tbl["m_hMyWeapons"]["000"];
		weapon2 = tbl["m_hMyWeapons"]["001"];
		if (weapon1 != null && weapon2 != null) {
			switch (weapon1.GetClassname()) {
				case "asw_weapon_rifle": //突击步枪
					break;
				case "asw_weapon_prifle": //原型突击步枪
					break;
				case "asw_weapon_autogun": //自动机枪
					break;
				case "asw_weapon_pistol": //双手枪
					veryHighDamageWeaponCount += 0;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_shotgun": //泵动式霰弹枪
					break;
				case "asw_weapon_pdw": //单兵防御武器
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_minigun": //迷你机枪
					break;
				case "asw_weapon_deagle": //斗牛犬
					break;
				case "asw_weapon_devastator": //毁灭者
					break;
				case "asw_weapon_combat_rifle": //战斗步枪
					break;
				case "asw_weapon_heavy_rifle": //重型突击步枪
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 0;
					break;
				case "asw_weapon_railgun": //导轨步枪
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 0;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_sniper_rifle": //神射手
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 0;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_medrifle": //医疗冲锋枪
					veryHighDamageWeaponCount += 0;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 0;
					break;
				default:
			}
			switch (weapon2.GetClassname()) {
				case "asw_weapon_rifle": //突击步枪
					break;
				case "asw_weapon_prifle": //原型突击步枪
					break;
				case "asw_weapon_autogun": //自动机枪
					break;
				case "asw_weapon_pistol": //双手枪
					veryHighDamageWeaponCount += 0;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_shotgun": //泵动式霰弹枪
					break;
				case "asw_weapon_pdw": //单兵防御武器
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_minigun": //迷你机枪
					break;
				case "asw_weapon_deagle": //斗牛犬
					break;
				case "asw_weapon_devastator": //毁灭者
					break;
				case "asw_weapon_combat_rifle": //战斗步枪
					break;
				case "asw_weapon_heavy_rifle": //重型突击步枪
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 0;
					break;
				case "asw_weapon_railgun": //导轨步枪
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 0;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_sniper_rifle": //神射手
					veryHighDamageWeaponCount += 1;
					highFiringRateWeaponCount += 0;
					accurateWeaponCount += 1;
					break;
				case "asw_weapon_medrifle": //医疗冲锋枪
					veryHighDamageWeaponCount += 0;
					highFiringRateWeaponCount += 1;
					accurateWeaponCount += 0;
					break;
				default:
			}
			if (veryHighDamageWeaponCount >= 2 || highFiringRateWeaponCount >= 2 || accurateWeaponCount >= 2) {
				if (hMarine != null && hMarine.IsValid()) {
					hMarine.DropWeapon(1);
					local hPlayer = hMarine.GetCommander();
					if (hPlayer != null && hPlayer.IsValid()) {
						local strLanguage = GetClientLanguage(hPlayer.entindex());
						LocalizedClientPrint(hPlayer, 3, GetLocalizedString("#challenge_traitors_drop_weapon_notify", strLanguage));
					}
				}
			}
		}
	}
}

function FixT75(interval = 1) {
	if (g_int_Counter % interval != 0) {
		return;
	}
	local hT75 = null;
	while (hT75 = Entities.FindByClassname(hT75, "asw_t75")) {
		hT75.__KeyValueFromInt("solid", 0);
	}
}

function RefreshSkillMenu(interval = 1) {
	if ((g_int_Counter + 1) % interval != 0) {
		return;
	}

	if (g_int_Counter <= g_int_ImmuneCounter) {
		g_tbl_MenuSkillInfo.scannerLastSkillTime = g_tbl_MenuSkillInfo.gameStartTime + 0.1 * g_int_ImmuneCounter;
		g_tbl_MenuSkillInfo.biochemistLastSkillTime = g_tbl_MenuSkillInfo.gameStartTime + 0.1 * g_int_ImmuneCounter;
		g_tbl_MenuSkillInfo.infectorLastSkillTime = g_tbl_MenuSkillInfo.gameStartTime + 0.1 * g_int_ImmuneCounter;
		g_tbl_MenuSkillInfo.shieldLastSkillTime = g_tbl_MenuSkillInfo.gameStartTime + 0.1 * g_int_ImmuneCounter;
		g_tbl_MenuSkillInfo.silencerLastSkillTime = g_tbl_MenuSkillInfo.gameStartTime + 0.1 * g_int_ImmuneCounter;
	}

	foreach(idx, hMarine in g_marine_Total_Unshuffled) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		if (g_marine_Mimic != null && g_marine_Mimic.IsValid() && hMarine == g_marine_Mimic) {
			g_lst_MenuProps[idx].role = ROLE.IAF;
		} else {
			g_lst_MenuProps[idx].role = hMarine.GetScriptScope().Role >= ROLE.INFECTED_OFFSET ? hMarine.GetScriptScope().Role - ROLE.INFECTED_OFFSET : hMarine.GetScriptScope().Role;
		}
	}

	local idx = (g_int_Counter / interval) % 5;
	switch (idx) {
		case 0:
			if (g_marine_Scanner != null && g_marine_Scanner.IsValid() && g_marine_Scanner != g_marine_SilencedMarine) {
				RefreshMenu(g_marine_Scanner);
			}
			break;
		case 1:
			if (g_marine_Biochemist != null && g_marine_Biochemist.IsValid() && g_marine_Biochemist != g_marine_SilencedMarine) {
				RefreshMenu(g_marine_Biochemist);
			}
			break;
		case 2:
			if (g_marine_Shield != null && g_marine_Shield.IsValid() && g_marine_Shield != g_marine_SilencedMarine) {
				RefreshMenu(g_marine_Shield);
			}
			break;
		case 3:
			if (g_marine_Infector != null && g_marine_Infector.IsValid() && g_marine_Infector != g_marine_SilencedMarine) {
				RefreshMenu(g_marine_Infector);
			}
			break;
		case 4:
			if (g_marine_Silencer != null && g_marine_Silencer.IsValid() && g_marine_Silencer != g_marine_SilencedMarine) {
				RefreshMenu(g_marine_Silencer);
			}
	}
}

g_float_AbetRatio <- 1.5;
function RefreshMenu(hMarine) {
	local i = -1;

	g_tbl_MenuSkillInfo.infectorIsSkillActive = false;
	if (g_bool_HasInfector && g_int_IafAliveCount >= 4 && g_int_TraitorAliveCount >= 1 && ((g_int_IafAliveCount - 1.0) / (g_int_TraitorAliveCount + 1.0) >= g_float_AbetRatio)) {
		g_tbl_MenuSkillInfo.infectorIsSkillActive = true;
	}

	foreach(tempMarine in g_marine_Total_Unshuffled) {
		i++;
		g_lst_MenuProps[i].isAlive = true;
		if (tempMarine == null || !tempMarine.IsValid()) {
			g_lst_MenuProps[i].isAlive = false;
			g_lst_MenuProps[i].scannerIsWithinRange = true;
			continue;
		}
		if (g_marine_Scanner != null && g_marine_Scanner.IsValid()) {
			if ((g_marine_Scanner.GetOrigin() - tempMarine.GetOrigin()).Length() > 1500) {
				if (Time() > g_lst_MenuProps[i].scannerLastWithinRangeTime + 3.0) {
					g_lst_MenuProps[i].scannerIsWithinRange = false;
				}
			} else {
				g_lst_MenuProps[i].scannerIsWithinRange = true;
				g_lst_MenuProps[i].scannerLastWithinRangeTime = Time();
			}
		}
	}
	i = -1;
	foreach(tempMarine in g_marine_Total_Unshuffled) {
		i++;
		local strVGuiRefresh = "VGui_Refresh" + i.tostring();
		local hVGuiRefresh = null;
		if (!(hVGuiRefresh = Entities.FindByName(hVGuiRefresh, strVGuiRefresh))) {
			hVGuiRefresh = Entities.CreateByClassname("rd_vgui_vscript");
			hVGuiRefresh.__KeyValueFromString("client_vscript", "challenge_traitors_client_refresh_menu.nut");
			hVGuiRefresh.ValidateScriptScope();
			hVGuiRefresh.GetScriptScope().Input <- Input;
			hVGuiRefresh.Spawn();
			hVGuiRefresh.SetName(strVGuiRefresh);
			hVGuiRefresh.Activate();
			hVGuiRefresh.SetEntity(0, hMarine);
			hVGuiRefresh.SetInteracter(null);
		}

		hVGuiRefresh.SetInt(0, i);
		hVGuiRefresh.SetInt(1, (g_lst_MenuProps[i].scannerIsRevealed) ? g_lst_MenuProps[i].role : ROLE.NONE);
		hVGuiRefresh.SetInt(2, g_marine_Total.len());
		hVGuiRefresh.SetInt(3, g_lst_MenuProps[i].isAlive.tointeger());

		//biochemist props
		hVGuiRefresh.SetInt(MENU_IDX_INT.BIOCHEMIST_IS_HEAL_USED, g_tbl_MenuSkillInfo.biochemistIsHealUsed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.BIOCHEMIST_IS_KILL_USED, g_tbl_MenuSkillInfo.biochemistIsKillUsed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.BIOCHEMIST_IS_HEALED, g_lst_MenuProps[i].biochemistIsHealed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.BIOCHEMIST_IS_KILLED, g_lst_MenuProps[i].biochemistIsKilled.tointeger());
		hVGuiRefresh.SetFloat(MENU_IDX_INT.BIOCHEMIST_NEXT_AVAILABLE_TIME, g_tbl_MenuSkillInfo.biochemistLastSkillTime + g_tbl_MenuSkillInfo.biochemistSkillCD);

		//infector props
		hVGuiRefresh.SetInt(MENU_IDX_INT.INFECTOR_IS_SKILL_ACTIVE, g_tbl_MenuSkillInfo.infectorIsSkillActive.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.INFECTOR_IS_SKILL_USED, g_tbl_MenuSkillInfo.infectorIsSkillUsed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.INFECTOR_IS_ABETED, g_lst_MenuProps[i].infectorIsAbeted.tointeger());
		hVGuiRefresh.SetFloat(MENU_IDX_INT.INFECTOR_NEXT_AVAILABLE_TIME, g_tbl_MenuSkillInfo.infectorLastSkillTime + g_tbl_MenuSkillInfo.infectorSkillCD);

		//scanner props
		hVGuiRefresh.SetInt(MENU_IDX_INT.SCANNER_IS_REVEALED, g_lst_MenuProps[i].scannerIsRevealed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.SCANNER_IS_WITHIN_RANGE, g_lst_MenuProps[i].scannerIsWithinRange.tointeger());
		hVGuiRefresh.SetFloat(MENU_IDX_INT.SCANNER_NEXT_AVAILABLE_TIME, g_tbl_MenuSkillInfo.scannerLastSkillTime + g_tbl_MenuSkillInfo.scannerSkillCD);

		//shield props
		hVGuiRefresh.SetInt(MENU_IDX_INT.SHIELD_IS_SKILL_USED, g_tbl_MenuSkillInfo.shieldIsSkillUsed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.SHIELD_IS_SELECTED, g_lst_MenuProps[i].shieldIsSelected.tointeger());
		hVGuiRefresh.SetFloat(MENU_IDX_INT.SHIELD_NEXT_AVAILABLE_TIME, g_tbl_MenuSkillInfo.shieldLastSkillTime + g_tbl_MenuSkillInfo.shieldSkillCD);

		//silencer props
		hVGuiRefresh.SetInt(MENU_IDX_INT.SILENCER_IS_SKILL_USED, g_tbl_MenuSkillInfo.silencerIsSkillUsed.tointeger());
		hVGuiRefresh.SetInt(MENU_IDX_INT.SILENCER_IS_SILENCED, g_lst_MenuProps[i].silencerIsSilenced.tointeger());
		hVGuiRefresh.SetFloat(MENU_IDX_INT.SILENCER_NEXT_AVAILABLE_TIME, g_tbl_MenuSkillInfo.silencerLastSkillTime + g_tbl_MenuSkillInfo.silencerSkillCD);

		if (g_lst_MenuProps[i].isAlive) {
			if (tempMarine.IsInhabited() && tempMarine.GetCommander() != null && tempMarine.GetCommander().IsValid()) {
				hVGuiRefresh.SetString(0, tempMarine.GetCommander().GetPlayerName());
			} else {
				hVGuiRefresh.SetString(0, tempMarine.GetMarineName());
			}
		}
	}
}

function DetectAndApplySkill(interval = 1) {
	if ((g_int_Counter + 3) % interval != 0) {
		return;
	}
	local tbl = getconsttable();
	if (tbl["entindex_scanned"] != -1 && g_marine_Scanner != null && g_marine_Scanner.IsValid()) {
		local entindex = tbl["entindex_scanned"];
		tbl["entindex_scanned"] = -1;
		foreach(idx, menuProps in g_lst_MenuProps) {
			if (menuProps.entIndex == entindex && Time() >= g_tbl_MenuSkillInfo.scannerLastSkillTime + g_tbl_MenuSkillInfo.scannerSkillCD) {
				if (g_tbl_MenuSkillInfo.scannerRandomScanCounter > 0) {
					local rand = RandomHQUniformIntDistribution(0, g_marine_Total_Unshuffled.len() - 5 + g_tbl_MenuSkillInfo.scannerRandomScanCounter);
					foreach(idx, prop in g_lst_MenuProps) {
						if (prop.scannerIsRevealed || prop.entIndex == g_marine_Scanner.entindex()) {
							continue;
						}
						if (rand == 0) {
							g_lst_MenuProps[idx].scannerIsRevealed = true;
							g_tbl_MenuSkillInfo.scannerRandomScanCounter--;

							local hPlayer = g_marine_Scanner.GetCommander();
							local strLanguage = GetClientLanguage(hPlayer.entindex());
							if (g_marine_Scanner.GetCommander() != null && g_marine_Scanner.GetCommander().IsValid()) {
								LocalizedClientPrint(g_marine_Scanner.GetCommander(), 3, GetLocalizedString("#challenge_traitors_role_chat_random_scan", strLanguage));
							}
							break;
						}
						rand--;
					}
				} else {
					if (g_lst_MenuProps[idx].scannerIsWithinRange) {
						g_lst_MenuProps[idx].scannerIsRevealed = true;
					}
				}
				g_tbl_MenuSkillInfo.scannerLastSkillTime = Time();
				g_tbl_MenuSkillInfo.scannerSkillCD = (g_tbl_MenuSkillInfo.scannerSkillCD * 1.618).tointeger();
			}
		}
	}
	if (tbl["entindex_silenced"] != -1 && g_marine_SilencedMarine == null) {
		local entindex = tbl["entindex_silenced"];
		tbl["entindex_silenced"] = -1;
		foreach(idx, hMarine in g_marine_Total_Unshuffled) {
			if (hMarine == null || !hMarine.IsValid() || hMarine.entindex() != entindex) {
				continue;
			}
			local role = hMarine.GetScriptScope().Role;
			local hasVGui = (role == ROLE.SCANNER) || (role == ROLE.BIOCHEMIST) || (role == ROLE.SHIELD) || (role == ROLE.INFECTOR) || (role == ROLE.SILENCER) || (role == ROLE.INFECTED_SCANNER) || (role == ROLE.INFECTED_BIOCHEMIST) || (role == ROLE.INFECTED_SHIELD);
			local hasSkill = !((role == ROLE.IAF) || (role == ROLE.TRAITOR));
			if (hasVGui && hMarine.GetScriptScope().IsOpen == true) {
				Entities.FindByName(null, hMarine.GetScriptScope().strVGuiNameBackground).SetInteracter(null);
				for (local i = 0; i < g_marine_Total.len(); i++) {
					local hVGuiButton = Entities.FindByName(null, hMarine.GetScriptScope().strVGuiNameButton[i]);
					hVGuiButton.SetInteracter(null);
				}
				hMarine.GetScriptScope().IsOpen = false;
			}
			if (hasSkill) {
				local hPlayer = hMarine.GetCommander();
				local strLanguage = GetClientLanguage(hPlayer.entindex());
				local hHud2 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName2);
				hHud2.SetString(0, GetLocalizedString("#challenge_traitors_marine_silenced_notify", strLanguage));

				LocalizedClientPrint(hMarine.GetCommander(), 3, TextColor(250, 250, 250) + "%s1", "#challenge_traitors_marine_silenced_notify");
			}
			g_tbl_MenuSkillInfo.silencerIsSkillUsed = true;
			g_lst_MenuProps[idx].silencerIsSilenced = true;
			g_marine_SilencedMarine = hMarine;
			break;
		}
	}
	if (tbl["entindex_healed"] != -1 && g_marine_HealedMarine == null) {
		local entindex = tbl["entindex_healed"];
		tbl["entindex_healed"] = -1;
		foreach(idx, hMarine in g_marine_Total_Unshuffled) {
			if (hMarine == null || !hMarine.IsValid() || hMarine.entindex() != entindex) {
				continue;
			}

			hMarine.Extinguish();
			hMarine.CureInfestation();
			//hMarine.SetHealth(hMarine.GetMaxHealth());
			g_tbl_ExtraHealth[entindex] = 100.0;
			g_tbl_MenuSkillInfo.biochemistIsHealUsed = true;
			g_lst_MenuProps[idx].biochemistIsHealed = true;
			g_marine_HealedMarine = hMarine;
			break;
		}
	}
	if (tbl["entindex_killed"] != -1 && g_marine_KilledMarine == null) {
		local entindex = tbl["entindex_killed"];
		tbl["entindex_killed"] = -1;
		foreach(idx, hMarine in g_marine_Total_Unshuffled) {
			if (hMarine == null || !hMarine.IsValid() || hMarine.entindex() != entindex) {
				continue;
			}

			if (hMarine != g_marine_Deserter) {
				hMarine.Die();
			} else {
				if (Time() > g_marine_Deserter.GetScriptScope().RevealTime + 30.0) {
					hMarine.SetHealth(1);
				}
				hMarine.TakeDamage(999, DAMAGE_TYPE.DMG_FALL, g_marine_Biochemist);
			}
			g_tbl_MenuSkillInfo.biochemistIsKillUsed = true;
			g_lst_MenuProps[idx].biochemistIsKilled = true;
			g_marine_KilledMarine = hMarine;
			break;
		}
	}
	if (tbl["entindex_abeted"] != -1 && g_marine_AbetedMarine == null) {
		local entindex = tbl["entindex_abeted"];
		tbl["entindex_abeted"] = -1;
		foreach(idx, hMarine in g_marine_Total_Unshuffled) {
			if (hMarine == null || !hMarine.IsValid() || hMarine.entindex() != entindex || hMarine.GetScriptScope().Role > ROLE.MAX_IAF_TEAM) {
				continue;
			}
			if (!(g_bool_HasInfector && g_int_IafAliveCount >= 4 && g_int_TraitorAliveCount >= 1 && ((g_int_IafAliveCount - 1.0) / (g_int_TraitorAliveCount + 1.0) >= g_float_AbetRatio))) {
				g_tbl_MenuSkillInfo.infectorIsSkillActive = false;
				break;
			}
			g_marine_Infected = hMarine;
			hMarine.GetScriptScope().Role += ROLE.INFECTED_OFFSET;

			g_marine_Traitor.append(hMarine); // 更新内鬼列表
			g_marine_TraitorAlive.append(hMarine); // 更新存活内鬼列表
			g_int_TraitorCount = g_marine_Traitor.len(); // 更新内鬼数量
			g_int_TraitorAliveCount = g_marine_TraitorAlive.len(); // 更新存活内鬼数量

			if (hMarine.GetCommander() != null && hMarine.GetCommander().IsValid()) {
				local hPlayer = hMarine.IsInhabited() ? hMarine.GetCommander() : null;
				local role = hMarine.GetScriptScope().Role;
				local strRole = GetRoleString(role);

				g_player_Traitor.append(hPlayer);

				GenerateTraitorList();
				if (hPlayer != null && hPlayer.IsValid()) {
					LocalizedClientPrint(hPlayer, 3, " ");
					LocalizedClientPrint(hPlayer, 3, TextColor(255, 0, 0) + "%s1", "#challenge_traitors_chat_on_game_start", null, null, null, role);
					LocalizedClientPrint(hPlayer, 3, TextColor(255, 0, 0) + "%s1%s2" + GetExtraHealthString(role), "#challenge_traitors_role_chat_role_description_" + strRole, "#challenge_traitors_role_chat_skill_description_" + strRole, null, null, role);
					LocalizedClientPrint(hPlayer, 3, g_str_TraitorNameList, "#challenge_traitors_traitor_list", "#challenge_traitors_traitor_player_unavailable");
					SetHudForTraitorPlayer(hMarine, role, 0);
				}

				//通知其他内鬼玩家
				foreach(hTempPlayer in g_player_TraitorHistory) {
					if (hTempPlayer == null || !hTempPlayer.IsValid()) {
						continue;
					}
					LocalizedClientPrint(hTempPlayer, 3, TextColor(255, 0, 0) + "%s1", "#challenge_traitors_traitor_list_changed");
					LocalizedClientPrint(hTempPlayer, 3, TextColor(255, 0, 0) + g_str_TraitorNameList, "#challenge_traitors_traitor_list", "#challenge_traitors_traitor_player_unavailable");

					local strLanguage = GetClientLanguage(hTempPlayer.entindex());
					local hHud4 = Entities.FindByName(null, hTempPlayer.GetScriptScope().strHudName4);
					hHud4.SetString(0, GenerateTraitorListHUD(strLanguage));

					local hHud3 = Entities.FindByName(null, hTempPlayer.GetScriptScope().strHudName3);
					hHud3.SetInt(0, ROLE.TRAITOR);
					hHud3.SetInt(63, 2);
					hHud3.SetFloat(0, 0.1);
					hHud3.SetFloat(1, 0.5);
					hHud3.SetFloat(2, 0.65);
					hHud3.SetFloat(30, Time() + 2.0);
					hHud3.SetFloat(31, 10.0);
					hHud3.SetString(0, GetLocalizedString("#challenge_traitors_traitor_list_changed", strLanguage));
				}
				g_player_TraitorHistory.append(hMarine.IsInhabited() ? hPlayer : null);
			}
			local flg = false;
			local temp = null;
			foreach(tempidx, tempMarine in g_marine_Iaf) {
				if (hMarine == tempMarine) {
					flg = true; // 标记Iaf队员
					temp = tempidx; // 标记Iaf队员的索引
				}
			}
			if (flg) {
				g_marine_Iaf.remove(temp); // 从列表中移除IAF队员
			}
			flg = false;
			temp = null;
			foreach(tempidx, tempMarine in g_marine_IafAlive) {
				if (hMarine == tempMarine) {
					flg = true; // 标记Iaf队员
					temp = tempidx; // 标记Iaf队员的索引
				}
			}
			if (flg) {
				g_marine_IafAlive.remove(temp); // 从存活列表中移除IAF队员
				g_int_IafAliveCount = g_marine_IafAlive.len(); // 更新存活的IAF队员数量
			}
			g_lst_MenuProps[idx].infectorIsAbeted = true;
			g_marine_AbetedMarine = hMarine;
			g_tbl_MenuSkillInfo.infectorIsSkillUsed = true;
			break;
		}
	}
	if (tbl["entindex_mecha_given"] != -1 && g_marine_ShieldedMarine == null) {
		local entindex = tbl["entindex_mecha_given"];
		tbl["entindex_mecha_given"] = -1;
		foreach(idx, hMarine in g_marine_Total_Unshuffled) {
			if (hMarine == null || !hMarine.IsValid() || hMarine.entindex() != entindex) {
				continue;
			}
			g_tbl_MenuSkillInfo.shieldIsSkillUsed = true;
			g_lst_MenuProps[idx].shieldIsSelected = true;
			g_tbl_Shield[entindex] = 200;
			g_marine_ShieldedMarine = hMarine;
			break;
		}
	}
}

function RemoveAmmoOnMap(interval) {
	if (g_int_Counter % interval != 0) {
		return;
	}
	foreach(classname in AMMO_NAME) {
		ammo <- null;
		while ((ammo = Entities.FindByClassname(ammo, classname)) != null) {
			ammo.Destroy();
		}
	}
}

function SetPlayerAmmo(interval) {
	if ((g_int_Counter + 6) % interval != 0) {
		return;
	}
	foreach(hMarine in g_marine_Total) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		local hWeapon = NetProps.GetPropEntity(hMarine, "m_hActiveWeapon");
		if (hWeapon) {
			if (hWeapon.GetMaxClips() > 0) {
				hWeapon.SetClips(hWeapon.GetMaxClips());
			}
		}
	}
}

function UpdateTraitorHud(interval) {
	if ((g_int_Counter + 5) % interval != 0) {
		return;
	}
	local hHud;
	foreach(hPlayer in g_player_TraitorHistory) {
		if (hPlayer == null || !hPlayer.IsValid()) {
			continue;
		}
		hHud = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName3);
		if (hHud == null) {
			continue;
		}

		local time = Time();
		if (hHud.GetInt(63) == 1 && Time() > hHud.GetFloat(31) + hHud.GetFloat(30)) {
			local strLanguage = GetClientLanguage(hPlayer.entindex());
			hHud.SetInt(63, 2);
			hHud.SetFloat(30, Time());
			hHud.SetFloat(31, 10.0);
			hHud.SetString(0, GetLocalizedString("#challenge_traitors_game_instruction_traitor2", strLanguage));
		}
	}
}

ProcessMapSpecialCases <- function() {}; //Used to process edge cases of different maps. This function is actually set in challenge_traitors_map_handler.nut 函数指针，在游戏开始时根据不同地图绑定对应的处理函数

function RemoveBot(interval) {
	if ((g_int_Counter + 7) % interval != 0) {
		return;
	}
	foreach(hMarine in g_marine_Total) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		if (!hMarine.IsInhabited()) {
			if (hMarine.GetHealth() > 15) {
				hMarine.SetHealth(15);
			}
			// hMarine.Die();// 这里不杀死机器人，这样意外掉出地图，还有希望丝血传送回来。This will kill bot, but it would be beter to set bot health to 1 so that one can have a chance to teleport back if they fall outside of the map, also preventing players from abuse bot tp.
		}
	}
}


function CheckSurrenderVotes(interval) {
	if ((g_int_Counter + 7) % interval != 0) {
		return;
	}

	g_tbl_surrenderVotes.TotalTickets.clear();
	foreach(hPlayer in g_player_TraitorHistory) //更新此刻的全部内鬼数量，不包含已经退出的玩家
	{
		if (hPlayer == null || !hPlayer.IsValid()) {
			continue;
		}
		g_tbl_surrenderVotes.TotalTickets[hPlayer.GetPlayerUserID()] <- hPlayer;
	}
	if (g_tbl_surrenderVotes.CurrentTickets.len() != 0 && Time() > g_tbl_surrenderVotes.VoteStartTime + g_tbl_surrenderVotes.Duration) { //如果超过投票时间，则清空当前投票
		g_tbl_surrenderVotes.CurrentTickets.clear();
	} else if (Time() < g_tbl_surrenderVotes.VoteStartTime + g_tbl_surrenderVotes.Duration) {
		local count = 0;
		foreach(hPlayer in g_tbl_surrenderVotes.CurrentTickets) //计算此刻的投赞成票内鬼数量，不包含已经退出的玩家
		{
			if (hPlayer != null && hPlayer.IsValid()) {
				count++;
			}
		}
		if ((count.tofloat() / g_tbl_surrenderVotes.TotalTickets.len().tofloat()) > (2.0 / 3.0)) {
			g_bool_IafWin = true;
			g_bool_TraitorSurrender = true;
			Director.MissionComplete(true);
		}
		foreach(hPlayer in g_player_TraitorHistory) //设置显示的投票信息
		{
			if (hPlayer == null || !hPlayer.IsValid()) {
				continue;
			}
			local hHud3 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName3);
			local strLanguage = GetClientLanguage(hPlayer.entindex());
			if (hHud3 != null && hHud3.GetInt(63) != 4) {
				hHud3.SetInt(3, count);
				hHud3.SetInt(4, g_tbl_surrenderVotes.TotalTickets.len());
				hHud3.SetInt(63, 4);
				hHud3.SetFloat(30, g_tbl_surrenderVotes.VoteStartTime);
				hHud3.SetFloat(31, g_tbl_surrenderVotes.Duration);
				hHud3.SetString(0, GetLocalizedString("#challenge_traitors_game_instruction_traitor3", strLanguage));
			}
		}
	}
}

function CheckAndShowWinner() {
	if (g_bool_IafWin) {
		if (!g_bool_IafWinDelayFlag) {
			g_bool_IafWinDelayFlag = true;
			return 1;
		}

		ShowSpeciallRolesList();
		LocalizedClientPrint(null, 3, g_str_TraitorNameList, "#challenge_traitors_traitor_list", "#challenge_traitors_traitor_player_unavailable");
		if (g_bool_TraitorSurrender) {
			LocalizedClientPrint(null, 3, TextColor(255, 128, 0) + "%s1", "#challenge_traitors_traitor_surrender");
		}
		LocalizedClientPrint(null, 3, TextColor(255, 128, 0) + "%s1", "#challenge_traitors_iaf_win");
		PlayMissionEndSound("IAF");
		WriteMatchResultToFile(1);
		DestroyHudAndVGui();
		return 65536;
	} else if (g_bool_TraitorWin) {
		if (!g_bool_TraitorWinDelyFlag) {
			g_bool_TraitorWinDelyFlag = true;
			return 1;
		}

		ShowSpeciallRolesList();
		LocalizedClientPrint(null, 3, g_str_TraitorNameList, "#challenge_traitors_traitor_list", "#challenge_traitors_traitor_player_unavailable");
		LocalizedClientPrint(null, 3, TextColor(255, 0, 0) + "%s1", "#challenge_traitors_traitor_win");
		PlayMissionEndSound("TRAITOR");
		WriteMatchResultToFile(-1);
		DestroyHudAndVGui();
		return 65536;
	}
	return 0.1;
}

g_float_GameStartTime <- Time();
g_str_GameResult <- ""; // MapName, PlayerCount, TraitorCount, Result, GameDuration, SpecialRoles
function WriteMatchResultToFile(winner) {
	return;
	//1		IAF
	//-1	Traitor
	//0		No winner
	//Already has MapName, PlayerCount, TraitorCount  + "\t"
	g_str_GameResult += winner.tostring() + "\t" + (Time() - g_float_GameStartTime).tostring() + "\t" + g_bool_HasScanner.tointeger().tostring() + "\t" + g_bool_HasBiochemist.tointeger().tostring() + "\t" + g_bool_HasIafLeader.tointeger().tostring() + "\t" + g_bool_HasShield.tointeger().tostring() + "\t" + g_bool_HasSniper.tointeger().tostring() + "\t" + g_bool_HasDemo.tointeger().tostring() + "\t" + g_bool_HasDeserter.tointeger().tostring() + "\t" + g_bool_HasTraitorLeader.tointeger().tostring() + "\t" + g_bool_HasInfector.tointeger().tostring() + "\t" + g_bool_HasBoomer.tointeger().tostring() + "\t" + g_bool_HasSilencer.tointeger().tostring() + "\t" + g_bool_HasMimic.tointeger().tostring();
	StringToFile("Challenge_Traitor_Result_" + GetLocalTime().tostring() + ".txt", g_str_GameResult);
}

function ShowSpeciallRolesList() {
	local hPlayer = null;
	while (hPlayer = Entities.FindByClassname(hPlayer, "player")) {
		local strLanguage = GetClientLanguage(hPlayer.entindex());
		local str = "";
		foreach(key, strPlayerName in g_tbl_RoleInfo) {
			if (strPlayerName != "") {
				str = str + "[" + GetLocalizedString("#challenge_traitors_role_name_" + key.tostring().tolower(), strLanguage) + "]" + strPlayerName + "  ";
			}
		}
		if (str != "") {
			LocalizedClientPrint(hPlayer, 3, TextColor(255, 255, 210) + "%s1  " + str, "#challenge_traitors_roles_in_this_round");
		}
	}
}

function DestroyHudAndVGui() {
	foreach(entity in g_ent_HudAndVGui) {
		if (entity != null && entity.IsValid()) {
			entity.Destroy();
		}
	}
}

function PlayMissionEndSound(strWinner) {
	local hPlayer = null;
	local music;
	local voice;
	music = MISSION_END_SOUND[strWinner].MUSIC;
	voice = MISSION_END_SOUND[strWinner].VOICE;
	while (hPlayer = Entities.FindByClassname(hPlayer, "player")) {
		hPlayer.PrecacheSoundScript(music);
		hPlayer.PrecacheSoundScript(voice);
		hPlayer.EmitSound(music);
		hPlayer.EmitSound(voice);
	}
}

function OnMissionStart() {
	// 修复Reduction 5没有武器的问题
	local hEntity = null;
	while ((hEntity = Entities.FindByName(hEntity, "WeaponStrip")) != null) {
		hEntity.Destroy();
	}
	hEntity = null;
	while ((hEntity = Entities.FindByName(hEntity, "BCar_Cut_Progress")) != null) {
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_sarge_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_jaeger_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_wildcat_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_wolf_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_faith_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_bastille_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_crash_weapon", "Kill", "");
		EntityOutputs.RemoveOutput(hEntity, "OnHitMax", "#asw_name_vegas_weapon", "Kill", "");
	}
}

function OnGameplayStart() {
	SetConVars(); //设置cvar
	SetTechMap(); //设置技术地图标识，暂时没有作用
	SetImmuneTimeAndFlags(); //设置出生保护，记录当前地图
	SetMapHandler(); //设置处理地图的函数

	InitializeMarineList(); //初始化士兵列表，移除机器人
	FixMapIssueOnStart(); //修复玩家出生点问题以及Reduction的几个地图无法正常玩的问题
	DeterminRoleCount(); //设置各种角色数量
	for (local i = 0; i < 5; i++) { // 多次洗牌
		g_marine_Total = FisherYatesShuffle(g_marine_Total);
	}
	SelectRoles(); //抽取角色
	GenerateTraitorList(); //生成内鬼列表
	InitializeExtraHealthAndShield(); //初始化额外血条和护盾

	CreatePlayerHudAndVGuiEntities(); //创建玩家HUD和VGui实体
	CreateMarineHudAndVGuiEntities(); //创建士兵HUD和VGui实体

	DisplayGameInstructions(); //显示游戏指引

	g_bool_Initialized = true; //设置初始化完成标识

	SetInitialAmmo(); //设置初始弹药量

	g_str_GameResult = GetMapName().tolower() + "\t" + g_int_MarineCount.tostring() + "\t" + g_int_TraitorCount.tostring() + "\t"; //MapName, PlayerCount, TraitorCount

	DelayFunctionCall("ClientPrintRoles", "", 2.0); // 2秒后显示内鬼名单
}

function DisplayGameInstructions() {
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction0");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction1");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction2");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction3");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction4");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction5");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction6");
	LocalizedClientPrint(null, 3, TextColor(255, 255, 0) + "%s1", "#challenge_traitors_game_instruction8");
}

function InitializeMarineList() {
	// 遍历士兵，同时移除机器人（防止有人在进入游戏瞬间掉线。这个概率很低，但需要排除）
	local hMarine = null;
	while ((hMarine = Entities.FindByClassname(hMarine, "asw_marine")) != null) {
		hMarine.ValidateScriptScope();
		hMarine.GetScriptScope().strFlashbangHudName <- "strFlashbangHudNameNotSet";
		hMarine.GetScriptScope().DamageMapModifier <- 1.0;
		if (!hMarine.IsInhabited()) {
			hMarine.RemoveWeapon(0);
			hMarine.RemoveWeapon(1);
			hMarine.RemoveWeapon(2);
			hMarine.SetOrigin(hMarine.GetOrigin() + Vector(0, -5000, 0));
			hMarine.Die();
		} else {
			g_marine_Total.append(hMarine); // 将所有士兵句柄存入列表，句柄作为士兵的唯一标识
			g_lst_MenuProps.append(InitializeMenuProps(hMarine));
			g_int_MarineCount += 1;
		}
	}
	g_marine_Total_Unshuffled = g_marine_Total.slice(0, g_int_MarineCount);
}

function SetInitialAmmo() {
	local hWeapon = null;
	while (hWeapon = Entities.FindByClassname(hWeapon, "asw_weapon_heavy_rifle")) {
		hWeapon.SetClip2(1);
	}
	hWeapon = null;
	while (hWeapon = Entities.FindByClassname(hWeapon, "asw_weapon_medrifle")) {
		hWeapon.SetClip2(20);
	}
	hWeapon = null;
	while (hWeapon = Entities.FindByClassname(hWeapon, "asw_weapon_sniper_rifle")) {
		hWeapon.SetClip1(1);
	}
	hWeapon = null;
	while (hWeapon = Entities.FindByClassname(hWeapon, "asw_weapon_sentry_cannon")) {
		NetProps.SetPropInt(hWeapon, "m_nSentryAmmo", 200);
	}
}

function InitializeMenuProps(hMarine) {
	local menuProps = {
		entIndex = hMarine.entindex(),
		isAlive = true,
		role = ROLE.NONE,
		biochemistIsHealed = false,
		biochemistIsKilled = false,
		infectorIsAbeted = false,
		scannerIsRevealed = false,
		scannerIsWithinRange = false,
		scannerLastWithinRangeTime = 0.0,
		shieldIsSelected = false,
		silencerIsSilenced = false,
	};
	return menuProps;
}

g_int_TraitorLeaderExtraHealth <- 150;
g_int_IafLeaderExtraHealth <- 100;
function InitializeExtraHealthAndShield() {
	foreach(hMarine in g_marine_Total) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		hMarine.ValidateScriptScope();
		g_tbl_ExtraHealth[hMarine.entindex()] <- -1.0;
		g_tbl_Shield[hMarine.entindex()] <- -1.0;
	}
	if (g_marine_IafLeader != null && g_marine_IafLeader.IsValid()) {
		g_tbl_ExtraHealth[g_marine_IafLeader.entindex()] = g_int_IafLeaderExtraHealth;
	}
	if (g_marine_TraitorLeader != null && g_marine_TraitorLeader.IsValid()) {
		g_tbl_ExtraHealth[g_marine_TraitorLeader.entindex()] = g_int_TraitorLeaderExtraHealth;
	}
}

function DeterminRoleCount() {
	// 确定各种数量
	local tempRandom = RandomHQUniformIntDistribution(0, 1)
	switch (g_int_MarineCount) {
		case 0:
		case 1:
		case 2:
			g_bool_ClhallengeEnable = false;

			g_bool_HasScanner = true;
			//g_bool_HasBiochemist = true;
			//g_bool_HasIafLeader = true;
			//g_bool_HasSniper = true;
			//g_bool_HasDemo = true;
			//g_bool_HasShield = true;
			//g_bool_HasDeserter = true;

			//g_bool_HasTraitorLeader = true;
			g_bool_HasBoomer = true;
			//g_bool_HasMimic = true;
			//g_bool_HasSilencer = true;
			//g_bool_HasInfector = true;

			LocalizedClientPrint(null, 3, TextColor(160, 32, 240) + "%s1", "#challenge_traitors_not_enough_players");
			if (DEBUG) {
				g_int_TraitorCount = 0; //RandomHQUniformIntDistribution(0, 1);
				//tempRandom;
				g_bool_ClhallengeEnable = true;
				LocalizedClientPrint(null, 3, TextColor(255, 0, 0) + "You are now in DEBUG mode");
			} // Debug only
			break;
		case 3:
			g_int_TraitorCount = 1;
			g_bool_HasTraitorLeader = (RandomHQUniformIntDistribution(0, 2) == 0);
			g_int_TraitorLeaderExtraHealth = 60;
			break; //1
		case 4:
			g_int_TraitorCount = 1;
			g_bool_HasTraitorLeader = true;
			g_int_TraitorLeaderExtraHealth = 130;
			break; //1
		case 5:
			g_int_TraitorCount = 1;
			g_bool_HasTraitorLeader = true;
			g_int_TraitorLeaderExtraHealth = 350;
			break; //1
		case 6:
			g_int_TraitorCount = 2;
			g_bool_HasTraitorLeader = true;
			g_int_TraitorLeaderExtraHealth = 150;
			break; //2
		case 7:
			g_int_TraitorCount = 2;
			g_bool_HasIafLeader = true;
			g_int_TraitorLeaderExtraHealth = 200;
			g_bool_HasScanner = (RandomHQUniformIntDistribution(0, 20) == 0);
			g_bool_HasBiochemist = (RandomHQUniformIntDistribution(0, 14) == 0);
			g_bool_HasInfector = g_bool_HasBiochemist;
			g_bool_HasSilencer = g_bool_HasScanner;
			g_bool_HasTraitorLeader = g_bool_HasIafLeader || (RandomHQUniformIntDistribution(0, 3) == 0);
			switch (RandomHQUniformIntDistribution(0, 6)) {
				case 0:
					//g_bool_HasInfector = true;
					break;
				case 1:
					g_bool_HasBoomer = true;
					break;
				case 2:
					g_bool_HasSilencer = true;
					break;
			}
			g_bool_HasShield = (RandomHQUniformIntDistribution(0, 12) == 0);
			g_bool_HasSniper = (RandomHQUniformIntDistribution(0, 12) == 0);
			g_bool_HasDemo = (RandomHQUniformIntDistribution(0, 12) == 0);
			g_bool_HasDeserter = (RandomHQUniformIntDistribution(0, 8) == 0);
			break; //2
		case 8:
			g_int_TraitorCount = 2;
			g_bool_HasScanner = (RandomHQUniformIntDistribution(0, 10) == 0);
			g_bool_HasBiochemist = (RandomHQUniformIntDistribution(0, 7) == 0);
			g_bool_HasIafLeader = (RandomHQUniformIntDistribution(0, 1) == 0);
			g_bool_HasInfector = (RandomHQUniformIntDistribution(0, 1) == 0);
			g_bool_HasTraitorLeader = true;
			g_int_TraitorLeaderExtraHealth = 350;
			switch (RandomHQUniformIntDistribution(0, 5)) {
				case 0:
					//g_bool_HasInfector = true;
					break;
				case 1:
					g_bool_HasBoomer = true;
					break;
				case 2:
					g_bool_HasSilencer = true;
					break;
			}
			g_bool_HasShield = (RandomHQUniformIntDistribution(0, 8) == 0);
			g_bool_HasSniper = (RandomHQUniformIntDistribution(0, 12) == 0);
			g_bool_HasDemo = (RandomHQUniformIntDistribution(0, 8) == 0);
			g_bool_HasDeserter = (RandomHQUniformIntDistribution(0, 6) == 0);
			break; //2
		case 9:
			g_int_TraitorCount = 3;
			break; //3
		case 10:
			g_int_TraitorCount = 3;
			break; //3
		case 11:
			g_int_TraitorCount = 3;
			break; //3
		case 12:
			g_int_TraitorCount = 4;
			break; //3-4
		case 13:
			g_int_TraitorCount = 4;
			break; //3-4
		case 14:
			g_int_TraitorCount = 4 + tempRandom;
			break; //4
		case 15:
			g_int_TraitorCount = 4 + tempRandom;
			break; //4
		default:
			g_int_TraitorCount = (g_int_MarineCount / 3).tointeger() - 1 + (g_int_MarineCount % 3 == 2 ? tempRandom : 0);
			break;
	}
	if (g_int_MarineCount >= 9 && g_int_MarineCount < 12) {
		g_bool_HasScanner = true;
		g_bool_HasBiochemist = true;
		g_bool_HasIafLeader = (RandomHQUniformIntDistribution(0, 14 - g_int_MarineCount) == 0);
		g_bool_HasShield = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);
		if ((RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0)) {
			g_bool_HasSniper = (RandomHQUniformIntDistribution(0, 1) == 0);
			g_bool_HasDemo = !g_bool_HasSniper;
		}
		g_bool_HasDeserter = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);

		g_bool_HasInfector = true;
		g_bool_HasSilencer = (RandomHQUniformIntDistribution(0, 3) != 0);
		g_bool_HasMimic = !g_bool_HasSilencer;
		g_bool_HasTraitorLeader = true;
		g_int_TraitorLeaderExtraHealth = 300;
		if ((RandomHQUniformIntDistribution(0, 12 - g_int_MarineCount) == 0)) {
			g_bool_HasBoomer = (RandomHQUniformIntDistribution(0, 1) == 0);
			g_bool_HasInfector = !g_bool_HasBoomer;
		}
	}
	if (g_int_MarineCount >= 12 && g_int_MarineCount < 16) {
		g_bool_HasScanner = true;
		g_bool_HasBiochemist = true;
		g_bool_HasIafLeader = true;
		g_bool_HasShield = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);
		g_bool_HasSniper = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);
		g_bool_HasDemo = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);
		g_bool_HasDeserter = true;

		g_bool_HasInfector = true;
		g_bool_HasSilencer = true;
		g_bool_HasTraitorLeader = true;
		g_int_TraitorLeaderExtraHealth = 350;
		g_bool_HasMimic = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);
		g_bool_HasBoomer = (RandomHQUniformIntDistribution(0, 16 - g_int_MarineCount) == 0);
		g_bool_HasInfector = !g_bool_HasBoomer;
	}
	if (g_int_MarineCount >= 16) {
		g_bool_HasScanner = true;
		g_bool_HasBiochemist = true;
		g_bool_HasIafLeader = true;
		g_bool_HasShield = true;
		g_bool_HasSniper = true;
		g_bool_HasDemo = true;
		g_bool_HasDeserter = true;

		g_bool_HasTraitorLeader = true;
		g_bool_HasInfector = true;
		g_bool_HasBoomer = true;
		g_bool_HasSilencer = true;
		g_bool_HasMimic = true;
	}
	g_int_TraitorAliveCount = g_int_TraitorCount;
	g_int_IafAliveCount = g_int_MarineCount - g_int_TraitorCount;
}

function DelayFunctionCall(function_name, function_params, delay) {
	if (!this["self"])
		return;

	// this[ function_name ]( function_params );
	EntFireByHandle(this["self"], "RunScriptCode", "this[\"" + function_name + "\"](" + function_params + ");", delay, null, null);
}

function ClientPrintRoles() {
	foreach(hMarine in g_marine_Total) {
		if (!hMarine || !hMarine.IsValid() || !hMarine.IsInhabited()) {
			continue;
		}
		local hPlayer = hMarine.GetCommander();
		local strLanguage = GetClientLanguage(hPlayer.entindex());

		local str = "";
		foreach(key, strPlayerName in g_tbl_RoleInfo) {
			if (strPlayerName != "") {
				str = str + GetLocalizedString("#challenge_traitors_role_name_" + key.tostring().tolower(), strLanguage) + "  ";
			}
		}
		if (str != "") {
			LocalizedClientPrint(hPlayer, 3, " ");
			LocalizedClientPrint(hPlayer, 3, TextColor(255, 255, 210) + "%s1  " + str, "#challenge_traitors_roles_in_this_round");
		}
	}

	foreach(hMarine in g_marine_TraitorAlive) {
		if (!hMarine || !hMarine.IsValid() || !hMarine.IsInhabited()) {
			continue;
		}
		local hPlayer = hMarine.GetCommander();
		local role = hMarine.GetScriptScope().Role;
		local strRole = GetRoleString(role);
		LocalizedClientPrint(hPlayer, 3, " ");
		LocalizedClientPrint(hPlayer, 3, TextColor(255, 0, 0) + "%s1", "#challenge_traitors_chat_on_game_start", null, null, null, role);
		LocalizedClientPrint(hPlayer, 3, TextColor(255, 0, 0) + "%s1%s2" + GetExtraHealthString(role), "#challenge_traitors_role_chat_role_description_" + strRole, "#challenge_traitors_role_chat_skill_description_" + strRole, null, null, role);
		LocalizedClientPrint(hPlayer, 3, g_str_TraitorNameList, "#challenge_traitors_traitor_list", "#challenge_traitors_traitor_player_unavailable");
	}
	foreach(hMarine in g_marine_IafAlive) {
		if (!hMarine || !hMarine.IsValid() || !hMarine.IsInhabited()) {
			continue;
		}
		local hPlayer = hMarine.GetCommander();
		local role = hMarine.GetScriptScope().Role;
		local strRole = GetRoleString(role);
		LocalizedClientPrint(hPlayer, 3, " ");
		LocalizedClientPrint(hPlayer, 3, TextColor(255, 255, 210) + "%s1", "#challenge_traitors_chat_on_game_start", null, null, null, role);
		LocalizedClientPrint(hPlayer, 3, TextColor(255, 255, 210) + "%s1%s2" + GetExtraHealthString(role), "#challenge_traitors_role_chat_role_description_" + strRole, "#challenge_traitors_role_chat_skill_description_" + strRole, null, null, role);
	}
}

function GetExtraHealthString(role) {
	local tempStr = "";
	if (role == ROLE.TRAITOR_LEADER) {
		tempStr = "[+" + g_int_TraitorLeaderExtraHealth.tostring() + "HP]";
	} else if (role == ROLE.IAF_LEADER) {
		tempStr = "[+" + g_int_IafLeaderExtraHealth.tostring() + "HP]";
	} else if (role == ROLE.IAF_LEADER + ROLE.INFECTED_OFFSET) {
		tempStr = "[+" + g_int_IafLeaderExtraHealth.tostring() + "HP]";
	}
	return tempStr;
}

function FixMapIssueOnStart() {
	foreach(hMarine in g_marine_Total) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		// 防止acc32-4中玩家出生在隧道里
		if (g_enum_CurrentMap == MAP.ACC_4 && hMarine.GetOrigin().y > -5000) {
			hMarine.SetOrigin(Vector(-1012 + RandomInt(0, 30), -6204 + RandomInt(0, 30), 569 + RandomInt(0, 30)));
		} else if (g_enum_CurrentMap == MAP.BON_1 && (hMarine.GetOrigin().y > -4000 || hMarine.GetOrigin().z < (-400))) {
			hMarine.SetOrigin(Vector(1100, -4500, -300));
		} else if (g_enum_CurrentMap == MAP.TILA_8 && hMarine.GetOrigin().z < 1600) {
			hMarine.SetOrigin(Vector(656, -5160, 1760));
		} else if (g_enum_CurrentMap == MAP.RES_4) {
			hMarine.SetOrigin(Vector(-4770 + RandomInt(0, 50), -3950 + RandomInt(0, 50), 1700));
		} else if (g_enum_CurrentMap == MAP.TFT_2) {
			hMarine.SetOrigin(Vector(RandomInt(0, 50), -3700 + RandomInt(0, 50), -959));
		} else if (g_enum_CurrentMap == MAP.BIO_1) {
			hMarine.SetOrigin(Vector(1080, 575, -110));
		} else if (g_enum_CurrentMap == MAP.BIO_2) {
			hMarine.SetOrigin(Vector(2800, 300, 100));
		} else if (g_enum_CurrentMap == MAP.RED_1) {
			g_int_JumpHeight = 80;
		} else if (g_enum_CurrentMap == MAP.RED_5) {
			g_int_JumpHeight = 100;
		} else if (g_enum_CurrentMap == MAP.RED_6) {
			local strArr = [
				"Cart1_AreYouGuardingMe_Trig_L",
				"Cart1_AreYouGuardingMe_Trig2",
				"Cart1_DmgCntr_1",
				"Cart1_DmgCntr_2",
				"Cart1_DmgCntr_3",
				"Cart1_DmgCntr_4",
				"Cart1_DmgCntr_5",
			];
			local hEntity = null;
			foreach(str in strArr) {
				if (hEntity = Entities.FindByName(null, str)) {
					hEntity.Destroy();
				}
			}
		} else if (g_enum_CurrentMap == MAP.OCS_3) {
			g_int_JumpHeight = 80;
		}
	}



	local hTurret = null;
	if (g_enum_CurrentMap == MAP.TILA_9) {
		hTurret = Entities.FindByName(null, "ship_turrets");
		NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
		hTurret = Entities.FindByName(null, "ship_turrets_2");
		NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
	} else if (g_enum_CurrentMap == MAP.BIO_1) {
		hTurret = Entities.FindByName(null, "turrets");
		NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
	}
	/* else if (g_enum_CurrentMap == MAP.JAC_3 || g_enum_CurrentMap == MAP.JAC_7) {
			hTurret = Entities.FindByName(null, "ship_turrets");
			NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
		}*/
}

function CreatePlayerHudAndVGuiEntities() {
	local hPlayer = null;
	while (hPlayer = Entities.FindByClassname(hPlayer, "player")) {
		CreatePlayerHud(hPlayer);
	}
	//根据角色设置hud文本
	//1.设置IAF队员
	foreach(hMarine in g_marine_Iaf) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		SetHudForIafPlayer(hMarine, hMarine.GetScriptScope().Role);
	}
	//2.设置内鬼，初始化阶段，内鬼玩家列表就是当前内鬼玩家
	foreach(hMarine in g_marine_Traitor) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		SetHudForTraitorPlayer(hMarine, hMarine.GetScriptScope().Role);
	}
}

function SetHudForIafPlayer(hMarine, role) {
	if (!hMarine || !hMarine.IsValid() || !hMarine.IsInhabited()) {
		return;
	}
	local strRole = GetRoleString(role);
	local hPlayer = hMarine.GetCommander();
	local strLanguage = GetClientLanguage(hPlayer.entindex());

	local hHud1 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName1);
	hHud1.SetInt(0, role);
	hHud1.SetString(0, GetLocalizedString("#challenge_traitors_hud_on_game_start", strLanguage, role));

	local hHud2 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName2);
	hHud2.SetInt(0, role);
	hHud2.SetString(0, GetLocalizedString("#challenge_traitors_role_hud_role_description_" + strRole, strLanguage, role) + GetExtraHealthString(role));
}

function SetHudForTraitorPlayer(hMarine, role, timeOffset = 2.0) {
	if (hMarine == null || !hMarine.IsValid() || !hMarine.IsInhabited()) {
		return;
	}
	local strRole = GetRoleString(role);
	local hPlayer = hMarine.GetCommander();
	local strLanguage = GetClientLanguage(hPlayer.entindex());
	local hHud1 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName1);
	hHud1.SetInt(0, role);
	hHud1.SetFloat(30, Time() + timeOffset);
	hHud1.SetString(0, GetLocalizedString("#challenge_traitors_hud_on_game_start", strLanguage, role));

	local hHud2 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName2);
	hHud2.SetInt(0, role);
	hHud2.SetFloat(30, Time() + timeOffset);
	hHud2.SetString(0, GetLocalizedString("#challenge_traitors_role_hud_role_description_" + strRole, strLanguage, role) + GetExtraHealthString(role));

	local hHud3 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName3);
	hHud3.SetInt(0, role);
	hHud3.SetInt(63, 1);
	hHud3.SetFloat(30, Time() + timeOffset);
	hHud3.SetFloat(31, 10.0);
	hHud3.SetString(0, GetLocalizedString("#challenge_traitors_game_instruction_traitor1", strLanguage));

	local hHud4 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName4);
	hHud4.SetInt(0, role);
	hHud4.SetFloat(30, Time() + timeOffset);
	hHud4.SetString(0, GenerateTraitorListHUD(strLanguage));
}

function CreatePlayerHud(hPlayer) {
	hPlayer.ValidateScriptScope();

	// Hud1 Hud2 Hud4 用于显示团队信息，如果是观战，则显示简要的游戏规则
	local strLanguage = GetClientLanguage(hPlayer.entindex());
	local hHud1 = Entities.CreateByClassname("rd_hud_vscript");
	g_ent_HudAndVGui.append(hHud1);
	hHud1.__KeyValueFromString("client_vscript", "challenge_traitors_player_info.nut");
	hHud1.Spawn();
	hHud1.Activate();
	hHud1.SetEntity(0, hPlayer);
	local strHud1 = "HUD_" + UniqueString();
	hHud1.SetName(strHud1);

	hPlayer.GetScriptScope().strHudName1 <- strHud1;
	hHud1.SetInt(0, ROLE.SPECTATOR);
	hHud1.SetInt(1, 2);
	hHud1.SetInt(2, 0);
	hHud1.SetFloat(0, 0.1);
	hHud1.SetFloat(1, 0.5);
	hHud1.SetFloat(2, 0.35);
	hHud1.SetFloat(3, 3.1);
	hHud1.SetFloat(4, 0.5);
	hHud1.SetFloat(5, 0.35);
	hHud1.SetFloat(6, 5.1);
	hHud1.SetFloat(7, 0.25);
	hHud1.SetFloat(8, 0.03);
	hHud1.SetFloat(30, Time() + 2.0);
	hHud1.SetFloat(31, 3.0 * pow(10, 30));
	hHud1.SetString(0, GetLocalizedString("#challenge_traitors_game_instruction1", strLanguage));

	// Hud1 Hud2 Hud4 用于显示团队信息，如果是观战，则显示简要的游戏规则
	local hHud2 = Entities.CreateByClassname("rd_hud_vscript");
	g_ent_HudAndVGui.append(hHud2);
	hHud2.__KeyValueFromString("client_vscript", "challenge_traitors_player_info.nut");
	hHud2.Spawn();
	hHud2.Activate();
	hHud2.SetEntity(0, hPlayer);
	local strHud2 = "HUD_" + UniqueString();
	hHud2.SetName(strHud2);

	hPlayer.ValidateScriptScope();
	hPlayer.GetScriptScope().strHudName2 <- strHud2;
	hHud2.SetInt(0, ROLE.SPECTATOR);
	hHud2.SetInt(1, 2);
	hHud2.SetInt(2, 1);
	hHud2.SetFloat(0, 0.1);
	hHud2.SetFloat(1, 0.5);
	hHud2.SetFloat(2, 0.35);
	hHud2.SetFloat(3, 3.1);
	hHud2.SetFloat(4, 0.5);
	hHud2.SetFloat(5, 0.35);
	hHud2.SetFloat(6, 5.1);
	hHud2.SetFloat(7, 0.25);
	hHud2.SetFloat(8, 0.03);
	hHud2.SetFloat(30, Time() + 2.0);
	hHud2.SetFloat(31, 3.0 * pow(10, 30));
	hHud2.SetString(0, GetLocalizedString("#challenge_traitors_game_instruction2", strLanguage));

	// Hud1 Hud2 Hud4 用于显示团队信息，如果是观战，则显示简要的游戏规则
	local hHud4 = Entities.CreateByClassname("rd_hud_vscript");
	g_ent_HudAndVGui.append(hHud4);
	hHud4.__KeyValueFromString("client_vscript", "challenge_traitors_player_info.nut");
	hHud4.Spawn();
	hHud4.Activate();
	hHud4.SetEntity(0, hPlayer);
	local strHud4 = "HUD_" + UniqueString();
	hHud4.SetName(strHud4);

	hPlayer.GetScriptScope().strHudName4 <- strHud4;
	hHud4.SetInt(0, ROLE.SPECTATOR);
	hHud4.SetInt(1, 2);
	hHud4.SetInt(2, 2);
	hHud4.SetFloat(0, 0.1);
	hHud4.SetFloat(1, 0.5);
	hHud4.SetFloat(2, 0.35);
	hHud4.SetFloat(3, 3.1);
	hHud4.SetFloat(4, 0.5);
	hHud4.SetFloat(5, 0.35);
	hHud4.SetFloat(6, 5.1);
	hHud4.SetFloat(7, 0.25);
	hHud4.SetFloat(8, 0.03);
	hHud4.SetFloat(30, Time() + 2.0);
	hHud4.SetFloat(31, 3.0 * pow(10, 30));
	hHud4.SetString(0, "");

	// hHud3 用于内鬼团队显示一些特殊提示以及投票信息
	local hHud3 = Entities.CreateByClassname("rd_hud_vscript");
	g_ent_HudAndVGui.append(hHud3);
	hHud3.__KeyValueFromString("client_vscript", "challenge_traitors_player_info.nut");
	hHud3.Spawn();
	hHud3.Activate();
	hHud3.SetEntity(0, hPlayer);
	local strhHud3 = "HUD_" + UniqueString();
	hHud3.SetName(strhHud3);

	hPlayer.ValidateScriptScope();
	hPlayer.GetScriptScope().strHudName3 <- strhHud3;
	hPlayer.GetScriptScope().intMsgFlag <- 0;
	hHud3.SetInt(0, ROLE.SPECTATOR);
	hHud3.SetInt(1, 0);
	hHud3.SetInt(2, 0);
	hHud3.SetInt(63, 0);
	hHud3.SetFloat(0, 0.1);
	hHud3.SetFloat(1, 0.5);
	hHud3.SetFloat(2, 0.65);
	hHud3.SetFloat(30, Time() + 2.0);
	hHud3.SetFloat(31, 10.0);
	hHud3.SetString(0, "");
}

function CreateMarineHudAndVGuiEntities() {
	foreach(hMarine in g_marine_Total) {
		if (hMarine == null || !hMarine.IsValid()) {
			continue;
		}
		CreateMarineFlashbangHud(hMarine); //用于闪光弹效果
	}
	//为特殊角色创建VGUI
	if (g_marine_Scanner != null) {
		CreateScannerVGui(g_marine_Scanner);
	}
	if (g_marine_Silencer != null) {
		CreateSilencerVGui(g_marine_Silencer);
	}
	if (g_marine_Shield != null) {
		CreateShieldVGui(g_marine_Shield);
	}
	if (g_marine_Biochemist != null) {
		CreateBiochemistVGui(g_marine_Biochemist);
	}
	if (g_marine_Infector != null) {
		CreateInfectorVGui(g_marine_Infector);
	}
}

function CreateMarineFlashbangHud(hMarine) {
	local hHud = Entities.CreateByClassname("rd_hud_vscript");
	g_ent_HudAndVGui.append(hHud);
	hHud.__KeyValueFromString("client_vscript", "challenge_traitors_flashbang.nut");
	hHud.Spawn();
	hHud.Activate();
	hHud.SetEntity(0, hMarine);
	local strHud = "HUDFlash_" + UniqueString();
	hHud.SetName(strHud);

	hMarine.ValidateScriptScope();
	hMarine.GetScriptScope().strFlashbangHudName = strHud;
}

getconsttable()["entindex_scanned"] <- -1;

function CreateScannerVGui(hMarine) {
	local time = Time();
	local cd = 30.0;
	local marineCount = g_marine_Total.len();
	hMarine.ValidateScriptScope();
	hMarine.GetScriptScope().strVGuiNameBackground <- "";
	hMarine.GetScriptScope().strVGuiNameButton <- [];
	hMarine.GetScriptScope().strVGuiNameButton.resize(32);
	hMarine.GetScriptScope().IsOpen <- false;

	local strLanguage = GetClientLanguage(hMarine.GetCommander().entindex());
	local hVGuiBackground = Entities.CreateByClassname("rd_vgui_vscript");
	g_ent_HudAndVGui.append(hVGuiBackground);
	hVGuiBackground.__KeyValueFromString("client_vscript", "challenge_traitors_client_scanner_background.nut");
	hVGuiBackground.ValidateScriptScope();
	hVGuiBackground.GetScriptScope().Input <- Input;
	hVGuiBackground.Spawn();
	hVGuiBackground.Activate();
	hVGuiBackground.SetEntity(0, hMarine);
	hVGuiBackground.SetInteracter(null);
	hVGuiBackground.SetInt(2, marineCount);
	local strVGuiBackground = "VGui_" + UniqueString();
	hVGuiBackground.SetName(strVGuiBackground);
	hVGuiBackground.SetFloat(0, pow(10, 30));
	hVGuiBackground.SetFloat(1, pow(10, 30));
	local str1 = GetLocalizedString("#challenge_traitors_vgui_menu_scanner_1", strLanguage);
	local str2 = GetLocalizedString("#challenge_traitors_vgui_menu_scanner_2", strLanguage);
	local str3 = GetLocalizedString("#challenge_traitors_vgui_menu_scanner_3", strLanguage);
	local str4 = GetLocalizedString("#challenge_traitors_vgui_menu_scanner_4", strLanguage);
	hVGuiBackground.SetInt(3, str1.len());
	hVGuiBackground.SetInt(4, str2.len());
	hVGuiBackground.SetInt(5, str3.len());
	hVGuiBackground.SetInt(6, str4.len());
	hVGuiBackground.SetString(0, str1 + str2 + str3 + str4);
	hMarine.GetScriptScope().strVGuiNameBackground <- strVGuiBackground;

	local i = 0;
	foreach(tempMarine in g_marine_Total_Unshuffled) {
		local hVGuiButton = Entities.CreateByClassname("rd_vgui_vscript");
		g_ent_HudAndVGui.append(hVGuiButton);
		hVGuiButton.__KeyValueFromString("client_vscript", "challenge_traitors_client_scanner_button.nut");
		hVGuiButton.ValidateScriptScope();
		hVGuiButton.GetScriptScope().Input <- Input;
		hVGuiButton.Spawn();
		hVGuiButton.Activate();
		hVGuiButton.SetEntity(0, hMarine);
		hVGuiButton.SetInteracter(null);
		hVGuiButton.SetInt(0, i);
		hVGuiButton.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
		hVGuiButton.SetFloat(1, cd);
		hVGuiButton.SetInt(1, tempMarine.entindex());
		hVGuiButton.SetInt(2, marineCount);
		if (tempMarine == hMarine) {
			hVGuiButton.SetInt(3, 1);
		} else {
			hVGuiButton.SetInt(3, 0);
		}
		hVGuiButton.SetInt(4, tempMarine.GetScriptScope().Role);
		local strVGuiButton = "VGui_" + UniqueString();
		hVGuiButton.SetName(strVGuiButton);
		if (tempMarine.GetCommander() != null) {
			hVGuiButton.SetString(0, tempMarine.GetCommander().GetPlayerName());
		} else {
			hVGuiButton.SetString(0, tempMarine.GetName());
		}
		hMarine.GetScriptScope().strVGuiNameButton[i] = strVGuiButton;
		i++;
	}
}

getconsttable()["entindex_silenced"] <- -1;

function CreateSilencerVGui(hMarine) {
	local time = Time();
	local marineCount = g_marine_Total.len();
	hMarine.ValidateScriptScope();
	hMarine.GetScriptScope().strVGuiNameBackground <- "";
	hMarine.GetScriptScope().strVGuiNameButton <- [];
	hMarine.GetScriptScope().strVGuiNameButton.resize(32);
	hMarine.GetScriptScope().IsOpen <- false;

	local strLanguage = GetClientLanguage(hMarine.GetCommander().entindex());
	local hVGuiBackground = Entities.CreateByClassname("rd_vgui_vscript");
	g_ent_HudAndVGui.append(hVGuiBackground);
	hVGuiBackground.__KeyValueFromString("client_vscript", "challenge_traitors_client_silencer_background.nut");
	hVGuiBackground.ValidateScriptScope();
	hVGuiBackground.GetScriptScope().Input <- Input;
	hVGuiBackground.Spawn();
	hVGuiBackground.Activate();
	hVGuiBackground.SetEntity(0, hMarine);
	hVGuiBackground.SetInteracter(null);
	hVGuiBackground.SetInt(2, marineCount);
	local strVGuiBackground = "VGui_" + UniqueString();
	hVGuiBackground.SetName(strVGuiBackground);
	hVGuiBackground.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
	local str1 = GetLocalizedString("#challenge_traitors_vgui_menu_silencer_1", strLanguage);
	local str2 = GetLocalizedString("#challenge_traitors_vgui_menu_silencer_2", strLanguage);
	local str3 = GetLocalizedString("#challenge_traitors_vgui_menu_silencer_3", strLanguage);
	local str4 = GetLocalizedString("#challenge_traitors_vgui_menu_silencer_4", strLanguage);
	hVGuiBackground.SetInt(3, str1.len());
	hVGuiBackground.SetInt(4, str2.len());
	hVGuiBackground.SetInt(5, str3.len());
	hVGuiBackground.SetInt(6, str4.len());
	hVGuiBackground.SetString(0, str1 + str2 + str3 + str4);
	hMarine.GetScriptScope().strVGuiNameBackground <- strVGuiBackground;

	local i = 0;
	foreach(tempMarine in g_marine_Total_Unshuffled) {
		local hVGuiButton = Entities.CreateByClassname("rd_vgui_vscript");
		g_ent_HudAndVGui.append(hVGuiButton);
		hVGuiButton.__KeyValueFromString("client_vscript", "challenge_traitors_client_silencer_button.nut");
		hVGuiButton.ValidateScriptScope();
		hVGuiButton.GetScriptScope().Input <- Input;
		hVGuiButton.Spawn();
		hVGuiButton.Activate();
		hVGuiButton.SetEntity(0, hMarine);
		hVGuiButton.SetInteracter(null);
		hVGuiButton.SetInt(0, i);
		hVGuiButton.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
		hVGuiButton.SetInt(1, tempMarine.entindex());
		hVGuiButton.SetInt(2, marineCount);
		if (tempMarine == hMarine) {
			hVGuiButton.SetInt(3, 0);
		} else {
			hVGuiButton.SetInt(3, 1);
		}
		local strVGuiButton = "VGui_" + UniqueString();
		hVGuiButton.SetName(strVGuiButton);
		if (tempMarine.GetCommander() != null) {
			hVGuiButton.SetString(0, tempMarine.GetCommander().GetPlayerName());
		} else {
			hVGuiButton.SetString(0, tempMarine.GetName());
		}
		hMarine.GetScriptScope().strVGuiNameButton[i] = strVGuiButton;
		i++;
	}
}

getconsttable()["entindex_mecha_given"] <- -1;

function CreateShieldVGui(hMarine) {
	local time = Time();
	local marineCount = g_marine_Total.len();
	hMarine.ValidateScriptScope();
	hMarine.GetScriptScope().strVGuiNameBackground <- "";
	hMarine.GetScriptScope().strVGuiNameButton <- [];
	hMarine.GetScriptScope().strVGuiNameButton.resize(32);
	hMarine.GetScriptScope().IsOpen <- false;

	local strLanguage = GetClientLanguage(hMarine.GetCommander().entindex());
	local hVGuiBackground = Entities.CreateByClassname("rd_vgui_vscript");
	g_ent_HudAndVGui.append(hVGuiBackground);
	hVGuiBackground.__KeyValueFromString("client_vscript", "challenge_traitors_client_shield_background.nut");
	hVGuiBackground.ValidateScriptScope();
	hVGuiBackground.GetScriptScope().Input <- Input;
	hVGuiBackground.Spawn();
	hVGuiBackground.Activate();
	hVGuiBackground.SetEntity(0, hMarine);
	hVGuiBackground.SetInteracter(null);
	hVGuiBackground.SetInt(2, marineCount);
	local strVGuiBackground = "VGui_" + UniqueString();
	hVGuiBackground.SetName(strVGuiBackground);
	hVGuiBackground.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
	local str1 = GetLocalizedString("#challenge_traitors_vgui_menu_shield_1", strLanguage);
	local str2 = GetLocalizedString("#challenge_traitors_vgui_menu_shield_2", strLanguage);
	local str3 = GetLocalizedString("#challenge_traitors_vgui_menu_shield_3", strLanguage);
	local str4 = GetLocalizedString("#challenge_traitors_vgui_menu_shield_4", strLanguage);
	hVGuiBackground.SetInt(3, str1.len());
	hVGuiBackground.SetInt(4, str2.len());
	hVGuiBackground.SetInt(5, str3.len());
	hVGuiBackground.SetInt(6, str4.len());
	hVGuiBackground.SetString(0, str1 + str2 + str3 + str4);
	hMarine.GetScriptScope().strVGuiNameBackground <- strVGuiBackground;

	local i = 0;
	foreach(tempMarine in g_marine_Total_Unshuffled) {
		local hVGuiButton = Entities.CreateByClassname("rd_vgui_vscript");
		g_ent_HudAndVGui.append(hVGuiButton);
		hVGuiButton.__KeyValueFromString("client_vscript", "challenge_traitors_client_shield_button.nut");
		hVGuiButton.ValidateScriptScope();
		hVGuiButton.GetScriptScope().Input <- Input;
		hVGuiButton.Spawn();
		hVGuiButton.Activate();
		hVGuiButton.SetEntity(0, hMarine);
		hVGuiButton.SetInteracter(null);
		hVGuiButton.SetInt(0, i);
		hVGuiButton.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
		hVGuiButton.SetInt(1, tempMarine.entindex());
		hVGuiButton.SetInt(2, marineCount);
		if (tempMarine == hMarine) {
			hVGuiButton.SetInt(3, 0);
		} else {
			hVGuiButton.SetInt(3, 1);
		}
		local strVGuiButton = "VGui_" + UniqueString();
		hVGuiButton.SetName(strVGuiButton);
		if (tempMarine.GetCommander() != null) {
			hVGuiButton.SetString(0, tempMarine.GetCommander().GetPlayerName());
		} else {
			hVGuiButton.SetString(0, tempMarine.GetName());
		}
		hMarine.GetScriptScope().strVGuiNameButton[i] = strVGuiButton;
		i++;
	}
}

getconsttable()["entindex_healed"] <- -1;
getconsttable()["entindex_killed"] <- -1;

function CreateBiochemistVGui(hMarine) {
	local time = Time();
	local marineCount = g_marine_Total.len();
	hMarine.ValidateScriptScope();
	hMarine.GetScriptScope().strVGuiNameBackground <- "";
	hMarine.GetScriptScope().strVGuiNameButton <- [];
	hMarine.GetScriptScope().strVGuiNameButton.resize(32);
	hMarine.GetScriptScope().IsOpen <- false;

	local strLanguage = GetClientLanguage(hMarine.GetCommander().entindex());
	local hVGuiBackground = Entities.CreateByClassname("rd_vgui_vscript");
	g_ent_HudAndVGui.append(hVGuiBackground);
	hVGuiBackground.__KeyValueFromString("client_vscript", "challenge_traitors_client_biochemist_background.nut");
	hVGuiBackground.ValidateScriptScope();
	hVGuiBackground.GetScriptScope().Input <- Input;
	hVGuiBackground.Spawn();
	hVGuiBackground.Activate();
	hVGuiBackground.SetEntity(0, hMarine);
	hVGuiBackground.SetInteracter(null);
	hVGuiBackground.SetInt(2, marineCount);
	local strVGuiBackground = "VGui_" + UniqueString();
	hVGuiBackground.SetName(strVGuiBackground);
	hVGuiBackground.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
	local str1 = GetLocalizedString("#challenge_traitors_vgui_menu_biochemist_1", strLanguage);
	local str2 = GetLocalizedString("#challenge_traitors_vgui_menu_biochemist_2", strLanguage);
	local str3 = GetLocalizedString("#challenge_traitors_vgui_menu_biochemist_3", strLanguage);
	local str4 = GetLocalizedString("#challenge_traitors_vgui_menu_biochemist_4", strLanguage);
	local str5 = GetLocalizedString("#challenge_traitors_vgui_menu_biochemist_5", strLanguage);
	hVGuiBackground.SetInt(3, str1.len());
	hVGuiBackground.SetInt(4, str2.len());
	hVGuiBackground.SetInt(5, str3.len());
	hVGuiBackground.SetInt(6, str4.len());
	hVGuiBackground.SetInt(7, str5.len());
	hVGuiBackground.SetString(0, str1 + str2 + str3 + str4 + str5);
	hMarine.GetScriptScope().strVGuiNameBackground <- strVGuiBackground;

	local i = 0;
	foreach(tempMarine in g_marine_Total_Unshuffled) {
		local hVGuiButton = Entities.CreateByClassname("rd_vgui_vscript");
		g_ent_HudAndVGui.append(hVGuiButton);
		hVGuiButton.__KeyValueFromString("client_vscript", "challenge_traitors_client_biochemist_button.nut");
		hVGuiButton.ValidateScriptScope();
		hVGuiButton.GetScriptScope().Input <- Input;
		hVGuiButton.Spawn();
		hVGuiButton.Activate();
		hVGuiButton.SetEntity(0, hMarine);
		hVGuiButton.SetInteracter(null);
		hVGuiButton.SetInt(0, i);
		hVGuiButton.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
		hVGuiButton.SetInt(1, tempMarine.entindex());
		hVGuiButton.SetInt(2, marineCount);
		if (tempMarine == hMarine) {
			hVGuiButton.SetInt(3, 0);
		} else {
			hVGuiButton.SetInt(3, 1);
		}
		local strVGuiButton = "VGui_" + UniqueString();
		hVGuiButton.SetName(strVGuiButton);
		if (tempMarine.GetCommander() != null) {
			hVGuiButton.SetString(0, tempMarine.GetCommander().GetPlayerName());
		} else {
			hVGuiButton.SetString(0, tempMarine.GetName());
		}
		hMarine.GetScriptScope().strVGuiNameButton[i] = strVGuiButton;
		i++;
	}
}

getconsttable()["entindex_abeted"] <- -1;

function CreateInfectorVGui(hMarine) {
	local time = Time();
	local marineCount = g_marine_Total.len();
	hMarine.ValidateScriptScope();
	hMarine.GetScriptScope().strVGuiNameBackground <- "";
	hMarine.GetScriptScope().strVGuiNameButton <- [];
	hMarine.GetScriptScope().strVGuiNameButton.resize(32);
	hMarine.GetScriptScope().IsOpen <- false;

	local strLanguage = GetClientLanguage(hMarine.GetCommander().entindex());
	local hVGuiBackground = Entities.CreateByClassname("rd_vgui_vscript");
	g_ent_HudAndVGui.append(hVGuiBackground);
	hVGuiBackground.__KeyValueFromString("client_vscript", "challenge_traitors_client_infector_background.nut");
	hVGuiBackground.ValidateScriptScope();
	hVGuiBackground.GetScriptScope().Input <- Input;
	hVGuiBackground.Spawn();
	hVGuiBackground.Activate();
	hVGuiBackground.SetEntity(0, hMarine);
	hVGuiBackground.SetInteracter(null);
	hVGuiBackground.SetInt(2, marineCount);
	local strVGuiBackground = "VGui_" + UniqueString();
	hVGuiBackground.SetName(strVGuiBackground);
	hVGuiBackground.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
	local str1 = GetLocalizedString("#challenge_traitors_vgui_menu_infector_1", strLanguage);
	local str2 = GetLocalizedString("#challenge_traitors_vgui_menu_infector_2", strLanguage);
	local str3 = GetLocalizedString("#challenge_traitors_vgui_menu_infector_3", strLanguage);
	local str4 = GetLocalizedString("#challenge_traitors_vgui_menu_infector_4", strLanguage);
	local str5 = GetLocalizedString("#challenge_traitors_vgui_menu_infector_5", strLanguage);
	hVGuiBackground.SetInt(3, str1.len());
	hVGuiBackground.SetInt(4, str2.len());
	hVGuiBackground.SetInt(5, str3.len());
	hVGuiBackground.SetInt(6, str4.len());
	hVGuiBackground.SetInt(7, str5.len());
	hVGuiBackground.SetString(0, str1 + str2 + str3 + str4 + str5);
	hMarine.GetScriptScope().strVGuiNameBackground <- strVGuiBackground;

	local i = 0;
	foreach(tempMarine in g_marine_Total_Unshuffled) {
		if (tempMarine == hMarine) {
			//continue;
		}
		local hVGuiButton = Entities.CreateByClassname("rd_vgui_vscript");
		g_ent_HudAndVGui.append(hVGuiButton);
		hVGuiButton.__KeyValueFromString("client_vscript", "challenge_traitors_client_infector_button.nut");
		hVGuiButton.ValidateScriptScope();
		hVGuiButton.GetScriptScope().Input <- Input;
		hVGuiButton.Spawn();
		hVGuiButton.Activate();
		hVGuiButton.SetEntity(0, hMarine);
		hVGuiButton.SetInteracter(null);
		hVGuiButton.SetInt(0, i);
		hVGuiButton.SetFloat(0, time + g_int_ImmuneCounter * 0.1);
		hVGuiButton.SetInt(1, tempMarine.entindex());
		hVGuiButton.SetInt(2, marineCount);
		hVGuiButton.SetInt(3, 1);
		foreach(hTraitor in g_marine_Traitor) {
			if (tempMarine == hTraitor) {
				hVGuiButton.SetInt(3, 0);
			}
		}
		local strVGuiButton = "VGui_" + UniqueString();
		hVGuiButton.SetName(strVGuiButton);
		if (tempMarine.GetCommander() != null) {
			hVGuiButton.SetString(0, tempMarine.GetCommander().GetPlayerName());
		} else {
			hVGuiButton.SetString(0, tempMarine.GetName());
		}
		hMarine.GetScriptScope().strVGuiNameButton[i] = strVGuiButton;
		i++;
	}
}

function SetTechMap() {
	// 这个部分在原始的代码里就在其他位置被禁用了。可能需要测试与平衡
	// 地图检查，意思是这些地图里把技术杀光也算是内鬼胜利。
	switch (GetMapName().tolower()) {
		case "asi-jac1-landingbay_01":
		case "asi-jac2-deima":
		case "asi-jac3-rydberg":
		case "asi-jac7-timorstation":
		case "rd-area9800lz":
		case "rd-area9800pp1":
		case "rd-area9800pp2":
		case "rd-area9800wl":
		case "rd-lan3_maintenance":
		case "rd-lan5_complex":
		case "rd-ocs2landingbay7":
		case "rd-par1unexpected_encounter":
		case "rd-par3close_contact":
		case "rd-par4high_tension":
		case "rd-res1forestentrance":
		case "rd-tft1desertoutpost":
		case "rd-tft3spaceport":
		case "rd-til1midnightport":
		case "rd-til3arcticinfiltration":
		case "rd-til4area9800":
		case "rd-til5coldcatwalks":
		case "rd-til6yanaurusmine":
		case "rd-til7factory":
		case "rd-til8comcenter":
		case "rd-til9syntekhospital":
			g_bool_TechMap = true;
	}
}

function SetImmuneTimeAndFlags() {
	local temp = RandomInt(0, 20) - 10; // -2~2
	switch (GetMapName().tolower()) {
		case "asi-jac1-landingbay_01":
			g_int_ImmuneCounter = 90 + temp;
			g_enum_CurrentMap = MAP.JAC_1;
			break;
		case "asi-jac1-landingbay_02":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.JAC_2;
			break;
		case "asi-jac1-landingbay_pract":
			g_int_ImmuneCounter = 80 + temp;
			break;
		case "asi-jac2-deima":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.JAC_3;
			break;
		case "asi-jac3-rydberg":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.JAC_4;
			break;
		case "asi-jac4-residential":
			g_int_ImmuneCounter = 90 + temp;
			g_enum_CurrentMap = MAP.JAC_5;
			break;
		case "asi-jac6-sewerjunction":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.JAC_6;
			break;
		case "asi-jac7-timorstation":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.JAC_7;
			break;
		case "rd-acc1_infodep":
			g_int_ImmuneCounter = 70 + temp;
			g_enum_CurrentMap = MAP.ACC_1;
			break;
		case "rd-acc2_powerhood":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.ACC_2;
			break;
		case "rd-acc3_rescenter":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.ACC_3;
			break;
		case "rd-acc4_confacility":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.ACC_4;
			break;
		case "rd-acc5_j5connector":
			g_int_ImmuneCounter = 110 + 2 * temp;
			g_enum_CurrentMap = MAP.ACC_5;
			break;
		case "rd-acc6_labruins":
			g_int_ImmuneCounter = 120 + temp;
			g_enum_CurrentMap = MAP.ACC_6;
			break;
		case "rd-acc_complex":
			g_int_ImmuneCounter = 120 + temp;
			g_enum_CurrentMap = MAP.BON_8;
			break;
		case "rd-area9800lz":
			g_int_ImmuneCounter = 110 + temp;
			g_enum_CurrentMap = MAP._9800_1;
			break;
		case "rd-area9800pp1":
			g_int_ImmuneCounter = 530 + 3 * temp;
			g_enum_CurrentMap = MAP._9800_2;
			break;
		case "rd-area9800pp2":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP._9800_3;
			break;
		case "rd-area9800wl":
			g_int_ImmuneCounter = 120 + 3 * temp;
			g_enum_CurrentMap = MAP._9800_4;
			break;
		case "rd-bio1operationx5":
			g_int_ImmuneCounter = 270 + 2 * temp;
			g_enum_CurrentMap = MAP.BIO_1;
			break;
		case "rd-bio2invisiblethreat":
			g_int_ImmuneCounter = 140 + temp;
			g_enum_CurrentMap = MAP.BIO_2;
			break;
		case "rd-bio3biogenlabs":
			g_int_ImmuneCounter = 120 + 2 * temp;
			g_enum_CurrentMap = MAP.BIO_3;
			break;
		case "rd-bonus10_sewrev":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.BON_9;
			break;
		case "rd-bonus12_rydrev":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.BON_10;
			break;
		case "rd-bonus14_cargrev":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.BON_11;
			break;
		case "rd-bonus15_landrev":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.BON_12;
			break;
		case "rd-bonus_mission1":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.BON_1;
			break;
		case "rd-bonus_mission2":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.BON_2;
			break;
		case "rd-bonus_mission3":
			g_int_ImmuneCounter = 140 + 2 * temp;
			g_enum_CurrentMap = MAP.BON_3;
			break;
		case "rd-bonus_mission4":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.BON_4;
			break;
		case "rd-bonus_mission5":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.BON_5;
			break;
		case "rd-bonus_mission6":
			g_int_ImmuneCounter = 600 + 5 * temp;
			g_enum_CurrentMap = MAP.BON_6;
			break;
		case "rd-bonus_mission7":
			g_int_ImmuneCounter = 800 + 3 * temp;
			g_enum_CurrentMap = MAP.BON_7;
			break;
		case "rd-lan1_bridge":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.LANA_1;
			break;
		case "rd-lan2_sewer":
			g_int_ImmuneCounter = 90 + temp;
			g_enum_CurrentMap = MAP.LANA_2;
			break;
		case "rd-lan3_maintenance":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.LANA_3;
			break;
		case "rd-lan4_vent":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.LANA_4;
			break;
		case "rd-lan5_complex":
			g_int_ImmuneCounter = 90 + temp;
			g_enum_CurrentMap = MAP.LANA_5;
			break;
		case "rd-nh01_logisticsarea":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.NH_1;
			break;
		case "rd-nh02_platformxvii":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.NH_2;
			break;
		case "rd-nh03_groundworklabs":
			g_int_ImmuneCounter = 140 + 2 * temp;
			g_enum_CurrentMap = MAP.NH_3;
			break;
		case "rd-ocs1storagefacility":
			g_int_ImmuneCounter = 60 + temp;
			g_enum_CurrentMap = MAP.OCS_1;
			break;
		case "rd-ocs2landingbay7":
			g_int_ImmuneCounter = 60 + temp;
			g_enum_CurrentMap = MAP.OCS_2;
			break;
		case "rd-ocs3uscmedusa":
			g_int_ImmuneCounter = 70 + temp;
			g_enum_CurrentMap = MAP.OCS_3;
			break;
		case "rd-par1unexpected_encounter":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.PARA_1;
			break;
		case "rd-par2hostile_places":
			g_int_ImmuneCounter = 90 + temp;
			g_enum_CurrentMap = MAP.PARA_2;
			break;
		case "rd-par3close_contact":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.PARA_3;
			break;
		case "rd-par4high_tension":
			g_int_ImmuneCounter = 600 + 5 * temp;
			g_enum_CurrentMap = MAP.PARA_4;
			break;
		case "rd-par5crucial_point":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.PARA_5;
			break;
		case "rd-reduction1":
			g_int_ImmuneCounter = 140 + 3 * temp;
			g_enum_CurrentMap = MAP.RED_1;
			break;
		case "rd-reduction2":
			g_int_ImmuneCounter = 350 + 4 * temp;
			g_enum_CurrentMap = MAP.RED_2;
			break;
		case "rd-reduction3":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.RED_3;
			break;
		case "rd-reduction4":
			g_int_ImmuneCounter = 130 + 2 * temp;
			g_enum_CurrentMap = MAP.RED_4;
			break;
		case "rd-reduction5":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.RED_5;
			break;
		case "rd-reduction6":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.RED_6;
			break;
		case "rd-res1forestentrance":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.RES_1;
			break;
		case "rd-res2research7":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.RES_2;
			break;
		case "rd-res3miningcamp":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.RES_3;
			break;
		case "rd-res4mines":
			g_int_ImmuneCounter = 450 + 2 * temp;
			g_enum_CurrentMap = MAP.RES_4;
			break;
		case "rd-tft1desertoutpost":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TFT_1;
			break;
		case "rd-tft2abandonedmaintenance":
			g_int_ImmuneCounter = 80 + temp;
			g_enum_CurrentMap = MAP.TFT_2;
			break;
		case "rd-tft3spaceport":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TFT_3;
			break;
		case "rd-til1midnightport":
			g_int_ImmuneCounter = 150 + temp;
			g_enum_CurrentMap = MAP.TILA_1;
			break;
		case "rd-til2roadtodawn":
			g_int_ImmuneCounter = 180 + temp;
			g_enum_CurrentMap = MAP.TILA_2;
			break;
		case "rd-til3arcticinfiltration":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TILA_3;
			break;
		case "rd-til4area9800":
			g_int_ImmuneCounter = 250 + 2 * temp;
			g_enum_CurrentMap = MAP.TILA_4;
			break;
		case "rd-til5coldcatwalks":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TILA_5;
			break;
		case "rd-til6yanaurusmine":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TILA_6;
			break;
		case "rd-til7factory":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TILA_7;
			break;
		case "rd-til8comcenter":
			g_int_ImmuneCounter = 140 + 3 * temp;
			g_enum_CurrentMap = MAP.TILA_8;
			break;
		case "rd-til9syntekhospital":
			g_int_ImmuneCounter = 100 + temp;
			g_enum_CurrentMap = MAP.TILA_9;
			break;
		default:
			g_int_ImmuneCounter = 100;
	}
}

function SetConVars(interval = 1) {
	if (g_int_Counter % interval != 0) {
		return;
	}
	Convars.SetValue("asw_marine_rolls", 0);
	Convars.SetValue("rd_marine_jump_height", g_int_JumpHeight);
	Convars.SetValue("rd_laser_mine_targets_everything", 1);
	Convars.SetValue("rd_laser_mine_takes_damage", 1);
	Convars.SetValue("rd_firemine_target_marine", 1);
	Convars.SetValue("rd_marine_ff_fist", 1);
	Convars.SetValue("asw_marine_ff", 2);
	Convars.SetValue("asw_marine_ff_guard_time", 0);
	Convars.SetValue("asw_marine_ff_absorption", 0);
	Convars.SetValue("rd_aim_marines", 1);
	Convars.SetValue("rd_hp_regen", 1);
	Convars.SetValue("rm_health_regen_amount", RandomHQNormalDistribution(3, 0.5));
	Convars.SetValue("rm_health_regen_interval", RandomHQNormalDistribution(10, 0.4));
	Convars.SetValue("rd_weapons_show_hidden", 1);
	Convars.SetValue("rd_jumpjet_knockdown_marines", RandomHQUniformIntDistribution(0, 10) >= 8 ? 0 : 1);
	Convars.SetValue("rd_marine_ignite_immediately", 1);
	Convars.SetValue("asw_marine_time_until_ignite", 0);
	Convars.SetValue("asw_marine_burn_time_easy", RandomHQNormalDistribution(45, 5));
	Convars.SetValue("asw_marine_burn_time_normal", RandomHQNormalDistribution(45, 5));
	Convars.SetValue("asw_marine_burn_time_hard", RandomHQNormalDistribution(45, 5));
	Convars.SetValue("asw_marine_burn_time_insane", RandomHQNormalDistribution(45, 5));
	Convars.SetValue("asw_blink_charge_time", RandomHQNormalDistribution(100, 25));
	Convars.SetValue("asw_minigun_spin_down_rate", 100);
	Convars.SetValue("asw_minigun_spin_rate_threshold", 1);
	Convars.SetValue("asw_minigun_spin_up_rate", 50);
	Convars.SetValue("rd_minigun_dmg_base", 10);
	Convars.SetValue("asw_gas_grenade_cloud_width", 128);
	Convars.SetValue("rd_techreq", 0);
	Convars.SetValue("rd_hackall", 1);
	Convars.SetValue("rd_biomass_ignite_from_explosions", 1);
	Convars.SetValue("rd_spawn_medkits", RandomHQNormalDistribution(61, 2).tointeger());
	Convars.SetValue("asw_fist_passive_damage_scale", RandomHQNormalDistribution(4500, 500));
	Convars.SetValue("rd_stuck_bot_teleport", 1);
	Convars.SetValue("rd_damage_buff_scale", RandomHQNormalDistribution(1.2, 0.02));

	Convars.SetValue("asw_cluster_grenade_fuse", RandomHQNormalDistribution(4, 0.7));
	local child_fuse_max = RandomHQNormalDistribution(2.4, 0.3);
	local child_fuse_min = child_fuse_max / RandomHQUniformFloatDistribution(1.5, 3.0);
	Convars.SetValue("asw_cluster_grenade_child_fuse_max", child_fuse_max);
	Convars.SetValue("asw_cluster_grenade_child_fuse_min", child_fuse_min);
	Convars.SetValue("asw_cluster_grenade_radius_check_scale", RandomHQNormalDistribution(0.55, 0.017));

	Convars.SetValue("rd_marine_passive_armor_layers_amount", RandomHQNormalDistribution(14, 1.5).tointeger());
	Convars.SetValue("rd_marine_passive_armor_layer_protection_value", RandomHQNormalDistribution(0.04, 0.0005));

	Convars.SetValue("asw_sentry_friendly_fire_scale", 0.2);
	Convars.SetValue("asw_sentry_friendly_target", g_enum_CurrentMap == MAP.RED_6 ? 0 : 1);
	Convars.SetValue("asw_sentry_health_base", 1000);
	Convars.SetValue("rd_sentry_take_damage_from_marine", 1);
	//Convars.SetValue("asw_horizontal_autoaim", 0);
	//Convars.SetValue("autoaim_max_dist", 0);

	Convars.SetValue("asw_ammo_count_sniper_rifle", 1);
	Convars.SetValue("asw_skill_accuracy_sniper_rifle_dmg_step", RandomHQNormalDistribution(5, 1).tointeger());
	Convars.SetValue("rd_sniper_dmg_base", RandomHQUniformFloatDistribution(100, 150));
	Convars.SetValue("rd_sniper_rifle_dmg_zoomed_bonus", RandomHQUniformFloatDistribution(500, 700));

	Convars.SetValue("asw_ammo_count_devastator", RandomHQUniformIntDistribution(10, 25));

	Convars.SetValue("rd_railgun_dmg_base", RandomHQUniformIntDistribution(100, 250));

	Convars.SetValue("rd_pistols_min_delay", RandomHQUniformFloatDistribution(0.116, 0.126));

	local hPlayer = null;
	local playerCount = 0;
	while (hPlayer = Entities.FindByClassname(hPlayer, "player")) {
		playerCount++;
	}
	Convars.SetValue("sv_minupdaterate", 20);
	Convars.SetValue("sv_mincmdrate", 20);
	Convars.SetValue("sv_maxupdaterate", 60 - 10 * (playerCount / 8).tointeger());
	Convars.SetValue("sv_maxcmdrate", 60 - 10 * (playerCount / 8).tointeger());
}

// 从一个数组中均匀随机选出count个，原理是用 Fisher-Yates 洗牌算法均匀洗牌，然后取前count项。
function SelectRoles() {
	g_marine_Traitor = g_marine_Total.slice(0, g_int_TraitorCount); // 将选出的内鬼句柄存入内鬼列表
	g_player_Traitor.resize(g_marine_Traitor.len());
	g_marine_TraitorAlive = g_marine_Total.slice(0, g_int_TraitorCount); // 将选出的内鬼句柄存入内鬼存活列表
	g_marine_Iaf = g_marine_Total.slice(g_int_TraitorCount, g_int_MarineCount); // 将选出的IAF句柄存入IAF列表
	g_marine_IafAlive = g_marine_Total.slice(g_int_TraitorCount, g_int_MarineCount); // 将选出的IAF句柄存入IAF存活列表

	//生成内鬼历史列表和当前内鬼玩家列表
	foreach(idx, hMarine in g_marine_Traitor) {
		if (hMarine && hMarine.IsValid() && hMarine.IsInhabited()) {
			g_player_TraitorHistory.append(hMarine.GetCommander());
			g_player_Traitor[idx] = hMarine.GetCommander();
		} else {
			g_player_Traitor[idx] = null;
		}
	}

	//选择特殊角色
	//1.临时列表
	local tempTraitor = [];
	local tempIaf = [];
	if (g_marine_Traitor.len() != 0) {
		tempTraitor = g_marine_Traitor.slice(0, g_marine_Traitor.len());
	}
	if (g_marine_Iaf.len() != 0) {
		tempIaf = g_marine_Iaf.slice(0, g_marine_Iaf.len());
	}
	//2.多次洗牌
	for (local i = 0; i < 5; i++) {
		tempTraitor = FisherYatesShuffle(tempTraitor);
		tempIaf = FisherYatesShuffle(tempIaf);
	}
	//3.从临时内鬼列表中依次抽取角色
	foreach(hMarine in tempTraitor) {
		hMarine.ValidateScriptScope();
		hMarine.GetScriptScope().Role <- ROLE.TRAITOR;
		if (g_marine_TraitorLeader == null && g_bool_HasTraitorLeader == true) {
			hMarine.GetScriptScope().Role = ROLE.TRAITOR_LEADER;
			g_marine_TraitorLeader = hMarine;
		} else if (g_marine_Silencer == null && g_bool_HasSilencer == true) {
			hMarine.GetScriptScope().Role = ROLE.SILENCER;
			g_marine_Silencer = hMarine;
		} else if (g_marine_Infector == null && g_bool_HasInfector == true) {
			hMarine.GetScriptScope().Role = ROLE.INFECTOR;
			g_marine_Infector = hMarine;
		} else if (g_marine_Boomer == null && g_bool_HasBoomer == true) {
			hMarine.GetScriptScope().Role = ROLE.BOOMER;
			g_marine_Boomer = hMarine;
		} else if (g_marine_Mimic == null && g_bool_HasMimic == true) {
			hMarine.GetScriptScope().Role = ROLE.MIMIC;
			g_marine_Mimic = hMarine;
		}
	}

	//4.从临时IAF列表中依次抽取角色
	foreach(hMarine in tempIaf) {
		hMarine.ValidateScriptScope();
		hMarine.GetScriptScope().Role <- ROLE.IAF;
		if (g_marine_Scanner == null && g_bool_HasScanner == true) {
			hMarine.GetScriptScope().Role = ROLE.SCANNER;
			g_marine_Scanner = hMarine;
			continue;
		}
		if (g_marine_Biochemist == null && g_bool_HasBiochemist == true) {
			hMarine.GetScriptScope().Role = ROLE.BIOCHEMIST;
			g_marine_Biochemist = hMarine;
			continue;
		}
		if (g_marine_IafLeader == null && g_bool_HasIafLeader == true) {
			hMarine.GetScriptScope().Role = ROLE.IAF_LEADER;
			g_marine_IafLeader = hMarine;
			continue;
		}
		if (g_marine_Shield == null && g_bool_HasShield == true) {
			hMarine.GetScriptScope().Role = ROLE.SHIELD;
			g_marine_Shield = hMarine;
			continue;
		}
		if (g_marine_Sniper == null && g_bool_HasSniper == true) {
			hMarine.GetScriptScope().Role = ROLE.SNIPER;
			g_marine_Sniper = hMarine;
			continue;
		}
		if (g_marine_Demo == null && g_bool_HasDemo == true) {
			hMarine.GetScriptScope().Role = ROLE.DEMO;
			g_marine_Demo = hMarine;
			continue;
		}
		if (g_marine_Deserter == null && g_bool_HasDeserter == true) {
			hMarine.GetScriptScope().Role = ROLE.IAF;
			hMarine.GetScriptScope().RevealTime <- INT_MAX.tofloat();
			g_marine_Deserter = hMarine;
		}
	}

	if (g_marine_Scanner) {
		g_tbl_RoleInfo.Scanner = g_marine_Scanner.IsInhabited() ? g_marine_Scanner.GetCommander().GetPlayerName() : g_marine_Scanner.GetMarineName();
	}
	if (g_marine_Biochemist) {
		g_tbl_RoleInfo.Biochemist = g_marine_Biochemist.IsInhabited() ? g_marine_Biochemist.GetCommander().GetPlayerName() : g_marine_Biochemist.GetMarineName();
	}
	if (g_marine_IafLeader) {
		g_tbl_RoleInfo.Iaf_Leader = g_marine_IafLeader.IsInhabited() ? g_marine_IafLeader.GetCommander().GetPlayerName() : g_marine_IafLeader.GetMarineName();
	}
	if (g_marine_Shield) {
		g_tbl_RoleInfo.Shield = g_marine_Shield.IsInhabited() ? g_marine_Shield.GetCommander().GetPlayerName() : g_marine_Shield.GetMarineName();
	}
	if (g_marine_Sniper) {
		g_tbl_RoleInfo.Sniper = g_marine_Sniper.IsInhabited() ? g_marine_Sniper.GetCommander().GetPlayerName() : g_marine_Sniper.GetMarineName();
	}
	if (g_marine_Demo) {
		g_tbl_RoleInfo.Demo = g_marine_Demo.IsInhabited() ? g_marine_Demo.GetCommander().GetPlayerName() : g_marine_Demo.GetMarineName();
	}
	if (g_marine_Deserter) {
		g_tbl_RoleInfo.Deserter = g_marine_Deserter.IsInhabited() ? g_marine_Deserter.GetCommander().GetPlayerName() : g_marine_Deserter.GetMarineName();
	}
	if (g_marine_TraitorLeader) {
		g_tbl_RoleInfo.Traitor_Leader = g_marine_TraitorLeader.IsInhabited() ? g_marine_TraitorLeader.GetCommander().GetPlayerName() : g_marine_TraitorLeader.GetMarineName();
	}
	if (g_marine_Infector) {
		g_tbl_RoleInfo.Infector = g_marine_Infector.IsInhabited() ? g_marine_Infector.GetCommander().GetPlayerName() : g_marine_Infector.GetMarineName();
	}
	if (g_marine_Boomer) {
		g_tbl_RoleInfo.Boomer = g_marine_Boomer.IsInhabited() ? g_marine_Boomer.GetCommander().GetPlayerName() : g_marine_Boomer.GetMarineName();
	}
	if (g_marine_Silencer) {
		g_tbl_RoleInfo.Silencer = g_marine_Silencer.IsInhabited() ? g_marine_Silencer.GetCommander().GetPlayerName() : g_marine_Silencer.GetMarineName();
	}
	if (g_marine_Mimic) {
		g_tbl_RoleInfo.Mimic = g_marine_Mimic.IsInhabited() ? g_marine_Mimic.GetCommander().GetPlayerName() : g_marine_Mimic.GetMarineName();
	}
}

function GenerateTraitorList() {
	// 生成名单
	g_str_TraitorNameList = TextColor(255, 0, 0) + "%s1";
	foreach(hPlayer in g_player_Traitor) {
		if (hPlayer != null && hPlayer.IsValid()) {
			g_str_TraitorNameList = g_str_TraitorNameList + " [ " + hPlayer.GetPlayerName() + " ]"; // 将选出的内鬼名字存入内鬼名单
		} else {
			g_str_TraitorNameList = g_str_TraitorNameList + " < " + "%s2" + " > "; // 将选出的内鬼名字存入内鬼名单
		}
	}
}

function GenerateTraitorListHUD(strLanguage) {
	// 生成HUD名单
	local str = "";
	if (strLanguage != "") {
		str = GetLocalizedString("#challenge_traitors_traitor_list", strLanguage);
		foreach(hPlayer in g_player_Traitor) {
			if (hPlayer != null && hPlayer.IsValid()) {
				str = str + " [ " + hPlayer.GetPlayerName() + " ]"; // 将选出的内鬼名字存入内鬼名单
			} else {
				str = str + " < " + GetLocalizedString("#challenge_traitors_traitor_player_unavailable", strLanguage) + " > "; // 将选出的内鬼名字存入内鬼名单

			}
		}
	}
	return str;
}

function PrintNetProps(hEntity) {
	local tbl = {};
	NetProps.GetTable(hEntity, 1, tbl);
	DeepPrintTable(tbl);
}