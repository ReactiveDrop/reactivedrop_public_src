#include "cbase.h"
#include "nb_lobby_row.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Panel.h"
#include "StatsBar.h"
#include "vgui_bitmapbutton.h"
#include <vgui/ILocalize.h>
#include "asw_marine_profile.h"
#include "asw_briefing.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "asw_player_shared.h"
#include "nb_lobby_tooltip.h"
#include "controller_focus.h"
#include "nb_main_panel.h"
#include "vgui_avatarimage.h"
#include "voice_status.h"
#include "gameui/swarm/vflyoutmenu.h"
#include "gameui/swarm/vdropdownmenu.h"
#include "gameui/swarm/vhybridbutton.h"
#include "rd_inventory_shared.h"
#include "c_asw_player.h"
//#include "convar.h"

#include "c_asw_weapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

extern ConVar cl_asw_laser_sight_color;

class C_ASW_Player;
class vgui::ISurface;

CNB_Lobby_Row::CNB_Lobby_Row( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	m_pAvatarBackground = new vgui::Panel( this, "AvatarBackground" );
	m_pAvatarImage = new CAvatarImagePanel( this, "AvatarImage" );
	m_pNameDropdown = new BaseModUI::DropDownMenu( this, "DrpNameLabel" );
	m_pLevelLabel = new vgui::Label( this, "LevelLabel", "" );
	m_pXPBar = new StatsBar( this, "XPBar" );
	m_pClassLabel = new vgui::Label( this, "ClassLabel", "" );
	m_pBackground = new vgui::ImagePanel( this, "Background" );
	m_pBackgroundWeapon0 = new vgui::Panel( this, "BackgroundWeapon0" );
	m_pBackgroundWeapon1 = new vgui::Panel( this, "BackgroundWeapon1" );
	m_pBackgroundWeapon2 = new vgui::Panel( this, "BackgroundWeapon2" );
	m_pBackgroundInnerWeapon0 = new vgui::Panel( this, "BackgroundInnerWeapon0" );
	m_pBackgroundInnerWeapon1 = new vgui::Panel( this, "BackgroundInnerWeapon1" );
	m_pBackgroundInnerWeapon2 = new vgui::Panel( this, "BackgroundInnerWeapon2" );
	m_pSilhouetteWeapon0 = new vgui::ImagePanel( this, "SilhouetteWeapon0" );
	m_pSilhouetteWeapon1 = new vgui::ImagePanel( this, "SilhouetteWeapon1" );
	m_pSilhouetteWeapon2 = new vgui::ImagePanel( this, "SilhouetteWeapon2" );
	m_pClassImage = new vgui::ImagePanel( this, "ClassImage" );
	// == MANAGED_MEMBER_CREATION_END ==
	m_pPortraitButton = new CBitmapButton( this, "PortraitButton", "" );
	m_pWeaponButton0 = new CBitmapButton( this, "WeaponButton0", "" );
	m_pWeaponButton1 = new CBitmapButton( this, "WeaponButton1", "" );
	m_pWeaponButton2 = new CBitmapButton( this, "WeaponButton2", "" );

	m_pLaserButton = new CBitmapButton(this, "ChangeLaserButton", "");
	m_pLaserOverlayButton = new CBitmapButton(this, "LaserOverlayButton", "");

	m_pPortraitButton->AddActionSignalTarget( this );
	m_pPortraitButton->SetCommand( "ChangeMarine" );

	m_pWeaponButton0->AddActionSignalTarget( this );
	m_pWeaponButton1->AddActionSignalTarget( this );
	m_pWeaponButton2->AddActionSignalTarget( this );
	m_pLaserButton->AddActionSignalTarget(this);

	m_pWeaponButton0->SetCommand( "ChangeWeapon0" );
	m_pWeaponButton1->SetCommand( "ChangeWeapon1" );
	m_pWeaponButton2->SetCommand( "ChangeWeapon2" );
	m_pLaserButton->SetCommand(	  "ChangeLaserColor");
	m_pLaserOverlayButton->SetCommand("ChangeLaserColor");

	m_pVoiceIcon = new vgui::ImagePanel( this, "VoiceIcon" );
	m_pPromotionIcon = new vgui::ImagePanel( this, "PromotionIcon" );
	m_pPromotionIcon->SetVisible( false );
	m_pMedalIcon = new vgui::ImagePanel( this, "MedalIcon" );
	m_pMedalIcon->SetVisible( false );

	m_nLobbySlot = -1;

	for ( int i = 0;i < ASW_NUM_INVENTORY_SLOTS; i++ )
	{
		m_szLastWeaponImage[i][0] = 0;
	}
	m_szLastPortraitImage[0] = 0;

	for ( int i = 0; i < 4; i++ )
	{
		m_pChangingSlot[ i ] = new vgui::ImagePanel( this, VarArgs( "ChangingSlot%d", i ) );
		m_pChangingSlot[ i ]->SetMouseInputEnabled( false );
	}

	m_pXPBar->SetShowMaxOnCounter( true );
	m_pXPBar->SetColors( Color( 255, 255, 255, 0 ), Color( 93,148,192,255 ), Color( 255, 255, 255, 255 ), Color( 17,37,57,255 ), Color( 35, 77, 111, 255 ) );
	//m_pXPBar->m_bShowCumulativeTotal = true;
	m_nLastPromotion = 0;
	m_nMedalUpdates = 0;
	m_hMedalResult = k_SteamInventoryResultInvalid;
	m_bWaitingForMedal = false;

	m_pXPBar->m_flBorder = 1.5f;
	m_nLobbySlot = 0;

	GetControllerFocus()->AddToFocusList( m_pPortraitButton );
	GetControllerFocus()->AddToFocusList( m_pWeaponButton0 );
	GetControllerFocus()->AddToFocusList( m_pWeaponButton1 );
	GetControllerFocus()->AddToFocusList( m_pWeaponButton2 );
	GetControllerFocus()->AddToFocusList(m_pLaserOverlayButton);
}

CNB_Lobby_Row::~CNB_Lobby_Row()
{
	GetControllerFocus()->RemoveFromFocusList( m_pPortraitButton );
	GetControllerFocus()->RemoveFromFocusList( m_pWeaponButton0 );
	GetControllerFocus()->RemoveFromFocusList( m_pWeaponButton1 );
	GetControllerFocus()->RemoveFromFocusList( m_pWeaponButton2 );
	GetControllerFocus()->RemoveFromFocusList(m_pLaserOverlayButton);

	if ( ISteamInventory *pInventory = SteamInventory() )
	{
		pInventory->DestroyResult( m_hMedalResult );
	}
}

void CNB_Lobby_Row::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_lobby_row.res" );

	for ( int i = 0;i < ASW_NUM_INVENTORY_SLOTS; i++ )
	{
		m_szLastWeaponImage[i][0] = 0;
	}
	m_szLastPortraitImage[ 0 ] = 0;	
	m_lastSteamID.Set( 0, k_EUniverseInvalid, k_EAccountTypeInvalid );
}

void CNB_Lobby_Row::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( m_pAvatarImage )
	{
		int wide, tall;
		m_pAvatarImage->GetSize( wide, tall );
		if ( ((CAvatarImage*)m_pAvatarImage->GetImage()) )
		{
			((CAvatarImage*)m_pAvatarImage->GetImage())->SetAvatarSize( wide, tall );
			((CAvatarImage*)m_pAvatarImage->GetImage())->SetPos( -AVATAR_INDENT_X, -AVATAR_INDENT_Y );
		}
	}
}

void CNB_Lobby_Row::OnThink()
{
	BaseClass::OnThink();

	UpdateDetails();
	UpdateChangingSlot();
}

void CNB_Lobby_Row::UpdateDetails()
{
	int screenWidth = ScreenWidth(), screenHeight = ScreenHeight();
	float widthMod = screenWidth / 640;
	float heightMod = screenHeight / 480;

	color32 white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;

	m_pVoiceIcon->SetVisible( Briefing()->IsCommanderSpeaking( m_nLobbySlot ) );

	if ( m_nLobbySlot == -1 || !Briefing()->IsLobbySlotOccupied( m_nLobbySlot ) )
	{
		m_pXPBar->SetVisible( false );
		m_pLevelLabel->SetVisible( false );
		m_pPromotionIcon->SetVisible( false );
		m_pMedalIcon->SetVisible( false );
		m_pNameDropdown->SetVisible( false );
		m_pAvatarImage->SetVisible( false );
		m_pClassLabel->SetVisible( false );
		m_pClassImage->SetVisible( false );

		if ( Briefing()->IsOfflineGame() )
		{
			// in singleplayer, empty slots show the empty portrait button
			const char *szEmptyFace = "vgui/briefing/face_empty";
			const char *szEmptyFaceLit = "vgui/briefing/face_empty_lit";
			if ( Q_strcmp( szEmptyFace, m_szLastPortraitImage ) )
			{
				Q_snprintf( m_szLastPortraitImage, sizeof( m_szLastPortraitImage ), "%s", szEmptyFace );
				m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED, szEmptyFace, white );
				m_pPortraitButton->SetImage( CBitmapButton::BUTTON_PRESSED, szEmptyFace, white );

				if ( Briefing()->IsLobbySlotLocal( m_nLobbySlot ) )
				{
					m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFaceLit, white );
				}
				else
				{
					m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFace, white );
				}
			}
			m_pPortraitButton->SetVisible( true );
		}
		else
		{
			return;
		}
	}

	color32 invisible;
	invisible.r = 0;
	invisible.g = 0;
	invisible.b = 0;
	invisible.a = 0;

	color32 lightblue;
	lightblue.r = 66;
	lightblue.g = 142;
	lightblue.b = 192;
	lightblue.a = 255;

	BaseModHybridButton	*pButton = m_pNameDropdown->GetButton();
	if ( pButton )
	{
		pButton->SetText( Briefing()->GetMarineOrPlayerName( m_nLobbySlot ) );
	}
	m_pNameDropdown->SetVisible( true );

#if !defined(NO_STEAM)
	CSteamID steamID = Briefing()->GetCommanderSteamID( m_nLobbySlot );
	if ( steamID.IsValid() )
	{
		if ( steamID.ConvertToUint64() != m_lastSteamID.ConvertToUint64() )
		{
			m_pAvatarImage->SetAvatarBySteamID( &steamID );

			int wide, tall;
			m_pAvatarImage->GetSize( wide, tall );
			((CAvatarImage*)m_pAvatarImage->GetImage())->SetAvatarSize( wide, tall );
			((CAvatarImage*)m_pAvatarImage->GetImage())->SetPos( -AVATAR_INDENT_X, -AVATAR_INDENT_Y );

			m_nMedalUpdates = 0;
			m_pMedalIcon->SetVisible( false );
			m_bWaitingForMedal = ReactiveDropInventory::DecodeItemData( m_hMedalResult, "" );
		}
		m_lastSteamID = steamID;

		int nMedalUpdates = Briefing()->GetMedalUpdateCount( m_nLobbySlot );
		if ( m_nMedalUpdates != nMedalUpdates )
		{
			m_pMedalIcon->SetVisible( false );
			m_bWaitingForMedal = ReactiveDropInventory::DecodeItemData( m_hMedalResult, Briefing()->GetEncodedMedalData( m_nLobbySlot ) );
			m_nMedalUpdates = nMedalUpdates;
		}
		bool bMedalOK = false;
		if ( m_bWaitingForMedal && ReactiveDropInventory::ValidateItemData( bMedalOK, m_hMedalResult, "medal", steamID ) )
		{
			m_bWaitingForMedal = false;
			if ( bMedalOK )
			{
				const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( ReactiveDropInventory::GetItemDetails( m_hMedalResult, 0 ).m_iDefinition );
				m_pMedalIcon->SetImage( pDef->IconSmall );
				m_pMedalIcon->SetVisible( true );
			}
		}
	}
#endif

	int nLevel = Briefing()->GetCommanderLevel( m_nLobbySlot ) + 1;		// levels start from 0 in code, but present as 1
	int nXP = Briefing()->GetCommanderXP( m_nLobbySlot );
	int nPromotion = Briefing()->GetCommanderPromotion( m_nLobbySlot );

	if ( nPromotion <= 0 || nPromotion > ASW_PROMOTION_CAP )
	{
		m_pPromotionIcon->SetVisible( false );
		m_nLastPromotion = 0;
	}
	else
	{
		m_pPromotionIcon->SetVisible( true );
		m_pPromotionIcon->SetImage( VarArgs( "briefing/promotion_%d", nPromotion ) );
		m_nLastPromotion = nPromotion;
	}
	m_pXPBar->ClearMinMax();
	m_pXPBar->AddMinMax( 0, g_iLevelExperience[ 0 ] * g_flPromotionXPScale[ m_nLastPromotion ] );
	for ( int i = 0; i < ASW_NUM_EXPERIENCE_LEVELS - 1; i++ )
	{
		m_pXPBar->AddMinMax( g_iLevelExperience[ i ] * g_flPromotionXPScale[ m_nLastPromotion ], g_iLevelExperience[ i + 1 ] * g_flPromotionXPScale[ m_nLastPromotion ] );
	}

	if ( !Briefing()->IsFullyConnected( m_nLobbySlot ) )
	{
		m_pXPBar->SetVisible( false );
		m_pLevelLabel->SetVisible( true );
		m_pLevelLabel->SetText( "#nb_commander_connecting" );
	}
	else if ( nLevel == -1 || nXP == -1 )
	{
		m_pXPBar->SetVisible( false );
		m_pLevelLabel->SetVisible( false );
	}
	else
	{
		m_pXPBar->SetVisible( true );
		m_pLevelLabel->SetVisible( true );

		m_pXPBar->Init( nXP, nXP, 1.0, true, false );
	
		wchar_t szLevelNum[16]=L"";
		V_snwprintf( szLevelNum, ARRAYSIZE( szLevelNum ), L"%i", nLevel );

		wchar_t wzLevelLabel[64];
		g_pVGuiLocalize->ConstructString( wzLevelLabel, sizeof( wzLevelLabel ), g_pVGuiLocalize->Find( "#nb_commander_level" ), 1, szLevelNum );
		m_pLevelLabel->SetText( wzLevelLabel );
	}

	ASW_Marine_Class nMarineClass = Briefing()->GetMarineClass( m_nLobbySlot );
	switch( nMarineClass )
	{
	case MARINE_CLASS_NCO: m_pClassLabel->SetText( "#marine_class_officer" ); break;
	case MARINE_CLASS_SPECIAL_WEAPONS: m_pClassLabel->SetText( "#marine_class_sw_short" ); break;
	case MARINE_CLASS_MEDIC: m_pClassLabel->SetText( "#marine_class_medic" ); break;
	case MARINE_CLASS_TECH: m_pClassLabel->SetText( "#marine_class_tech" ); break;
	default: m_pClassLabel->SetText( "" ); break;
	}
	m_pClassLabel->SetVisible( true );

	switch( nMarineClass )
	{
	case MARINE_CLASS_NCO: m_pClassImage->SetImage( "swarm/ClassIcons/NCOClassIcon" ); break;
	case MARINE_CLASS_SPECIAL_WEAPONS: m_pClassImage->SetImage( "swarm/ClassIcons/SpecialWeaponsClassIcon" ); break;
	case MARINE_CLASS_MEDIC: m_pClassImage->SetImage( "swarm/ClassIcons/MedicClassIcon" ); break;
	case MARINE_CLASS_TECH: m_pClassImage->SetImage( "swarm/ClassIcons/TechClassIcon" ); break;
	}
	m_pClassImage->SetVisible( nMarineClass != MARINE_CLASS_UNDEFINED );
		
	if ( Briefing()->IsOfflineGame() && m_nLobbySlot != 0 )
	{
		// AI slots
		m_pLevelLabel->SetVisible( false );
		m_pPromotionIcon->SetVisible( false );
		m_pAvatarImage->SetVisible( false );

		int nAvatarX, nAvatarY;
		m_pAvatarImage->GetPos( nAvatarX, nAvatarY );

		int nNameX, nNameY;
		m_pNameDropdown->GetPos( nNameX, nNameY );

		m_pNameDropdown->SetPos( nAvatarX + YRES( 5 ), nNameY );
	}
	else
	{
		m_pAvatarImage->SetVisible( true );
		m_pAvatarImage->SetVisible(true);
	}
	

	CASW_Marine_Profile *pProfile = Briefing()->GetMarineProfile( m_nLobbySlot );
	if ( !pProfile )
	{
		// Set to some blank image
		const char *szEmptyFace = "vgui/briefing/face_empty";
		const char *szEmptyFaceLit = "vgui/briefing/face_empty_lit";
		if ( Q_strcmp( szEmptyFace, m_szLastPortraitImage ) )
		{
			Q_snprintf( m_szLastPortraitImage, sizeof( m_szLastPortraitImage ), "%s", szEmptyFace );
			m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED, szEmptyFace, white );
			m_pPortraitButton->SetImage( CBitmapButton::BUTTON_PRESSED, szEmptyFace, white );

			if ( Briefing()->IsLobbySlotLocal( m_nLobbySlot ) )
			{
				m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFaceLit, white );
			}
			else
			{
				m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFace, white );
			}
		}
	}
	else
	{
		char imagename[255];
		Q_snprintf( imagename, sizeof(imagename), "vgui/briefing/face_%s", pProfile->m_PortraitName );		
		if ( Q_strcmp( imagename, m_szLastPortraitImage ) )
		{
			Q_snprintf( m_szLastPortraitImage, sizeof( m_szLastPortraitImage ), "%s", imagename );
			m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED, imagename, white );
			m_pPortraitButton->SetImage( CBitmapButton::BUTTON_PRESSED, imagename, white );

			if ( Briefing()->IsLobbySlotLocal( m_nLobbySlot ) )
			{
				Q_snprintf( imagename, sizeof(imagename), "vgui/briefing/face_%s_lit", pProfile->m_PortraitName );
			}
			m_pPortraitButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, imagename, white );
		}
	}


	for ( int i = 0; i < ASW_NUM_INVENTORY_SLOTS; i++ )
	{
		bool bSetButtonImage = false;
		CBitmapButton *pButton = NULL;
		vgui::ImagePanel *pSilhouette = NULL;
		switch ( i )
		{
			case ASW_INVENTORY_SLOT_PRIMARY:  pButton = m_pWeaponButton0; pSilhouette = m_pSilhouetteWeapon0; break;
			case ASW_INVENTORY_SLOT_SECONDARY:  pButton = m_pWeaponButton1; pSilhouette = m_pSilhouetteWeapon1; break;
			case ASW_INVENTORY_SLOT_EXTRA:  pButton = m_pWeaponButton2; pSilhouette = m_pSilhouetteWeapon2; break;
		}

		int nWeapon = Briefing()->GetMarineSelectedWeapon( m_nLobbySlot, i );	
		if ( nWeapon != -1 && ASWEquipmentList() )
		{
			CASW_EquipItem *pItem = ASWEquipmentList()->GetItemForSlot( i, nWeapon );
			if ( pItem )
			{
				CASW_WeaponInfo* pWeaponInfo = ASWEquipmentList()->GetWeaponDataFor( STRING( pItem->m_EquipClass ) );
				if ( pButton && pWeaponInfo )
				{				
					bSetButtonImage = true;
					char imagename[255];
					Q_snprintf( imagename, sizeof(imagename), "vgui/%s", pWeaponInfo->szEquipIcon );
					if ( Q_strcmp( imagename, m_szLastWeaponImage[i] ) )
					{
						Q_snprintf( m_szLastWeaponImage[i], sizeof( m_szLastWeaponImage[i] ), "%s", imagename );
						pButton->SetImage( CBitmapButton::BUTTON_ENABLED, imagename, lightblue );
						pButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, imagename, Briefing()->IsLobbySlotLocal( m_nLobbySlot ) ? white : lightblue );
						pButton->SetImage( CBitmapButton::BUTTON_PRESSED, imagename, lightblue );
					}
				}
			}
		}
		if ( !bSetButtonImage )
		{
			if ( Q_strcmp( "vgui/white", m_szLastWeaponImage[i] ) )
			{
				Q_snprintf( m_szLastWeaponImage[i], sizeof( m_szLastWeaponImage[i] ), "%s", "vgui/white" );
				pButton->SetImage( CBitmapButton::BUTTON_ENABLED, "vgui/white", invisible );
				pButton->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/white", invisible );
				pButton->SetImage( CBitmapButton::BUTTON_PRESSED, "vgui/white", invisible );
			}
			pSilhouette->SetVisible( true );
		}
		else
		{
			pSilhouette->SetVisible( false );
		}
	}

	CASW_Briefing* ASW_Briefing = dynamic_cast<CASW_Briefing*>(Briefing());
	Color _BoopColor = ASW_Briefing->GetMarineLaserColor(m_nLobbySlot);


	//AG3 Test Code

	if (Briefing()->IsLobbySlotOccupied(m_nLobbySlot) && Briefing()->IsLobbySlotLocal(m_nLobbySlot) && !Briefing()->IsLobbySlotBot(m_nLobbySlot))
	{



		Color _inputRGBColor = cl_asw_laser_sight_color.GetColor();
		color32 rgbLaserColor = color32();
		rgbLaserColor.r = _inputRGBColor.r();
		rgbLaserColor.g = _inputRGBColor.g();
		rgbLaserColor.b = _inputRGBColor.b();
		rgbLaserColor.a = 255;

		//m_pLaserButton->SetSize(25, 25);
		//m_pLaserButton->SetPos(0, 45);
		//m_pLaserButton->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/laser_icon", rgbLaserColor);
		//m_pLaserButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/laser_icon", Briefing()->IsLobbySlotLocal(m_nLobbySlot) ? white : rgbLaserColor);
		//m_pLaserButton->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/swarm/color/laser_icon", rgbLaserColor);

		//float avgMod = (heightMod + widthMod) * 0.5f;
		int tempBGSizeX, tempBGSizeY;
		m_pBackground->GetSize(tempBGSizeX, tempBGSizeY);

		float lsOverlayWidthMod = tempBGSizeX / 319.0f; //319 is base width
		float lsOverlayHeightMod = tempBGSizeY / 76.0f; //76 is base height

		m_pLaserOverlayButton->SetSize(20.0f * lsOverlayWidthMod, 20.0f * lsOverlayHeightMod);
		m_pLaserOverlayButton->SetPos(lsOverlayWidthMod * 0.0f, lsOverlayHeightMod * 25.0f);
		
		const char* szEmptyFaceLit = "vgui/swarm/color/laser_icon";

		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED, szEmptyFaceLit, rgbLaserColor);
		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFaceLit, white);
		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_PRESSED, szEmptyFaceLit, rgbLaserColor);


		//m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED, "vgui/swarm/color/laser_icon", rgbLaserColor);
		//m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, "vgui/swarm/color/laser_icon", white);
		//m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_PRESSED, "vgui/swarm/color/laser_icon", rgbLaserColor);


		m_pLaserButton->SetVisible(false);
		m_pLaserOverlayButton->SetVisible(true);
	}
	else if (Briefing()->IsLobbySlotOccupied(m_nLobbySlot) && !Briefing()->IsLobbySlotLocal(m_nLobbySlot) && Briefing()->IsFullyConnected(m_nLobbySlot))
	{
		CASW_Briefing* ASW_Briefing = dynamic_cast<CASW_Briefing*>(Briefing());
		Color _BoopColor = ASW_Briefing->GetMarineLaserColor(m_nLobbySlot); //Do not defy the power of the boop!


		color32 rgbLaserColor = color32();
		rgbLaserColor.r = _BoopColor.r();
		rgbLaserColor.g = _BoopColor.g();
		rgbLaserColor.b = _BoopColor.b();
		rgbLaserColor.a = 255;

		int tempBGSizeX, tempBGSizeY;
		m_pClassImage->GetSize(tempBGSizeX, tempBGSizeY);

		float lsOverlayWidthMod = tempBGSizeX / 16.0f; //16 is base width
		float lsOverlayHeightMod = tempBGSizeY / 16.0f; //16 is base height

		m_pLaserOverlayButton->SetSize(10.0f * lsOverlayWidthMod, 10.0f * lsOverlayHeightMod);
		m_pLaserOverlayButton->SetPos(lsOverlayWidthMod * 5.0f, lsOverlayHeightMod * 30.0f);

		const char* szEmptyFaceLit = "vgui/swarm/color/laser_icon";

		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED, szEmptyFaceLit, rgbLaserColor);
		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFaceLit, rgbLaserColor);
		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_PRESSED, szEmptyFaceLit, rgbLaserColor);

		m_pLaserButton->SetVisible(false);
		m_pLaserOverlayButton->SetVisible(true);
	}
	/*
	else if (true)
	{
		Color _inputRGBColor = _BoopColor;
		color32 rgbLaserColor = color32();
		rgbLaserColor.r = _inputRGBColor.r();
		rgbLaserColor.g = _inputRGBColor.g();
		rgbLaserColor.b = _inputRGBColor.b();
		rgbLaserColor.a = 255;

		int tempBGSizeX, tempBGSizeY;
		//m_pBackground->GetSize(tempBGSizeX, tempBGSizeY);
		m_pClassImage->GetSize(tempBGSizeX, tempBGSizeY);

		float lsOverlayWidthMod = tempBGSizeX / 16.0f; //16 is base width
		float lsOverlayHeightMod = tempBGSizeY / 16.0f; //16 is base height

		m_pLaserOverlayButton->SetSize(10.0f * lsOverlayWidthMod, 10.0f * lsOverlayHeightMod);
		m_pLaserOverlayButton->SetPos(lsOverlayWidthMod * 5.0f, lsOverlayHeightMod * 30.0f);

		const char* szEmptyFaceLit = "vgui/swarm/color/laser_icon";

		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED, szEmptyFaceLit, rgbLaserColor);
		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, szEmptyFaceLit, rgbLaserColor);
		m_pLaserOverlayButton->SetImage(CBitmapButton::BUTTON_PRESSED, szEmptyFaceLit, rgbLaserColor);

		m_pLaserButton->SetVisible(false);
		m_pLaserOverlayButton->SetVisible(true);
	}
	*/
	else
	{
		m_pLaserButton->SetVisible(false);
		m_pLaserOverlayButton->SetVisible(false);
	}





	/*
	m_pVoiceIcon->SetVisible(true); //Setting this to true to force showing for debugging visuals
	m_pXPBar->SetVisible(true);
	m_pLevelLabel->SetVisible(true);
	m_pPromotionIcon->SetVisible(true);
	m_pMedalIcon->SetVisible(true);
	m_pNameDropdown->SetVisible(true);
	m_pAvatarImage->SetVisible(true);
	m_pClassLabel->SetVisible(true);
	m_pClassImage->SetVisible(true);
	*/
}

void CNB_Lobby_Row::CheckTooltip( CNB_Lobby_Tooltip *pTooltip )
{
	if ( CControllerFocus::IsPanelReallyVisible( m_pPortraitButton ) && m_pPortraitButton->IsCursorOver() )
	{
		pTooltip->ShowMarineTooltip( m_nLobbySlot );
	}
	else if ( CControllerFocus::IsPanelReallyVisible( m_pWeaponButton0 ) && m_pWeaponButton0->IsCursorOver() )
	{
		pTooltip->ShowWeaponTooltip( m_nLobbySlot, ASW_INVENTORY_SLOT_PRIMARY );
	}
	else if ( CControllerFocus::IsPanelReallyVisible( m_pWeaponButton1 ) && m_pWeaponButton1->IsCursorOver() )
	{
		pTooltip->ShowWeaponTooltip( m_nLobbySlot, ASW_INVENTORY_SLOT_SECONDARY );
	}
	else if ( CControllerFocus::IsPanelReallyVisible( m_pWeaponButton2 ) && m_pWeaponButton2->IsCursorOver() )
	{
		pTooltip->ShowWeaponTooltip( m_nLobbySlot, ASW_INVENTORY_SLOT_EXTRA );
	}
	else if ( CControllerFocus::IsPanelReallyVisible( m_pPromotionIcon ) && m_pPromotionIcon->IsCursorOver() )
	{
		pTooltip->ShowMarinePromotionTooltip( m_nLobbySlot );
	}
	else if ( CControllerFocus::IsPanelReallyVisible( m_pMedalIcon ) && m_pMedalIcon->IsCursorOver() )
	{
		pTooltip->ShowMarineMedalTooltip( m_nLobbySlot, m_hMedalResult );
	}
}

extern ConVar developer;

void CNB_Lobby_Row::OnCommand( const char *command )
{
	CNB_Main_Panel *pMainPanel = GetMainPanel();
	if ( !pMainPanel )
		return;

	if ( !Q_stricmp( command, "ChangeMarine" ) )
	{
		pMainPanel->ChangeMarine( m_nLobbySlot );
	}
	else if ( !Q_stricmp( command, "ChangeWeapon0" ) )
	{
		pMainPanel->ChangeWeapon( m_nLobbySlot, 0 );
	}	
	else if ( !Q_stricmp( command, "ChangeWeapon1" ) )
	{
		pMainPanel->ChangeWeapon( m_nLobbySlot, 1 );
	}
	else if ( !Q_stricmp( command, "ChangeWeapon2" ) )
	{
		pMainPanel->ChangeWeapon( m_nLobbySlot, 2 );
	}
	else if (!Q_stricmp(command, "ChangeLaserColor"))
	{
		bool bResult = pMainPanel->ChangeLaserColor(m_nLobbySlot);

		if (bResult)
		{
			GetControllerFocus()->RemoveFromFocusList(m_pPortraitButton);
			GetControllerFocus()->RemoveFromFocusList(m_pWeaponButton0);
			GetControllerFocus()->RemoveFromFocusList(m_pWeaponButton1);
			GetControllerFocus()->RemoveFromFocusList(m_pWeaponButton2);
		}
		else
		{
			GetControllerFocus()->AddToFocusList(m_pPortraitButton);
			GetControllerFocus()->AddToFocusList(m_pWeaponButton0);
			GetControllerFocus()->AddToFocusList(m_pWeaponButton1);
			GetControllerFocus()->AddToFocusList(m_pWeaponButton2);
		}
	}
	else if ( !Q_stricmp( command, "PlayerFlyout" ) )
	{
		if ( !Briefing()->IsLobbySlotBot( m_nLobbySlot ) )
		{
			OpenPlayerFlyout();
		}
	}
	else if ( !Q_stricmp( command, "#L4D360UI_SendMessage" ) )
	{
		char steamCmd[64];
		Q_snprintf( steamCmd, sizeof( steamCmd ), "chat/%I64u", pMainPanel->m_FlyoutSteamID );
		BaseModUI::CUIGameData::Get()->ExecuteOverlayCommand( steamCmd );
	}
	else if ( !Q_stricmp( command, "#L4D360UI_ViewSteamID" ) )
	{
		char steamCmd[64];
		Q_snprintf( steamCmd, sizeof( steamCmd ), "steamid/%I64u", pMainPanel->m_FlyoutSteamID );
		BaseModUI::CUIGameData::Get()->ExecuteOverlayCommand( steamCmd );
	}
	else if ( !Q_stricmp( command, "#L4D360UI_ViewSteamStats" ) )
	{
#if !defined( _X360 ) && !defined( NO_STEAM )
		if ( SteamUser() )
		{
			if ( developer.GetBool() )
			{
				Msg( "Local player SteamID = %I64u\n", SteamUser()->GetSteamID().ConvertToUint64() );
				Msg( "Activating stats for SteamID = %I64u\n", Briefing()->GetCommanderSteamID( m_nLobbySlot ).ConvertToUint64() );
			}
			char statsWeb[256];
			Q_snprintf( statsWeb, sizeof( statsWeb ), "https://stats.reactivedrop.com/profiles/%I64u?lang=%s&utm_source=briefing",
				Briefing()->GetCommanderSteamID( m_nLobbySlot ).ConvertToUint64(),
				SteamApps()->GetCurrentGameLanguage() );
			BaseModUI::CUIGameData::Get()->ExecuteOverlayUrl( statsWeb );
		}
#endif
	}
}

void CNB_Lobby_Row::OpenPlayerFlyout()
{
	vgui::Panel *pButton = m_pNameDropdown->GetButton();
	if ( !pButton )
		return;

	// Determine the anchor point
	int x, y;
	pButton->GetPos( x, y );
	pButton->LocalToScreen( x, y );

	CNB_Main_Panel *pMainPanel = GetMainPanel();
	if ( !pMainPanel )
		return;

	BaseModUI::FlyoutMenu *pFlyout = dynamic_cast< BaseModUI::FlyoutMenu * >( pMainPanel->FindChildByName( "FlmPlayerFlyout" ) );

	pFlyout->OpenMenu( pButton );
	pFlyout->SetPos( x + pButton->GetWide(), y );
	pFlyout->SetOriginalTall( pButton->GetTall() );

	pMainPanel->m_FlyoutSteamID = Briefing()->GetCommanderSteamID( m_nLobbySlot ).ConvertToUint64();

	// Disable Send Message if self
	BaseModHybridButton *sendMessasge = dynamic_cast< BaseModHybridButton * >( pFlyout->FindChildByName( "BtnSendMessage" ) );
	if ( sendMessasge )
	{
		sendMessasge->SetEnabled( !Briefing()->IsLobbySlotLocal( m_nLobbySlot ) );
	}
	
}

CNB_Main_Panel *CNB_Lobby_Row::GetMainPanel()
{
	CNB_Main_Panel *pMainPanel = NULL;
	for ( vgui::Panel *pPanel = GetParent(); pPanel; pPanel = pPanel->GetParent() )
	{
		pMainPanel = dynamic_cast<CNB_Main_Panel *>( pPanel );
		if ( pMainPanel )
		{
			return pMainPanel;
		}
	}

	Warning( "Error, parent of lobby row is not the main panel\n" );
	return NULL;
}

void CNB_Lobby_Row::UpdateChangingSlot()
{
	int nSlot = Briefing()->GetChangingWeaponSlot( m_nLobbySlot );
	m_pChangingSlot[ 0 ]->SetVisible( nSlot == 1 );
	m_pChangingSlot[ 1 ]->SetVisible( nSlot == 2 );
	m_pChangingSlot[ 2 ]->SetVisible( nSlot == 3 );
	m_pChangingSlot[ 3 ]->SetVisible( nSlot == 4 );
}
