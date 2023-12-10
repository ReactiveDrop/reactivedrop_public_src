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
#include "c_asw_weapon.h"
#include "asw_deathmatch_mode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_deathmatch_team_colors( "rd_deathmatch_team_colors", "2", FCVAR_ARCHIVE, "1: allies green enemies red / 2: Terrorists yellow Counter-Terrorists blue" );
ConVar rd_team_color_alpha( "rd_team_color_alpha", "255 240 150", FCVAR_NONE );
ConVar rd_team_color_beta( "rd_team_color_beta", "20 100 255", FCVAR_NONE );
ConVar rd_team_color_ally( "rd_team_color_ally", "100 255 100", FCVAR_NONE );
ConVar rd_team_color_enemy( "rd_team_color_enemy", "255 10 10", FCVAR_NONE );

ConVar rd_highlight_active_character( "rd_highlight_active_character", "0", FCVAR_ARCHIVE );
ConVar rd_highlight_allies( "rd_highlight_allies", "0", FCVAR_ARCHIVE );
ConVar rd_highlight_allies_color( "rd_highlight_allies_color", "100 100 255", FCVAR_ARCHIVE );
ConVar rd_highlight_enemies( "rd_highlight_enemies", "0", FCVAR_ARCHIVE );
ConVar rd_highlight_enemies_color( "rd_highlight_enemies_color", "255 10 10", FCVAR_ARCHIVE );

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
	void OnBindRagdoll( C_ASW_ClientRagdoll *pRagdoll );
	void OnBindGib( C_ASW_Mesh_Emitter *pGib );
	void OnBindStatue( C_ASWStatueProp *pStatue );
	void OnBindWeapon( C_ASW_Weapon *pWeapon );

	static float GetNightVisionAmount();

	// team color (RGBA)
	IMaterialVar *m_pTeamColor{};
	// status effect FX (fire, ice, shock, night vision)
	IMaterialVar *m_pStatusFx{};
};

bool CASW_Character_Proxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	Assert( !V_stricmp( pMaterial->GetShaderName(), "Character" ) );
	if ( V_stricmp( pMaterial->GetShaderName(), "Character" ) )
	{
		Warning( "Character material proxy used with unexpected shader %s in material %s\n", pMaterial->GetShaderName(), pMaterial->GetName() );
		return false;
	}

	bool bFound;

	m_pTeamColor = pMaterial->FindVar( "$character_team_color", &bFound );
	if ( !bFound )
		return false;

	m_pStatusFx = pMaterial->FindVar( "$character_status_fx", &bFound );
	if ( !bFound )
		return false;

	return true;
}

void CASW_Character_Proxy::OnBind( void *pRenderable )
{
	if ( !pRenderable )
	{
		OnBindNull();
		return;
	}

	CEntityMaterialProxy::OnBind( pRenderable );
}

void CASW_Character_Proxy::OnBind( C_BaseEntity *pEnt )
{
	if ( !pEnt )
	{
		OnBindNull();
		return;
	}

	if ( pEnt->IsInhabitableNPC() )
	{
		OnBindCharacter( assert_cast< C_ASW_Inhabitable_NPC * >( pEnt ) );
		return;
	}

	if ( pEnt->Classify() == CLASS_ASW_EGG )
	{
		OnBindEgg( assert_cast< C_ASW_Egg * >( pEnt ) );
		return;
	}

	if ( C_ASW_ClientRagdoll *pRagdoll = dynamic_cast< C_ASW_ClientRagdoll * >( pEnt ) )
	{
		OnBindRagdoll( pRagdoll );
		return;
	}

	if ( C_ASW_Mesh_Emitter *pGib = dynamic_cast< C_ASW_Mesh_Emitter * >( pEnt ) )
	{
		OnBindGib( pGib );
		return;
	}

	if ( C_ASWStatueProp *pStatue = dynamic_cast< C_ASWStatueProp * >( pEnt ) )
	{
		OnBindStatue( pStatue );
		return;
	}

	if ( C_ASW_Weapon *pWeapon = dynamic_cast< C_ASW_Weapon * >( pEnt ) )
	{
		OnBindWeapon( pWeapon );
		return;
	}

	Assert( !"Unhandled entity type for CASW_Character_Proxy" );
	OnBindNull();
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

	bool bEnemy = false;
	if ( pNPC->GetHealth() <= 0.0f )
	{
		m_pTeamColor->SetVecValue( 0, 0, 0, 0 );
	}
	else
	{
		if ( pNPC == pViewNPC )
		{
			bEnemy = false;
		}
		else if ( pNPC->Classify() != CLASS_ASW_MARINE )
		{
			Assert( !pViewNPC || pViewNPC->Classify() == CLASS_ASW_MARINE );
			bEnemy = true;
		}
		else if ( C_ASW_Deathmatch_Mode *pDM = ASWDeathmatchMode() )
		{
			if ( !pDM->IsTeamDeathmatchEnabled() )
			{
				bEnemy = true;
			}
			else if ( !pNPC->InSameTeam( pViewNPC ) )
			{
				bEnemy = true;
			}
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
				m_pTeamColor->SetVecValue( VectorExpand( vecTeamColor ), 1.0f );
				break;
			}
			case 2:
			{
				if ( pNPC->GetTeamNumber() == TEAM_ALPHA )
				{
					Vector vecTeamColor = rd_team_color_alpha.GetColorAsVector();
					m_pTeamColor->SetVecValue( VectorExpand( vecTeamColor ), 1.0f );
				}
				else if ( pNPC->GetTeamNumber() == TEAM_BETA )
				{
					Vector vecTeamColor = rd_team_color_beta.GetColorAsVector();
					m_pTeamColor->SetVecValue( VectorExpand( vecTeamColor ), 1.0f );
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
				m_pTeamColor->SetVecValue( VectorExpand( pMarine->m_vecMarineColor ), clamp( rd_highlight_active_character.GetFloat(), 0.0f, 1.0f ) );
			else
				m_pTeamColor->SetVecValue( 1.0f, 1.0f, 1.0f, clamp( rd_highlight_active_character.GetFloat(), 0.0f, 1.0f ) );
		}
		else if ( bEnemy && rd_highlight_enemies.GetFloat() > 0.0f )
		{
			Vector vecEnemyColor = rd_highlight_enemies_color.GetColorAsVector();
			m_pTeamColor->SetVecValue( VectorExpand( vecEnemyColor ), clamp( rd_highlight_enemies.GetFloat(), 0.0f, 1.0f ) );
		}
		else if ( !bEnemy && rd_highlight_allies.GetFloat() > 0.0f )
		{
			Vector vecAllyColor = rd_highlight_allies_color.GetColorAsVector();
			m_pTeamColor->SetVecValue( VectorExpand( vecAllyColor ), clamp( rd_highlight_allies.GetFloat(), 0.0f, 1.0f ) );
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

void CASW_Character_Proxy::OnBindRagdoll( C_ASW_ClientRagdoll *pRagdoll )
{
	m_pTeamColor->SetVecValue( 0, 0, 0, 0 );

	float flFire = ( pRagdoll->GetFlags() & FL_ONFIRE ) ? 1.0f : 0.0f;
	float flIce = 0.0f;
	float flShock = pRagdoll->m_bElectroShock;
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

EXPOSE_MATERIAL_PROXY( CASW_Character_Proxy, Character );
