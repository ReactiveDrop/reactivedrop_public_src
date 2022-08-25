#ifndef _INCLUDED_CAMPAIGN_MAP_SEARCH_LIGHTS_H
#define _INCLUDED_CAMPAIGN_MAP_SEARCH_LIGHTS_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include "asw_shareddefs.h"

struct RD_Campaign_t;

// draws oscillating search lights over the campaign map
class CampaignMapSearchLights : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CampaignMapSearchLights, vgui::Panel );
public:
	CampaignMapSearchLights(vgui::Panel *parent, const char *panelName);
	virtual void Paint();

	void SetCampaign( const RD_Campaign_t *pCampaign );

	int m_iSearchLightX[ASW_NUM_SEARCH_LIGHTS];
	int m_iSearchLightY[ASW_NUM_SEARCH_LIGHTS];
	int m_iSearchLightAngle[ASW_NUM_SEARCH_LIGHTS];

	static int s_nSearchLightTexture;
};


#endif // _INCLUDED_CAMPAIGN_MAP_SEARCH_LIGHTS_H