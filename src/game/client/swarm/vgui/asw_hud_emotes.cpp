#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "iclientmode.h"
#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

#include <filesystem.h>
#include <keyvalues.h>

#include "asw_hudelement.h"
#include "c_asw_marine.h"
#include "asw_marine_profile.h"
#include "c_asw_marine_resource.h"
#include "c_asw_sentry_base.h"
#include "c_asw_game_resource.h"
#include "c_asw_door.h"
#include "c_asw_use_area.h"
#include "asw_input.h"

#include "ConVar.h"
#include "tier0/vprof.h"
#include "idebugoverlaypanel.h"
#include "engine/IVDebugOverlay.h"
#include "vguimatsurface/imatsystemsurface.h"
#include "tier1/fmtstr.h"
#ifdef CLIENT_DLL
	#include <vector>
	#include <list>
	#include "c_basetempentity.h"
	#include "c_te_legacytempents.h"
	#include "tempent.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar asw_draw_hud;

#ifdef CLIENT_DLL
extern float TRACE_FADE_TIME;
extern ConVar cl_trace_player_opacity;
extern ConVar cl_trace_player_max_targets;
//extern ConVar cl_trace_player_line_thickness;
//extern ConVar cl_trace_player_border_thickness;
extern Color g_TraceColorArray[8];
extern std::vector<int> g_nTracePlayer2Color;
extern std::vector<int> g_nTraceColor2Player;
extern void RemoveInvalidTracePlayersAndColors();
extern void ToggleTraceColor(int playerIndex);
#endif

//-----------------------------------------------------------------------------
// Purpose: Shows the marines emote graphics
//-----------------------------------------------------------------------------
class CASWHudEmotes : public CASW_HudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CASWHudEmotes, vgui::Panel );

public:
	CASWHudEmotes( const char *pElementName );
	~CASWHudEmotes();

	virtual void Paint();
	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void PaintEmotes();
	virtual void PaintEmotesFor( C_ASW_Marine *pMarine );
	virtual void PaintEmote( C_BaseEntity *pEnt, float fTime, int iTexture, float fScale = 1.0f );
	virtual bool ShouldDraw( void ) { return asw_draw_hud.GetBool() && CASW_HudElement::ShouldDraw(); }

	virtual void PaintTraces();
	virtual void PaintTracesFor(C_ASW_Marine* pMarine, Color color);

	CPanelAnimationVarAliasType( int, m_nMedicTexture, "MedicEmoteTexture", "vgui/swarm/Emotes/EmoteMedic", "textureid" );
	CPanelAnimationVarAliasType( int, m_nAmmoTexture, "AmmoEmoteTexture", "vgui/swarm/Emotes/EmoteAmmo", "textureid" );
	CPanelAnimationVarAliasType( int, m_nSmileTexture, "SmileEmoteTexture", "vgui/swarm/Emotes/EmoteSmile", "textureid" );
	CPanelAnimationVarAliasType( int, m_nStopTexture, "StopEmoteTexture", "vgui/swarm/Emotes/EmoteStop", "textureid" );
	CPanelAnimationVarAliasType( int, m_nGoTexture, "GoEmoteTexture", "vgui/swarm/Emotes/EmoteGo", "textureid" );
	CPanelAnimationVarAliasType( int, m_nExclaimTexture, "ExclaimEmoteTexture", "vgui/swarm/Emotes/EmoteExclaim", "textureid" );
	CPanelAnimationVarAliasType( int, m_nAnimeTexture, "AnimeEmoteTexture", "vgui/swarm/Emotes/EmoteAnime", "textureid" );
	CPanelAnimationVarAliasType( int, m_nQuestionTexture, "QuestionTexture", "vgui/swarm/Emotes/EmoteQuestion", "textureid" );
	CPanelAnimationVarAliasType( int, m_nWrenchTexture, "WrenchTexture", "vgui/swarm/ClassIcons/EngineerIcon", "textureid" );
	CPanelAnimationVarAliasType( int, m_nSentryUpTexture, "SentryUpTexture", "vgui/swarm/ClassIcons/SentryBuild", "textureid" );
	CPanelAnimationVarAliasType( int, m_nSentryDnTexture, "SentryDnTexture", "vgui/swarm/ClassIcons/SentryDismantle", "textureid" );
	CPanelAnimationVarAliasType( int, m_nHackTexture, "HackTexture", "vgui/swarm/ClassIcons/HackIcon", "textureid" );
	CPanelAnimationVarAliasType( int, m_nWeldTexture, "WeldTexture", "vgui/swarm/ClassIcons/WeldIcon", "textureid" );
	CPanelAnimationVarAliasType( int, m_nReviveMarineTexture, "ReviveMarineTexture", "vgui/swarm/ClassIcons/revivemarine", "textureid" );
	CPanelAnimationVarAliasType(int, m_nTraceTexture, "TraceTexture", "vgui/arrow_right", "textureid");
};

DECLARE_HUDELEMENT( CASWHudEmotes );

CASWHudEmotes::CASWHudEmotes( const char *pElementName ) : CASW_HudElement( pElementName ), BaseClass( NULL, "ASWHudEmotes" )
{
	vgui::Panel *pParent = GetClientMode()->GetViewport();
	SetParent( pParent );
	SetHiddenBits( HIDEHUD_REMOTE_TURRET );

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );
}

CASWHudEmotes::~CASWHudEmotes()
{
}

void CASWHudEmotes::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetPaintBackgroundEnabled( false );
}

void CASWHudEmotes::Paint()
{
	VPROF_BUDGET( "CASWHudEmotes::Paint", VPROF_BUDGETGROUP_ASW_CLIENT );
	BaseClass::Paint();
	PaintEmotes();
	PaintTraces();
}

void CASWHudEmotes::PaintEmotes()
{
	C_ASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;

	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		C_ASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
		if (!pMR)
			continue;

		C_ASW_Marine *marine = pMR->GetMarineEntity();
		if ( !marine )
			continue;

		PaintEmotesFor(marine);
	}
}

void CASWHudEmotes::PaintEmotesFor( C_ASW_Marine *pMarine )
{
	if ( pMarine->m_iClientEmote & ( 1 << 0 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteMedicTime, m_nMedicTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 1 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteAmmoTime, m_nAmmoTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 2 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteSmileTime, m_nSmileTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 3 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteStopTime, m_nStopTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 4 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteGoTime, m_nGoTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 5 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteExclaimTime, m_nExclaimTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 6 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteAnimeSmileTime, m_nAnimeTexture );
	if ( pMarine->m_iClientEmote & ( 1 << 7 ) )
		PaintEmote( pMarine, pMarine->m_fEmoteQuestionTime, m_nQuestionTexture );

	bool bBuildingSentry = false;
	bool bWelding = false;

	// paint wrench over a sentry gun if a marine is setting one up within an engineer's aura
	C_BaseEntity *pUsing = pMarine->m_hUsingEntity.Get();
	if ( pUsing )
	{
		C_ASW_Sentry_Base *pSentry = dynamic_cast< C_ASW_Sentry_Base * >( pUsing );
		if ( pSentry )
		{
			if ( pSentry->IsAssembled() )
				PaintEmote( pMarine, 1.5f, m_nSentryDnTexture, 0.9f );
			else
				PaintEmote( pMarine, 1.5f, m_nSentryUpTexture, 0.9f );

			if ( pSentry->m_bSkillMarineHelping )
			{
				PaintEmote( pSentry, 1.0f, m_nWrenchTexture, 0.4f );
			}

			bBuildingSentry = true;
		}
	}

	// paint medic icon over a marine if he is reviving another marine
	if ( pUsing )
	{
		if ( pUsing->Classify() == CLASS_ASW_MARINE )
		{
			C_ASW_Marine *pOtherMarine = assert_cast< C_ASW_Marine * >( pUsing );
			if ( pOtherMarine->m_bKnockedOut )
				PaintEmote( pMarine, 1.5f, m_nReviveMarineTexture, 0.9f );
		}
	}

	// and a welding door
	if ( pMarine->GetMarineResource() )
	{
		C_ASW_Door *pDoor = pMarine->GetMarineResource()->m_hWeldingDoor.Get();
		if ( pDoor )
		{
			bWelding = true;
			if ( pDoor->m_bSkillMarineHelping )
				PaintEmote( pDoor, 1.0f, m_nWrenchTexture, 0.4f );

			PaintEmote( pMarine, 1.5f, m_nWeldTexture, 0.75f );
		}
	}

	// paint wrench over an engineer if he's boosting a sentry/weld
	if ( pMarine->GetMarineResource() )
	{
		if ( pMarine->GetMarineResource()->m_bUsingEngineeringAura && !bBuildingSentry && !bWelding )
		{
			PaintEmote( pMarine, 0.9f, m_nWrenchTexture, 0.4f );
		}
	}

	bool bDrawHackIcon = false;

	if ( pMarine->IsInhabited() )
	{
		if ( pMarine->IsHacking() )
			bDrawHackIcon = true;
	}
	else
	{
		if ( pUsing )
		{
			C_ASW_Use_Area *pArea = static_cast< C_ASW_Use_Area * >( pUsing );
			if ( pArea->Classify() == CLASS_ASW_BUTTON_PANEL )
				bDrawHackIcon = true;
		}
	}

	if ( bDrawHackIcon )
		PaintEmote( pMarine, 1.0f, m_nHackTexture, 0.75f );
}

void CASWHudEmotes::PaintEmote( C_BaseEntity *pEnt, float fTime, int iTexture, float fEmoteScale )
{
	//Msg("PaintEmote scale = %f\n", fEmoteScale);
	if ( fEmoteScale < 0 )
		fEmoteScale = 0;
	Vector screenPos;
	Vector vecFacing;
	AngleVectors( pEnt->GetRenderAngles(), &vecFacing );
	vecFacing *= 5;
	// BenLubar: Fix emotes being offset when the camera is rotated
	float flYaw = ( ASWInput() ? ASWInput()->ASW_GetCameraYaw() : 90 ) / 180 * M_PI;
	Vector vecOffset( cosf( flYaw ) * 40, sinf( flYaw ) * 40, 70 );
	if ( !debugoverlay->ScreenPosition( pEnt->GetRenderOrigin() + vecOffset + vecFacing, screenPos ) )
	{
		//Msg("Emote marinepos: %s\n", VecToString(pEnt->GetRenderOrigin()));
		//Msg("Emote marinefacing: %s\n", VecToString(vecFacing));

		float xPos = screenPos[0];
		float yPos = screenPos[1];

		if ( iTexture != -1 )
		{
			float fAlpha = 0;
			float fSize = 0.5 + 0.5 * ( ( 2.0 - fTime ) / 2.0f );
			if ( fTime < 1.0f )		// fade in
				fAlpha = fTime;
			else						// fade out
				fAlpha = 2.0 - fTime;
			float f = clamp( 1.0f - fAlpha, 0.0f, 1.0f );
			f = f * f * f;
			fAlpha = 1.0f - f;

			fSize = ( 0.5f + 0.5f * fAlpha ) * 0.5f;
			float fScale = ( ScreenHeight() / 768.0f ) * fEmoteScale;
			float HalfW = 128.0f * fScale * fSize * 0.5f;
			float HalfH = 128.0f * fScale * fSize * 0.5f;

			surface()->DrawSetColor( Color( 255, 255, 255, fAlpha * 255.0f ) );
			surface()->DrawSetTexture( iTexture );
			//int x = xPos - HalfW;
			//int y = yPos - HalfH;

			Vertex_t points[4] =
			{
			Vertex_t( Vector2D( xPos - HalfW, yPos - HalfH ), Vector2D( 0,0 ) ),
			Vertex_t( Vector2D( xPos + HalfW, yPos - HalfH ), Vector2D( 1,0 ) ),
			Vertex_t( Vector2D( xPos + HalfW, yPos + HalfH ), Vector2D( 1,1 ) ),
			Vertex_t( Vector2D( xPos - HalfW, yPos + HalfH ), Vector2D( 0,1 ) )
			};
			surface()->DrawTexturedPolygon( 4, points );

			//surface()->DrawTexturedRect(xPos - HalfW, yPos - HalfH,
										//xPos + HalfW, yPos + HalfH);
		}
	}
}

void CASWHudEmotes::PaintTraces()
{
	RemoveInvalidTracePlayersAndColors();
	C_ASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
		return;

	for (int i = 0; i < pGameResource->GetMaxMarineResources(); i++)
	{
		C_ASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if (!pMR)
			continue;

		C_ASW_Marine* marine = pMR->GetMarineEntity();
		if (!pMR->IsInhabited() || !marine || g_nTracePlayer2Color[pMR->GetCommanderIndex()] == 0)
			continue;

		PaintTracesFor(marine, g_TraceColorArray[g_nTracePlayer2Color[pMR->GetCommanderIndex()]]);
	}
}

void CASWHudEmotes::PaintTracesFor(C_ASW_Marine* pMarine, Color color)
{
	// Not sure if the engine is single threaded or not, meanwhile, the list is accessed form other places. Therefore, lock m_TraceLock, just in case we are accessing the list from multiple threads, preventing concurrent modification of m_lstTracePlayerMovementList. Can safely remove the lock if the engine is gauranteed to be single threaded.
	std::lock_guard<std::mutex> lock(pMarine->m_TraceLock);

	float fTimeRatio = 1.0 - gpGlobals->curtime / 3.0 + (int)(gpGlobals->curtime / 3.0); // current time
	color[3] = 255 * pow(cl_trace_player_opacity.GetFloat(), 2.2);
	int omx, omy;
	float xPos, yPos;
	float fScale = (ScreenHeight() / 768.0f);
	float HalfW = 16.0f * fScale;
	float HalfH = 16.0f * fScale;
	Vector vecPosition, vecPositionNext, screenPos, screenPosNext;
	Vector vecCameraFocus;
	QAngle cameraAngle;

	ASWInput()->ASW_GetCameraLocation(C_ASW_Player::GetLocalASWPlayer(), vecCameraFocus, cameraAngle, omx, omy, false);

	// draw trace lines
	surface()->DrawSetColor(color);
	for (int i = 0; i < pMarine->m_vecTraceInterpolated.size() - 2; i++)
	{
		Vector vecPosition = pMarine->m_vecTraceInterpolated[i].m_vecPosition;
		Vector vecPositionNext = pMarine->m_vecTraceInterpolated[i + 1].m_vecPosition;

		if (!debugoverlay->ScreenPosition(vecPosition, screenPos) && !debugoverlay->ScreenPosition(vecPositionNext, screenPosNext))
		{
			surface()->DrawLine(screenPos[0], screenPos[1], screenPosNext[0], screenPosNext[1]);
		}
	}

	// draw trace direction icons
	surface()->DrawSetColor(color);
	surface()->DrawSetTexture(m_nTraceTexture);
	for (auto iter = pMarine->m_vecTraceInterpolated.begin(); iter != pMarine->m_vecTraceInterpolated.end(); ++iter)
	{
		if (iter->m_flTimestamp < 0.0f || iter->m_flTimestamp + 1.0f > gpGlobals->curtime)
		{
			continue;
		}
		Vector vecPosition = iter->m_vecPosition;

		if (iter->m_flTraceTime <= 0)
		{
			continue; // no trace to draw
		}

		if (!debugoverlay->ScreenPosition(vecPosition, screenPos))
		{
			xPos = screenPos[0];
			yPos = screenPos[1];

			if (m_nTraceTexture != -1)
			{
				QAngle angFacing(0, -iter->m_flAngleDegree + cameraAngle.y - 90, 0);

				Vector vecCornerTL(-HalfW, -HalfH, 0);
				Vector vecCornerTR(HalfW, -HalfH, 0);
				Vector vecCornerBR(HalfW, HalfH, 0);
				Vector vecCornerBL(-HalfW, HalfH, 0);
				Vector vecCornerTL_rotated, vecCornerTR_rotated, vecCornerBR_rotated, vecCornerBL_rotated;

				// rotate it by our facing yaw
				VectorRotate(vecCornerTL, angFacing, vecCornerTL_rotated);
				VectorRotate(vecCornerTR, angFacing, vecCornerTR_rotated);
				VectorRotate(vecCornerBR, angFacing, vecCornerBR_rotated);
				VectorRotate(vecCornerBL, angFacing, vecCornerBL_rotated);

				Vertex_t points[4] =
				{
					Vertex_t(Vector2D(xPos + vecCornerTL_rotated.x, yPos + vecCornerTL_rotated.y),
					Vector2D(0,0)),
					Vertex_t(Vector2D(xPos + vecCornerTR_rotated.x, yPos + vecCornerTR_rotated.y),
					Vector2D(1,0)),
					Vertex_t(Vector2D(xPos + vecCornerBR_rotated.x, yPos + vecCornerBR_rotated.y),
					Vector2D(1,1)),
					Vertex_t(Vector2D(xPos + vecCornerBL_rotated.x, yPos + vecCornerBL_rotated.y),
					Vector2D(0,1))
				};

				surface()->DrawTexturedPolygon(4, points);
			}
		}
	}
}