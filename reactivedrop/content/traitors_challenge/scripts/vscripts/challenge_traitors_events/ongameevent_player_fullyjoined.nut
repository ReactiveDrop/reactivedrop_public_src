g_ModeScript.OnGameEvent_player_fullyjoined <-  function(params) {
	local hPlayer = GetPlayerFromUserID(params["userid"]);
	// During setup, OnGameplayStart performs a final reconciliation after it
	// publishes g_bool_Initialized.  After DestroyHudAndVGui marks the round
	// closed, do not recreate HUDs or guides for late fully-joined events.
	if (hPlayer != null && g_bool_Initialized && !g_bool_IafWin && !g_bool_TraitorWin) {
		CreatePlayerHud(hPlayer);
		// A player can join after OnGameplayStart.  Build the guide lazily in
		// that case; CreatePlayerGameplayGuide is idempotent and also handles a
		// stale entity name after a map transition.
		if (g_bool_ClhallengeEnable) {
			CreatePlayerGameplayGuide(hPlayer);
		}
	}
}