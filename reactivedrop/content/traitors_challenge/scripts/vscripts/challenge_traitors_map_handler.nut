g_int_MapKillCounter <- array(64, INT_MAX);
g_vec_Jac2Offset <- Vector(0, 0, 0);
function SetMapHandler() {
	switch (g_enum_CurrentMap) {
		case MAP._9800_1:
		case MAP.TILA_4: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z <= -64) {
						continue;
					} else if ((temp.x > -695 && temp.y < 2670 && temp.z > -64) || (temp.x <= -466 && temp.z > 128) || (temp.x > -466 && temp.z > 128)) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP._9800_2:
		case MAP._9800_3:
		case MAP.BON_7: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if ((temp.z > 424) || (temp.y < (-909) && temp.z > -80) || (temp.x > -900 && temp.y <= (-115) && !(temp.x >= -435 && temp.x <= 2567 && temp.y >= -527 && temp.y <= -83) && temp.z > -32)) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP._9800_4:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 200) {
						hMarine.TakeDamage(3, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-5000) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.ACC_1: // 禁止钻管道
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if ((temp.x > 80 && temp.x < 690 && temp.y > 700 && temp.y < 2350 && temp.z > -15)) {
						hMarine.SetOrigin(Vector(85, 688, 0));
					}
				}
			};
			break;
		case MAP.ACC_2:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-500) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.ACC_3:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 300) {
						hMarine.TakeDamage(3, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.y < 6133 && temp.z > 150) {
						hMarine.TakeDamage(3, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.x >= 2659 && temp.y >= 6627 && temp.z > -20) {
						hMarine.TakeDamage(3, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.x > 1418 && temp.z > 150) {
						hMarine.TakeDamage(3, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.x > 1875 && temp.y >= 7434 && temp.z > -20) {
						hMarine.TakeDamage(3, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-500) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.ACC_4: // 防止蹲守下落点
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					temp = hMarine.GetOrigin() - Vector(350, -2100, 468);
					temp.z = 0;
					local radius = temp.Norm();
					if (radius < 100) {
						hMarine.SetOrigin(Vector(RandomHQUniformFloatDistribution(-650, 870), RandomHQUniformFloatDistribution(-1590, -1320), -30));
					}
				}
			};
			break;
		case MAP.ACC_6: // 防止打女王
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.x > 1580 && temp.x < 2080 && temp.y > 4820 && temp.y < 5120) {
						hMarine.SetOrigin(Vector(1480, temp.y, -350));
					}
				}
			};
			break;
		case MAP.BIO_1: // 防止上墙+防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					hMarine.GetScriptScope().DamageMapModifier = 1.0;
					local temp = hMarine.GetOrigin();
					if ((temp.x > -300 && temp.x < 100 && temp.y > 2745 && temp.y < 2946)) {
						hMarine.SetOrigin(Vector(-210, 2730, 120));
					} else if ((temp.x < (-2155) && temp.x > -3840) && (temp.y > 2515 && temp.y < 2800)) {
						hMarine.SetOrigin(Vector(temp.x, 2410, -20));
					} else if (temp.x < (-930) && temp.x > (-1280) && temp.y < 1540) {
						hMarine.SetOrigin(Vector(temp.x, 1580, 0));
					} else if ((temp.x < (-4033) && temp.y > 4500 && temp.z < (-300) && temp.z > (-800)) || (temp.x >= (-4033) && temp.y > 4500 && temp.z < (-300) && temp.z > (-450))) {
						hMarine.GetScriptScope().DamageMapModifier = 0.0;
					}

					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 3950) {
						g_int_MapKillCounter[0] = g_int_Counter + 1200;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.y >= 5700) {
						g_int_MapKillCounter[1] = g_int_Counter + 1200;
					}
					if (g_int_MapKillCounter[2] == INT_MAX && temp.y >= 8400) {
						g_int_MapKillCounter[2] = g_int_Counter + 1200;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y < 3950) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && temp.y < 5700) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[2] && temp.z < 7200) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-1900) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
				if (idx_end == 0) {
					local hTurret = null;
					hTurret = Entities.FindByName(null, "turrets");
					NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
				}
			};
			break;
		case MAP.BIO_2: // 防止开超级爪虫+防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					hMarine.GetScriptScope().DamageMapModifier = 1.0;
					local temp = hMarine.GetOrigin();
					if ((temp.x > 400 && temp.x < 800 && temp.y > 1500 && temp.y < 1900) || (temp.x < 100 && temp.y < 1600)) {
						hMarine.Die();
					} else if (temp.x < 0 && temp.z > 720) {
						hMarine.TakeDamage(10, DAMAGE_TYPE.DMG_FALL, null);
					}

					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 2200) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.y >= 4950) {
						g_int_MapKillCounter[1] = g_int_Counter + 600;
					}
					if (g_int_MapKillCounter[2] == INT_MAX && temp.y >= 6068) {
						g_int_MapKillCounter[2] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y < 2200) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && temp.y < 4950) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[2] && temp.z < 6068) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.BIO_3: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 255) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.x > 2700 && temp.y > 2300 && temp.z > 255) {
						hMarine.TakeDamage(40, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.BON_1: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (g_int_MapKillCounter[0] == INT_MAX && temp.z > -330 && temp.y > -800) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && (temp.z <= -330 || temp.y <= -800)) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.z > -340 && temp.y > -3400) {
						g_int_MapKillCounter[1] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && temp.y <= -2900 && (temp.z <= -340 || temp.y <= -3400)) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}


				}
			};
			break;
		case MAP.BON_2: // 防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (g_int_MapKillCounter[0] == INT_MAX && temp.x > -5100 && temp.y > -9200) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && !(temp.x > -5100 && temp.y > -9200)) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.BON_3: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > -444) {
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.BON_11: // 防止被地图秒杀+防止路径被卡住+防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (g_int_MapKillCounter[0] == INT_MAX && temp.y > 7250) {
						g_int_MapKillCounter[0] = g_int_Counter + 5;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.y <= --1000) {
						g_int_MapKillCounter[1] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && temp.y > 1550) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.y >= 5129 && temp.y <= 5350) {
						hMarine.SetOrigin(Vector(RandomHQUniformIntDistribution(-5606, -5257), RandomHQUniformIntDistribution(4388, 4996), 810));
					}
				}
				if (g_int_Counter == g_int_MapKillCounter[0]) {
					foreach(hMarine in g_marine_Total) {
						if (hMarine != null && hMarine.IsValid() && hMarine.GetOrigin().x > -3200) {
							hMarine.SetOrigin(Vector(RandomHQUniformIntDistribution(-3426, -3228), RandomHQUniformIntDistribution(6834, 7258), 810));
						}
					}
				}
			};
			break;
		case MAP.BON_4:
			//1632,2328
			g_vec_Jac2Offset = Vector(1632, 2328, 0);
		case MAP.JAC_2: // 防止被落在电梯外+防止路径被卡住
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin() - g_vec_Jac2Offset;
					if (g_int_MapKillCounter[0] == INT_MAX && temp.x > -3900) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (temp.y >= 4947 && temp.y <= 5019) {
						hMarine.SetOrigin(Vector(RandomHQUniformIntDistribution(-5642, -5316), RandomHQUniformIntDistribution(5375, 5989), 810) + g_vec_Jac2Offset);
					}
				}
				if (g_int_Counter == g_int_MapKillCounter[0]) {
					foreach(hMarine in g_marine_Total) {
						if (hMarine != null && hMarine.IsValid() && (hMarine.GetOrigin().x - g_vec_Jac2Offset.x) <= -3900) {
							hMarine.SetOrigin(Vector(RandomHQUniformIntDistribution(-3426, -3228), RandomHQUniformIntDistribution(6834, 7258), 810) + g_vec_Jac2Offset);
						}
					}
				}
			};
			break;
		case MAP.JAC_3: // 修改飞船机枪友伤
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				if (idx_end == 0) {
					local hTurret = null;
					hTurret = Entities.FindByName(null, "ship_turrets");
					NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
				}
			};
			break;
		case MAP.JAC_4:
		case MAP.BON_10: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z <= 690) {
						continue;
					} else if (temp.z > 920 && temp.x < (-802) && temp.y > 218) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.z > 690 && temp.x >= (-802) && temp.y > 218) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.z > 768 && temp.y <= 218) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.JAC_5: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.x > (-1453) && temp.z > 24) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.x <= (-1453) && temp.z > 328) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-1000) && hMarine.IsInhabited()) { // 防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.JAC_6:
		case MAP.BON_9: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.y < (-530) && temp.z > -468) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.y >= (-530) && temp.y < 3107 && temp.z > -365) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.x > -1651 && temp.y >= 3107 && temp.z > -365) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					} else if (temp.y >= 3107 && temp.z > -232) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-900) && hMarine.IsInhabited()) { // 防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.JAC_7: // 防止上墙+防止将任务拖到失败
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.x > (-600) && temp.x < 550 && temp.y > -4600 && temp.y <= (-3000) && temp.z > 120) {
						hMarine.TakeDamage(60, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_MapKillCounter[0] == INT_MAX && temp.x > 2600 && temp.y > -4600) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
						g_int_MapKillCounter[1] = g_int_Counter + 900;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && !(temp.x > 550 && temp.y > -4600 && temp.y <= -1600)) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && !(temp.x > 550 && temp.y > -4600 && temp.y <= -1600)) {
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
				if (idx_end == 0) {
					local hTurret = null;
					hTurret = Entities.FindByName(null, "ship_turrets");
					NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
				}
			};
			break;
		case MAP.LANA_1: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.y > 2226 && temp.z > -510) {
						hMarine.TakeDamage(10, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < -800) {
						hMarine.TakeDamage(10, DAMAGE_TYPE.DMG_FALL, null);
					}

					if (g_int_MapKillCounter[0] == INT_MAX && temp.y > 2250) {
						g_int_MapKillCounter[0] = g_int_Counter + 400;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y <= 2250) {
						hMarine.SetHealth(1);
						hMarine.TakeDamage(999, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.x > -1297 && temp.y > 2781) {
						g_int_MapKillCounter[1] = g_int_Counter + 200;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && !(temp.x > -1297 && temp.y > 2781)) {
						hMarine.SetHealth(1);
						hMarine.TakeDamage(999, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.NH_1:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-1000) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.NH_2:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-500) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.NH_3:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-2000) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.OCS_1: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > -222) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-1000) && hMarine.IsInhabited()) { // 防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.OCS_2:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-1500) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.PARA_1: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0;
					(i * 10 + idx_end) < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 310) {
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-1500) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.PARA_2:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0;
					(i * 10 + idx_end) < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-1000) && hMarine.IsInhabited()) { // 防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.PARA_3:
		case MAP.BON_5: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 715) {
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-1000) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.PARA_4:
		case MAP.BON_6: // 防止电梯乱杀及躲在电梯下
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.x > 1502 && temp.y > -1558 && temp.z < -270) {
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (temp.z < (-1000) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
				if (g_int_Counter % 49 != 0) {
					return;
				}
				local flg = false;
				foreach(hMarine in g_marine_Total) {
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 150) {
						flg = true;
					}
				}
				if (flg) {
					local temp = RandomInt(0, 20) - 10; // -2~2
					g_int_ImmuneCounter = g_int_Counter + 125 + temp;
				}
			};
			break;
		case MAP.PARA_5:
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z < (-1000) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.RED_2: // 防止拖延
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (g_int_Counter > 300 && g_int_Counter <= 1800 && temp.y <= (9.33333 * g_int_Counter - 11800)) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					} else if (g_int_Counter > 1800 && temp.y <= 5000) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 8200) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y <= 8200) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.RED_3: // 防止将任务拖到失败
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (g_int_MapKillCounter[0] == INT_MAX && temp.z <= -600) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.z <= -1000) {
						g_int_MapKillCounter[1] = g_int_Counter + 700;
					}
					if (g_int_MapKillCounter[2] == INT_MAX && temp.z <= -1400) {
						g_int_MapKillCounter[2] = g_int_Counter + 800;
					}
					if (g_int_MapKillCounter[3] == INT_MAX && temp.z <= -2000) {
						g_int_MapKillCounter[3] = g_int_Counter + 900;
					}
					if (g_int_MapKillCounter[4] == INT_MAX && temp.x >= 3000) {
						g_int_MapKillCounter[4] = g_int_Counter + 1000;
					}
					if (g_int_MapKillCounter[5] == INT_MAX && temp.x >= 3000 && temp.y >= 600) {
						g_int_MapKillCounter[5] = g_int_Counter + 1100;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.z > -600) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && temp.z > -1000) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[2] && temp.z > -1400) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[3] && temp.z > -2000) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[4] && temp.x < 3000) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[5] && temp.x < 3000 && temp.y < 600) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break; //(-3361.671875, -4576.698730, 2379.031250))
		case MAP.RED_5: // 防止拖延任务
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (g_int_MapKillCounter[0] == INT_MAX && temp.x >= -3400 && temp.y >= -4000 && temp.z >= 2350) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && !(temp.x >= -6360 && temp.x < (-3613) && temp.y >= -3043 && temp.y < 479 && temp.z >= 1700 && temp.z <= 2150)) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.RES_4: // 特殊处理res-4导致的游戏体验问题
			ProcessMapSpecialCases = function() {
				if (g_int_Counter < 350) {
					local idx_end = g_int_Counter % 10;
					for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
						local hMarine = g_marine_Total[i * 10 + idx_end];
						if (hMarine == null || !hMarine.IsValid()) {
							continue;
						}
						local temp = hMarine.GetOrigin();
						if (temp.x < (-4947.5) || temp.x > -4463 || temp.y < (-4120) || temp.y > -3760.5 || temp.y > (0.4731 * temp.x - 1479) || temp.y > (-0.48326 * temp.x - 5975) || temp.y < (0.46243 * temp.x - 1998.5) || temp.y < (-0.44139 * temp.x - 6246.5)) {
							hMarine.SetOrigin(Vector(-4770 + RandomInt(0, 50), -3950 + RandomInt(0, 50), temp.z));
						}
					}
				}
			};
			break;
		case MAP.TFT_1: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 250) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.TFT_2: // 防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					hMarine.GetScriptScope().DamageMapModifier = 1.0;
					local temp = hMarine.GetOrigin();

					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 910) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y < 910) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.TILA_1: // 防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					hMarine.GetScriptScope().DamageMapModifier = 1.0;
					local temp = hMarine.GetOrigin();

					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 5050) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.x <= -2500 && temp.y >= 6643) {
						g_int_MapKillCounter[1] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y < 5050) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && !(temp.x <= -2369 && temp.y >= 5139 && temp.z <= 250)) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.TILA_2: // 防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					hMarine.GetScriptScope().DamageMapModifier = 1.0;
					local temp = hMarine.GetOrigin();

					if (temp.z > 300 && temp.y > 3400) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}

					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 3400) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.x <= -301 && temp.y >= 5709) {
						g_int_MapKillCounter[1] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y < 3400) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && !(temp.x <= -301 && temp.y >= 5709)) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.TILA_3: // 防止拖时间
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					hMarine.GetScriptScope().DamageMapModifier = 1.0;
					local temp = hMarine.GetOrigin();
					if (g_int_MapKillCounter[0] == INT_MAX && temp.y >= 1400) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_MapKillCounter[1] == INT_MAX && temp.x <= -301 && temp.y >= 5709) {
						g_int_MapKillCounter[1] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && temp.y < 1400) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}
					if (g_int_Counter > g_int_MapKillCounter[1] && !(temp.x <= -301 && temp.y >= 5709)) {
						hMarine.TakeDamage(2, DAMAGE_TYPE.DMG_FALL, null);
					}

					if (temp.z < (-500) && hMarine.IsInhabited()) { //防止下地底
						hMarine.TakeDamage(5, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.TILA_5: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 270) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}

					//-2200  -3000 -280
					if (g_int_MapKillCounter[0] == INT_MAX && temp.z <= -350 && temp.x > -1800 && temp.x <= -1130 && temp.y <= -3300) {
						g_int_MapKillCounter[0] = g_int_Counter + 600;
					}
					if (g_int_Counter > g_int_MapKillCounter[0] && !(temp.x > -2200 && temp.y <= -3000 && temp.z <= -250)) {
						hMarine.TakeDamage(4, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
			};
			break;
		case MAP.TILA_7: // 恢复飞船友伤
			ProcessMapSpecialCases = function() {
				if (g_int_Counter % 10 == 0) {
					local hTurret = null;
					hTurret = Entities.FindByName(null, "ship_turrets");
					NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
				}
			};
			break;
		case MAP.TILA_8: // 防止上墙
			ProcessMapSpecialCases = function() {
				if (g_int_Counter >= 150 && g_int_Counter < 180) {
					local idx_end = g_int_Counter % 10;
					for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
						local hMarine = g_marine_Total[i * 10 + idx_end];
						if (hMarine == null || !hMarine.IsValid()) {
							continue;
						}
						local temp = hMarine.GetOrigin();
						if (temp.z > 1600) {
							hMarine.SetOrigin(Vector(656, -5160, 1300));
						}
					}
				}
			};
			break;
		case MAP.TILA_9: // 防止上墙
			ProcessMapSpecialCases = function() {
				local idx_end = g_int_Counter % 10;
				for (local i = 0; i * 10 + idx_end < g_int_MarineCount; i++) {
					local hMarine = g_marine_Total[i * 10 + idx_end];
					if (hMarine == null || !hMarine.IsValid()) {
						continue;
					}
					local temp = hMarine.GetOrigin();
					if (temp.z > 4760) {
						hMarine.TakeDamage(1, DAMAGE_TYPE.DMG_FALL, null);
					}
				}
				if (idx_end == 0) {
					local hTurret = null;
					hTurret = Entities.FindByName(null, "ship_turrets");
					NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
					hTurret = Entities.FindByName(null, "ship_turrets_2");
					NetProps.SetPropBool(hTurret, "m_bFriendlyFire", true);
				}
			};
			break;
		default:
			ProcessMapSpecialCases = function() {};
	}
};