#include "cbase.h"
#include "vitemshowcase.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "asw_equipment_list.h"
#include "asw_model_panel.h"
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/RichText.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include "engine/IEngineSound.h"
#include "asw_weapon_parse.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


using namespace vgui;
using namespace BaseModUI;

extern const MaterialLightingState_t &SwarmopediaDefaultLightingState();

extern ConVar rd_reduce_motion;

ItemShowcase::ItemShowcase( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pBanner = new CNB_Gradient_Bar( this, "Banner" );
	m_pTitle = new Label( this, "Title", "" );
	m_pWeaponLabel = new Label( this, "WeaponLabel", "" );
	m_pSubTitle = new Label( this, "SubTitle", "" );
	m_pBackButton = new CNB_Button( this, "BackButton", "", this, "BackButton" );
	m_pBackButton->SetControllerButton( KEY_XBUTTON_A );

	m_pItemModelPanel = new CASW_Model_Panel( this, "ItemModelPanel" );
	m_pItemModelPanel->m_bShouldPaint = false;
	m_pItemModelPanel->SetMouseInputEnabled( false );
	m_pItemModelPanel->SetLighting( SwarmopediaDefaultLightingState() );

	m_pDescriptionArea = new RichText( this, "DescriptionArea" );
	m_pDescriptionArea->SetUnusedScrollbarInvisible( true );
	m_pDescriptionArea->SetPaintBackgroundEnabled( false );
	m_pDescriptionArea->SetCursor( vgui::dc_arrow );
	m_pDescriptionArea->SetPanelInteractive( false );

	m_iWeaponLabelX = 0;
	m_iDescriptionAreaY = 0;
	m_iRepositionDescription = 0;

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
	SetPostChildPaintEnabled( true );
}

ItemShowcase::~ItemShowcase()
{
}

void ItemShowcase::OnOpen()
{
	BaseClass::OnOpen();

	if ( ASWGameRules() && dynamic_cast<CBaseModPanel *>( GetParent() ) )
	{
		SetVisible( false );
		ivgui()->AddTickSignal( GetVPanel() );
		return;
	}

	m_bNeedsMoveToFront = true;
	m_bShowWeaponOnNextFrame = true;

	SetAlpha( 0 );
	GetAnimationController()->RunAnimationCommand( this, "alpha", 255, 0, 0.5f, AnimationController::INTERPOLATOR_LINEAR );

	const char *szSoundName = soundemitterbase->GetWavFileForSound( "ASW_XP.LevelUp", GENDER_NONE );
	enginesound->EmitAmbientSound( szSoundName, 1.0f, 100, 0, 0.5f );
}

void ItemShowcase::OnTick()
{
	BaseClass::OnTick();

	if ( CBaseModFrame *pMainMenu = CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) )
	{
		SetParent( pMainMenu );
	}

	if ( !ASWGameRules() || !dynamic_cast< CBaseModPanel * >( GetParent() ) )
	{
		SetVisible( true );
		ivgui()->RemoveTickSignal( GetVPanel() );

		CBaseModPanel::GetSingleton().SetActiveWindow( this );
	}
}

void ItemShowcase::OnThink()
{
	BaseClass::OnThink();

	if ( m_QueueType.Count() == 0 )
	{
		m_bShowWeaponOnNextFrame = false;
		MarkForDeletion();
		return;
	}

	if ( m_bShowWeaponOnNextFrame )
	{
		const char *szSoundName = soundemitterbase->GetWavFileForSound( "ASWInterface.SelectWeapon", GENDER_NONE );
		enginesound->EmitAmbientSound( szSoundName, 1.0f, 100, 0, 0.25f );

		switch ( m_QueueType[0] )
		{
		case MODE_INSPECT:
		case MODE_ITEM_DROP:
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_Queue[0]->ItemDefID );

			m_pTitle->SetText( m_QueueType[0] == MODE_ITEM_DROP ? "#nb_item_dropped" : "#nb_item_inspect" );

			m_pWeaponLabel->SetText( pDef->Name );
			m_pWeaponLabel->SetVisible( true );

			m_pItemModelPanel->m_bShouldPaint = false;
			m_pItemModelPanel->SetVisible( false );

			m_pSubTitle->SetText( pDef->DisplayType );
			CRD_ItemInstance instance{ *m_Queue[0] };
			instance.FormatDescription( m_pDescriptionArea );

			break;
		}
		case MODE_UNLOCK_REGULAR_WEAPON:
		case MODE_UNLOCK_EXTRA_WEAPON:
		{
			m_pTitle->SetText( m_QueueType[0] == MODE_UNLOCK_REGULAR_WEAPON ? "#nb_weapon_unlocked" : "#nb_equipment_unlocked" );

			KeyValues::AutoDelete pKV( "ItemModelPanel" );
			pKV->SetInt( "fov", 20 );
			pKV->SetInt( "start_framed", 0 );
			pKV->SetInt( "disable_manipulation", 1 );
			m_pItemModelPanel->ApplySettings( pKV );

			m_pItemModelPanel->ClearMergeMDLs();

			CASW_EquipItem *pItem = m_QueueType[0] == MODE_UNLOCK_REGULAR_WEAPON ?
				g_ASWEquipmentList.GetRegular( m_QueueExtra[0] ) :
				g_ASWEquipmentList.GetExtra( m_QueueExtra[0] );
			CASW_WeaponInfo *pWeaponData = pItem ? g_ASWEquipmentList.GetWeaponDataFor( pItem->m_szEquipClass ) : NULL;
			if ( !pWeaponData )
			{
				m_pItemModelPanel->m_bShouldPaint = false;
				m_pItemModelPanel->SetVisible( false );

				m_pWeaponLabel->SetVisible( false );
				return;
			}

			if ( V_stricmp( pWeaponData->szDisplayModel, "" ) )
			{
				m_pItemModelPanel->SetMDL( pWeaponData->szDisplayModel );
				if ( V_stricmp( pWeaponData->szDisplayModel2, "" ) )
				{
					m_pItemModelPanel->SetMergeMDL( pWeaponData->szDisplayModel2 );
				}
			}
			else
			{
				m_pItemModelPanel->SetMDL( pWeaponData->szWorldModel );
			}

			m_pSubTitle->SetText( pItem->m_szAttributeDescription );
			m_pDescriptionArea->SetText( pItem->m_szDescription1 );

			int nSkin = 0;
			if ( pWeaponData->m_iDisplayModelSkin > 0 )
				nSkin = pWeaponData->m_iDisplayModelSkin;
			else
				nSkin = pWeaponData->m_iPlayerModelSkin;

			m_pItemModelPanel->SetSkin( nSkin );
			m_pItemModelPanel->SetModelAnim( m_pItemModelPanel->FindAnimByName( "idle" ) );
			m_pItemModelPanel->m_bShouldPaint = true;
			m_pItemModelPanel->SetVisible( true );

			m_pWeaponLabel->SetVisible( true );
			m_pWeaponLabel->SetText( pItem->m_szLongName );

			break;
		}
		}

		int x, y;
		m_pItemModelPanel->GetPos( x, y );
		m_pItemModelPanel->SetPos( m_iWeaponLabelX, y );
		GetAnimationController()->RunAnimationCommand( m_pItemModelPanel, "xpos", m_iWeaponLabelX - m_pDescriptionArea->GetWide() / 2, 0.25f, 0.5f, AnimationController::INTERPOLATOR_ACCEL );

		m_pWeaponLabel->GetPos( x, y );
		m_pWeaponLabel->SetPos( m_iWeaponLabelX, y );
		GetAnimationController()->RunAnimationCommand( m_pWeaponLabel, "xpos", m_iWeaponLabelX - m_pDescriptionArea->GetWide() / 2, 0.25f, 0.5f, AnimationController::INTERPOLATOR_ACCEL );

		m_pSubTitle->GetPos( x, y );
		m_pSubTitle->SetPos( m_iWeaponLabelX, y );
		GetAnimationController()->RunAnimationCommand( m_pSubTitle, "xpos", m_iWeaponLabelX - m_pDescriptionArea->GetWide() / 2, 0.25f, 0.5f, AnimationController::INTERPOLATOR_ACCEL );

		m_pDescriptionArea->SetAlpha( 0 );
		GetAnimationController()->RunAnimationCommand( m_pDescriptionArea, "alpha", 255, 0.75f, 0.25f, AnimationController::INTERPOLATOR_ACCEL );

		m_bShowWeaponOnNextFrame = false;
		m_iRepositionDescription = 3;
	}

	if ( m_iRepositionDescription > 0 && --m_iRepositionDescription == 0 )
	{
		m_pDescriptionArea->SetToFullHeight();
		int x, y;
		m_pDescriptionArea->GetPos( x, y );
		m_pDescriptionArea->SetPos( x, m_iDescriptionAreaY - m_pDescriptionArea->GetTall() / 2 );
	}

	switch ( m_QueueType[0] )
	{
	case MODE_INSPECT:
	case MODE_ITEM_DROP:
	{
		break;
	}
	case MODE_UNLOCK_REGULAR_WEAPON:
	case MODE_UNLOCK_EXTRA_WEAPON:
	{
		Vector vecPos( -275, 0, 170 );
		QAngle angRot( 32, 0, 0 );
		CASW_WeaponInfo *pWeaponData = g_ASWEquipmentList.GetWeaponDataFor(
			m_QueueType[0] == MODE_UNLOCK_REGULAR_WEAPON ?
			g_ASWEquipmentList.GetRegular( m_QueueExtra[0] )->m_szEquipClass :
			g_ASWEquipmentList.GetExtra( m_QueueExtra[0] )->m_szEquipClass );
		if ( pWeaponData )
		{
			vecPos.z += pWeaponData->m_flModelPanelZOffset;
		}

		Vector vecBoundsMins, vecBoundsMax;
		m_pItemModelPanel->GetBoundingBox( vecBoundsMins, vecBoundsMax );
		int iMaxBounds = -vecBoundsMins.x + vecBoundsMax.x;
		iMaxBounds = MAX( iMaxBounds, -vecBoundsMins.y + vecBoundsMax.y );
		iMaxBounds = MAX( iMaxBounds, -vecBoundsMins.z + vecBoundsMax.z );
		vecPos *= ( float )iMaxBounds / 64.0f;

		m_pItemModelPanel->SetCameraPositionAndAngles( vecPos, angRot );
		m_pItemModelPanel->SetModelAnglesAndPosition( QAngle( 0, rd_reduce_motion.GetBool() ? 0 : gpGlobals->curtime * 45, 0 ), vec3_origin );

		break;
	}
	}
}

void ItemShowcase::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "BackButton" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_A, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void ItemShowcase::OnKeyCodePressed( KeyCode keycode )
{
	int userId = GetJoystickForCode( keycode );
	KeyCode code = GetBaseButtonCode( keycode );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( userId );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
		if ( m_Queue.Count() <= 1 )
		{
			if ( !NavigateBack() )
			{
				Close();
			}
		}
		else
		{
			delete m_Queue[0];
			m_Queue.Remove( 0 );
			m_QueueExtra.Remove( 0 );
			m_QueueType.Remove( 0 );
			m_bShowWeaponOnNextFrame = true;
		}
		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void ItemShowcase::OnKeyCodeTyped( KeyCode code )
{
	switch ( code )
	{
	case KEY_SPACE:
	case KEY_ENTER:
	case KEY_ESCAPE:
		return OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_A, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}

	BaseClass::OnKeyTyped( code );
}

void ItemShowcase::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_iWeaponLabelX == 0 )
	{
		int y;
		m_pWeaponLabel->GetPos( m_iWeaponLabelX, y );
	}

	if ( m_iDescriptionAreaY == 0 )
	{
		int x;
		m_pDescriptionArea->GetPos( x, m_iDescriptionAreaY );
		m_iDescriptionAreaY += m_pDescriptionArea->GetTall() / 2;
	}
}

void ItemShowcase::PaintBackground()
{
	float flAlpha = 200.0f / 255.0f;
	surface()->DrawSetColor( Color( 16, 32, 46, 230 * flAlpha ) );
	surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );

	if ( m_bNeedsMoveToFront )
	{
		ipanel()->MoveToFront( GetVPanel() );
		m_bNeedsMoveToFront = false;
	}
}

void ItemShowcase::PostChildPaint()
{
	BaseClass::PostChildPaint();

	switch ( m_QueueType[0] )
	{
	case MODE_INSPECT:
	case MODE_ITEM_DROP:
	{
		vgui::IImage *pIcon = m_Queue[0]->GetIcon();

		int x, y;
		m_pItemModelPanel->GetPos( x, y );
		int w, t;
		m_pItemModelPanel->GetSize( w, t );
		x += w / 6;
		t -= w / 3;
		w -= w / 3;

		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawSetTexture( pIcon ? pIcon->GetID() : 0 );
		surface()->DrawTexturedRect( x, y, x + w, y + t );

		break;
	}
	case MODE_UNLOCK_REGULAR_WEAPON:
	case MODE_UNLOCK_EXTRA_WEAPON:
	{
		break;
	}
	}
}

void ItemShowcase::ShowItems( SteamInventoryResult_t hResult, int iStart, int iCount, Mode_t mode )
{
	SteamItemDetails_t itemDetails{};
	uint32 nItemCount{ 1 };
	if ( SteamInventory()->GetResultItems( hResult, &itemDetails, &nItemCount ) && nItemCount == 1 && itemDetails.m_itemId == 0 )
	{
		nItemCount = 0;
	}

	Assert( iStart >= 0 && iStart <= int( nItemCount ) );
	if ( iCount == -1 )
	{
		iCount = nItemCount;
	}
	Assert( iCount >= 0 && iStart + iCount <= int( nItemCount ) );

	if ( iCount == 0 )
	{
		return;
	}

	ItemShowcase *pItemShowcase = assert_cast<ItemShowcase *>( CBaseModPanel::GetSingleton().GetWindow( WT_ITEMSHOWCASE ) );
	if ( !pItemShowcase )
	{
		CBaseModFrame *pCaller = CUIGameData::Get()->GetParentWindowForSystemMessageBox();
		pItemShowcase = assert_cast< ItemShowcase * >( CBaseModPanel::GetSingleton().OpenWindow( WT_ITEMSHOWCASE, pCaller, false ) );
	}

	for ( int i = 0; i < iCount; i++ )
	{
		ReactiveDropInventory::ItemInstance_t *pInstance = new ReactiveDropInventory::ItemInstance_t{ hResult, uint32( iStart + i ) };
		if ( pInstance->Quantity == 0 )
		{
			delete pInstance;
			continue;
		}
		( void )ReactiveDropInventory::GetItemDef( pInstance->ItemDefID );
		pItemShowcase->m_Queue.AddToTail( pInstance );
		pItemShowcase->m_QueueExtra.AddToTail( 0 );
		pItemShowcase->m_QueueType.AddToTail( mode );
	}
}

void ItemShowcase::ShowItems( int iWeapon, Mode_t mode )
{
	ItemShowcase *pItemShowcase = assert_cast< ItemShowcase * >( CBaseModPanel::GetSingleton().GetWindow( WT_ITEMSHOWCASE ) );
	if ( !pItemShowcase )
	{
		CBaseModFrame *pCaller = CUIGameData::Get()->GetParentWindowForSystemMessageBox();
		pItemShowcase = assert_cast< ItemShowcase * >( CBaseModPanel::GetSingleton().OpenWindow( WT_ITEMSHOWCASE, pCaller, false ) );
	}

	pItemShowcase->m_Queue.AddToTail( NULL );
	pItemShowcase->m_QueueExtra.AddToTail( iWeapon );
	pItemShowcase->m_QueueType.AddToTail( mode );
}

bool ItemShowcase::ShowWeaponByClass( const char *szWeaponClass )
{
	int iRegular = g_ASWEquipmentList.GetRegularIndex( szWeaponClass );
	if ( iRegular != -1 )
	{
		ShowItems( iRegular, ItemShowcase::MODE_UNLOCK_REGULAR_WEAPON );
		return true;
	}

	int iExtra = g_ASWEquipmentList.GetExtraIndex( szWeaponClass );
	if ( iExtra != -1 )
	{
		ShowItems( iExtra, ItemShowcase::MODE_UNLOCK_EXTRA_WEAPON );
		return true;
	}

	return false;
}

CON_COMMAND_F( rd_debug_fake_weapon_unlock, "", FCVAR_HIDDEN )
{
	if ( !ItemShowcase::ShowWeaponByClass( args.Arg( 1 ) ) )
	{
		Msg( "Usage: rd_debug_fake_weapon_unlock [weapon entity name]\n" );
	}
}
