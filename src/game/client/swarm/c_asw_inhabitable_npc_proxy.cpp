#include "cbase.h"
#include "imaterialproxydict.h"
#include "proxyentity.h"
#include "materialsystem/imaterialvar.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_egg.h"
#include "c_asw_clientragdoll.h"
#include "c_asw_mesh_emitter_entity.h"
#include "c_asw_physics_prop_statue.h"
#include "c_asw_simple_alien.h"
#include "c_asw_weapon.h"
#include "asw_deathmatch_mode.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_deathmatch_team_colors( "rd_deathmatch_team_colors", "2", FCVAR_ARCHIVE, "1: allies blue enemies red / 2: ALPHA yellow BETA blue" );
ConVar rd_deathmatch_team_color_highlight( "rd_deathmatch_team_color_highlight", "1", FCVAR_ARCHIVE, "0 = use tinting instead" );
ConVar rd_team_color_alpha( "rd_team_color_alpha", "255 240 150", FCVAR_NONE );
ConVar rd_team_color_beta( "rd_team_color_beta", "20 100 255", FCVAR_NONE );
ConVar rd_team_color_ally( "rd_team_color_ally", "100 100 255", FCVAR_NONE );
ConVar rd_team_color_enemy( "rd_team_color_enemy", "255 10 10", FCVAR_NONE );

ConVar rd_highlight_active_character( "rd_highlight_active_character", "0", FCVAR_ARCHIVE );
ConVar rd_highlight_allies( "rd_highlight_allies", "0", FCVAR_ARCHIVE );
ConVar rd_highlight_allies_color( "rd_highlight_allies_color", "100 100 255", FCVAR_ARCHIVE );
ConVar rd_highlight_enemies( "rd_highlight_enemies", "0", FCVAR_ARCHIVE );
ConVar rd_highlight_enemies_color( "rd_highlight_enemies_color", "255 10 10", FCVAR_ARCHIVE );
// if these asserts fail, update the number here and in the convar below's description
COMPILE_TIME_ASSERT( CLASS_ASW_DRONE == 27 );
COMPILE_TIME_ASSERT( CLASS_ASW_MARINE == 40 );
static CUtlVector<Class_T> s_RD_Highlight_Ignore_Class;
static void UpdateIgnoredHighlightClasses( IConVar *pVar, const char *szOldValue, float flOldValue )
{
	ConVarRef var( pVar );
	CSplitString split( var.GetString(), "," );

	s_RD_Highlight_Ignore_Class.Purge();

	FOR_EACH_VEC( split, i )
	{
		if ( V_isdigit( split[i][0] ) )
		{
			int iClass = V_atoi( split[i] );
			s_RD_Highlight_Ignore_Class.AddToTail( ( Class_T )iClass );
		}
	}
}
ConVar rd_highlight_ignore_class( "rd_highlight_ignore_class", "", FCVAR_NONE, "advanced option. comma-separated list of integers. any integer that is a Class_T from C++ will block that Class_T from using ally/enemy/team glow colors. not guaranteed to keep working between updates. examples: drone = 27, marine = 40", UpdateIgnoredHighlightClasses );

ConVar rd_debug_status_fx( "rd_debug_status_fx", "0", FCVAR_CHEAT, "bitfield: 1=fire, 2=ice, 4=shock, 8=nightvision. for testing RDCharacter shader." );

extern ConVar asw_night_vision_self_illum_multiplier;
extern ConVar asw_sniper_scope_self_illum_multiplier;

extern bool g_bRenderingGlows;

class CASW_Character_Proxy : public CEntityMaterialProxy
{
public:
	bool Init( IMaterial *pMaterial, KeyValues *pKeyValues ) override;
	void OnBind( void *pRenderable ) override;
	void OnBind( C_BaseEntity *pEnt ) override;
	IMaterial *GetMaterial() override;

private:
	void OnBindNull();
	void OnBindCharacter( C_ASW_Inhabitable_NPC *pNPC );
	void OnBindEgg( C_ASW_Egg *pEgg );
	void OnBindSimpleAlien( C_ASW_Simple_Alien *pAlien );
	void OnBindRagdoll( C_ClientRagdoll *pRagdoll );
	void OnBindGib( C_ASW_Mesh_Emitter *pGib );
	void OnBindStatue( C_ASWStatueProp *pStatue );
	void OnBindWeapon( C_ASW_Weapon *pWeapon );

	static float GetNightVisionAmount();

	// $time variable (so we don't need to also have the CurrentTime proxy)
	IMaterialVar *m_pTime{};
	// signal to the shader that we attached the proxy
	IMaterialVar *m_pProxyAttached{};
	// team color (RGB+width)
	IMaterialVar *m_pTeamColor{};
	// status effect FX (fire, ice, shock, night vision)
	IMaterialVar *m_pStatusFx{};

	bool m_bForceAlly{};
	bool m_bForceEnemy{};
};

bool CASW_Character_Proxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	Assert( !V_stricmp( pMaterial->GetShaderName(), "RDCharacter" ) );
	if ( V_stricmp( pMaterial->GetShaderName(), "RDCharacter" ) )
	{
		// don't spew warning for GPUs that can't support shader model 2.0b
		if ( engine->GetDXSupportLevel() >= 92 )
		{
			Warning( "Character material proxy used with unexpected shader %s in material %s\n", pMaterial->GetShaderName(), pMaterial->GetName() );
		}
		return false;
	}

	bool bFound;

	m_pTime = pMaterial->FindVar( "$time", &bFound );
	if ( !bFound )
		return false;

	m_pProxyAttached = pMaterial->FindVar( "$character_proxy_attached", &bFound );
	if ( !bFound )
		return false;

	m_pTeamColor = pMaterial->FindVar( "$character_team_color", &bFound );
	if ( !bFound )
		return false;

	m_pStatusFx = pMaterial->FindVar( "$character_status_fx", &bFound );
	if ( !bFound )
		return false;

	m_bForceAlly = pKeyValues->GetBool( "force_ally" );
	m_bForceEnemy = pKeyValues->GetBool( "force_enemy" );

	return true;
}

void CASW_Character_Proxy::OnBind( void *pRenderable )
{
	if ( !pRenderable )
	{
		OnBind( ( C_BaseEntity * )NULL );
		return;
	}

	CEntityMaterialProxy::OnBind( pRenderable );
}

void CASW_Character_Proxy::OnBind( C_BaseEntity *pEnt )
{
	m_pTime->SetFloatValue( gpGlobals->curtime );
	m_pProxyAttached->SetIntValue( 1 );

	if ( !pEnt )
	{
		OnBindNull();
	}
	else if ( pEnt->IsInhabitableNPC() )
	{
		OnBindCharacter( assert_cast< C_ASW_Inhabitable_NPC * >( pEnt ) );
	}
	else if ( pEnt->Classify() == CLASS_ASW_EGG )
	{
		OnBindEgg( assert_cast< C_ASW_Egg * >( pEnt ) );
	}
	else if ( C_ASW_Simple_Alien *pAlien = dynamic_cast< C_ASW_Simple_Alien * >( pEnt ) )
	{
		OnBindSimpleAlien( pAlien );
	}
	else if ( C_ClientRagdoll *pRagdoll = dynamic_cast< C_ClientRagdoll * >( pEnt ) )
	{
		OnBindRagdoll( pRagdoll );
	}
	else if ( C_ASW_Mesh_Emitter *pGib = dynamic_cast< C_ASW_Mesh_Emitter * >( pEnt ) )
	{
		OnBindGib( pGib );
	}
	else if ( C_ASWStatueProp *pStatue = dynamic_cast< C_ASWStatueProp * >( pEnt ) )
	{
		OnBindStatue( pStatue );
	}
	else if ( C_ASW_Weapon *pWeapon = dynamic_cast< C_ASW_Weapon * >( pEnt ) )
	{
		OnBindWeapon( pWeapon );
	}
	else if ( pEnt->IsProp() )
	{
		OnBindNull();
	}
	else
	{
		Assert( !"Unhandled entity type for CASW_Character_Proxy" );
		OnBindNull();
	}

	if ( m_bForceEnemy && rd_highlight_enemies.GetFloat() > 0.0f )
	{
		Vector vecEnemyColor = rd_highlight_enemies_color.GetColorAsVector();
		m_pTeamColor->SetVecValue( VectorExpand( vecEnemyColor ), rd_highlight_enemies.GetFloat() );
	}
	else if ( m_bForceAlly && rd_highlight_allies.GetFloat() > 0.0f )
	{
		Vector vecAllyColor = rd_highlight_allies_color.GetColorAsVector();
		m_pTeamColor->SetVecValue( VectorExpand( vecAllyColor ), rd_highlight_allies.GetFloat() );
	}
	else if ( s_RD_Highlight_Ignore_Class.Find( pEnt ? pEnt->Classify() : CLASS_NONE ) != s_RD_Highlight_Ignore_Class.InvalidIndex() )
	{
		m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
	}

	if ( int debug = rd_debug_status_fx.GetInt() )
	{
		if ( debug & 1 )
		{
			m_pStatusFx->SetVecComponentValue( 1, 0 );
		}
		if ( debug & 2 )
		{
			m_pStatusFx->SetVecComponentValue( 1, 1 );
		}
		if ( debug & 4 )
		{
			m_pStatusFx->SetVecComponentValue( 1, 2 );
		}
		if ( debug & 8 )
		{
			m_pStatusFx->SetVecComponentValue( 25, 3 );
		}
	}
}

IMaterial *CASW_Character_Proxy::GetMaterial()
{
	return m_pTeamColor->GetOwningMaterial();
}

void CASW_Character_Proxy::OnBindNull()
{
	m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
	m_pStatusFx->SetVecValue( 0, 0, 0, 0 );
}

void CASW_Character_Proxy::OnBindCharacter( C_ASW_Inhabitable_NPC *pNPC )
{
	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	C_ASW_Inhabitable_NPC *pViewNPC = pLocalPlayer ? pLocalPlayer->GetViewNPC() : NULL;

	bool bAlly = false;
	bool bEnemy = false;
	if ( pNPC->GetHealth() <= 0.0f )
	{
		m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
	}
	else
	{
		if ( pNPC == pViewNPC )
		{
			bAlly = true;
			bEnemy = false;
		}
		else if ( pNPC->m_nFaction == FACTION_NEUTRAL )
		{
			bAlly = false;
			bEnemy = false;
		}
		else if ( ASWDeathmatchMode() && pNPC->Classify() == CLASS_ASW_MARINE )
		{
			if ( !ASWDeathmatchMode()->IsTeamDeathmatchEnabled() )
			{
				bEnemy = true;
			}
			else if ( !pNPC->InSameTeam( pViewNPC ) )
			{
				bEnemy = true;
			}
			else
			{
				bAlly = true;
			}
		}
		else
		{
			bEnemy = pViewNPC ? pViewNPC->m_nFaction != pNPC->m_nFaction : true;
			bAlly = !bEnemy;
		}

		C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pNPC );
		if ( ASWDeathmatchMode() && ASWDeathmatchMode()->IsTeamDeathmatchEnabled() && pMarine )
		{
			switch ( rd_deathmatch_team_colors.GetInt() )
			{
			default:
			case 0:
			{
				m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
				break;
			}
			case 1:
			{
				Vector vecTeamColor = bEnemy ? rd_team_color_enemy.GetColorAsVector() : rd_team_color_ally.GetColorAsVector();
				m_pTeamColor->SetVecValue( VectorExpand( vecTeamColor ), rd_deathmatch_team_color_highlight.GetFloat() );
				break;
			}
			case 2:
			{
				if ( pNPC->GetTeamNumber() == TEAM_ALPHA )
				{
					Vector vecTeamColor = rd_team_color_alpha.GetColorAsVector();
					m_pTeamColor->SetVecValue( VectorExpand( vecTeamColor ), rd_deathmatch_team_color_highlight.GetFloat() );
				}
				else if ( pNPC->GetTeamNumber() == TEAM_BETA )
				{
					Vector vecTeamColor = rd_team_color_beta.GetColorAsVector();
					m_pTeamColor->SetVecValue( VectorExpand( vecTeamColor ), rd_deathmatch_team_color_highlight.GetFloat() );
				}
				else
				{
					m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
				}
				break;
			}
			}
		}
		else if ( rd_highlight_active_character.GetFloat() > 0.0f && pNPC == pViewNPC )
		{
			if ( pMarine )
				m_pTeamColor->SetVecValue( VectorExpand( pMarine->m_vecMarineColor ), rd_highlight_active_character.GetFloat() );
			else
				m_pTeamColor->SetVecValue( 1.0f, 1.0f, 1.0f, rd_highlight_active_character.GetFloat() );
		}
		else if ( bEnemy && rd_highlight_enemies.GetFloat() > 0.0f )
		{
			Vector vecEnemyColor = rd_highlight_enemies_color.GetColorAsVector();
			m_pTeamColor->SetVecValue( VectorExpand( vecEnemyColor ), rd_highlight_enemies.GetFloat() );
		}
		else if ( bAlly && rd_highlight_allies.GetFloat() > 0.0f )
		{
			Vector vecAllyColor = rd_highlight_allies_color.GetColorAsVector();
			m_pTeamColor->SetVecValue( VectorExpand( vecAllyColor ), rd_highlight_allies.GetFloat() );
		}
		else
		{
			m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
		}
	}

	float flFire = pNPC->m_bOnFire ? 1.0f : 0.0f;
	float flIce = pNPC->GetMoveType() != MOVETYPE_NONE ? pNPC->GetFrozenAmount() : 0.0f;
	float flShock = pNPC->m_bElectroStunned ? 1.0f : 0.0f;
	float flNightVision = bEnemy ? GetNightVisionAmount() : 0.0f;
	m_pStatusFx->SetVecValue( flFire, flIce, flShock, flNightVision );
}

void CASW_Character_Proxy::OnBindEgg( C_ASW_Egg *pEgg )
{
	bool bEnemy = C_ASW_Marine::GetViewMarine() != NULL;
	if ( pEgg->GetHealth() > 0.0f && bEnemy && rd_highlight_enemies.GetFloat() > 0.0f )
	{
		Vector vecEnemyColor = rd_highlight_enemies_color.GetColorAsVector();
		m_pTeamColor->SetVecValue( VectorExpand( vecEnemyColor ), clamp( rd_highlight_enemies.GetFloat(), 0.0f, 1.0f ) );
	}
	else if ( pEgg->GetHealth() > 0.0f && !bEnemy && rd_highlight_allies.GetFloat() > 0.0f )
	{
		Vector vecAllyColor = rd_highlight_allies_color.GetColorAsVector();
		m_pTeamColor->SetVecValue( VectorExpand( vecAllyColor ), clamp( rd_highlight_allies.GetFloat(), 0.0f, 1.0f ) );
	}
	else
	{
		m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
	}

	float flFire = pEgg->m_bOnFire ? 1.0f : 0.0f;
	float flIce = pEgg->GetFrozenAmount();
	float flShock = 0.0f;
	float flNightVision = bEnemy ? GetNightVisionAmount() : 0.0f;
	m_pStatusFx->SetVecValue( flFire, flIce, flShock, flNightVision );
}

void CASW_Character_Proxy::OnBindSimpleAlien( C_ASW_Simple_Alien *pAlien )
{
	bool bEnemy = C_ASW_Marine::GetViewMarine() != NULL;
	if ( pAlien->GetHealth() > 0.0f && bEnemy && rd_highlight_enemies.GetFloat() > 0.0f )
	{
		Vector vecEnemyColor = rd_highlight_enemies_color.GetColorAsVector();
		m_pTeamColor->SetVecValue( VectorExpand( vecEnemyColor ), clamp( rd_highlight_enemies.GetFloat(), 0.0f, 1.0f ) );
	}
	else if ( pAlien->GetHealth() > 0.0f && !bEnemy && rd_highlight_allies.GetFloat() > 0.0f )
	{
		Vector vecAllyColor = rd_highlight_allies_color.GetColorAsVector();
		m_pTeamColor->SetVecValue( VectorExpand( vecAllyColor ), clamp( rd_highlight_allies.GetFloat(), 0.0f, 1.0f ) );
	}
	else
	{
		m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
	}

	float flFire = pAlien->IsOnFire() ? 1.0f : 0.0f;
	float flIce = pAlien->GetFrozenAmount();
	float flShock = 0.0f;
	float flNightVision = bEnemy ? GetNightVisionAmount() : 0.0f;
	m_pStatusFx->SetVecValue( flFire, flIce, flShock, flNightVision );
}

void CASW_Character_Proxy::OnBindRagdoll( C_ClientRagdoll *pRagdoll )
{
	C_ASW_ClientRagdoll *pASWRagdoll = dynamic_cast< C_ASW_ClientRagdoll * >( pRagdoll );

	m_pTeamColor->SetVecValue( 0, 0, 0, 0 );

	float flFire = ( pRagdoll->GetFlags() & FL_ONFIRE ) ? 1.0f : 0.0f;
	float flIce = 0.0f;
	float flShock = pASWRagdoll && pASWRagdoll->m_bElectroShock ? 1.0f : 0.0f;
	float flNightVision = 0.0f;
	m_pStatusFx->SetVecValue( flFire, flIce, flShock, flNightVision );
}

void CASW_Character_Proxy::OnBindGib( C_ASW_Mesh_Emitter *pGib )
{
	m_pTeamColor->SetVecValue( 0, 0, 0, 0 );

	float flFire = 0.0f;
	float flIce = pGib->m_bFrozen ? 1.0f : 0.0f;
	float flShock = 0.0f;
	float flNightVision = 0.0f;
	m_pStatusFx->SetVecValue( flFire, flIce, flShock, flNightVision );
}

void CASW_Character_Proxy::OnBindStatue( C_ASWStatueProp *pStatue )
{
	m_pTeamColor->SetVecValue( 0, 0, 0, 0 );

	float flFire = 0.0f;
	float flIce = 1.0f;
	float flShock = 0.0f;
	float flNightVision = 0.0f;
	m_pStatusFx->SetVecValue( flFire, flIce, flShock, flNightVision );
}

void CASW_Character_Proxy::OnBindWeapon( C_ASW_Weapon *pWeapon )
{
	OnBind( pWeapon->GetOwner() );

	if ( pWeapon->Classify() == CLASS_ASW_ELECTRIFIED_ARMOR )
	{
		C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pWeapon->GetOwner() );
		if ( pMarine && pMarine->IsElectrifiedArmorActive() )
		{
			m_pStatusFx->SetVecComponentValue( 1.0f, 2 );
		}
	}
}

float CASW_Character_Proxy::GetNightVisionAmount()
{
	C_ASW_Marine *pViewMarine = C_ASW_Marine::GetViewMarine();
	if ( pViewMarine && pViewMarine->m_flVisionAlpha > 0.0f )
	{
		return pViewMarine->m_flVisionAlpha / 255.0f * asw_night_vision_self_illum_multiplier.GetFloat();
	}

	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( pLocalPlayer && pLocalPlayer->IsSniperScopeActive() && g_bRenderingGlows )
	{
		return asw_sniper_scope_self_illum_multiplier.GetFloat();
	}

	return 0.0f;
}

EXPOSE_MATERIAL_PROXY( CASW_Character_Proxy, RDCharacter );
