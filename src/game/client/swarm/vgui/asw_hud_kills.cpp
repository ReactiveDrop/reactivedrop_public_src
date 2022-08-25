#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>
#include "asw_hudelement.h"
#include "c_asw_game_resource.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include "c_asw_alien.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_asw_game_resource.h"
#include "c_playerresource.h"
#include "asw_gamerules.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar asw_draw_hud;
ConVar asw_draw_kills( "asw_draw_kills", "0", FCVAR_NONE, "", true, 0, true, 1 );
ConVar asw_player_lb_color_r("asw_player_lb_color_r", "255", FCVAR_ARCHIVE, "The r value of the player name color in the range of 0 to 255", TRUE, 0, TRUE, 255);
ConVar asw_player_lb_color_g("asw_player_lb_color_g", "255", FCVAR_ARCHIVE, "The g value of the player name color in the range of 0 to 255", TRUE, 0, TRUE, 255);
ConVar asw_player_lb_color_b("asw_player_lb_color_b", "0", FCVAR_ARCHIVE, "The b value of the player name color in the range of 0 to 255", TRUE, 0, TRUE, 255);

struct LeaderboardEntry
{
	int score;
	bool isPlayer;
	char name[MAX_PLAYER_NAME_LENGTH];
};

class CASWHudKills : public CASW_HudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CASWHudKills, vgui::EditablePanel );

public:
	CASWHudKills( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();
	virtual bool ShouldDraw();
	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

#ifdef USE_KILL_EVENT
	virtual void FireGameEvent(IGameEvent *event);
#endif

private:
	vgui::Label *m_pKillsLabel;
	vgui::Label *m_pScoresLabel1;
	vgui::Label *m_pScoresLabel2;
	vgui::Label *m_pScoresLabel3;
	vgui::Label *m_pScoresLabel4;
	vgui::Label *m_pScoresLabel5;
	vgui::Label *m_pScoresLabel6;
	vgui::Label *m_pScoresLabel7;
	vgui::Label *m_pScoresLabel8;

	static int __cdecl LeaderboardSortFunc( const void *lhs, const void *rhs );

#ifdef USE_KILL_EVENT
	void OnKillEvent( CBaseEntity *pVictim, CBaseEntity *pAttacker, CBaseEntity *pInflictor, IGameEvent *event );
#endif

	void ClearLabels();
	void SetLeaderboardLabel(vgui::Label *label, const char* entry, bool isPlayer);

#ifdef USE_KILL_EVENT
	int m_iKills;
	int m_Scores[ASW_NUM_MARINE_PROFILES];
#endif
};	

DECLARE_HUDELEMENT( CASWHudKills );

CASWHudKills::CASWHudKills( const char *pElementName ) : CASW_HudElement( pElementName ), vgui::EditablePanel( NULL, "ASWHudKills" )
{
	SetParent( GetClientMode()->GetViewport() );
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );

	m_pKillsLabel = NULL;
	m_pScoresLabel1 = NULL;
	m_pScoresLabel2 = NULL;
	m_pScoresLabel3 = NULL;
	m_pScoresLabel4 = NULL;
	m_pScoresLabel5 = NULL;
	m_pScoresLabel6 = NULL;
	m_pScoresLabel7 = NULL;
	m_pScoresLabel8 = NULL;

#ifdef USE_KILL_EVENT
	ListenForGameEvent( "entity_killed" );
#endif
}

void CASWHudKills::PerformLayout()
{
	if ( !m_pScoresLabel1 )
	{
		m_pScoresLabel1 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel1" ) );
		m_pScoresLabel1->SetName("");
	}

	if ( !m_pScoresLabel2 )
	{
		m_pScoresLabel2 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel2" ) );
		m_pScoresLabel2->SetName("");
	}

	if ( !m_pScoresLabel3 )
	{
		m_pScoresLabel3 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel3" ) );
		m_pScoresLabel3->SetName("");
	}

	if ( !m_pScoresLabel4 )
	{
		m_pScoresLabel4 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel4" ) );
		m_pScoresLabel4->SetName("");
	}

	if ( !m_pScoresLabel5 )
	{
		m_pScoresLabel5 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel5" ) );
		m_pScoresLabel5->SetName("");
	}

	if ( !m_pScoresLabel6 )
	{
		m_pScoresLabel6 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel6" ) );
		m_pScoresLabel6->SetName("");
	}

	if ( !m_pScoresLabel7 )
	{
		m_pScoresLabel7 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel7" ) );	
		m_pScoresLabel7->SetName("");
	}

	if ( !m_pScoresLabel8 )
	{
		m_pScoresLabel8 = dynamic_cast<vgui::Label*>( FindChildByName( "ScoresLabel8" ) );
		m_pScoresLabel8->SetName("");
	}

	if ( !m_pKillsLabel )
	{
		m_pKillsLabel = dynamic_cast<vgui::Label*>( FindChildByName( "KillsLabel" ) );
		m_pKillsLabel->SetName("");
		SetName("");
	}

	BaseClass::PerformLayout();
}

void CASWHudKills::Init()
{
	Reset();
}

void CASWHudKills::Reset()
{
#ifdef USE_KILL_EVENT
	m_iKills = 0;
	
	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		m_Scores[i] = 0;
	}
#endif

	ClearLabels();
	if ( m_pKillsLabel )
	{
		wchar_t *localized = g_pVGuiLocalize->Find( "#asw_stats_tkills" );
		if ( localized )
		{
			char text[256];
			g_pVGuiLocalize->ConvertUnicodeToANSI( localized, text, sizeof( text ) );
			m_pKillsLabel->SetText( VarArgs( "0 %s", text ) );
		}	
	}
}

void CASWHudKills::ClearLabels()
{
	Color textColor(255, 255, 255, 255);
	
	if (m_pScoresLabel1) { m_pScoresLabel1->SetText(""); m_pScoresLabel1->SetFgColor(textColor); m_pScoresLabel1->SetVisible(false); }
	if (m_pScoresLabel2) { m_pScoresLabel2->SetText(""); m_pScoresLabel2->SetFgColor(textColor); m_pScoresLabel2->SetVisible(false); }
	if (m_pScoresLabel3) { m_pScoresLabel3->SetText(""); m_pScoresLabel3->SetFgColor(textColor); m_pScoresLabel3->SetVisible(false); }
	if (m_pScoresLabel4) { m_pScoresLabel4->SetText(""); m_pScoresLabel4->SetFgColor(textColor); m_pScoresLabel4->SetVisible(false); }
	if (m_pScoresLabel5) { m_pScoresLabel5->SetText(""); m_pScoresLabel5->SetFgColor(textColor); m_pScoresLabel5->SetVisible(false); }
	if (m_pScoresLabel6) { m_pScoresLabel6->SetText(""); m_pScoresLabel6->SetFgColor(textColor); m_pScoresLabel6->SetVisible(false); }
	if (m_pScoresLabel7) { m_pScoresLabel7->SetText(""); m_pScoresLabel7->SetFgColor(textColor); m_pScoresLabel7->SetVisible(false); }
	if (m_pScoresLabel8) { m_pScoresLabel8->SetText(""); m_pScoresLabel8->SetFgColor(textColor); m_pScoresLabel8->SetVisible(false); }
}

void CASWHudKills::VidInit()
{
	Reset();
}

void CASWHudKills::OnThink()
{
	C_AlienSwarm* pRules = ASWGameRules();
	if ( !pRules || pRules->GetGameState() != ASW_GS_INGAME )
		return;

	ClearLabels();

	int killsTotal = 0;
	CUtlVector<LeaderboardEntry>scores;
	C_ASW_Player* pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	for (int i = 0; i < ASW_NUM_MARINE_PROFILES; i++)
	{
		C_ASW_Game_Resource* pResource = ASWGameResource();
		if (pResource)
		{
			C_ASW_Marine_Resource* pMarineResource = pResource->GetMarineResource(i);
			if (pMarineResource && pMarineResource->GetMarineEntity())
			{
				char szMarineName[MAX_PLAYER_NAME_LENGTH];
				pMarineResource->GetDisplayName(szMarineName, sizeof(szMarineName));
				LeaderboardEntry lbe;
				lbe.isPlayer = pLocalPlayer ? pLocalPlayer->GetViewNPC() == pMarineResource->GetMarineEntity() : false;
				Q_strncpy(lbe.name, szMarineName, sizeof(lbe.name));

#ifdef USE_KILL_EVENT
				lbe.score = m_Scores[i];
#else
				if (pMarineResource->IsInhabited())
				{
					C_ASW_Player* pPlayer = pMarineResource->GetMarineEntity()->GetCommander();
					if (g_PR && pPlayer)
						lbe.score = g_PR->GetPlayerScore(pPlayer->entindex());
				}
				else
					lbe.score = pMarineResource->m_iBotFrags;

				if(ASWDeathmatchMode())
					killsTotal += lbe.score;
				else
					killsTotal += pMarineResource->m_iAliensKilled;
#endif
				scores.AddToTail(lbe);
			}
		}
	}

	qsort(scores.Base(), scores.Count(), sizeof(LeaderboardEntry), &LeaderboardSortFunc);

	for (int i = 0; i < scores.Count(); i++)
	{
		LeaderboardEntry lbe = scores[i];
		wchar_t *localized = g_pVGuiLocalize->Find( "#asw_xp_kills" );
		if ( localized )
		{
			char text[256];
			g_pVGuiLocalize->ConvertUnicodeToANSI( localized, text, sizeof( text ) );
			const char* entry = VarArgs( "%s %d %s", lbe.name, lbe.score, text );

			switch (i)
			{
			case 0:
				SetLeaderboardLabel( m_pScoresLabel1, entry, lbe.isPlayer );
				break;
			case 1:
				SetLeaderboardLabel( m_pScoresLabel2, entry, lbe.isPlayer );
				break;
			case 2:
				SetLeaderboardLabel( m_pScoresLabel3, entry, lbe.isPlayer );
				break;
			case 3:
				SetLeaderboardLabel( m_pScoresLabel4, entry, lbe.isPlayer );
				break;
			case 4:
				SetLeaderboardLabel( m_pScoresLabel5, entry, lbe.isPlayer );
				break;
			case 5:
				SetLeaderboardLabel( m_pScoresLabel6, entry, lbe.isPlayer );
				break;
			case 6:
				SetLeaderboardLabel( m_pScoresLabel7, entry, lbe.isPlayer );
				break;
			case 7:
				SetLeaderboardLabel( m_pScoresLabel8, entry, lbe.isPlayer );
				break;
			}
		}
	}

	scores.RemoveAll();

#ifdef USE_KILL_EVENT
	killsTotal = m_iKills;
#endif

	if ( m_pKillsLabel )
	{
		wchar_t *localized = g_pVGuiLocalize->Find( "#asw_stats_tkills" );
		if ( localized )
		{
			char text[256];
			g_pVGuiLocalize->ConvertUnicodeToANSI( localized, text, sizeof( text ) );
			m_pKillsLabel->SetText( VarArgs( "%d %s", killsTotal, text ) );
		}
	}
}

void CASWHudKills::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/UI/HudKills.res" );
}

bool CASWHudKills::ShouldDraw()
{
	return asw_draw_kills.GetBool() && CASW_HudElement::ShouldDraw();
}

#ifdef USE_KILL_EVENT
void CASWHudKills::FireGameEvent( IGameEvent *event )
{
	const char *name = event->GetName();
	if ( 0 == Q_strcmp( name, "entity_killed" ) )
	{
		CBaseEntity *pVictim = ClientEntityList().GetEnt( event->GetInt( "entindex_killed", 0 ) );
		CBaseEntity *pAttacker = ClientEntityList().GetEnt( event->GetInt( "entindex_attacker", 0 ) );
		CBaseEntity *pInflictor = ClientEntityList().GetEnt( event->GetInt( "entindex_inflictor", 0 ) );
		OnKillEvent( pVictim, pAttacker, pInflictor, event );
	}
}

void CASWHudKills::OnKillEvent( CBaseEntity *pVictim, CBaseEntity *pAttacker, CBaseEntity *pInflictor, IGameEvent *event )
{
	if ( pVictim && IsAlienClass( pVictim->Classify() ) )
	{
		m_iKills++;

		if ( pAttacker &&  pAttacker->Classify() == CLASS_ASW_MARINE )
		{
			for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
			{
				C_ASW_Game_Resource* pResource = ASWGameResource();
				if ( pResource )
				{
					C_ASW_Marine_Resource* pMR = pResource->GetMarineResource( i );
					if ( pMR && pMR->GetMarineEntity() == pAttacker )
					{
						m_Scores[i]++;
						break;
					}
				}
			}
		}
	}
}
#endif

void CASWHudKills::SetLeaderboardLabel(vgui::Label *label, const char* entry, bool isPlayer)
{
	if (label) 
	{ 
		if (isPlayer)
		{
			Color playerColor(asw_player_lb_color_r.GetInt(), asw_player_lb_color_g.GetInt(), asw_player_lb_color_b.GetInt(), 255);
			label->SetFgColor(playerColor);
		}
		label->SetText(entry); 
		label->SetVisible(true); 
	}
}

int CASWHudKills::LeaderboardSortFunc(const void *lhs, const void *rhs)
{
	const LeaderboardEntry *s1 = (const LeaderboardEntry *)lhs;
	const LeaderboardEntry *s2 = (const LeaderboardEntry *)rhs;

	if (s1->score < s2->score)
		return 1;
	else if (s1->score > s2->score)
		return -1;

	// scores are equal so sort by name
	return Q_strcmp(s1->name, s2->name);
}