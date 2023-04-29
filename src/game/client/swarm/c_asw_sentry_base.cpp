#include "cbase.h"
#include "c_asw_sentry_base.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>
#include "asw_util_shared.h"
#include "c_asw_marine.h"
#include "c_asw_player.h"
#include "c_asw_pickup_weapon.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_hud_use_icon.h"
#include "c_user_message_register.h"
#include "c_gib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_sentry_gib( "rd_sentry_gib", "1", FCVAR_NONE, "should destroyed sentries create gibs? (if not, they just disappear)" );
ConVar rd_sentry_gib_force( "rd_sentry_gib_force", "150", FCVAR_NONE, "amount of random force to apply to sentry gibs" );
ConVar rd_sentry_gib_spin( "rd_sentry_gib_spin", "30", FCVAR_NONE, "amount of random spin to apply to sentry gibs" );
ConVar rd_sentry_gib_lifetime( "rd_sentry_gib_lifetime", "30", FCVAR_NONE, "time in seconds before sentry gibs fade out" );
ConVar rd_sentry_gib_lifetime_jitter( "rd_sentry_gib_lifetime_jitter", "1", FCVAR_NONE, "random time to add to lifetime to avoid all gibs fading at the same instant" );

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Sentry_Base, DT_ASW_Sentry_Base, CASW_Sentry_Base )
	RecvPropBool( RECVINFO( m_bAssembled ) ),
	RecvPropBool( RECVINFO( m_bIsInUse ) ),
	RecvPropFloat( RECVINFO( m_fAssembleProgress ) ),
	RecvPropFloat( RECVINFO( m_fAssembleCompleteTime ) ),
	RecvPropInt( RECVINFO( m_iAmmo ) ),
	RecvPropInt( RECVINFO( m_iMaxAmmo ) ),
	RecvPropBool( RECVINFO( m_bSkillMarineHelping ) ),
	RecvPropInt( RECVINFO( m_nGunType ) ),
	RecvPropEHandle( RECVINFO( m_hOriginalOwnerPlayer ) ),
	RecvPropIntWithMinusOneFlag( RECVINFO( m_iInventoryEquipSlot ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ASW_Sentry_Base )

END_PREDICTION_DATA()

CUtlVector<C_ASW_Sentry_Base*>	g_SentryGuns;

vgui::HFont C_ASW_Sentry_Base::s_hAmmoFont = vgui::INVALID_FONT;

C_ASW_Sentry_Base::C_ASW_Sentry_Base()
{
	m_bAssembled = false;
	m_bIsInUse = false;
	m_fAssembleProgress = 0;
	m_bSkillMarineHelping = false;
	g_SentryGuns.AddToTail( this );
	m_nUseIconTextureID = -1;
	m_hOriginalOwnerPlayer = NULL;
	m_iInventoryEquipSlot = -1;
}


C_ASW_Sentry_Base::~C_ASW_Sentry_Base()
{
	g_SentryGuns.FindAndRemove( this );
}

bool C_ASW_Sentry_Base::ShouldDraw()
{
	return true;
}

int C_ASW_Sentry_Base::DrawModel( int flags, const RenderableInstance_t &instance )
{
	int d = BaseClass::DrawModel(flags, instance);

	return d;
}

int C_ASW_Sentry_Base::GetSentryIconTextureID()
{
	if ( m_nUseIconTextureID == -1 )
	{
		m_nUseIconTextureID = g_ASWEquipmentList.GetEquipIconTexture( true, g_ASWEquipmentList.GetRegularIndex( GetWeaponClass() ) );
	}

	return m_nUseIconTextureID;
}

bool C_ASW_Sentry_Base::WantsDismantle( void ) const
{
	if ( IsInUse() || !IsAssembled() || GetAmmo() <= 0 || gpGlobals->curtime < m_fAssembleCompleteTime + 5.0f )
		return false;

	return true;
}

bool C_ASW_Sentry_Base::IsUsable(C_BaseEntity *pUser)
{
	return (pUser && pUser->GetAbsOrigin().DistTo(GetAbsOrigin()) < ASW_MARINE_USE_RADIUS);	// near enough?
}

bool C_ASW_Sentry_Base::GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser)
{
	if ( !pUser || pUser->Classify() != CLASS_ASW_MARINE )
	{
		return false;
	}

	if ( IsAssembled() )
	{
		C_ASW_Player *pPlayer = pUser->GetCommander();
		if ( pPlayer && pUser->IsInhabited() && pPlayer->m_flUseKeyDownTime != 0.0f && ( gpGlobals->curtime - pPlayer->m_flUseKeyDownTime ) > 0.2f )		// if player has started holding down the USE key
		{
			TryLocalize( "#asw_disassembling_sentry", action.wszText, sizeof( action.wszText ) );
			action.fProgress = ( ( gpGlobals->curtime - pPlayer->m_flUseKeyDownTime ) - 0.2f ) / ( ASW_USE_KEY_HOLD_SENTRY_TIME - 0.2f );
			action.fProgress = clamp<float>( action.fProgress, 0.0f, 1.0f );
			action.bShowHoldButtonUseKey = false;
			action.bShowUseKey = false;
			action.bNoFadeIfSameUseTarget = true;
		}
		else
		{
			TryLocalize( "#asw_turn_sentry", action.wszText, sizeof( action.wszText ) );
			action.fProgress = -1;
			TryLocalize( "#asw_disassemble_sentry", action.wszHoldButtonText, sizeof( action.wszHoldButtonText ) );
			action.bShowHoldButtonUseKey = true;
			action.bShowUseKey = true;
		}
		action.iUseIconTexture = GetSentryIconTextureID();		
		action.UseTarget = this;
	}
	else
	{
		if ( m_bIsInUse )
		{
			TryLocalize( "#asw_assembling_sentry", action.wszText, sizeof( action.wszText ) );
			action.bShowUseKey = false;
			action.bNoFadeIfSameUseTarget = true;
		}
		else
		{
			TryLocalize( "#asw_assemble_sentry", action.wszText, sizeof( action.wszText ) );
			action.bShowUseKey = true;
		}
		action.iUseIconTexture = GetSentryIconTextureID();		
		action.UseTarget = this;
		action.fProgress = GetAssembleProgress();
	}
	action.UseIconRed = 66;
	action.UseIconGreen = 142;
	action.UseIconBlue = 192;
	action.iInventorySlot = -1;
	action.bWideIcon = true;
	return true;
}

void C_ASW_Sentry_Base::CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon )
{	
	if ( !m_bAssembled )
		return;

	if (s_hAmmoFont == vgui::INVALID_FONT)
	{
		vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SwarmSchemeNew.res", "SwarmSchemeNew");
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(scheme);
		if (pScheme)
			s_hAmmoFont = vgui::scheme()->GetIScheme(scheme)->GetFont("DefaultSmall", true);
	}

	if (s_hAmmoFont == vgui::INVALID_FONT || alpha <= 0)
		return;
	
	int nAmmo = MAX( 0, GetAmmo() );
	Color textColor( 255, 255, 255, 255 );
	if ( nAmmo < 50 )
	{
		textColor = Color( 255, 255, 0, 255 );
	}

	if ( pUseIcon )
	{
		CASW_HUD_Use_Icon *pUseIconPanel = static_cast<CASW_HUD_Use_Icon*>(pUseIcon);
		float flProgress = (float) nAmmo / (float) GetMaxAmmo();
		char szCountText[64];
		Q_snprintf( szCountText, sizeof( szCountText ), "%d", MAX( nAmmo, 0 ) );
		pUseIconPanel->CustomPaintProgressBar( ix, iy, alpha / 255.0f, flProgress, szCountText, s_hAmmoFont, textColor, "#asw_ammo_label" );
	}
}

const char* C_ASW_Sentry_Base::GetWeaponClass()
{
	switch( m_nGunType.Get() )
	{
		case 0: return "asw_weapon_sentry";
		case 1: return "asw_weapon_sentry_cannon";
		case 2: return "asw_weapon_sentry_flamer";
		case 3: return "asw_weapon_sentry_freeze";
	}
	return "asw_weapon_sentry";
}

static const char *const s_szSentryGibs[] =
{
	"models/sentry_gun/sentry_gibs/sentry_base_gib.mdl",
	"models/sentry_gun/sentry_gibs/sentry_cam_gib.mdl",
	"models/sentry_gun/sentry_gibs/sentry_holder_gib.mdl",
	"models/sentry_gun/sentry_gibs/sentry_left_arm_gib.mdl",
	"models/sentry_gun/sentry_gibs/sentry_left_backleg_gib.mdl",
	"models/sentry_gun/sentry_gibs/sentry_right_backleg_gib.mdl",
	"models/sentry_gun/sentry_gibs/sentry_top_gib.mdl",
};

static const Vector s_vecSentryGibOffset[] =
{
	Vector( 0, 0, 0 ),
	Vector( 0, 0, 0 ),
	Vector( 0, 0, 0 ),
	Vector( 0, 0, 0 ),
	Vector( 0, 0, 0 ),
	Vector( 0, 0, 0 ),
	Vector( 0, 0, 0 ),
};

void __MsgFunc_SentryGib( bf_read &msg )
{
	Vector vecOrigin( msg.ReadFloat(), msg.ReadFloat(), msg.ReadFloat() );
	QAngle angles( 0, msg.ReadBitAngle( 8 ), 0 );

	if ( !rd_sentry_gib.GetBool() )
		return;

	matrix3x4_t matOrigin;
	AngleMatrix( angles, vecOrigin, matOrigin );

	for ( int i = 0; i < NELEMS( s_szSentryGibs ); i++ )
	{
		Vector vecGibOrigin;
		VectorTransform( s_vecSentryGibOffset[i], matOrigin, vecGibOrigin );

		C_Gib *pGib = C_Gib::CreateClientsideGib( s_szSentryGibs[i], vecGibOrigin, RandomVector( -rd_sentry_gib_force.GetFloat(), rd_sentry_gib_force.GetFloat() ), RandomAngularImpulse( -rd_sentry_gib_spin.GetFloat(), rd_sentry_gib_spin.GetFloat() ), rd_sentry_gib_lifetime.GetFloat() + RandomFloat( 0, rd_sentry_gib_lifetime_jitter.GetFloat() ) );
		if ( pGib )
		{
			pGib->SetAbsAngles( angles );
		}
	}
}
USER_MESSAGE_REGISTER( SentryGib );
