/*参数说明
	int		0  - 按钮序号
	int		1  - 士兵角色
	int		2  - 总人数
	int		3  - 是否存活
	int		4  - 蛊惑者是否可以使用技能

	string	0  - 名字

	entity	0  - marine
*/
isServer <- false;
IncludeScript("challenge_traitors_enums");
IncludeScript("challenge_traitors_client_shared");

function Paint() {}

function Control(tbl) {}

function OnUpdate() {
	self.ForceSync();
	local idx = self.GetInt(0);
	if (!("marine_info" in getconsttable())) {
		getconsttable()["marine_info"] <- [];
	}
	getconsttable()["marine_info"].resize(self.GetInt(2));
	local tbl = {
		role = self.GetInt(1),
		name = self.GetString(0),
		isAlive = self.GetInt(3) != 0 ? true : false,
		biochemistIsHealed = self.GetInt(MENU_IDX_INT.BIOCHEMIST_IS_HEALED) != 0 ? true : false,
		biochemistIsKilled = self.GetInt(MENU_IDX_INT.BIOCHEMIST_IS_KILLED) != 0 ? true : false,
		infectorIsAbeted = self.GetInt(MENU_IDX_INT.INFECTOR_IS_ABETED) != 0 ? true : false,
		scannerIsRevealed = self.GetInt(MENU_IDX_INT.SCANNER_IS_REVEALED) != 0 ? true : false,
		scannerIsWithinRange = self.GetInt(MENU_IDX_INT.SCANNER_IS_WITHIN_RANGE) != 0 ? true : false,
		shieldIsSelected = self.GetInt(MENU_IDX_INT.SHIELD_IS_SELECTED) != 0 ? true : false,
		silencerIsSilenced = self.GetInt(MENU_IDX_INT.SILENCER_IS_SILENCED) != 0 ? true : false,
	};
	getconsttable()["marine_info"][idx] = tbl;

	if ("biochemist_next_active_time" in getconsttable()) {
		getconsttable()["biochemist_next_active_time"] = self.GetFloat(MENU_IDX_INT.BIOCHEMIST_NEXT_AVAILABLE_TIME);
	}
	if ("infector_next_active_time" in getconsttable()) {
		getconsttable()["infector_next_active_time"] = self.GetFloat(MENU_IDX_INT.INFECTOR_NEXT_AVAILABLE_TIME);
	}
	if ("scanner_next_active_time" in getconsttable()) {
		getconsttable()["scanner_next_active_time"] = self.GetFloat(MENU_IDX_INT.SCANNER_NEXT_AVAILABLE_TIME);
	}
	if ("shield_next_active_time" in getconsttable()) {
		getconsttable()["shield_next_active_time"] = self.GetFloat(MENU_IDX_INT.SHIELD_NEXT_AVAILABLE_TIME);
	}
	if ("silencer_next_active_time" in getconsttable()) {
		getconsttable()["silencer_next_active_time"] = self.GetFloat(MENU_IDX_INT.SILENCER_NEXT_AVAILABLE_TIME);
	}

	if ("biochemist_is_heal_used" in getconsttable()) {
		getconsttable()["biochemist_is_heal_used"] = self.GetInt(MENU_IDX_INT.BIOCHEMIST_IS_HEAL_USED) != 0 ? true : false;
	}
	if ("biochemist_is_kill_used" in getconsttable()) {
		getconsttable()["biochemist_is_kill_used"] = self.GetInt(MENU_IDX_INT.BIOCHEMIST_IS_KILL_USED) != 0 ? true : false;
	}
	if ("infector_is_skill_active" in getconsttable()) {
		getconsttable()["infector_is_skill_active"] = self.GetInt(MENU_IDX_INT.INFECTOR_IS_SKILL_ACTIVE) != 0 ? true : false;
	}
	if ("infector_is_skill_used" in getconsttable()) {
		getconsttable()["infector_is_skill_used"] = self.GetInt(MENU_IDX_INT.INFECTOR_IS_SKILL_USED) != 0 ? true : false;
	}
	if ("scanner_is_skill_used" in getconsttable()) {
		getconsttable()["scanner_is_skill_used"] = self.GetInt(MENU_IDX_INT.SCANNER_IS_SKILL_USED) != 0 ? true : false;
	}
	if ("shield_is_skill_used" in getconsttable()) {
		getconsttable()["shield_is_skill_used"] = self.GetInt(MENU_IDX_INT.SHIELD_IS_SKILL_USED) != 0 ? true : false;
	}
	if ("silencer_is_skill_used" in getconsttable()) {
		getconsttable()["silencer_is_skill_used"] = self.GetInt(MENU_IDX_INT.SILENCER_IS_SKILL_USED) != 0 ? true : false;
	}
}