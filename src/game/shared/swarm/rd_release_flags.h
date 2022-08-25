#pragma once

//#define RD_DISABLE_ALL_RELEASE_FLAGS
#ifndef RD_DISABLE_ALL_RELEASE_FLAGS

// new campaigns sort at the start of the list until some time has passed,
// after which point they are put in approximate order of difficulty
#define RD_NEW_CAMPAIGN_SPOTLIGHT

// build includes 6th anniversary campaigns
#define RD_6A_CAMPAIGNS_ACCIDENT32
//#define RD_6A_CAMPAIGNS_ADANAXIS

// features that are not ready for prime time
#define RD_COLLECTIONS_WEAPONS_ENABLED
#define RD_COLLECTIONS_SWARMOPEDIA_ENABLED
//#define RD_USE_FONT_HACK

#endif
