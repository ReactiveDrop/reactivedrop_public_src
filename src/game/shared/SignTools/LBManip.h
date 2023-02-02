#ifndef _LBMANIP_H_
#define _LBMANIP_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "./PicoSHA2/picosha2.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "./qTESLA/api.h"
#ifdef __cplusplus
}
#endif

#include "cbase.h"
#include "c_asw_steamstats.h"
#include "c_asw_debrief_stats.h"
#include "asw_gamerules.h"
#include "c_asw_game_resource.h"
#include "fmtstr.h"
#include "asw_equipment_list.h"
#include "string_t.h"
#include "asw_util_shared.h"
#include "asw_marine_profile.h"
#include <vgui/ILocalize.h>
#include "asw_shareddefs.h"
#include "c_asw_marine_resource.h"
#include "c_asw_campaign_save.h"
#include "asw_deathmatch_mode.h"
#include "rd_workshop.h"
#include "rd_lobby_utils.h"
#include "clientmode_asw.h"
#include "missioncompleteframe.h"
#include "missioncompletepanel.h"
#include "rd_missions_shared.h"
#include "c_user_message_register.h"
#include "asw_alien_classes.h"

struct LBRawData {
	int32 m_iLeaderboardScore;
	LeaderboardScoreDetails_v2_t m_LeaderboardScoreDetails;
	unsigned char userIDHash[picosha2::k_digest_size];
	SteamLeaderboard_t m_hSteamLeaderboard;
	unsigned char SHA256[picosha2::k_digest_size];
};

union LBDataUnion {
	struct LBRawData rawData;
	unsigned char byteData[sizeof(struct LBRawData)];
};

std::string LBRawToHexString(CSteamID userID, SteamLeaderboard_t m_hSteamLeaderboard, int32 m_iLeaderboardScore, LeaderboardScoreDetails_v2_t m_LeaderboardScoreDetails);

#endif //_LBMANIP_H_
