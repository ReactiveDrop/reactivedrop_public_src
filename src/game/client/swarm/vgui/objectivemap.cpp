#include "cbase.h"

#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/TextImage.h"
#include "WrappedLabel.h"
#include "c_asw_game_resource.h"
#include "ObjectiveMap.h"
#include "ObjectiveMapMarkPanel.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_asw_objective.h"
#include "c_asw_marker.h"
#include "vgui_controls/AnimationController.h"
#include "ObjectiveTitlePanel.h"
#include "ObjectiveDetailsPanel.h"
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include "filesystem.h"
#include "asw_hud_minimap.h"
#include "hud_element_helper.h"
#include "SoftLine.h"
#include "stats_report.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

extern ConVar asw_hud_scale;
extern ConVar rd_hud_minimap_drawing;
extern ConVar rd_reduce_motion;

Color GetColorPerIndex(int player_index)
{
	C_ASW_Player *pPlayer = dynamic_cast<C_ASW_Player *>( UTIL_PlayerByIndex( player_index ) );
	C_ASW_Game_Resource *pGameResource = ASWGameResource();
	C_ASW_Marine_Resource *pMR = pPlayer && pGameResource ? pGameResource->GetFirstMarineResourceForPlayer( pPlayer ) : NULL;
	int iMarineResourceIndex = pGameResource && pMR ? pGameResource->GetMarineResourceIndex( pMR ) : -1;
	if ( iMarineResourceIndex < 0 || iMarineResourceIndex >= NELEMS( g_rgbaStatsReportPlayerColors ) )
	{
		return Color( 255, 255, 255, 255 ); // white
	}
	return g_rgbaStatsReportPlayerColors[ iMarineResourceIndex ];
}

ObjectiveMap::ObjectiveMap(Panel *parent, const char *name) : Panel(parent, name)
{	
	m_pDetailsPanel = NULL;
	m_pObjective = NULL;
	m_pQueuedObjective = NULL;

	m_pMapImage = new vgui::ImagePanel(this, "ObjectiveMapImage");
	m_pMapImage->SetShouldScaleImage(true);
	m_pMapImage->SetMouseInputEnabled(false);
	m_pMapImage->SetVisible(true);

	m_pNoMapLabel = new vgui::Label(this, "NoMapLabel", "#asw_briefing_no_map");	
	m_pNoMapLabel->SetContentAlignment(vgui::Label::a_center);
	m_pNoMapLabel->SetVisible(false);

	m_bHaveQueuedItem = false;
	m_iNumMapMarks = 0;

	for (int i=0;i<ASW_NUM_MAP_MARKS;i++)
	{
		m_MapMarkPanels[i] = new ObjectiveMapMarkPanel(this, "ObjectiveMapMarkPanel");
	}

	m_pMapDrawing = new ObjectiveMapDrawingPanel(this, "MapDrawing");
	m_pMapDrawing->SetZPos(200);

	SetMap(engine->GetLevelNameShort());
}
	
ObjectiveMap::~ObjectiveMap()
{
}

void ObjectiveMap::PerformLayout()
{
	BaseClass::PerformLayout();
	
	if (m_pMapImage)
	{
		int mx, my, mw, mt;
		GetDesiredMapBounds(mx, my, mw, mt);
		m_pMapImage->SetBounds(mx, my, mw, mt);
		m_pNoMapLabel->SetBounds(mx, my, mw, mt);

		m_pMapDrawing->SetBounds(mx, my, mw, mt);
	}

	for (int i=0;i<ASW_NUM_MAP_MARKS;i++)
	{
		m_MapMarkPanels[i]->SetBracketScale(GetWide() / 1024.0f);
	}	
}

void ObjectiveMap::OnThink()
{
	BaseClass::OnThink();

	if (m_bHaveQueuedItem)
	{
		if (m_pQueuedObjective != NULL)
		{			
			m_pObjective = m_pQueuedObjective;
			FindMapMarks();
			InvalidateLayout(true);
			m_pQueuedObjective = NULL;
			m_bHaveQueuedItem = false;			
		}
		else
		{
			m_pObjective = NULL;			
			m_bHaveQueuedItem = false;			
		}
	}

	InvalidateLayout(true);
	bool bSetPulsing = false;
	bool bFadeIn = false;
	bool bFadeOut = false;
	if ( m_iNumMapMarks > 0 )
	{
		//m_MapMarkPanels[0]->m_bDebug = true;
		if ( m_MapMarkPanels[0]->GetAlpha() >= 255 )
		{
			bSetPulsing = true;
		}
		if ( m_MapMarkPanels[0]->m_bPulsing && !rd_reduce_motion.GetBool() )
		{
			if ( m_MapMarkPanels[0]->GetAlpha() <= 128 )
			{
				bFadeIn = true;
			}
			else if ( m_MapMarkPanels[0]->GetAlpha() >= 255 )
			{
				bFadeOut = true;
			}
		}
	}
	for ( int i = 0; i < m_iNumMapMarks; i++ )
	{
		if ( bSetPulsing )
			m_MapMarkPanels[i]->m_bPulsing = true;
		if ( bFadeIn )
			vgui::GetAnimationController()->RunAnimationCommand(m_MapMarkPanels[i], "alpha", 255, 0, 0.3f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		if ( bFadeOut )
			vgui::GetAnimationController()->RunAnimationCommand(m_MapMarkPanels[i], "alpha", 128, 0, 0.3f, vgui::AnimationController::INTERPOLATOR_LINEAR);
	}
}

void ObjectiveMap::SetObjective(C_ASW_Objective* pObjective)
{
	if (pObjective == m_pObjective)
		return;

	// delete and recreate map markers, cos they vanish for some reason :/
	for (int i=0;i<ASW_NUM_MAP_MARKS;i++)
	{
		if (m_MapMarkPanels[i])
			delete m_MapMarkPanels[i];
	}
	for (int i=0;i<ASW_NUM_MAP_MARKS;i++)
	{
		m_MapMarkPanels[i] = new ObjectiveMapMarkPanel(this, "ObjectiveMapMarkPanel");
		m_MapMarkPanels[i]->SetBracketScale(GetWide() / 1024.0f);
	}

	m_pQueuedObjective = pObjective;
	m_bHaveQueuedItem = true;
}

// converts a coord from 0->1023 map texture into x+y in this panel
Vector2D ObjectiveMap::MapTextureToPanel( const Vector2D &texturepos )
{
	int x, y, w, t;
	m_pMapImage->GetBounds( x, y, w, t );

	Vector2D vResult = texturepos;

	vResult.x *= ( w / 1024.0f );
	vResult.y *= ( t / 1024.0f );
	vResult.x += x;
	vResult.y += y;

	return vResult;
}

int ObjectiveMap::GetArcSize( void )
{
	return GetWidth() * 0.04f * asw_hud_scale.GetFloat();
}

void ObjectiveMap::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(true);
	SetPaintBackgroundType(0);
	SetBgColor(Color(0,0,0,255));
	SetBorder(pScheme->GetBorder("ASWBriefingButtonBorder"));
	SetPaintBorderEnabled(true);

	m_pNoMapLabel->SetFgColor(Color(128,128,128,255));

	for (int i=0;i<ASW_NUM_MAP_MARKS;i++)
	{
		m_MapMarkPanels[i]->SetAlpha(0);		
	}
}


// loads in the script file for a particular map to set scale/origin
void ObjectiveMap::SetMap( const char *levelname )
{
	if ( !m_pMapImage )
	{
		DevMsg( 1, "Error, no image panel to load overview into\n" );
		return;
	}

	// load new KeyValues
	if ( m_szLastLevelName == levelname )
	{
		return;	// map didn't change
	}

	m_szLastLevelName = levelname;

	const RD_Mission_t *pOverview = ReactiveDropMissions::GetMission( levelname );
	if ( !pOverview )
	{
		DevMsg( 1, "ObjectiveMap::SetMap: couldn't load overview file for map %s.\n", levelname );
		m_pNoMapLabel->SetVisible( true );
		return;
	}

	// strip the vgui/ off the start
	m_pMapImage->SetImage( VarArgs( "../%s", STRING( pOverview->BriefingMaterial ) ) );
	int mx, my, mw, mt;
	GetDesiredMapBounds( mx, my, mw, mt );
	m_pMapImage->SetBounds( mx, my, mw, mt );
	m_pNoMapLabel->SetVisible( false );

	m_MapOrigin.x = pOverview->PosX;
	m_MapOrigin.y = pOverview->PosY;
	m_fMapScale = pOverview->Scale;
}

void ObjectiveMap::AddMapMark( const MapMarkCandidate & candidate, bool bComplete )
{
	if ( m_iNumMapMarks >= ASW_NUM_MAP_MARKS || candidate.size.IsZero() )
	{
		return;
	}

	Vector2D absSinCos;
	SinCos( DEG2RAD( candidate.yaw ), &absSinCos.y, &absSinCos.x );
	absSinCos.x = fabsf( absSinCos.x );
	absSinCos.y = fabsf( absSinCos.y );

	Vector2D rotatedHalfSize
	{
		( candidate.size.x * absSinCos.x + candidate.size.y * absSinCos.y ) * 0.5f,
		( candidate.size.y * absSinCos.x + candidate.size.x * absSinCos.y ) * 0.5f,
	};

	Vector2D mapSpaceMins, mapSpaceMaxs;
	Vector2DSubtract( candidate.center, rotatedHalfSize, mapSpaceMins );
	Vector2DAdd( candidate.center, rotatedHalfSize, mapSpaceMaxs );

	int mx, my, mw, mt;
	GetDesiredMapBounds( mx, my, mw, mt );

	int markx = mx + ( mapSpaceMins.x / 1024.0f ) * mw;
	int marky = my + ( mapSpaceMins.y / 1024.0f ) * mt;
	int markw = ( ( mapSpaceMaxs.x - mapSpaceMins.x ) / 1024.0f ) * mw;
	int markh = ( ( mapSpaceMaxs.y - mapSpaceMins.y ) / 1024.0f ) * mt;

	const float fDuration = rd_reduce_motion.GetBool() ? 0.0f : 0.3f;

	int i = m_iNumMapMarks++;
	m_MapMarkPanels[i]->SetVisible( true );
	vgui::GetAnimationController()->RunAnimationCommand( m_MapMarkPanels[i], "xpos", markx, 0, fDuration, vgui::AnimationController::INTERPOLATOR_LINEAR );
	vgui::GetAnimationController()->RunAnimationCommand( m_MapMarkPanels[i], "ypos", marky, 0, fDuration, vgui::AnimationController::INTERPOLATOR_LINEAR );
	vgui::GetAnimationController()->RunAnimationCommand( m_MapMarkPanels[i], "wide", markw, 0, fDuration, vgui::AnimationController::INTERPOLATOR_LINEAR );
	vgui::GetAnimationController()->RunAnimationCommand( m_MapMarkPanels[i], "tall", markh, 0, fDuration, vgui::AnimationController::INTERPOLATOR_LINEAR );
	vgui::GetAnimationController()->RunAnimationCommand( m_MapMarkPanels[i], "alpha", 255, 0, fDuration, vgui::AnimationController::INTERPOLATOR_LINEAR );
	m_MapMarkPanels[i]->m_bComplete = bComplete || m_pObjective->IsObjectiveComplete();
}

void ObjectiveMap::FindMapMarks()
{
	if ( !m_pObjective )
	{
		return;
	}

	// hide all markers for now
	m_iNumMapMarks = 0;
	for ( int i = 0; i < ASW_NUM_MAP_MARKS; i++ )
	{
		m_MapMarkPanels[i]->SetAlpha( 0 );
		m_MapMarkPanels[i]->SetVisible( false );
		m_MapMarkPanels[i]->SetPos( 0, 0 );
		m_MapMarkPanels[i]->SetSize( GetWide(), GetTall() );
		m_MapMarkPanels[i]->m_bPulsing = false;
	}

	CASWHudMinimap *pMiniMap = GET_HUDELEMENT( CASWHudMinimap );
	Assert( pMiniMap );
	if ( !pMiniMap )
	{
		return;
	}

	bool bAnyMarkers = false;

	// make the markers start to animate in
	FOR_EACH_VEC( IObjectiveMarkerList::AutoList(), iMarker )
	{
		C_ASW_Marker *pMarker = assert_cast< C_ASW_Marker * >( IObjectiveMarkerList::AutoList()[iMarker] );
		Assert( pMarker );
		if ( !pMarker || !FStrEq( pMarker->GetObjectiveName(), m_pObjective->GetEntityName() ) )
		{
			continue;
		}

		bAnyMarkers = true;

		if ( !pMarker->IsEnabled() )
		{
			continue;
		}

		AddMapMark( pMarker, pMarker->IsComplete() );
	}

	if ( !bAnyMarkers && m_pObjective->m_LegacyMapMarkings != NULL_STRING )
	{
		AddMapMark( m_pObjective, false );
	}

	if ( m_iNumMapMarks > 0 )
	{
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, -1 /*SOUND_FROM_LOCAL_PLAYER*/, "ASWInterface.AreaBrackets" );
	}
}

void ObjectiveMap::Paint()
{
	BaseClass::Paint();

	if ( ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME )
	{
		PaintMarineBlips();
	}

	PaintExtraBlips();
}

void ObjectiveMap::GetDesiredMapBounds(int &mx, int &my, int &mw, int &mt)
{
	// get the size of our panel
	int w = GetWide();
	int t = GetTall();

	// ideally we'd be square, but if we're shorter on the tall then we have to scale the map down by that much
	float fMapScale = float(t) / float(w);	
	fMapScale *= 0.95f;	// inset it slightly

	// size the map to the calculated scale, then centre it
	mw = mt = w * fMapScale;
	mx = (w * 0.5f) - (mw * 0.5f);
	my = (t * 0.5f) - (mt * 0.5f);
}

ObjectiveMapDrawingPanel::ObjectiveMapDrawingPanel(Panel *parent, const char*name) : Panel(parent, name)
{
	m_bDrawingMapLines = false;
	m_fLastMapLine = 0;
	m_iMouseX = 0;
	m_iMouseY = 0;
}

void ObjectiveMapDrawingPanel::Paint()
{
	CASWHudMinimap *pMinimap = GET_HUDELEMENT( CASWHudMinimap );		
	if (!pMinimap)
		return;
	// paint a black outline over the lines
	for (int i=0;i<pMinimap->m_MapLines.Count();i++)
	{		
		float x,y;
		if (!pMinimap->m_MapLines[i].bSetBlipCentre)
		{
			Vector vecBlipPos;
			vecBlipPos.x = pMinimap->m_MapLines[i].worldpos.x;
			vecBlipPos.y = pMinimap->m_MapLines[i].worldpos.y;
			vecBlipPos.z = 0;
			//Msg("drawing line with blippos %f, %f\n", vecBlipPos.x, vecBlipPos.y);
			pMinimap->m_MapLines[i].blipcentre = pMinimap->WorldToMapTexture(vecBlipPos);
			//Msg("  which is blipcentre=%f, %f\n", pMinimap->m_MapLines[i].blipcentre.x, pMinimap->m_MapLines[i].blipcentre.y);
			pMinimap->m_MapLines[i].bSetBlipCentre = true;
		}
		Vector2D vecBlipCentre = pMinimap->m_MapLines[i].blipcentre;
		TextureToLinePanel(pMinimap, vecBlipCentre, x, y);
		if (pMinimap->m_MapLines[i].bLink)
		{
			bool bFound = false;
			if (i>1)
			{
				for (int k=i-1; k>0; k--)	// find the previous line from this player, if any
				{
					if (pMinimap->m_MapLines[i].player_index == pMinimap->m_MapLines[k].player_index)
					{
						pMinimap->m_MapLines[i].linkpos = pMinimap->m_MapLines[k].worldpos;
						bFound = true;
						break;
					}
				}				
			}
			if (bFound)
			{				
				float x2,y2;
				if (!pMinimap->m_MapLines[i].bSetLinkBlipCentre)
				{
					Vector vecBlipPos2;
					vecBlipPos2.x = pMinimap->m_MapLines[i].linkpos.x;
					vecBlipPos2.y = pMinimap->m_MapLines[i].linkpos.y;
					vecBlipPos2.z = 0;
					pMinimap->m_MapLines[i].linkblipcentre = pMinimap->WorldToMapTexture(vecBlipPos2);
					pMinimap->m_MapLines[i].bSetLinkBlipCentre = true;
				}
				Vector2D vecBlipCentre2 = pMinimap->m_MapLines[i].linkblipcentre;
				TextureToLinePanel(pMinimap, vecBlipCentre2, x2, y2);
				float t = gpGlobals->curtime - pMinimap->m_MapLines[i].created_time;
				int alpha = 255;
				if (t < MAP_LINE_SOLID_TIME)
				{
					
				}
				else if (t < MAP_LINE_SOLID_TIME + MAP_LINE_FADE_TIME)
				{
					alpha = 255 - ((t - MAP_LINE_SOLID_TIME) / MAP_LINE_FADE_TIME) * 255.0f;
				}
				else
				{
					continue;
				}
				//Msg("drawing line from %f,%f to %f,%f\n", x, y, x2, y2);

				vgui::surface()->DrawSetTexture(pMinimap->m_nWhiteTexture);
				vgui::Vertex_t start, end;

				// draw black outline around the line to give it some softness	
				vgui::surface()->DrawSetColor(Color(0,0,0, alpha));

				start.Init(Vector2D(x - 1.50f,y - 1.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 - 1.50f,y2 - 1.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				start.Init(Vector2D(x + 1.50f,y - 1.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 + 1.50f,y2 - 1.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				start.Init(Vector2D(x - 1.50f,y + 1.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 - 1.50f,y2 + 1.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				start.Init(Vector2D(x + 1.50f,y + 1.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 + 1.50f,y2 + 1.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);
			}
		}
	}

	// paint map line dots	
	for (int i=0;i<pMinimap->m_MapLines.Count();i++)
	{		
		//pMinimap->PaintWorldBlip(vecBlipPos, 0.5f, Color(0,255,0,255));
		float x,y;
		if (!pMinimap->m_MapLines[i].bSetBlipCentre)
		{
			Vector vecBlipPos;
			vecBlipPos.x = pMinimap->m_MapLines[i].worldpos.x;
			vecBlipPos.y = pMinimap->m_MapLines[i].worldpos.y;
			vecBlipPos.z = 0;
			pMinimap->m_MapLines[i].blipcentre = pMinimap->WorldToMapTexture(vecBlipPos);
			pMinimap->m_MapLines[i].bSetBlipCentre = true;
		}
		Vector2D vecBlipCentre = pMinimap->m_MapLines[i].blipcentre;
		TextureToLinePanel(pMinimap, vecBlipCentre, x, y);
		if (pMinimap->m_MapLines[i].bLink)
		{
			bool bFound = false;
			if (i>1)
			{
				for (int k=i-1; k>0; k--)	// find the previous line from this player, if any
				{
					if (pMinimap->m_MapLines[i].player_index == pMinimap->m_MapLines[k].player_index)
					{
						pMinimap->m_MapLines[i].linkpos = pMinimap->m_MapLines[k].worldpos;
						bFound = true;
						break;
					}
				}				
			}
			if (bFound)
			{
				
				float x2,y2;
				if (!pMinimap->m_MapLines[i].bSetLinkBlipCentre)
				{
					Vector vecBlipPos2;
					vecBlipPos2.x = pMinimap->m_MapLines[i].linkpos.x;
					vecBlipPos2.y = pMinimap->m_MapLines[i].linkpos.y;
					vecBlipPos2.z = 0;
					pMinimap->m_MapLines[i].linkblipcentre = pMinimap->WorldToMapTexture(vecBlipPos2);
					pMinimap->m_MapLines[i].bSetLinkBlipCentre = true;
				}
				Vector2D vecBlipCentre2 = pMinimap->m_MapLines[i].linkblipcentre;
				TextureToLinePanel(pMinimap, vecBlipCentre2, x2, y2);
				float t = gpGlobals->curtime - pMinimap->m_MapLines[i].created_time;
				int alpha = 255;
				if (t < MAP_LINE_SOLID_TIME)
				{
					//surface()->DrawSetColor(Color(255,255,255,255));
					//surface()->DrawLine(x,y,x2,y2);
				}
				else if (t < MAP_LINE_SOLID_TIME + MAP_LINE_FADE_TIME)
				{
					alpha = 255 - ((t - MAP_LINE_SOLID_TIME) / MAP_LINE_FADE_TIME) * 255.0f;
				}
				else
				{
					continue;
				}

				vgui::surface()->DrawSetTexture(pMinimap->m_nWhiteTexture);
				vgui::Vertex_t start, end;		

				// draw main line
				vgui::surface()->DrawSetColor(GetColorPerIndex(pMinimap->m_MapLines[i].player_index));

				//vgui::surface()->DrawTexturedRect(x, y, x2, y2);
				//surface()->DrawLine(x,y,x2,y2);
				start.Init(Vector2D(x,y), Vector2D(0,0));
				end.Init(Vector2D(x2,y2), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				// draw translucent ones around it to give it some softness	
				vgui::surface()->DrawSetColor(GetColorPerIndex(pMinimap->m_MapLines[i].player_index));

				start.Init(Vector2D(x - 0.50f,y - 0.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 - 0.50f,y2 - 0.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				start.Init(Vector2D(x + 0.50f,y - 0.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 + 0.50f,y2 - 0.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				start.Init(Vector2D(x - 0.50f,y + 0.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 - 0.50f,y2 + 0.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);

				start.Init(Vector2D(x + 0.50f,y + 0.50f), Vector2D(0,0));
				end.Init(Vector2D(x2 + 0.50f,y2 + 0.50f), Vector2D(1,1));
				SoftLine::DrawPolygonLine(start, end);
			}
		}
	}
}

// input blip_centre is from 0 to 1023, convert it to width/height of this panel
void ObjectiveMapDrawingPanel::TextureToLinePanel(CASWHudMinimap* pMap, const Vector2D &blip_centre, float &x, float &y)
{		
	x = (blip_centre.x / 1024.0f) * float(GetWide());
	y = (blip_centre.y / 1024.0f) * float(GetTall());

	//int sx, sy;
	//sx = sy = 0;
	//LocalToScreen(sx, sy);
	//x += sx;
	//y += sy;
}

// drawing a map line at point x and y on the panel
void ObjectiveMapDrawingPanel::SendMapLine(int x, int y, bool bInitial)
{
	if ( !rd_hud_minimap_drawing.GetBool() )
		return;

	CASWHudMinimap *pMinimap = GET_HUDELEMENT( CASWHudMinimap );		
	if (!pMinimap)
		return;

	C_ASW_Player *local = C_ASW_Player::GetLocalASWPlayer();
	if ( local )
	{
		int wide, tall;
		GetSize(wide,tall);
		
		Vector vecMapCentre = pMinimap->m_MapOrigin;
								
		// pixel difference between where we clicked and the centre of the panel
		/*
		int diff_x = x - (GetWide() * 0.5f);
		int diff_y = (GetTall() * 0.5f - y);

		// find the world position we clicked on
		Vector vecMapLinePos;
		vecMapLinePos.x = vecMapCentre.x + (diff_x * pMinimap->m_fMapScale);
		vecMapLinePos.y = vecMapCentre.y + (diff_y * pMinimap->m_fMapScale);		
		vecMapLinePos.z = local->GetAbsOrigin().z;
		*/

		Vector2D offset;

		// convert from panel to texture
		offset.x = (x * 1024.0f) / float(GetWide());
		offset.y = (y * 1024.0f) / float(GetTall());

		// convert from texture to world
		offset.x += 128;
		offset.y *= -pMinimap->m_fMapScale;
		offset.x *= pMinimap->m_fMapScale;

		Vector vecMapLinePos;
		vecMapLinePos.x = offset.x + pMinimap->m_MapOrigin.x;
		vecMapLinePos.y = offset.y + pMinimap->m_MapOrigin.y;
		vecMapLinePos.z = local->GetAbsOrigin().z;

		//Msg("mapcentre.x=%f scale=%f\n", vecMapCentre.x, pMinimap->m_fMapScale);

		// notify the server of this!
		char buffer[64];
		int linetype = bInitial ? 0 : 1;
		Q_snprintf(buffer, sizeof(buffer), "cl_mapline %d %d %d", linetype, (int) vecMapLinePos.x, (int) vecMapLinePos.y);
		engine->ClientCmd(buffer);
		//Msg("%s\n", buffer);

		m_fLastMapLine = gpGlobals->realtime;

		// short circuit add it to your own list
		MapLine line;
		line.player_index = local->entindex();
		line.worldpos.x = (int) vecMapLinePos.x;
		line.worldpos.y = (int) vecMapLinePos.y;
		line.created_time = gpGlobals->curtime;
		if (linetype == 1)		// links to a previous
		{
			line.bLink = true;
		}
		else
		{
			if (gpGlobals->curtime > pMinimap->m_fLastMinimapDrawSound + 5.0f)
			{
				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound( filter, -1, "ASWScanner.Drawing" );
				pMinimap->m_fLastMinimapDrawSound = gpGlobals->curtime;
			}
		}
		pMinimap->m_MapLines.AddToTail(line);
	}
}

void ObjectiveMapDrawingPanel::OnMousePressed(vgui::MouseCode code)
{	
	if ( code != MOUSE_LEFT )
		return;

	SendMapLine(m_iMouseX,m_iMouseY,true);
	m_bDrawingMapLines = true;
}

void ObjectiveMapDrawingPanel::OnMouseReleased(vgui::MouseCode code)
{
	if ( code != MOUSE_LEFT )
		return;

	m_bDrawingMapLines = false;
}

void ObjectiveMapDrawingPanel::OnCursorExited()
{
	//Msg("ObjectiveMapDrawingPanel::OnCursorExited\n");
	m_bDrawingMapLines = false;
}

void ObjectiveMapDrawingPanel::OnThink()
{

}

void ObjectiveMapDrawingPanel::OnCursorMoved( int x, int y )
{
	//Msg("ObjectiveMapDrawingPanel::OnCursorMoved %d,%d\n", x, y);
	m_iMouseX = x;
	m_iMouseY = y;
	C_ASW_Player *local = C_ASW_Player::GetLocalASWPlayer();
	if ( local )
	{
		if (m_bDrawingMapLines && gpGlobals->realtime >= m_fLastMapLine + MAP_LINE_INTERVAL)
		{
			SendMapLine(x,y,false);
		}
	}
}