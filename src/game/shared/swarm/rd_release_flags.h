#pragma once

//#define RD_DISABLE_ALL_RELEASE_FLAGS
#ifndef RD_DISABLE_ALL_RELEASE_FLAGS

// new campaigns sort at the start of the list until some time has passed,
// after which point they are put in approximate order of difficulty
#define RD_NEW_CAMPAIGN_SPOTLIGHT

// new campaigns
#define RD_6A_CAMPAIGNS_ACCIDENT32
//#define RD_6A_CAMPAIGNS_ADANAXIS
//#define RD__CAMPAIGNS_DEADCITY
//#define RD__CAMPAIGNS_REDUCTION

// 7th anniversary
//#define RD_7A_CRAFTING
//#define RD_7A_DROPS
//#define RD_7A_ENEMIES
//#define RD_7A_QUESTS
//#define RD_7A_WEAPONS

// features that are not ready for prime time
//#define RD_USE_FONT_HACK
//#define RD_BONUS_MISSION_ACHIEVEMENTS
//#define RD_STEAM_INPUT_ACTIONS
//#define RD_SPLITSCREEN_ENABLED

#define RD_IS_RELEASE 0
#else
// new campaigns sort at the start of the list until some time has passed,
// after which point they are put in approximate order of difficulty
#define RD_NEW_CAMPAIGN_SPOTLIGHT

// new campaigns that are ready to ship
#define RD_6A_CAMPAIGNS_ACCIDENT32

#define RD_IS_RELEASE 1
#endif
