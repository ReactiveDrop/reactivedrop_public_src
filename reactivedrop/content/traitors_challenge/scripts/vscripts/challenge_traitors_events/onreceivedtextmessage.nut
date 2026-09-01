g_bool_BombActivited <- false;

// This scripted user function is the bindable replacement for the legacy chat menu trigger.
// This script is included by challenge_traitors.nut in the challenge thinker
// scope, so the listener is only visible to this challenge's listener scope.
function UserConsoleCommand(player, value) {
	if (player == null || !player.IsValid() || !g_bool_ClhallengeEnable || !g_bool_Initialized) {
		return;
	}

	// This command intentionally runs before the role/Marine checks below.
	// The guide belongs to the player, not to a marine, so it remains available
	// to spectators, dead players, and players who joined without a Marine.
	if (value == "traitors_gameplay_info" || value == "rd_traitors_gameplay_info") {
		ToggleGameplayGuide(player);
		return;
	}

	if (value != "traitors_use_skill" || g_bool_IafWin || g_bool_TraitorWin) {
		return;
	}

	if (g_marine_Silencer != null && g_marine_Silencer.IsValid() && player == g_marine_Silencer.GetCommander() && player.GetMarine() == g_marine_Silencer) {
		ToggleVGuiMenu(player, g_marine_Silencer);
	} else if (g_marine_Boomer != null && g_marine_Boomer.IsValid() && player == g_marine_Boomer.GetCommander() && player.GetMarine() == g_marine_Boomer) {
		SetBomb();
	} else if (g_marine_Infector != null && g_marine_Infector.IsValid() && player == g_marine_Infector.GetCommander() && player.GetMarine() == g_marine_Infector) {
		ToggleVGuiMenu(player, g_marine_Infector);
	} else if (g_marine_Scanner != null && g_marine_Scanner.IsValid() && player == g_marine_Scanner.GetCommander() && player.GetMarine() == g_marine_Scanner) {
		ToggleVGuiMenu(player, g_marine_Scanner);
	} else if (g_marine_Biochemist != null && g_marine_Biochemist.IsValid() && player == g_marine_Biochemist.GetCommander() && player.GetMarine() == g_marine_Biochemist) {
		ToggleVGuiMenu(player, g_marine_Biochemist);
	} else if (g_marine_Shield != null && g_marine_Shield.IsValid() && player == g_marine_Shield.GetCommander() && player.GetMarine() == g_marine_Shield) {
		ToggleVGuiMenu(player, g_marine_Shield);
	}
}

function ToggleGameplayGuide(hPlayer) {
	if (hPlayer == null || !hPlayer.IsValid() || !g_bool_ClhallengeEnable || !g_bool_Initialized) {
		return;
	}
	local hGuide = GetGameplayGuideEntity(hPlayer);
	if (g_bool_IafWin || g_bool_TraitorWin) {
		if (hGuide != null && hGuide.IsValid()) {
			hGuide.SetInt(0, 0);
		}
		return;
	}
	if (hGuide == null || !hGuide.IsValid()) {
		// Do not create an entity from a key press.  The challenge start and
		// fully-joined paths own creation; this keeps the command inert until the
		// challenge has completed initialization.
		return;
	}
	if (hGuide.GetInt(0) != 0) {
		hGuide.SetInt(0, 0);
	} else {
		// Reset the local page whenever opening.  The client also watches this
		// replicated state and returns to MAIN after a close/reopen transition.
		hPlayer.ValidateScriptScope();
		local playerScope = hPlayer.GetScriptScope();
		if (!(g_str_GameplayGuideGenerationField in playerScope)) {
			playerScope[g_str_GameplayGuideGenerationField] <- 0;
		}
		playerScope[g_str_GameplayGuideGenerationField]++;
		hGuide.SetInt(1, playerScope[g_str_GameplayGuideGenerationField]);
		hGuide.SetInt(0, 1);
	}
}

function OnReceivedTextMessage(recipient, sender, message) {
	// 这个函数会在服务端为所有玩家执行一次，因此需要通过这个判断避免重复执行
	if (sender != recipient) {
		return;
	}
	local tempSenderNameLen = sender.GetPlayerName().len();
	local tempRealMsg = message.slice(tempSenderNameLen + 2);
	local tempTraitorMsg = message.slice(0, tempSenderNameLen + 2) + message.slice(tempSenderNameLen + 3);

	if (tempRealMsg.len() == 2 && tempRealMsg.slice(0, 1).tolower() == "/") { //判断是否是操作技能菜单
		if (g_marine_Silencer != null && g_marine_Silencer.IsValid() && sender == g_marine_Silencer.GetCommander() && sender.GetMarine() == g_marine_Silencer) {
			ToggleVGuiMenu(sender, g_marine_Silencer);
		} else if (g_marine_Boomer != null && g_marine_Boomer.IsValid() && sender == g_marine_Boomer.GetCommander() && sender.GetMarine() == g_marine_Boomer) {
			SetBomb();
		} else if (g_marine_Infector != null && g_marine_Infector.IsValid() && sender == g_marine_Infector.GetCommander() && sender.GetMarine() == g_marine_Infector) {
			ToggleVGuiMenu(sender, g_marine_Infector);
		} else if (g_marine_Scanner != null && g_marine_Scanner.IsValid() && sender == g_marine_Scanner.GetCommander() && sender.GetMarine() == g_marine_Scanner) {
			ToggleVGuiMenu(sender, g_marine_Scanner);
		} else if (g_marine_Biochemist != null && g_marine_Biochemist.IsValid() && sender == g_marine_Biochemist.GetCommander() && sender.GetMarine() == g_marine_Biochemist) {
			ToggleVGuiMenu(sender, g_marine_Biochemist);
		} else if (g_marine_Shield != null && g_marine_Shield.IsValid() && sender == g_marine_Shield.GetCommander() && sender.GetMarine() == g_marine_Shield) {
			ToggleVGuiMenu(sender, g_marine_Shield);
		}
		return null;
	}
	if (tempRealMsg[0] == '/') {
		local flgTraitor = false;
		foreach(hPlayer in g_player_TraitorHistory) // 判断是否是内鬼进行队内发言
		{
			if (hPlayer == null || !hPlayer.IsValid()) {
				continue;
			}
			if (hPlayer == sender) {
				flgTraitor = true;
			}
		}
		if (flgTraitor) // 如果是内鬼发言，则发送到队伍里
		{
			if (tempRealMsg.len() == 4 && tempRealMsg.slice(0, 3).tolower() == "/gg") {
				g_tbl_surrenderVotes.VoteStartTime = Time() - 0.2;
				g_tbl_surrenderVotes.Duration = 20.2;
				local playerID = sender.GetPlayerUserID();
				if (!(playerID in g_tbl_surrenderVotes.CurrentTickets)) {
					g_tbl_surrenderVotes.CurrentTickets[playerID] <- sender;
				}
				tempTraitorMsg = message;
			}
			foreach(hPlayer in g_player_TraitorHistory) // 将消息发送给队伍内的内鬼玩家.
			{
				if (hPlayer == null || !hPlayer.IsValid()) {
					continue;
				}
				LocalizedClientPrint(hPlayer, 3, TextColor(250, 0, 0) + "%s1" + TextColor(250, 250, 250) + tempTraitorMsg, "#challenge_traitors_traitor_message_mark");
			}
		}
		// 否则是普通IAF队员发言，需要直接丢弃，否则IAF队员可以用这个方式自证清白。
		return null;
	}
	local tempHPlayers = null;
	while ((tempHPlayers = Entities.FindByClassname(tempHPlayers, sender.GetClassname())) != null) {
		LocalizedClientPrint(tempHPlayers, 3, TextColor(250, 250, 250) + message);
	}
	return null;
}

function ToggleVGuiMenu(hSender, hMarine) {
	hMarine.ValidateScriptScope();
	if (hSender.GetMarine() != g_marine_SilencedMarine) {
		if (hMarine.GetScriptScope().IsOpen == true) {
			local hVGuiBackground = Entities.FindByName(null, hMarine.GetScriptScope().strVGuiNameBackground)
			if (hVGuiBackground) {
				hVGuiBackground.SetInteracter(null);
			}
			for (local i = 0; i < g_marine_Total.len(); i++) {
				local hVGuiButton = Entities.FindByName(null, hMarine.GetScriptScope().strVGuiNameButton[i]);
				if (hVGuiButton) {
					hVGuiButton.SetInteracter(null);
				}
			}
			hMarine.GetScriptScope().IsOpen = false;
		} else {
			local tempVGui = Entities.FindByName(null, hMarine.GetScriptScope().strVGuiNameBackground)
			if (tempVGui != null && tempVGui.IsValid()) {
				tempVGui.SetInteracter(hMarine);
			}
			for (local i = 0; i < g_marine_Total.len(); i++) {
				local hVGuiButton = Entities.FindByName(null, hMarine.GetScriptScope().strVGuiNameButton[i]);
				hVGuiButton.SetInteracter(hMarine);
			}
			hMarine.GetScriptScope().IsOpen = true;
		}
	} else {
		LocalizedClientPrint(hSender, 3, TextColor(250, 250, 250) + "%s1", "#challenge_traitors_marine_silenced_notify");
	}
}

function SetBomb() {
	if (g_bool_BombActivited || g_marine_Boomer == null || !g_marine_Boomer.IsValid()) {
		return;
	}
	g_bool_BombActivited = true;
	if (g_marine_Boomer != g_marine_SilencedMarine) {
		local delay = RandomHQUniformIntDistribution(0, 5);
		DelayFunctionCall("BoomerAlert", "", delay + 0.01);

		g_marine_Boomer.PrecacheSoundScript(BOOMER_SOUND.COUNT_DOWN[delay]);
		g_marine_Boomer.EmitSound(BOOMER_SOUND.COUNT_DOWN[delay]);
		DelayFunctionCall("BoomerSelfExplode", "", 5.0); // 5秒后自爆
	} else {
		LocalizedClientPrint(hSender, 3, TextColor(250, 250, 250) + "%s1", "#challenge_traitors_marine_silenced_notify");
	}
}

function BoomerAlert() {
	if (g_marine_Boomer != null && g_marine_Boomer.IsValid()) {
		g_marine_Boomer.PrecacheSoundScript(BOOMER_SOUND.ALERT);
		g_marine_Boomer.EmitSound(BOOMER_SOUND.ALERT);
	}
}

function BoomerSelfExplode() {
	if (g_marine_Boomer == null || !g_marine_Boomer.IsValid()) {
		return;
	}
	// 查找所有类名为 asw_marine 的实体
	local hMarine = null
	local ratio = 1.0;
	local radius = 150;
	local hAttacker = g_marine_Boomer;
	local explosionPos = hAttacker.GetOrigin() + Vector(0, 0, 60);
	local damageScale = RandomHQNormalDistribution(1.05, 0.05);
	while (hMarine = Entities.FindByClassnameWithin(hMarine, "asw_marine", explosionPos, 4 * radius)) {
		// 检查实体是否有效且存活
		if (g_int_Counter > g_int_ImmuneCounter && hMarine.IsValid() && hMarine != hAttacker && hMarine.GetHealth() > 0) {
			local targetPos = hMarine.GetOrigin();
			local hitCount = GetHitCount(explosionPos, targetPos);
			if (hitCount < 110) {
				local distance = (targetPos - explosionPos).Length();
				local maxHealth = hMarine.GetMaxHealth();
				local currentHealth = hMarine.GetHealth();
				if (distance > 4 * radius) {
					ratio = 0.0;
				} else if (distance > radius && distance <= 4 * radius) {
					ratio = -pow((distance / radius - 1), 2.0) / 9.0 + 1.0;
				}
				// 应用伤害
				local finalDamage = maxHealth * damageScale * ratio;
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
	hAttacker.SetHealth(1);
	hAttacker.TakeDamage(999, DAMAGE_TYPE.DMG_BLAST, hMarine);
}