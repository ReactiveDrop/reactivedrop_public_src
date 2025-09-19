function OnGameEvent_weapon_reload(params) {
	local hMarine = EntIndexToHScript(params["marine"]);
	local hWeapon = NetProps.GetPropEntity(hMarine, "m_hActiveWeapon");

	switch (hWeapon.GetClassname()) {
		case "asw_weapon_railgun": //导轨步枪
			local time = Time() + RandomHQUniformFloatDistribution(1.5, 2.1);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadStart\", " + time.tostring() + " );", 0.0, null, null);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadEnd\", " + (time + 0.1).tostring() + " );", 0.0, null, null);
			break;
		case "asw_weapon_sniper_rifle": //神射手
			local time = Time() + RandomHQUniformFloatDistribution(1.0, 2.0);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadStart\", " + time.tostring() + " );", 0.0, null, null);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadEnd\", " + (time + 0.1).tostring() + " );", 0.0, null, null);
			break;
		case "asw_weapon_pistol": //双手枪
			local time = Time() + RandomHQUniformFloatDistribution(0.05, 0.9);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadStart\", " + time.tostring() + " );", 0.0, null, null);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadEnd\", " + (time + 0.05).tostring() + " );", 0.0, null, null);
			break;
		case "asw_weapon_pdw": //单兵防御武器
			local time = Time() + RandomHQUniformFloatDistribution(0.05, 0.9);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadStart\", " + time.tostring() + " );", 0.0, null, null);
			EntFireByHandle(hWeapon, "RunScriptCode", "NetProps.SetPropFloat( self, \"m_fFastReloadEnd\", " + (time + 0.05).tostring() + " );", 0.0, null, null);
			break;
	}


}