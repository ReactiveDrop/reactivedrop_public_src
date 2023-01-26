#include "cbase.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"
#include "asw_util_shared.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/TextImage.h>
#include "animation.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_swarmopedia_timescale( "rd_swarmopedia_timescale", "0.3", FCVAR_ARCHIVE, "Speed for Swarmopedia specimen animations" );
ConVar rd_swarmopedia_grid( "rd_swarmopedia_grid", "0", FCVAR_ARCHIVE, "Draw a grid in the Swarmopedia model viewer" );
ConVar rd_collections_last_tab( "rd_collections_last_tab", "0", FCVAR_ARCHIVE, "Remembers last accessed tab index of the collections screen." );
extern ConVar rd_reduce_motion;

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
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_weapons", NULL, ASW_INVENTORY_SLOT_PRIMARY ) );
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_equipment", NULL, ASW_INVENTORY_SLOT_EXTRA ) );
	pFrame->AddTab( new CRD_Collection_Tab_Swarmopedia( pFrame, "#rd_collection_swarmopedia" ) );
	pFrame->RememberTabIndex( &rd_collections_last_tab );
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
	m_LightingState = pDisplay->LightingState;

	ClearMergeMDLs();

	// The parent class model is only used for sizing.
	SetMDL( pDisplay->Models[0]->ModelName );

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
		m_Models[i].m_MDL.m_nSequence = LookupSequence( &studioHdr, pModel->Animation );
		Assert( m_Models[i].m_MDL.m_nSequence != -1 || pModel->Animation.IsEmpty() );
		if ( m_Models[i].m_MDL.m_nSequence == -1 )
		{
			Assert( studioHdr.GetNumSeq() > 0 );
			m_Models[i].m_MDL.m_nSequence = 0;
		}

		m_Models[i].m_MDL.m_nSkin = pModel->Skin;
		m_Models[i].m_MDL.m_Color = pModel->Color;

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
	float flTime = rd_reduce_motion.GetBool() ? 4.5f : Plat_FloatTime() * rd_swarmopedia_timescale.GetFloat();
	SetModelAnglesAndPosition( QAngle( 0.0f, flTime * 30.0f, 0.0f ), vec3_origin );

	if ( m_bDisplayChanged )
	{
		m_bDisplayChanged = false;

		// Added optimization: only compute camera pos on new display.
		Vector vecPos, vecOffset;
		QAngle angRot( 32.0, 0.0, 0.0 );
		AngleVectors( angRot, &vecOffset );

		Vector vecMins, vecMaxs, vecOverallMins, vecOverallMaxs;
		FOR_EACH_VEC( m_Models, i )
		{
			GetMDLBoundingBox( &vecMins, &vecMaxs, m_Models[i].m_MDL.m_MDLHandle, m_Models[i].m_MDL.m_nSequence );
			vecOverallMins = i ? vecOverallMins.Min( vecMins ) : vecMins;
			vecOverallMaxs = i ? vecOverallMaxs.Max( vecMaxs ) : vecMaxs;
		}

		Vector vecCenter = ( vecOverallMins + vecOverallMaxs ) * 0.5f;
		float flRadius = vecCenter.DistTo( vecOverallMaxs );

		VectorMA( vecCenter, -3.5f * flRadius, vecOffset, vecPos );

		SetCameraPositionAndAngles( vecPos, angRot );

		// Camera position is used in Paint, which calls this function.
		// If the model changes, we have one frame where we are using a camera position calculated using the old model.
		// Render that frame as blank and then immediately render another frame as a workaround.

		Paint();

		return;
	}

	FOR_EACH_VEC( m_Models, i )
	{
		matrix3x4_t mat;
		ConcatTransforms( m_RootMDL.m_MDLToWorld, m_Models[i].m_MDLToWorld, mat );
		m_Models[i].m_MDL.m_flTime = flTime;
		m_Models[i].m_MDL.Draw( mat );
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
