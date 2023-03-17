#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "vguimatsurface/imatsystemsurface.h"
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>

using namespace vgui;

#include "asw_hudelement.h"
#include "hud_numericdisplay.h"
#include "asw_marine_profile.h"
#include "asw_alien_classes.h"
#include "c_asw_game_resource.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_playerresource.h"
#include "c_asw_weapon.h"
#include "c_asw_computer_area.h"
#include "c_asw_button_area.h"

#include "tier0/vprof.h"
#include "idebugoverlaypanel.h"
#include "engine/IVDebugOverlay.h"
#include "asw_gamerules.h"
#include "asw_marine_skills.h"
#include "c_asw_ammo_drop.h"
#include "nb_vote_panel.h"
#include "asw_hud_master.h"
#include "asw_input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar asw_draw_hud;
extern ConVar asw_hud_alpha;
//extern ConVar asw_simple_hacking;

ConVar asw_paint_ammo_bar("asw_paint_ammo_bar", "1", FCVAR_NONE, "Set to 0 to do not paint bars under ammo drops");

#define ASW_SPECTATING_BAR_HEIGHT 0.06f

// shows various messages overlayed on the screen (e.g. INFESTED, POISONED, etc.)
class CASWHudOverlayMessages : public CASW_HudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CASWHudOverlayMessages, CHudNumericDisplay );

public:
	CASWHudOverlayMessages( const char *pElementName );
	~CASWHudOverlayMessages();
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual void Paint();
	void PaintOverlays();
	void PaintSpectatingOverlay();
	void PaintStimStatus( int &yPos );
	void PaintAmmoDrops();
	bool PaintDeathMessage();
	//virtual void PaintHackingProgress( int &yPos );
	virtual void DrawMessage(int row, const char *szToken, int r, int g, int b);
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void PerformLayout();
	virtual bool ShouldDraw( void ) { return asw_draw_hud.GetBool() && CASW_HudElement::ShouldDraw(); }
	void UpdateSpectatingLabel();
	bool PaintGenericBar( int iTextureID, float xPos, float yPos, int width, int height, float flProgress, Color BgColor );

	vgui::Label *m_pSpectatingLabel;
	CHandle<C_ASW_Inhabitable_NPC> m_hLastSpectatingNPC;
	CHandle<C_ASW_Player> m_hLastSpectatingPlayer;
	CNB_Vote_Panel *m_pVotePanel;

	CPanelAnimationVar( vgui::HFont, m_hOverlayFont, "OverlayFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hSmallFont, "DefaultMedium", "DefaultMedium" );	
	CPanelAnimationVarAliasType( int, m_nBlackBarTexture, "BlackBarTexture", "vgui/swarm/HUD/ASWHUDBlackBar", "textureid" );
	CPanelAnimationVarAliasType( int, m_nWhiteTexture, "WhiteTexture", "vgui/white", "textureid" );
	CPanelAnimationVarAliasType( int, m_nVertAmmoBar, "VertAmmoBarTexture", "vgui/swarm/HUD/VertAmmoBar", "textureid" );

	float m_flDeathMessageStartTime;
};	

DECLARE_HUDELEMENT( CASWHudOverlayMessages );

CASWHudOverlayMessages::CASWHudOverlayMessages( const char *pElementName ) : CASW_HudElement( pElementName ), CHudNumericDisplay(NULL, "ASWHudOverlayMessages")
{
	//SetHiddenBits( HIDEHUD_PLAYERDEAD );
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SwarmSchemeNew.res", "SwarmSchemeNew");
	SetScheme(scheme);

	m_pSpectatingLabel = new vgui::Label( this, "SpectatingLabel", "" );
	m_hLastSpectatingNPC = NULL;
	m_hLastSpectatingPlayer = NULL;

	m_pVotePanel = new CNB_Vote_Panel( this, "VotePanel" );
	m_pVotePanel->m_VotePanelType = VPT_INGAME;

	m_flDeathMessageStartTime = 0.0f;
}

CASWHudOverlayMessages::~CASWHudOverlayMessages()
{

}

void CASWHudOverlayMessages::Init()
{
	Reset();
}

void CASWHudOverlayMessages::Reset()
{
	SetLabelText(L" ");

	SetShouldDisplayValue(false);
	SetPaintBackgroundEnabled(false);
}

void CASWHudOverlayMessages::VidInit()
{
	Reset();
}


void CASWHudOverlayMessages::OnThink()
{
	VPROF_BUDGET( "CASWHudOverlayMessages::OnThink", VPROF_BUDGETGROUP_ASW_CLIENT );
	SetPaintEnabled( asw_draw_hud.GetBool() && ASWGameRules() && ASWGameRules()->GetGameState() >= ASW_GS_LAUNCHING );

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	C_ASW_Inhabitable_NPC *pSpectatingNPC = pPlayer ? pPlayer->GetSpectatingNPC() : NULL;
	C_ASW_Player *pSpectatingPlayer = pSpectatingNPC && pSpectatingNPC->IsInhabited() ? pSpectatingNPC->GetCommander() : NULL;

	m_pSpectatingLabel->SetVisible( pSpectatingNPC != NULL );
	if ( pSpectatingNPC && ( pSpectatingNPC != m_hLastSpectatingNPC.Get() || pSpectatingPlayer != m_hLastSpectatingPlayer.Get() ) )
	{
		UpdateSpectatingLabel();
		m_hLastSpectatingNPC = pSpectatingNPC;
		m_hLastSpectatingPlayer = pSpectatingPlayer;
	}
}

void CASWHudOverlayMessages::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(Color(0,0,0,0));
	m_pSpectatingLabel->SetFgColor(Color(255,255,255,255));
	m_pSpectatingLabel->SetContentAlignment(vgui::Label::a_northwest);
}

void CASWHudOverlayMessages::PerformLayout()
{
	BaseClass::PerformLayout();
	int iBorder = ((float) ScreenHeight() / 768.0f) * 8;
	int iBarHeight = ScreenHeight() * ASW_SPECTATING_BAR_HEIGHT;
	m_pSpectatingLabel->SetBounds(iBorder, iBorder, ScreenWidth() - (iBorder * 2), iBarHeight - iBorder*2 );
}

void CASWHudOverlayMessages::Paint()
{
	VPROF_BUDGET( "CASWHudOverlayMessages::Paint", VPROF_BUDGETGROUP_ASW_CLIENT );
	BaseClass::Paint();
	PaintOverlays();
	PaintSpectatingOverlay();

	int yPos = YRES( 80 );
	PaintStimStatus( yPos );
	//PaintHackingProgress( yPos );
	if (asw_paint_ammo_bar.GetBool())
		PaintAmmoDrops();
}

void CASWHudOverlayMessages::PaintAmmoDrops()
{
	Vector vecOffset( 0, -20, 0 );
	if ( ASWInput() )
	{
		VectorRotate( Vector( -20, 0, 0 ), QAngle( 0, ASWInput()->ASW_GetCameraYaw(), 0 ), vecOffset );
	}

	int nDrops = g_AmmoDrops.Count();
	for ( int i = 0; i < nDrops; i++ )
	{
		C_ASW_Ammo_Drop *pDrop = g_AmmoDrops[i];
		if ( !pDrop )
			continue;
		
		C_ASW_Player *pLocal = C_ASW_Player::GetLocalASWPlayer();
		if ( pLocal )
			if ( pDrop->GetAbsOrigin().DistTo( pLocal->EyePosition() ) > 1024 )
				continue;
		
		Vector vecWorldPos = pDrop->GetRenderOrigin() + vecOffset;

		const int nMaxX = ScreenWidth() - 150;
		const int nMaxY = ScreenHeight() - 100;
		Vector screenPos;
		debugoverlay->ScreenPosition( vecWorldPos, screenPos );
		if ( ( screenPos.x  >= 0 ) && ( screenPos.x <= nMaxX ) &&
			( screenPos.y  >= 0 ) && ( screenPos.y <= nMaxY ) )
		{
			float flProgress = (float) pDrop->GetAmmoUnitsRemaining() / 100.0f;
			PaintGenericBar( m_nVertAmmoBar, screenPos.x - YRES( 18 ), screenPos.y, YRES( 36 ), YRES( 5 ), flProgress, Color( 0, 0, 0, 255 ) );
		}
	}
}

void CASWHudOverlayMessages::UpdateSpectatingLabel()
{
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer )
		return;

	C_ASW_Inhabitable_NPC *pNPC = pPlayer->GetSpectatingNPC();
	if ( !pNPC )
		return;

	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pNPC );
	CASW_Marine_Profile *pProfile = pMarine ? pMarine->GetMarineProfile() : NULL;
	const char *szShortName = pProfile ? pProfile->GetShortName() : GetAlienClassname( pNPC );

	wchar_t wbuffer[128];
	if ( pNPC->GetCommander() && pNPC->IsInhabited() )
	{
		wchar_t wplayer[34];
		g_pVGuiLocalize->ConvertANSIToUnicode( pNPC->GetCommander()->GetPlayerName(), wplayer, sizeof( wplayer ) );	
		g_pVGuiLocalize->ConstructString( wbuffer, sizeof( wbuffer ),
			g_pVGuiLocalize->Find( "#asw_spectating_player" ), 2,
			g_pVGuiLocalize->FindSafe( szShortName ), wplayer);
	}
	else
	{
		g_pVGuiLocalize->ConstructString( wbuffer, sizeof( wbuffer ),
			g_pVGuiLocalize->Find( "#asw_spectating_marine" ), 1,
			g_pVGuiLocalize->FindSafe( szShortName ) );
	}
	
	m_pSpectatingLabel->SetText(wbuffer);
}

void CASWHudOverlayMessages::PaintSpectatingOverlay()
{
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer )
		return;

	if ( !pPlayer->GetSpectatingNPC() )
		return;

	// draw black bar
	int iBarHeight = ScreenHeight() * ASW_SPECTATING_BAR_HEIGHT;
	vgui::surface()->DrawSetColor( Color(255,255,255, asw_hud_alpha.GetInt()) );
	vgui::surface()->DrawSetTexture( m_nBlackBarTexture );
	vgui::surface()->DrawTexturedRect( 0, 0, ScreenWidth() + 1, iBarHeight);
}

void CASWHudOverlayMessages::PaintOverlays()
{
	C_ASW_Player *local = C_ASW_Player::GetLocalASWPlayer();
	if (!local)
		return;

	if ( PaintDeathMessage() )
	{
		return;
	}
	else
	{
		m_flDeathMessageStartTime = 0.0f;
	}

	C_ASW_Marine *marine = C_ASW_Marine::AsMarine( local->GetViewNPC() );
	if ( !marine || !marine->GetMarineResource() )
		return;

	const bool bReloading = marine->GetActiveASWWeapon() && marine->GetActiveASWWeapon()->IsReloading();
	const bool bInfested = marine->IsInfested();
	const bool bPoisoned = marine->m_fPoison > 0;
	const bool bLowAmmo = marine->GetMarineResource()->IsLowAmmo();
	const bool bFFGuard = marine->m_fFFGuardTime > gpGlobals->curtime;

	int row = 0;

	if (!bReloading && !bInfested && !bPoisoned && !bLowAmmo && !bFFGuard)
		return;
	
	float f=(gpGlobals->curtime - int(gpGlobals->curtime) );
	if (f < 0) f = -f;
	if (f<=0.25 || (f>0.5 && f<=0.75))
	{		
		if (bInfested)
		{
			DrawMessage(row, "#asw_alert_infested", 0, 255, 0);
			row++;
		}
		if (false && bPoisoned)
		{
			DrawMessage(row, "#asw_alert_poisoned", 0, 255, 0);
			row++;
		}
		if (bReloading)
		{
			DrawMessage(row, "#asw_alert_reloading", 255, 0, 0);
			row++;
		}
		if (bFFGuard)
		{
			DrawMessage(row, "#asw_alert_ffguard", 255, 0, 0);
			row++;
		}
		else if (false && bLowAmmo)
		{
			DrawMessage(row, "#asw_low_ammo", 255, 255, 255);
			row++;
		}
	}	
}

void CASWHudOverlayMessages::DrawMessage(int row, const char *szToken, int r, int g, int b)
{
	const wchar_t *pwszMessage = g_pVGuiLocalize->Find( szToken );
	wchar_t wszMessageMissing[64];
	if ( !pwszMessage )
	{
		V_UTF8ToUnicode( szToken, wszMessageMissing, sizeof( wszMessageMissing ) );
		pwszMessage = wszMessageMissing;
	}

	int len = V_wcslen( pwszMessage );

	float xPos = ScreenWidth() * 0.5f;
	float yPos = ScreenHeight() * 0.2f;

	int wide, tall;
	g_pMatSystemSurface->GetTextSize( m_hOverlayFont, pwszMessage, wide, tall );

	// centre it on the x
	xPos -= (0.5f * wide);
	// count down rows
	yPos += tall * 1.5f * row;
	// drop shadow
	g_pMatSystemSurface->DrawSetTextColor( 0, 0, 0, 200 );
	g_pMatSystemSurface->DrawSetTextPos( xPos + 1, yPos + 1 );
	g_pMatSystemSurface->DrawSetTextFont( m_hOverlayFont );
	g_pMatSystemSurface->DrawPrintText( pwszMessage, len );
	// actual text
	g_pMatSystemSurface->DrawSetTextColor( r, g, b, 200 );
	g_pMatSystemSurface->DrawSetTextPos( xPos, yPos );
	g_pMatSystemSurface->DrawPrintText( pwszMessage, len );
}

bool CASWHudOverlayMessages::PaintGenericBar( int iTextureID, float xPos, float yPos, int width, int height, float flProgress, Color BgColor )
{
	vgui::surface()->DrawSetColor( BgColor );
	// draw black back
	vgui::surface()->DrawSetTexture( m_nWhiteTexture );

	// draw health bar
	int bar_y = yPos;
	int bar_y2 = bar_y + height;
	if ( flProgress <= 0 )
		return false;
	int using_pixels = (width * flProgress);

	if (flProgress > 0 && using_pixels <= 0)
		using_pixels = 1;
	int bar_x2 = xPos + using_pixels;
	int border = YRES( 1 );
	// black part first
	vgui::surface()->DrawTexturedRect( xPos - border, bar_y - border, xPos + width + border, bar_y2 + border );

	// red part
	vgui::surface()->DrawSetTexture( iTextureID );
	vgui::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
	vgui::Vertex_t hpoints[4] = 
	{ 
		vgui::Vertex_t( Vector2D(xPos, bar_y),	Vector2D(0,0) ), 
		vgui::Vertex_t( Vector2D(bar_x2, bar_y),		Vector2D(flProgress,0) ), 
		vgui::Vertex_t( Vector2D(bar_x2, bar_y2),		Vector2D(flProgress,1) ), 
		vgui::Vertex_t( Vector2D(xPos, bar_y2),	Vector2D(0,1) )
	}; 
	vgui::surface()->DrawTexturedPolygon( 4, hpoints );

	return true;
}

void CASWHudOverlayMessages::PaintStimStatus( int &yPos )
{
	if ( !ASWGameRules() )
		return;

	float flStimEndTime = ASWGameRules()->GetStimEndTime();
	if ( flStimEndTime > gpGlobals->curtime )
	{
		float flStartStimTime = ASWGameRules()->GetStartStimTime();

		C_BasePlayer *pPlayer = dynamic_cast<C_BasePlayer*>( ASWGameRules()->GetStimSource() );
		if ( !pPlayer )
		{
			return;
		}

		wchar_t wszMarineName[ 32 ];
		g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName() , wszMarineName, sizeof( wszMarineName ) );

		wchar_t wszMarineStimMessage[ 64 ];
		g_pVGuiLocalize->ConstructString( wszMarineStimMessage, sizeof( wszMarineStimMessage ), g_pVGuiLocalize->Find( "#asw_hud_adrenaline" ), 1, wszMarineName );

		int nLineHeight, nTextWidth;
		vgui::surface()->GetTextSize( m_hOverlayFont, wszMarineStimMessage, nTextWidth, nLineHeight );

		int nLeftAlign = ( ScreenWidth() - nTextWidth ) / 2 ;

		vgui::surface()->DrawSetTextFont( m_hOverlayFont );

		// drop shadow
		vgui::surface()->DrawSetTextColor( 0, 0, 0, 200 );
		vgui::surface()->DrawSetTextPos( nLeftAlign+1, yPos+1 );
		vgui::surface()->DrawPrintText( wszMarineStimMessage, V_wcslen( wszMarineStimMessage ) );

		// actual text
		vgui::surface()->DrawSetTextColor( 66, 142, 192, 200 );
		vgui::surface()->DrawSetTextPos( nLeftAlign, yPos );
		vgui::surface()->DrawPrintText( wszMarineStimMessage, V_wcslen( wszMarineStimMessage ) );

		yPos += nLineHeight + ( nLineHeight >> 2 );

		float flTimeLeft = flStimEndTime - gpGlobals->curtime;
		float flStimProgress = flTimeLeft / ( flStimEndTime - flStartStimTime );
		PaintGenericBar( m_nVertAmmoBar, nLeftAlign, yPos, nTextWidth, YRES( 10 ), flStimProgress, Color( 0, 0, 0, 255 ) );

		yPos += YRES( 15 );
	}
}

bool CASWHudOverlayMessages::PaintDeathMessage()
{
	C_AlienSwarm *pGameRules = ASWGameRules();
	if ( !pGameRules )
		return false;

	bool bDeathmatchVictory = pGameRules->m_szDeathmatchWinnerName[0] != '\0';
	if ( !bDeathmatchVictory && pGameRules->GetMarineDeathCamInterp( true ) <= 0.0f )
		return false;

	C_ASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
		return false;

	C_ASW_Marine_Resource *pMR = pGameResource->GetMarineResource( pGameRules->m_nMarineForDeathCam );
	if ( !bDeathmatchVictory && !pMR )
		return false;
		
	CASW_Marine_Profile *pProfile = pMR ? pMR->GetProfile() : NULL;
	if ( !bDeathmatchVictory && !pProfile )
		return false;

	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pLocalPlayer )
		return false;

	if ( m_flDeathMessageStartTime == 0 )
	{
		m_flDeathMessageStartTime = Plat_FloatTime();
	}

	const float flFadeInTime = 1.8f;
	float flFadeAmount = ( Plat_FloatTime() - m_flDeathMessageStartTime ) / flFadeInTime;
	flFadeAmount = clamp<float>( flFadeAmount, 0.0f, 1.0f );
	flFadeAmount *= flFadeAmount;
	flFadeAmount *= flFadeAmount;

	const float flSkullFadeInTime = 1.2f;
	float flSkullFadeAmount = ( Plat_FloatTime() - m_flDeathMessageStartTime ) / flSkullFadeInTime;
	flSkullFadeAmount = clamp<float>( flSkullFadeAmount, 0.0f, 1.0f );
	flSkullFadeAmount *= flSkullFadeAmount;
	flSkullFadeAmount *= flSkullFadeAmount;

	const char *szMarineName = pProfile ? pProfile->GetShortName() : "";
	if ( gpGlobals->maxClients > 1 && pMR && pMR->IsInhabited() && pLocalPlayer != pMR->GetCommander() )
	{
		szMarineName = g_PR->GetPlayerName( pMR->m_iCommanderIndex );
	}

	wchar_t wszMarineName[ 32 ];
	const wchar_t *pwchLocalizedMarineName = g_pVGuiLocalize->Find( szMarineName );

	if ( pwchLocalizedMarineName )
	{
		Q_wcsncpy( wszMarineName, pwchLocalizedMarineName, sizeof( wszMarineName ) );
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( szMarineName, wszMarineName, sizeof( wszMarineName ) );
	}

	wchar_t wszMarineDeathMessage[255];
	if ( bDeathmatchVictory )
	{
		V_UTF8ToUnicode( ASWGameRules()->m_szDeathmatchWinnerName, wszMarineName, sizeof( wszMarineName ) );
		g_pVGuiLocalize->ConstructString( wszMarineDeathMessage, sizeof( wszMarineDeathMessage ), g_pVGuiLocalize->Find( "#asw_player_won_deathmatch_short" ), 1, wszMarineName );
	}
	else if ( pLocalPlayer == pMR->GetCommander() && pMR->IsInhabited() && ( pLocalPlayer->GetNPC() == NULL || pLocalPlayer->GetNPC()->GetHealth() <= 0 ) )
	{
		V_snwprintf( wszMarineDeathMessage, ARRAYSIZE( wszMarineDeathMessage ), L"%s", g_pVGuiLocalize->Find( "#asw_hud_you_died" ) );
	}
	else
	{
		g_pVGuiLocalize->ConstructString( wszMarineDeathMessage, sizeof( wszMarineDeathMessage ), g_pVGuiLocalize->Find( "#asw_hud_died" ), 1, wszMarineName );
	}
	_wcsupr_s( wszMarineDeathMessage );		// upper case it

	int nMarineNameWidth = 0, nMarineNameHeight = 0;
	g_pMatSystemSurface->GetTextSize( m_hOverlayFont, wszMarineDeathMessage, nMarineNameWidth, nMarineNameHeight );

	int nSkullEndWidth = YRES( 15 );
	int nSkullEndHeight = YRES( 15 );
	int border = YRES( 4 );
	int gap_between_skull_and_text = YRES( 8 );
	int nTotalWidth = nMarineNameWidth + nSkullEndWidth + gap_between_skull_and_text + border;
	int nPosX = ( ScreenWidth() * 0.5f ) - ( nTotalWidth * 0.5f ) + nSkullEndWidth;	// x pos of the text
	int nPosY = YRES( 150 );

	// set skull pos
	int nSkullStartWidth = nSkullEndWidth * 3.2f;
	int nSkullStartHeight = nSkullEndHeight * 3.2f;
	int nSkullEndX = nPosX - ( gap_between_skull_and_text + nSkullEndWidth );
	int nSkullEndY = nPosY + ( nMarineNameHeight / 2 );
	int nSkullStartX = nSkullEndX;
	int nSkullStartY = nSkullEndY;
	float flX = (float) nSkullStartX + ( (float) nSkullEndX - (float) nSkullStartX ) * flSkullFadeAmount;
	float flY = (float) nSkullStartY + ( (float) nSkullEndY - (float) nSkullStartY ) * flSkullFadeAmount;
	float flWidth = (float) YRES( nSkullStartWidth ) + ( (float) YRES( nSkullEndWidth ) - (float) YRES( nSkullStartWidth ) ) * flSkullFadeAmount;
	float flHeight = (float) YRES( nSkullStartHeight ) + ( (float) YRES( nSkullEndHeight ) - (float) YRES( nSkullStartHeight ) ) * flSkullFadeAmount;
	flWidth *= 0.5f;
	flHeight *= 0.5f;

	// paint a box behind the text
	int box_x = nPosX - ( border + flWidth );
	int box_y = nPosY - border;
	int box_w = nMarineNameWidth + border * 2 + flWidth;
	int box_h = nMarineNameHeight + border * 2;
	DrawBox( box_x, box_y, box_w, box_h, Color( 0, 0, 0, 192 ), flFadeAmount, false );

	CASW_Hud_Master *pMaster = GET_HUDELEMENT( CASW_Hud_Master );
	if ( pMaster )
	{
		const HudSheetTexture_t *pDeadIcon = pMaster->GetDeadIconUVs();
		Vertex_t points[4] = 
		{ 
			Vertex_t( Vector2D(flX - flWidth, flY - flHeight), Vector2D( pDeadIcon->u, pDeadIcon->v ) ), 
			Vertex_t( Vector2D(flX + flWidth, flY - flHeight), Vector2D( pDeadIcon->s, pDeadIcon->v ) ), 
			Vertex_t( Vector2D(flX + flWidth, flY + flHeight), Vector2D( pDeadIcon->s, pDeadIcon->t ) ), 
			Vertex_t( Vector2D(flX - flWidth, flY + flHeight), Vector2D( pDeadIcon->u, pDeadIcon->t ) ) 
		}; 
		surface()->DrawSetTexture( pMaster->GetDeadIconTextureID() );
		surface()->DrawSetColor( Color( 255, 255, 255, 255.0f * flSkullFadeAmount ) );
		surface()->DrawTexturedPolygon( 4, points );
	}

	vgui::surface()->DrawSetTextFont( m_hOverlayFont );
	vgui::surface()->DrawSetTextColor( 0, 0, 0, 200 * flFadeAmount );
	vgui::surface()->DrawSetTextPos( nPosX + 1, nPosY + 1 );
	vgui::surface()->DrawPrintText( wszMarineDeathMessage, Q_wcslen( wszMarineDeathMessage ) );

	vgui::surface()->DrawSetTextColor( 169, 213, 255, 200 * flFadeAmount );
	vgui::surface()->DrawSetTextPos( nPosX, nPosY );
	vgui::surface()->DrawPrintText( wszMarineDeathMessage, Q_wcslen( wszMarineDeathMessage ) );

	return true;
}
