#include "cbase.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "asw_util_shared.h"
#include <vgui/IInput.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextImage.h>
#include "animation.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "convar_serverbounded.h"
#include "rd_steam_input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_swarmopedia_timescale( "rd_swarmopedia_timescale", "0.3", FCVAR_ARCHIVE, "Speed for Swarmopedia specimen animations" );
ConVar rd_swarmopedia_grid( "rd_swarmopedia_grid", "0", FCVAR_ARCHIVE, "Draw a grid in the Swarmopedia model viewer" );
ConVar rd_swarmopedia_last_tab( "rd_swarmopedia_last_tab", "0", FCVAR_ARCHIVE, "Remembers last accessed tab index of the Swarmopedia screen." );
ConVar rd_collections_last_tab( "rd_collections_last_tab", "0", FCVAR_ARCHIVE, "Remembers last accessed tab index of the collections screen." );
extern ConVar rd_reduce_motion;
extern ConVar asw_weapon_pitch;
extern ConVar_ServerBounded *m_pitch;

vgui::DHANDLE<TabbedGridDetails> g_hCollectionFrame;
void LaunchCollectionsFrame()
{
	TabbedGridDetails *pFrame = g_hCollectionFrame;
	if ( pFrame )
	{
		pFrame->SetVisible( false );
		pFrame->MarkForDeletion();
		g_hCollectionFrame = NULL;
	}

	pFrame = new TabbedGridDetails();
	pFrame->SetTitle( "#rd_collection_title", true );
	pFrame->AddTab( new CRD_Collection_Tab_Inventory( pFrame, "#rd_collection_inventory_medals", "medal" ) );
	pFrame->RememberTabIndex( &rd_collections_last_tab );
	pFrame->UseMainMenuLayout( CRD_VGUI_Main_Menu_Top_Bar::BTN_INVENTORY );
	pFrame->ShowFullScreen();

	g_hCollectionFrame = pFrame;
}

void LaunchSwarmopediaFrame()
{
	TabbedGridDetails *pFrame = g_hCollectionFrame;
	if ( pFrame )
	{
		pFrame->SetVisible( false );
		pFrame->MarkForDeletion();
		g_hCollectionFrame = NULL;
	}

	pFrame = new TabbedGridDetails();
	pFrame->SetTitle( "#rd_collection_title", true );
	pFrame->AddTab( new CRD_Collection_Tab_Swarmopedia( pFrame, "#rd_collection_swarmopedia" ) );
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_weapons", NULL, ASW_INVENTORY_SLOT_PRIMARY ) );
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_equipment", NULL, ASW_INVENTORY_SLOT_EXTRA ) );
	pFrame->RememberTabIndex( &rd_swarmopedia_last_tab );
	pFrame->UseMainMenuLayout( CRD_VGUI_Main_Menu_Top_Bar::BTN_SWARMOPEDIA );
	pFrame->ShowFullScreen();

	g_hCollectionFrame = pFrame;
}

static int rd_collections_completion( const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH] )
{
	return 0;
}

CON_COMMAND_F_COMPLETION( rd_collections, "open collections view", FCVAR_CLIENTCMD_CAN_EXECUTE, rd_collections_completion )
{
	if ( args.ArgC() > 1 )
	{
		CmdMsg( "Usage: rd_collections\n" );
		return;
	}

	LaunchCollectionsFrame();
}

DECLARE_BUILD_FACTORY( CRD_Collection_StatLine );

CRD_Collection_StatLine::CRD_Collection_StatLine( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_pLblTitle = new vgui::Label( this, "LblTitle", L"" );
	m_pLblStat = new vgui::Label( this, "LblStat", L"" );
}

void CRD_Collection_StatLine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "resource/UI/CollectionStatLine.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_StatLine::SetLabel( const char *szLabel )
{
	m_pLblTitle->SetText( szLabel );
}

void CRD_Collection_StatLine::SetLabel( const wchar_t *wszLabel )
{
	m_pLblTitle->SetText( wszLabel );
}

void CRD_Collection_StatLine::SetValue( int64_t nValue )
{
	m_pLblStat->SetText( UTIL_RD_CommaNumber( nValue ) );
}

CRD_Swarmopedia_Model_Panel::CRD_Swarmopedia_Model_Panel( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	for ( int i = 0; i < MATERIAL_MAX_LIGHT_COUNT; i++ )
	{
		SetIdentityMatrix( m_LightToWorld[i] );
	}

	m_bDisplayChanged = false;
}

void CRD_Swarmopedia_Model_Panel::SetDisplay( const RD_Swarmopedia::Display *pDisplay )
{
	Assert( pDisplay && pDisplay->Models.Count() != 0 );
	if ( !pDisplay || pDisplay->Models.Count() == 0 )
	{
		return;
	}

	m_bDisplayChanged = true;
	SetLighting( pDisplay->LightingState );

	ClearMergeMDLs();

	// The parent class model is only used for sizing.
	SetMDL( pDisplay->Models[0]->ModelName );

	CStudioHdr RootHdr{ m_RootMDL.m_MDL.GetStudioHdr(), mdlcache };

	m_Models.SetCount( pDisplay->Models.Count() );

	FOR_EACH_VEC( pDisplay->Models, i )
	{
		const RD_Swarmopedia::Model *pModel = pDisplay->Models[i];

		const model_t *pWorldModel = modelinfo->FindOrLoadModel( pModel->ModelName );
		MDLHandle_t hStudioHdr = pWorldModel ? modelinfo->GetCacheHandle( pWorldModel ) : MDLHANDLE_INVALID;
		const studiohdr_t *pStudioHdr = hStudioHdr == MDLHANDLE_INVALID ? NULL : mdlcache->GetStudioHdr( hStudioHdr );
		if ( !pStudioHdr )
		{
			DevWarning( "Could not load model %s\n", pModel->ModelName.Get() );
			continue;
		}

		CStudioHdr studioHdr( pStudioHdr, mdlcache );

		m_Models[i].m_MDL.SetMDL( hStudioHdr );
		m_Models[i].m_SequenceOverlay.SetCount( pModel->Animations.Count() );
		FOR_EACH_VEC( m_Models[i].m_SequenceOverlay, j )
		{
			m_Models[i].m_SequenceOverlay[j].m_nSequenceIndex = LookupSequence( &studioHdr, pModel->Animations.GetElementName( j ) );
			m_Models[i].m_SequenceOverlay[j].m_flWeight = pModel->Animations.Element( j );
			Assert( m_Models[i].m_SequenceOverlay[j].m_nSequenceIndex != ACT_INVALID );
			if ( m_Models[i].m_SequenceOverlay[j].m_nSequenceIndex == ACT_INVALID )
			{
				DevWarning( "Model '%s' has no animation named '%s'\n", pModel->ModelName.Get(), pModel->Animations.GetElementName( j ) );
				m_Models[i].m_SequenceOverlay[j].m_nSequenceIndex = 0;
				m_Models[i].m_SequenceOverlay[j].m_flWeight = 0;
			}
		}

		for ( int j = 0; j < MAXSTUDIOPOSEPARAM; j++ )
		{
			m_Models[i].m_flPoseParameters[j] = 0.5f;
			if ( j < studioHdr.GetNumPoseParameters() )
			{
				const mstudioposeparamdesc_t &param = studioHdr.pPoseParameter( j );
				int iParam = pModel->PoseParameters.Find( param.pszName() );
				if ( pModel->PoseParameters.IsValidIndex( iParam ) )
				{
					m_Models[i].m_flPoseParameters[j] = pModel->PoseParameters.Element( iParam );
				}
				else if ( param.start < 0.0f && param.end > 0.0f )
				{
					m_Models[i].m_flPoseParameters[j] = param.start / ( param.start - param.end );
				}
			}
		}

		m_Models[i].m_MDL.m_nSkin = pModel->Skin;
		m_Models[i].m_MDL.m_Color = pModel->Color;
		m_Models[i].m_iBoneMerge = pModel->BoneMerge;
		Assert( m_Models[i].m_iBoneMerge < i );
		m_Models[i].m_bIsWeapon = pModel->IsWeapon;

		const QAngle angles( pModel->Pitch, pModel->Yaw, pModel->Roll );
		const Vector position( pModel->X, pModel->Y, pModel->Z );
		matrix3x4_t anglePos, scale;
		AngleMatrix( angles, position, anglePos );
		SetScaleMatrix( pModel->Scale, scale );
		ConcatTransforms( scale, anglePos, m_Models[i].m_MDLToWorld );

		FOR_EACH_MAP_FAST( pModel->BodyGroups, j )
		{
			::SetBodygroup( &studioHdr, m_Models[i].m_MDL.m_nBody, pModel->BodyGroups.Key( j ), pModel->BodyGroups.Element( j ) );
		}
	}
}

void CRD_Swarmopedia_Model_Panel::OnPaint3D()
{
	float flTime = rd_reduce_motion.GetBool() ? 4.5f : Plat_FloatTime() * ( m_bUseTimeScale ? rd_swarmopedia_timescale.GetFloat() : 1.0f );
	if ( m_eMode == MODE_SPINNER )
	{
		SetModelAnglesAndPosition( QAngle( 0.0f, flTime * 30.0f, 0.0f ), vec3_origin );
	}
	else if ( m_eMode == MODE_FULLSCREEN_MOUSE )
	{
		int x, y, w, h;
		vgui::input()->GetCursorPos( x, y );
		GetHudSize( w, h );

		int iSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

		float flMoveX{}, flMoveY{}, flLookX{}, flLookY{};
		g_RD_Steam_Input.GetGameAxes( iSlot, &flMoveX, &flMoveY, &flLookX, &flLookY );

		float xPan = ( x - w / 2.0f ) / h + flLookX / MAX_BUTTONSAMPLE / 2.0f * w / h;
		float yPan = ( y - h / 2.0f ) / h + flLookY / MAX_BUTTONSAMPLE / 2.0f;
		if ( rd_reduce_motion.GetBool() )
		{
			xPan = roundf( xPan * 2.0f ) / 2.0f;
			yPan = roundf( yPan * 2.0f ) / 2.0f;

			m_flSmoothXPan = xPan;
			m_flSmoothYPan = yPan;
		}
		else
		{
			m_flSmoothXPan = Lerp( MIN( gpGlobals->frametime * m_flPanSpeed, 1.0f ), m_flSmoothXPan, xPan );
			m_flSmoothYPan = Lerp( MIN( gpGlobals->frametime * m_flPanSpeed, 1.0f ), m_flSmoothYPan, yPan );

			xPan = m_flSmoothXPan;
			yPan = m_flSmoothYPan;
		}

		float flPitchInvert = Sign( m_pitch->GetFloat() );

		// we need to apply pitch *after* yaw or it looks weird.
		VMatrix mat1, mat2, mat3;
		MatrixFromAngles( m_angPanOrigin, mat1 );
		MatrixFromAngles( QAngle{ yPan * m_flPitchIntensity * flPitchInvert, 0.0f, 0.0f }, mat2 );
		MatrixMultiply( mat1, mat2, mat3 );

		MatrixFromAngles( QAngle{ 0.0f, 180.0f + xPan * m_flYawIntensity, 0.0f }, mat2 );
		MatrixMultiply( mat3, mat2, mat1 );

		QAngle angle;
		MatrixToAngles( mat1, angle );

		Vector origin = mat1.ApplyRotation( -m_vecCenter ) + m_vecCenter;
		SetModelAnglesAndPosition( angle, origin );
	}

	if ( m_bDisplayChanged )
	{
		m_bDisplayChanged = false;

		// Added optimization: only compute camera pos on new display.
		Vector vecPos, vecOffset;
		QAngle angRot( 32.0, 0.0, 0.0 );
		AngleVectors( angRot, &vecOffset );

		if ( m_bAutoPosition )
		{
			Vector vecMins, vecMaxs, vecOverallMins, vecOverallMaxs;
			FOR_EACH_VEC( m_Models, i )
			{
				GetMDLBoundingBox( &vecMins, &vecMaxs, m_Models[i].m_MDL.m_MDLHandle, m_Models[i].m_MDL.m_nSequence );
				vecOverallMins = i ? vecOverallMins.Min( vecMins ) : vecMins;
				vecOverallMaxs = i ? vecOverallMaxs.Max( vecMaxs ) : vecMaxs;
			}

			m_vecCenter = ( vecOverallMins + vecOverallMaxs ) * 0.5f;
			m_flRadius = m_vecCenter.DistTo( vecOverallMaxs );
		}

		VectorMA( m_vecCenter, -3.5f * m_flRadius, vecOffset, vecPos );

		SetCameraPositionAndAngles( vecPos, angRot );

		// Camera position is used in Paint, which calls this function.
		// If the model changes, we have one frame where we are using a camera position calculated using the old model.
		// Render that frame as blank and then immediately render another frame as a workaround.

		Paint();

		return;
	}

	CUtlMemory<matrix3x4_t[MAXSTUDIOBONES]> Bones{ 0, m_Models.Count() };

	FOR_EACH_VEC( m_Models, i )
	{
		matrix3x4_t mat;
		ConcatTransforms( m_RootMDL.m_MDLToWorld, m_Models[i].m_MDLToWorld, mat );
		CStudioHdr MergeHdr{ m_Models[i].m_MDL.GetStudioHdr(), mdlcache };
		m_Models[i].m_MDL.m_flTime = flTime;
		if ( m_Models[i].m_iBoneMerge == -1 )
		{
			m_Models[i].m_MDL.SetUpBones( mat, MergeHdr.numbones(), Bones[i], m_Models[i].m_flPoseParameters, m_Models[i].m_SequenceOverlay.Base(), m_Models[i].m_SequenceOverlay.Count() );
		}
		else
		{
			CStudioHdr ParentHdr{ m_Models[m_Models[i].m_iBoneMerge].m_MDL.GetStudioHdr(), mdlcache };
			matrix3x4_t ParentBones[MAXSTUDIOBONES];
			V_memcpy( ParentBones, Bones[m_Models[i].m_iBoneMerge], sizeof( ParentBones ) );
			if ( m_Models[i].m_bIsWeapon )
			{
				for ( int j = 0; j < ParentHdr.numbones(); j++ )
				{
					if ( !V_stricmp( ParentHdr.pBone( j )->pszName(), "ValveBiped.Bip01_R_Hand" ) )
					{
						matrix3x4_t matPitchUp;
						AngleMatrix( QAngle( asw_weapon_pitch.GetFloat(), 0, 0 ), matPitchUp );
						matrix3x4_t tmp = ParentBones[j];
						ConcatTransforms( tmp, matPitchUp, ParentBones[j] );

						break;
					}
				}
			}

			m_Models[i].m_MDL.SetupBonesWithBoneMerge( &MergeHdr, Bones[i], &ParentHdr, ParentBones, mat );
		}
		m_Models[i].m_MDL.Draw( mat, Bones[i], m_Models[i].m_iRenderFlags );
	}

	if ( rd_swarmopedia_grid.GetBool() )
	{
		DrawGrid();
	}
}

void CRD_Swarmopedia_Model_Panel::OnMouseDoublePressed( vgui::MouseCode code )
{
	// do nothing
}
