g_ModeScript.OnGameEvent_marine_selected <- function(params) {
	if (!g_bool_ClhallengeEnable || g_bool_TraitorWin || g_bool_IafWin) {
		return;
	}
	if (g_bool_Initialized) {
		// 获取实体句柄
		local hMarine = EntIndexToHScript(params["new_marine"]);
		local hPlayer = GetPlayerFromUserID(params["userid"]);
		if (hPlayer != null && !hPlayer.IsValid()) {
			return;
		}

		local flg = false;
		foreach(hTraitor in g_marine_TraitorAlive) {
			if (hMarine == hTraitor) {
				flg = true;
			}
		}
		if (flg) // 被接管的是内鬼
		{
			local tempIdx1 = null;
			local tempIdx2 = null;
			foreach(idx, tempMarine in g_marine_Traitor) // 首先找到被接管的机器人在原始内鬼列表里的索引
			{
				if (tempMarine == hMarine) {
					tempIdx1 = idx;
				}
			}
			local flg2 = false;
			foreach(idx, tempPlayer in g_player_Traitor) // 然后检查是否是一个内鬼休息，另一个内鬼接管，并找到索引
			{
				if (tempPlayer != null && tempPlayer.IsValid() && tempPlayer == hPlayer) {
					flg2 = true;
					tempIdx2 = idx;
				}
			}
			if (flg2) {
				// 内鬼队内接管，直接调换两人的顺序
				local temp = g_player_Traitor[tempIdx2];
				g_player_Traitor[tempIdx2] = g_player_Traitor[tempIdx1];
				g_player_Traitor[tempIdx1] = temp;

				GenerateTraitorList(); // 重新生成名单
				ResetHudAndChatForTraitorPlayer(hMarine);
			} else {
				// 非内鬼玩家接管了内鬼，转换阵营
				g_player_Traitor[tempIdx1] = hPlayer;

				GenerateTraitorList(); // 重新生成名单
				ResetHudAndChatForTraitorPlayer(hMarine, true);

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
				// 将该玩家计入历史内鬼
				g_player_TraitorHistory.append(hPlayer);
			}
		} else // 被接管的不是内鬼
		{
			foreach(tempPlayer in g_player_TraitorHistory) // 检查玩家是否曾经做过内鬼
			{
				if (tempPlayer == null || !tempPlayer.IsValid()) {
					continue;
				}
				if (tempPlayer == hPlayer) // 如果曾经做过内鬼，直接处死反骨仔
				{
					hMarine.SetHealth(1);
					hMarine.Die();
					LocalizedClientPrint(tempPlayer, 3, "%s1", "#challenge_traitors_iaf_bot_killed_notify");

					local strLanguage = GetClientLanguage(hMarine.GetCommander().entindex());
					local hHud3 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName3);
					hHud3.SetInt(63, 1);
					hHud3.SetFloat(30, Time() + 2.0);
					hHud3.SetFloat(31, 10.0);
					hHud3.SetString(0, GetLocalizedString("#challenge_traitors_iaf_bot_killed_notify", strLanguage));
					return;
				}
			}
			// 经过检查，该玩家背景清白，是个好人
			ResetHudAndChatForIafPlayer(hMarine);
		}
	}
}

function ResetHudAndChatForIafPlayer(hMarine) {
	if (hMarine == null) {
		return;
	}
	hMarine.ValidateScriptScope();
	local role = hMarine.GetScriptScope().Role;
	local strRole = GetRoleString(role);
	local hPlayer = hMarine.GetCommander();
	LocalizedClientPrint(hPlayer, 3, " ");
	LocalizedClientPrint(hPlayer, 3, TextColor(255, 255, 210) + "%s1", "#challenge_traitors_chat_on_bot_taken", null, null, null, role);
	LocalizedClientPrint(hPlayer, 3, TextColor(255, 255, 210) + "%s1%s2" + GetExtraHealthString(role), "#challenge_traitors_role_chat_role_description_" + strRole, "#challenge_traitors_role_chat_skill_description_" + strRole, null, null, role);

	local strLanguage = GetClientLanguage(hPlayer.entindex());
	local hHud1 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName1);
	hHud1.SetInt(0, role);
	hHud1.SetString(0, GetLocalizedString("#challenge_traitors_hud_on_bot_taken", strLanguage, role));
	hHud1.SetFloat(30, Time());
	local hHud2 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName2);
	hHud2.SetInt(0, role);
	hHud2.SetString(0, GetLocalizedString("#challenge_traitors_role_hud_role_description_" + strRole.tolower(), strLanguage, role) + GetExtraHealthString(role));
	hHud2.SetFloat(30, Time());
}

function ResetHudAndChatForTraitorPlayer(hMarine, flg = false) {
	if (hMarine == null) {
		return;
	}
	local role = hMarine.GetScriptScope().Role;
	local strRole = GetRoleString(role);
	local hPlayer = hMarine.GetCommander();
	local strLanguage = GetClientLanguage(hPlayer.entindex());
	LocalizedClientPrint(hPlayer, 3, " ");
	LocalizedClientPrint(hPlayer, 3, TextColor(255, 0, 0) + "%s1", "#challenge_traitors_chat_on_bot_taken", null, null, null, role);
	LocalizedClientPrint(hPlayer, 3, TextColor(255, 0, 0) + "%s1%s2" + GetExtraHealthString(role), "#challenge_traitors_role_chat_role_description_" + strRole, "#challenge_traitors_role_chat_skill_description_" + strRole, null, null, role);
	LocalizedClientPrint(hPlayer, 3, g_str_TraitorNameList, "#challenge_traitors_traitor_list", "#challenge_traitors_traitor_player_unavailable");

	local hHud1 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName1);
	hHud1.SetInt(0, role);
	hHud1.SetFloat(30, Time());
	hHud1.SetString(0, GetLocalizedString("#challenge_traitors_hud_on_bot_taken", strLanguage, role));

	local hHud2 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName2);
	hHud2.SetInt(0, role);
	hHud2.SetString(0, GetLocalizedString("#challenge_traitors_role_hud_role_description_" + strRole.tolower(), strLanguage, role) + GetExtraHealthString(role));
	hHud2.SetFloat(30, Time());

	local hHud4 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName4);
	hHud4.SetInt(0, role);
	hHud4.SetFloat(30, Time());
	hHud4.SetString(0, GenerateTraitorListHUD(strLanguage));

	if (flg) {
		local hHud3 = Entities.FindByName(null, hPlayer.GetScriptScope().strHudName3);
		hHud3.SetInt(0, role);
		hHud3.SetInt(63, 1);
		hHud3.SetFloat(30, Time() + 2.0);
		hHud3.SetFloat(31, 10.0);
		hHud3.SetString(0, GetLocalizedString("#challenge_traitors_game_instruction_traitor1", strLanguage));
	}
}