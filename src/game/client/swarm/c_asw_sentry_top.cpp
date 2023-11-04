#include "cbase.h"
#include "c_asw_sentry_top.h"
#include "c_asw_sentry_base.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "c_te_legacytempents.h"
#include "c_asw_fx.h"
#include "c_user_message_register.h"
#include "ai_debug_shared.h"
#include "c_asw_player.h"
#include "c_asw_weapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Sentry_Top, DT_ASW_Sentry_Top, CASW_Sentry_Top )
	RecvPropEHandle( RECVINFO( m_hSentryBase ) ),
	RecvPropInt( RECVINFO( m_iSentryAngle ) ),
	RecvPropFloat( RECVINFO( m_fDeployYaw ) ),
	RecvPropFloat( RECVINFO( m_fCenterAimYaw ) ),
	RecvPropFloat( RECVINFO( m_fGoalPitch ) ),
	RecvPropFloat( RECVINFO( m_fGoalYaw ) ),
	RecvPropBool( RECVINFO( m_bLowAmmo ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ASW_Sentry_Top )

END_PREDICTION_DATA()

C_ASW_Sentry_Top::C_ASW_Sentry_Top() :
	m_iv_fCenterAimYaw( "C_ASW_Sentry_Top::m_iv_fCenterAimYaw" ),
	m_iv_fGoalYaw( "C_ASW_Sentry_Top::m_iv_fGoalYaw" ),
	m_iv_fGoalPitch( "C_ASW_Sentry_Top::m_iv_fGoalPitch" )
{
	UseClientSideAnimation();

	AddVar( &m_fCenterAimYaw, &m_iv_fCenterAimYaw, LATCH_SIMULATION_VAR );
	AddVar( &m_fGoalYaw, &m_iv_fGoalYaw, LATCH_SIMULATION_VAR );
	AddVar( &m_fGoalPitch, &m_iv_fGoalPitch, LATCH_SIMULATION_VAR );

	m_bSpawnedDisplayEffects = false;

	m_iPoseParamPitch = -2;
	m_iPoseParamYaw = -2;
	m_iPoseParamAmmoRemaining = -2;
}

C_ASW_Sentry_Top::~C_ASW_Sentry_Top()
{
	if ( m_hRadiusDisplay )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_hRadiusDisplay );
		m_hRadiusDisplay = NULL;
	}

	if ( m_hWarningLight )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_hWarningLight );
		m_hWarningLight = NULL;
	}

	if ( m_hPilotLight )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_hPilotLight );
		m_hPilotLight = NULL;
	}

	for ( int i = 0; i < NELEMS( m_hWeaponAccessory ); i++ )
	{
		if ( m_hWeaponAccessory[i].Get() )
		{
			UTIL_Remove( m_hWeaponAccessory[i].Get() );
			m_hWeaponAccessory[i] = NULL;
		}
	}
}

void C_ASW_Sentry_Top::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		C_ASW_Sentry_Base *pBase = GetSentryBase();
		if ( pBase && pBase->IsInventoryEquipSlotValid() )
		{
			static KeyValues *s_pKVAccessoryPosition[kGUNTYPE_MAX]{};
			constexpr const char *const s_szAccessoryPositionFiles[kGUNTYPE_MAX] =
			{
				"scripts/strange_device_positions_sentry_machinegun.txt",
				"scripts/strange_device_positions_sentry_cannon.txt",
				"scripts/strange_device_positions_sentry_flamer.txt",
				"scripts/strange_device_positions_sentry_icer.txt",
#ifdef RD_7A_WEAPONS
				"scripts/strange_device_positions_sentry_railgun.txt",
#endif
			};

			C_RD_Weapon_Accessory::CreateWeaponAccessories( this, pBase->m_hOriginalOwnerPlayer->m_EquippedItemDataDynamic[pBase->m_iInventoryEquipSlot], m_hWeaponAccessory, s_pKVAccessoryPosition[pBase->m_nGunType], s_szAccessoryPositionFiles[pBase->m_nGunType] );
		}

		m_fAimPitch = -30.0f; // should match the angle that the build animation uses
		m_fCameraYaw = 0.0f;

		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	if ( m_bLowAmmo && !m_hWarningLight )
	{
		m_hWarningLight = ParticleProp()->Create( "sentry_light_lowammo", PATTACH_ABSORIGIN_FOLLOW );
		if ( m_hWarningLight )
		{
			ParticleProp()->AddControlPoint( m_hWarningLight, 0, this, PATTACH_POINT_FOLLOW, "attach_light" );
		}
	}

	BaseClass::OnDataChanged( updateType );
}

void C_ASW_Sentry_Top::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
	
	if ( m_hRadiusDisplay )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_hRadiusDisplay );
		m_hRadiusDisplay = NULL;
	}

	if ( m_hWarningLight )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_hWarningLight );
		m_hWarningLight = NULL;
	}

	if ( m_hPilotLight )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_hPilotLight );
		m_hPilotLight = NULL;
	}

	for ( int i = 0; i < NELEMS( m_hWeaponAccessory ); i++ )
	{
		if ( m_hWeaponAccessory[i].Get() )
		{
			UTIL_Remove( m_hWeaponAccessory[i].Get() );
			m_hWeaponAccessory[i] = NULL;
		}
	}
}

void C_ASW_Sentry_Top::ClientThink()
{
	BaseClass::ClientThink();

	UpdatePose();
	Scan();
}

int C_ASW_Sentry_Top::GetMuzzleAttachment( void )
{
	return LookupAttachment( "muzzle" );
}

float C_ASW_Sentry_Top::GetMuzzleFlashScale( void )
{
	return 2.0f;
}

void C_ASW_Sentry_Top::ProcessMuzzleFlashEvent()
{
	// attach muzzle flash particle system effect
	int iAttachment = GetMuzzleAttachment();

	if ( iAttachment > 0 )
	{
		float flScale = GetMuzzleFlashScale();
		FX_ASW_MuzzleEffectAttached( flScale, GetRefEHandle(), iAttachment, NULL, false );
	}

	BaseClass::ProcessMuzzleFlashEvent();
}

void C_ASW_Sentry_Top::ASWSentryTracer( const Vector &vecEnd )
{
	MDLCACHE_CRITICAL_SECTION();
	Vector vecStart;
	QAngle vecAngles;

	if ( IsDormant() )
		return;

	C_BaseAnimating::PushAllowBoneAccess( true, false, "sentgun" );

	// Get the muzzle origin
	if ( !GetAttachment( GetMuzzleAttachment(), vecStart, vecAngles ) )
	{
		return;
	}

	ASWDoParticleTracer( "tracer_autogun", vecStart, vecEnd );

	C_BaseAnimating::PopBoneAccess( "sentgun" );

	ResetSequence( SelectWeightedSequence( ACT_OBJ_RUNNING ) );
}

void __MsgFunc_ASWSentryTracer( bf_read &msg )
{
	int iSentry = msg.ReadShort();
	C_ASW_Sentry_Top *pSentry = dynamic_cast< C_ASW_Sentry_Top * >( ClientEntityList().GetEnt( iSentry ) );

	Vector vecEnd;
	vecEnd.x = msg.ReadFloat();
	vecEnd.y = msg.ReadFloat();
	vecEnd.z = msg.ReadFloat();

	if ( pSentry )
	{
		pSentry->ASWSentryTracer( vecEnd );
	}
}
USER_MESSAGE_REGISTER( ASWSentryTracer );

C_ASW_Sentry_Base *C_ASW_Sentry_Top::GetSentryBase()
{
	return m_hSentryBase.Get();
}

void C_ASW_Sentry_Top::Scan()
{
	C_ASW_Sentry_Base *RESTRICT pBase = GetSentryBase();
	if ( !pBase )
	{
		if ( m_hRadiusDisplay )
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_hRadiusDisplay );
			m_hRadiusDisplay = NULL;
		}
		if ( m_hWarningLight )
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_hWarningLight );
			m_hWarningLight = NULL;
		}
		return;
	}

	if ( pBase->GetAmmo() <= 0 )
	{
		if ( m_hRadiusDisplay )
		{
			ParticleProp()->StopEmissionAndDestroyImmediately( m_hRadiusDisplay );
			m_hRadiusDisplay = NULL;

			if ( m_hWarningLight )
			{
				ParticleProp()->StopEmissionAndDestroyImmediately( m_hWarningLight );
				m_hWarningLight = NULL;
			}

			if ( !m_hWarningLight )
			{
				m_hWarningLight = ParticleProp()->Create( "sentry_light_noammo", PATTACH_ABSORIGIN_FOLLOW );
				if ( m_hWarningLight )
				{
					ParticleProp()->AddControlPoint( m_hWarningLight, 0, this, PATTACH_POINT_FOLLOW, "attach_light" );
				}
			}
		}
		return;
	}
	else if ( !m_bLowAmmo && m_hWarningLight )
	{
		m_hWarningLight->StopEmission( false, false, true );
		m_hWarningLight = NULL;
	}

	QAngle	scanAngle;
	Vector	forward;
	Vector	vecEye = pBase->GetAbsOrigin() + Vector( 0, 0, 38 );// + m_vecLightOffset;

	if ( !m_hRadiusDisplay )
	{
		// this sets up the outter, "heavier" lines that make up the edges and the middle
		m_hRadiusDisplay = ParticleProp()->Create( "sentry_radius_display_beam", PATTACH_CUSTOMORIGIN );

		// Draw the outer extents
		scanAngle = pBase->GetAbsAngles();
		scanAngle.y -= GetSentryAngle();
		AngleVectors( scanAngle, &forward, NULL, NULL );
		CreateRadiusBeamEdges( vecEye, forward, 1 );

		scanAngle = pBase->GetAbsAngles();
		scanAngle.y += GetSentryAngle();
		AngleVectors( scanAngle, &forward, NULL, NULL );
		CreateRadiusBeamEdges( vecEye, forward, 2 );

		// scanAngle = pBase->GetAbsAngles();
		// AngleVectors( scanAngle, &forward, NULL, NULL );
		CreateRadiusBeamEdges( vecEye, pBase->Forward(), 3 );

		// create the sweeping beam control points
		ParticleProp()->AddControlPoint( m_hRadiusDisplay, 4, this, PATTACH_CUSTOMORIGIN );
		m_hRadiusDisplay->SetControlPointEntity( 4, this );
		ParticleProp()->AddControlPoint( m_hRadiusDisplay, 5, this, PATTACH_CUSTOMORIGIN );
		m_hRadiusDisplay->SetControlPointEntity( 5, this );
	}

	if ( m_hRadiusDisplay )
	{
		m_hRadiusDisplay->SetControlPoint( 0, vecEye );

		// now move the sweeping beams
		QAngle baseAngle = pBase->GetAbsAngles();
		baseAngle.y = m_fCenterAimYaw;

		AngleVectors( baseAngle + QAngle( 0, -GetSentryAngle(), 0 ), &forward, NULL, NULL );
		AdjustRadiusBeamEdges( vecEye, forward, 1 );

		AngleVectors( baseAngle + QAngle( 0, GetSentryAngle(), 0 ), &forward, NULL, NULL );
		AdjustRadiusBeamEdges( vecEye, forward, 2 );

		AngleVectors( baseAngle, &forward, NULL, NULL );
		AdjustRadiusBeamEdges( vecEye, forward, 3 );

		scanAngle = baseAngle;
		scanAngle.y += GetScanAngle();

		Vector	vecBase = pBase->GetAbsOrigin() + Vector( 0, 0, 2 );// + m_vecLightOffset;
		AngleVectors( scanAngle, &forward, NULL, NULL );

		m_hRadiusDisplay->SetControlPoint( 4, vecBase + forward * 190.0f );
		m_hRadiusDisplay->SetControlPoint( 5, vecBase + forward * 80.0f );
	}
}

void C_ASW_Sentry_Top::CreateRadiusBeamEdges( const Vector &vecStart, const Vector &vecDir, int iControlPoint )
{
	if ( !m_hRadiusDisplay )
		return;

	ParticleProp()->AddControlPoint( m_hRadiusDisplay, iControlPoint, this, PATTACH_CUSTOMORIGIN );
	m_hRadiusDisplay->SetControlPointEntity( iControlPoint, this );

	AdjustRadiusBeamEdges( vecStart, vecDir, iControlPoint );
}

void C_ASW_Sentry_Top::AdjustRadiusBeamEdges( const Vector &vecStart, const Vector &vecDir, int iControlPoint )
{
	CTraceFilterSkipTwoEntities traceFilter( this, GetSentryBase(), COLLISION_GROUP_DEBRIS );

	trace_t tr;
	AI_TraceLine( vecStart, vecStart + vecDir * 500.0f, MASK_SHOT, &traceFilter, &tr );

	m_hRadiusDisplay->SetControlPoint( iControlPoint, tr.endpos );
}

float C_ASW_Sentry_Top::GetDeployYaw()
{
	float fDeployYaw = m_fDeployYaw;
	if ( GetMoveParent() )
	{
		fDeployYaw += GetMoveParent()->GetAbsAngles().y;
	}

	float fCurrentYaw = GetAbsAngles().y;

	return fCurrentYaw + anglemod( fDeployYaw - fCurrentYaw );
}
