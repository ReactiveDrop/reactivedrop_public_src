#include "cbase.h"
#include "c_asw_queen.h"
#include "engine/IVDebugOverlay.h"
#include "asw_shareddefs.h"
#include "asw_util_shared.h"
#include "tier0/vprof.h"
#include "asw_vgui_queen_health.h"
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Queen, DT_ASW_Queen, CASW_Queen )
	RecvPropEHandle( RECVINFO( m_hQueenEnemy ) ),
	RecvPropBool( RECVINFO( m_bChestOpen ) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
	RecvPropBool( RECVINFO( m_bHealthBarEnabled ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ASW_Queen )
	DEFINE_PRED_FIELD( m_flPoseParameter, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

ConVar asw_queen_health_bars( "asw_queen_health_bars", "5", FCVAR_CHEAT | FCVAR_REPLICATED, "Number of health bar segments for queen", true, 1, false, 0 );

C_ASW_Queen::C_ASW_Queen()
{
	m_fLastShieldPose = 0;
	m_fLastHeadYaw = 0;

	for ( int i = 0; i < MAXSTUDIOPOSEPARAM; i++ )
	{
		m_flClientPoseParameter[i] = 0;
	}
}

C_ASW_Queen::~C_ASW_Queen()
{

}

void C_ASW_Queen::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		// We want to think every frame.
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		FOR_EACH_VALID_SPLITSCREEN_PLAYER( hh )
		{
			ACTIVE_SPLITSCREEN_PLAYER_GUARD( hh );

			if ( m_bHealthBarEnabled )
			{
				// launch panel to show our health
				CASW_VGUI_Queen_Health_Panel *m_pQueenHealthPanel = new CASW_VGUI_Queen_Health_Panel( GetFullscreenClientMode()->GetViewport(), "QueenHealthPanel", this );
				vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
				m_pQueenHealthPanel->SetScheme( scheme );
				m_pQueenHealthPanel->SetVisible( true );
			}
		}
	}
}

void C_ASW_Queen::ClientThink()
{
	BaseClass::ClientThink();

	UpdatePoseParams();
}

Vector  C_ASW_Queen::BodyDirection3D( void )
{
	QAngle angles = GetAbsAngles();

	// FIXME: cache this
	Vector vBodyDir;
	AngleVectors( angles, &vBodyDir );
	return vBodyDir;
}

float C_ASW_Queen::VecToYaw( const Vector &vecDir )
{
	if (vecDir.x == 0 && vecDir.y == 0 && vecDir.z == 0)
		return GetLocalAngles().y;

	return UTIL_VecToYaw( vecDir );
}

void C_ASW_Queen::UpdatePoseParams()
{
	if ( IsFrozen() )
	{
		// don't look around while fully frozen!
		return;
	}

	int pose_index = LookupPoseParameter( "head_yaw" );
	if (pose_index >= 0)
	{
		m_flClientPoseParameter[pose_index] = 0;
	}

	float shield = m_fLastShieldPose; //GetPoseParameter( LookupPoseParameter("shield_open") );
	float targetshield = m_bChestOpen ? 1.0f : 0.0f;
	
	if (shield != targetshield)
	{
		shield = ASW_Linear_Approach(shield, targetshield, gpGlobals->frametime * 3.0f);		
		m_fLastShieldPose = shield;
	}	
	pose_index = LookupPoseParameter( "shield_open" );
	if (pose_index >= 0)
	{
		m_flClientPoseParameter[pose_index] = shield;
	}
}

// autoaim goes to her mouth
const Vector& C_ASW_Queen::GetAimTargetPos(const Vector &vecFiringSrc, bool bWeaponPrefersFlatAiming)
{	
	return GetAbsOrigin();

	static Vector vecSpitSource;
	QAngle angSpit;
	if (!GetAttachment( "SpitSource", vecSpitSource, angSpit ))
	{		
		return WorldSpaceCenter();
	}
	return vecSpitSource;
}

void C_ASW_Queen::GetPoseParameters( CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM])
{
	if ( !pStudioHdr )
		return;

	for( int i=0; i < pStudioHdr->GetNumPoseParameters(); i++)
	{
		poseParameter[i] = m_flClientPoseParameter[i];
	}
}