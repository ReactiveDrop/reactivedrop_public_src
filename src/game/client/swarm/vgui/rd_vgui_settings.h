#pragma once

#include <vgui_controls/EditablePanel.h>
#include "asw_shareddefs.h"

namespace BaseModUI
{
	class BaseModHybridButton;
	class GenericPanelList;
}

class CCvarToggleCheckButton;
class CRD_VGUI_Bind;
class CRD_VGUI_Settings_Controls;
class CRD_VGUI_Settings_Options;
class CRD_VGUI_Settings_Audio;
class CRD_VGUI_Settings_Video;
class CRD_VGUI_Settings_About;

class CRD_VGUI_Settings : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings, vgui::EditablePanel );
public:
	CRD_VGUI_Settings( vgui::Panel *parent, const char *panelName );

	BaseModUI::BaseModHybridButton *m_pBtnControls;
	BaseModUI::BaseModHybridButton *m_pBtnOptions;
	BaseModUI::BaseModHybridButton *m_pBtnAudio;
	BaseModUI::BaseModHybridButton *m_pBtnVideo;
	BaseModUI::BaseModHybridButton *m_pBtnAbout;

	CRD_VGUI_Settings_Controls *m_pPnlControls;
	CRD_VGUI_Settings_Options *m_pPnlOptions;
	CRD_VGUI_Settings_Audio *m_pPnlAudio;
	CRD_VGUI_Settings_Video *m_pPnlVideo;
	CRD_VGUI_Settings_About *m_pPnlAbout;
};

class CRD_VGUI_Settings_Panel_Base : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Panel_Base, vgui::EditablePanel );
public:
	CRD_VGUI_Settings_Panel_Base( vgui::Panel *parent, const char *panelName );

	virtual BaseModUI::BaseModHybridButton *GetButton( CRD_VGUI_Settings *pSettings ) = 0;
};

class CRD_VGUI_Bind : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Bind, vgui::EditablePanel );
public:
	CRD_VGUI_Bind( vgui::Panel *parent, const char *panelName, const char *szLabel, const char *szBind, bool bUseRowLayout );

	void AddFallbackBind( const char *szBind );

	vgui::Label *m_pLblDescription;
	char m_szBind[63];
	bool m_bUseRowLayout;
	CUtlStringList m_AlternateBind;
};

class CRD_VGUI_Settings_Controls : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Controls, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Controls( vgui::Panel *parent, const char *panelName );

	BaseModUI::BaseModHybridButton *GetButton( CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnControls; }

	CRD_VGUI_Bind *m_pBindMoveForward;
	CRD_VGUI_Bind *m_pBindMoveLeft;
	CRD_VGUI_Bind *m_pBindMoveBack;
	CRD_VGUI_Bind *m_pBindMoveRight;
	CRD_VGUI_Bind *m_pBindWalk;
	CRD_VGUI_Bind *m_pBindJump;

	CRD_VGUI_Bind *m_pBindPrimaryAttack;
	CRD_VGUI_Bind *m_pBindSecondaryAttack;
	CRD_VGUI_Bind *m_pBindUse;
	CRD_VGUI_Bind *m_pBindSelectPrimary;
	CRD_VGUI_Bind *m_pBindDropWeapon;
	CRD_VGUI_Bind *m_pBindSwapWeapons;
	CRD_VGUI_Bind *m_pBindMeleeAttack;
	CRD_VGUI_Bind *m_pBindReload;
	CRD_VGUI_Bind *m_pBindSelectSecondary;
	CRD_VGUI_Bind *m_pBindDropEquipment;

	CRD_VGUI_Bind *m_pBindTextChat;
	CRD_VGUI_Bind *m_pBindVoiceChat;
	CRD_VGUI_Bind *m_pBindWheelDefault;
	CRD_VGUI_Bind *m_pBindEmoteGo;
	CRD_VGUI_Bind *m_pBindEmoteStop;
	CRD_VGUI_Bind *m_pBindMarinePosition;
	CRD_VGUI_Bind *m_pBindEmoteMedic;
	CRD_VGUI_Bind *m_pBindEmoteAmmo;
	CRD_VGUI_Bind *m_pBindEmoteQuestion;
	CRD_VGUI_Bind *m_pBindEmoteExclaim;
	CRD_VGUI_Bind *m_pBindVoteYes;
	CRD_VGUI_Bind *m_pBindVoteNo;
	CRD_VGUI_Bind *m_pBindMissionOverview;
	CRD_VGUI_Bind *m_pBindPlayerList;
	CRD_VGUI_Bind *m_pBindRotateCameraLeft;
	CRD_VGUI_Bind *m_pBindRotateCameraRight;
	CRD_VGUI_Bind *m_pBindSecondaryAttackAlt;
	CRD_VGUI_Bind *m_pBindChooseMarine;

	CRD_VGUI_Bind *m_pBindActivatePrimary;
	CRD_VGUI_Bind *m_pBindActivateSecondary;
	CRD_VGUI_Bind *m_pBindActivateEquipment[ASW_NUM_MARINE_PROFILES];
	CRD_VGUI_Bind *m_pBindWheelEquipment;
	CRD_VGUI_Bind *m_pBindWheelEquipment1;
	CRD_VGUI_Bind *m_pBindWheelEquipment2;

	CRD_VGUI_Bind *m_pBindSelectMarine[ASW_NUM_MARINE_PROFILES];
	CRD_VGUI_Bind *m_pBindWheelMarine;

	BaseModUI::BaseModHybridButton *m_pBtnCustomWheels;
	BaseModUI::BaseModHybridButton *m_pBtnResetDefaults;
	CCvarToggleCheckButton *m_pChkDeveloperConsole;
};

class CRD_VGUI_Settings_Options : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Options, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Options( vgui::Panel *parent, const char *panelName );

	BaseModUI::BaseModHybridButton *GetButton( CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnOptions; }
};

class CRD_VGUI_Settings_Audio : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Audio, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Audio( vgui::Panel *parent, const char *panelName );

	BaseModUI::BaseModHybridButton *GetButton( CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnAudio; }
};

class CRD_VGUI_Settings_Video : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Video, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Video( vgui::Panel *parent, const char *panelName );

	BaseModUI::BaseModHybridButton *GetButton( CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnVideo; }
};

class CRD_VGUI_Settings_About : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_About, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_About( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	BaseModUI::BaseModHybridButton *GetButton( CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnAbout; }

	vgui::Label *m_pLblBuildID;
	vgui::Label *m_pLblNetworkVersion;
	vgui::Label *m_pLblCurrentBranch;
	vgui::Label *m_pLblWineVersion;
	vgui::ImagePanel *m_pImgSourceEngine;
	BaseModUI::GenericPanelList *m_pCopyrightDisclaimers;
};
