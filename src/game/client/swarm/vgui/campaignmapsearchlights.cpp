#include "cbase.h"
#include "CampaignMapSearchLights.h"
#include "vgui/isurface.h"
#include "asw_gamerules.h"
#include "rd_missions_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

extern ConVar rd_reduce_motion;

int CampaignMapSearchLights::s_nSearchLightTexture = -1;

CampaignMapSearchLights::CampaignMapSearchLights(vgui::Panel *parent, const char *panelName) :
	vgui::Panel(parent, panelName)
{
	for ( int i = 0; i < ASW_NUM_SEARCH_LIGHTS; i++ )
	{
		m_iSearchLightX[i] = 0;
		m_iSearchLightY[i] = 0;
		m_iSearchLightAngle[i] = 0;
	}
}

void CampaignMapSearchLights::Paint()
{
	if ( s_nSearchLightTexture == -1 )
	{
		s_nSearchLightTexture = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( s_nSearchLightTexture, "vgui/swarm/Campaign/CampaignSearchlight", true, false );
		if ( s_nSearchLightTexture == -1 )
			return;
	}

	vgui::surface()->DrawSetTexture( s_nSearchLightTexture );
	vgui::surface()->DrawSetColor( Color( 255, 255, 255, 128 ) );

	float flTime = rd_reduce_motion.GetBool() ? M_PI_F / 2 : gpGlobals->curtime;

	for ( int i = 0; i < ASW_NUM_SEARCH_LIGHTS; i++ )
	{
		if ( m_iSearchLightX[i] != 0 )
		{
			Vector2D blip_centre( ( m_iSearchLightX[i] / 1024.0f ) * GetWide(),
				( m_iSearchLightY[i] / 1024.0f ) * GetTall() );

			int xoffset = 0;	// rotates around centre so can have no offset
			int yoffset = 0;
			float fLightSize = GetWide() * 0.11f;
			float fOscSize = 40.0f;
			float fOscScale = 0.5f + 0.05f * i;
			float fOscOffset = 1.9f * i;
			float fFacingYaw = m_iSearchLightAngle[i] + ( sin( ( flTime * fOscScale ) + fOscOffset ) * fOscSize );
			Vector vecCornerTL( xoffset, fLightSize * -0.5f + yoffset, 0 );
			Vector vecCornerTR( fLightSize + xoffset, fLightSize * -0.5f + yoffset, 0 );
			Vector vecCornerBR( fLightSize + xoffset, fLightSize * 0.5f + yoffset, 0 );
			Vector vecCornerBL( xoffset, fLightSize * 0.5f + yoffset, 0 );
			Vector vecCornerTL_rotated, vecCornerTR_rotated, vecCornerBL_rotated, vecCornerBR_rotated;

			// rotate it by our facing yaw
			QAngle angFacing( 0, -fFacingYaw, 0 );
			VectorRotate( vecCornerTL, angFacing, vecCornerTL_rotated );
			VectorRotate( vecCornerTR, angFacing, vecCornerTR_rotated );
			VectorRotate( vecCornerBR, angFacing, vecCornerBR_rotated );
			VectorRotate( vecCornerBL, angFacing, vecCornerBL_rotated );

			vgui::Vertex_t points[4] =
			{
				vgui::Vertex_t( Vector2D( blip_centre.x + vecCornerTL_rotated.x, blip_centre.y + vecCornerTL_rotated.y ), Vector2D( 0,0 ) ),
				vgui::Vertex_t( Vector2D( blip_centre.x + vecCornerTR_rotated.x, blip_centre.y + vecCornerTR_rotated.y ), Vector2D( 1,0 ) ),
				vgui::Vertex_t( Vector2D( blip_centre.x + vecCornerBR_rotated.x, blip_centre.y + vecCornerBR_rotated.y ), Vector2D( 1,1 ) ),
				vgui::Vertex_t( Vector2D( blip_centre.x + vecCornerBL_rotated.x, blip_centre.y + vecCornerBL_rotated.y ), Vector2D( 0,1 ) ),
			};
			vgui::surface()->DrawTexturedPolygon( 4, points );
		}
	}
}

void CampaignMapSearchLights::SetCampaign( const RD_Campaign_t *pCampaign )
{
	if ( !pCampaign )
	{
		for ( int i = 0; i < ASW_NUM_SEARCH_LIGHTS; i++ )
		{
			m_iSearchLightX[i] = 0;
			m_iSearchLightY[i] = 0;
			m_iSearchLightAngle[i] = 0;
		}

		return;
	}

	for ( int i = 0; i < ASW_NUM_SEARCH_LIGHTS; i++ )
	{
		m_iSearchLightX[i] = pCampaign->SearchLightX[i];
		m_iSearchLightY[i] = pCampaign->SearchLightY[i];
		m_iSearchLightAngle[i] = pCampaign->SearchLightAngle[i];
	}
}
