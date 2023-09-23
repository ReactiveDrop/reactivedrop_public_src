#pragma once

#include "gameui/swarm/basemodui.h"
#include "asw_shareddefs.h"

namespace BaseModUI
{
	class BaseModHybridButton;
	class GenericPanelList;
}

class CNB_Header_Footer;
class CRD_VGUI_Main_Menu_Top_Bar;
class CRD_VGUI_Microphone_Tester;
class CRD_VGUI_Option_Dropdown;
class CRD_VGUI_Settings_Panel_Base;
class CRD_VGUI_Settings_Controls;
class CRD_VGUI_Settings_Options_1;
class CRD_VGUI_Settings_Options_2;
class CRD_VGUI_Settings_Audio;
class CRD_VGUI_Settings_Video;
class CRD_VGUI_Settings_About;

namespace BaseModUI
{
class CRD_VGUI_Settings : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings, CBaseModFrame );
public:
	// static so it can be set without having to find the panel
	static bool s_bWantSave;

	CRD_VGUI_Settings( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Settings();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;

	void NavigateToTab( BaseModHybridButton *pButton, CRD_VGUI_Settings_Panel_Base *pPanel, const char *szTabID );

	CNB_Header_Footer *m_pHeaderFooter;
	CRD_VGUI_Main_Menu_Top_Bar *m_pTopBar;

	BaseModHybridButton *m_pBtnControls;
	BaseModHybridButton *m_pBtnOptions1;
	BaseModHybridButton *m_pBtnOptions2;
	BaseModHybridButton *m_pBtnAudio;
	BaseModHybridButton *m_pBtnVideo;
	BaseModHybridButton *m_pBtnAbout;

	CRD_VGUI_Settings_Controls *m_pPnlControls;
	CRD_VGUI_Settings_Options_1 *m_pPnlOptions1;
	CRD_VGUI_Settings_Options_2 *m_pPnlOptions2;
	CRD_VGUI_Settings_Audio *m_pPnlAudio;
	CRD_VGUI_Settings_Video *m_pPnlVideo;
	CRD_VGUI_Settings_About *m_pPnlAbout;
};
}

class CRD_VGUI_Settings_Panel_Base : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Panel_Base, vgui::EditablePanel );
public:
	CRD_VGUI_Settings_Panel_Base( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void NavigateTo() override;

	virtual void Activate() = 0;
	virtual BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) = 0;
};

class CRD_VGUI_Option : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Option, vgui::EditablePanel );
public:
	enum Mode_t
	{
		MODE_RADIO,    // all options displayed in a row
		MODE_DROPDOWN, // only current option is displayed; when active all options displayed in a column
		MODE_CHECKBOX, // value is either min or max; checkbox displayed before label
		MODE_SLIDER,   // value is a float; can have specific options outside of min-max range
		MODE_COLOR,    // no recommended/current value support; must be linked to convar
		MODE_CUSTOM,   // no value; still handles label, hint, etc.
	};

	CRD_VGUI_Option( vgui::Panel *parent, const char *panelName, const char *szLabel, Mode_t eMode = MODE_RADIO );
	~CRD_VGUI_Option();

	void OnCursorEntered() override;
	void OnCursorMoved( int x, int y ) override;
	void NavigateTo() override;
	void OnKillFocus() override;
	void ApplySettings( KeyValues *pSettings ) override;
	void PerformLayout() override;
	void OnThink() override;
	void Paint() override;
	void OnKeyCodeTyped( vgui::KeyCode code ) override;
	void OnKeyCodePressed( vgui::KeyCode code ) override;
	void OnMousePressed( vgui::MouseCode code ) override;
	void OnMouseReleased( vgui::MouseCode code ) override;
	MESSAGE_FUNC( OnTextChanged, "TextChanged" );
	MESSAGE_FUNC( OnTextKillFocus, "TextKillFocus" );

	enum OptionFlags_t
	{
		FLAG_DISABLED = 0x0001, // cannot be selected; slightly faded
		FLAG_HIDDEN   = 0x0002, // still takes up visual space, but not rendered
	};

	// for MODE_RADIO and MODE_DROPDOWN
	// can also be used for MODE_SLIDER, but iOption must be outside of the slider range
	void RemoveAllOptions();
	void AddOption( int iOption, const char *szLabel, const char *szHint, int iFlags = 0 );
	void SetOptionFlags( int iOption, int iFlags );

	// for MODE_RADIO, MODE_DROPDOWN, and MODE_CHECKBOX
	void SetCurrentAndRecommended( int iCurrent, int iRecommended );
	int GetCurrentOption();
	void SetCurrentOption( int iCurrent );
	void SetRecommendedOption( int iRecommended );

	// for MODE_SLIDER
	void SetSliderMinMax( float flMin, float flMax );
	void SetCurrentSliderValue( float flValue );
	void SetRecommendedSliderValue( float flValue );
	void ClearRecommendedSliderValue();
	float GetCurrentSliderValue();

	// for MODE_CHECKBOX, MODE_SLIDER, and MODE_CUSTOM
	// as well as MODE_RADIO and MODE_DROPDOWN when no option is current.
	void SetDefaultHint( const char *szHint );

	// if we are MODE_CHECKBOX, the slider min/max are the values used for the off/on state.
	// if we're MODE_CUSTOM, this does nothing. otherwise, the values are used as-is.
	void LinkToConVar( const char *szName, bool bSetRecommendedToDefaultValue );
	// the szValue version of this function does not copy the string; it must remain valid.
	void LinkToConVarAdvanced( int iOption, const char *szName, const char *szValue );
	void LinkToConVarAdvanced( int iOption, const char *szName, int iValue );
	void SetCurrentUsingConVars();
	void SetRecommendedUsingConVars();

	static void WriteConfig( bool bForce );

private:
	Mode_t m_eMode;
	// if m_eMode is MODE_CHECKBOX or MODE_SLIDER, m_flValue is set to the value.
	// if m_eMode is MODE_RADIO or MODE_DROPDOWN, m_iOption is set to the index of the option.
	// these fields are unused for MODE_COLOR and MODE_CUSTOM.
	bool m_bHaveCurrent : 1;
	bool m_bHaveRecommended : 1;
	bool m_bSetUsingConVars : 1;
	bool m_bReverseSlider : 1;
	bool m_bSliderActive : 1;
	bool m_bSliderActiveMouse : 1;
	bool m_bStartedSliderActiveAtRecommended : 1;
	int m_iActiveOption;
	union
	{
		int m_iOption;
		float m_flValue;
	} m_Current;
	union
	{
		int m_iOption;
		float m_flValue;
	} m_Recommended;
	float m_flMinValue{ 0.0f }, m_flMaxValue{ 1.0f };

	struct ConVarLink_t
	{
		ConVarLink_t( ConVar *pConVar, const char *szValue );
		ConVarLink_t( ConVar *pConVar, int iValue );

		ConVar *m_pConVar;
		const char *m_szValue;
		int m_iValue;
	};
	struct Option_t
	{
		Option_t( int iValue, const char *szLabel, const char *szHint, int iFlags );

		int m_iValue;
		wchar_t m_wszLabel[256]{};
		wchar_t m_wszHint[1024]{};
		int m_iFlags;
		int m_iWidth;

		CUtlVector<ConVarLink_t> m_ConVars;
	};
	CUtlVectorAutoPurge<Option_t *> m_Options;
	ConVar *m_pSliderLink{};
	wchar_t m_wszDefaultHint[1024]{};

	vgui::Panel *m_pInteractiveArea;
	vgui::Label *m_pLblFieldName;
	vgui::Label *m_pLblHint;
	vgui::TextEntry *m_pTextEntry;
	CRD_VGUI_Option_Dropdown *m_pDropdown;

	float m_flDisplayMultiplier;
	int m_nDecimalDigits;
	wchar_t m_wszDisplaySuffix[64];

	friend class CRD_VGUI_Option_Dropdown;

	static bool s_bCVarChanged;
	static float s_flLastRepeatLEFT, s_flLastRepeatRIGHT;
	static float s_flLastRepeatUP, s_flLastRepeatDOWN;
	static int s_iRepeatCountLEFT, s_iRepeatCountRIGHT;
	static int s_iRepeatCountUP, s_iRepeatCountDOWN;
	bool OnActivateButton( bool bMouse );
	bool OnDeactivateButton( bool bMouse );
	bool OnMovementButton( int iDirection, bool bVertical );
	void CheckButtonRepeat( float &flLastRepeat, int &iRepeatCount, int iDirection, bool bVertical, float flSpeed, float flAccel, int iMaxAccel );
	void ToggleCheckbox();
	void SelectActiveRadioButton();
	void ChangeActiveRadioButton( int iActive );
	void ToggleSliderActive( bool bMouse );
	void ToggleColorActive();
	void ToggleDropdownActive();
};

class CRD_VGUI_Bind : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Bind, vgui::EditablePanel );
public:
	CRD_VGUI_Bind( vgui::Panel *parent, const char *panelName, const char *szLabel, const char *szBind, bool bUseRowLayout );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnKeyCodePressed( vgui::KeyCode keycode ) override;
	void OnKeyCodeTyped( vgui::KeyCode keycode ) override;
	void OnMouseReleased( vgui::MouseCode code ) override;
	void OnCursorEntered() override;
	void NavigateTo() override;
	void OnThink() override;
	void Paint() override;

	void AddFallbackBind( const char *szBind );
	void StartKeyboardCapture();
	void ClearKeyboardBind();

	vgui::Label *m_pLblKeyboardIcon;
	vgui::Label *m_pLblKeyboardIconLong;
	vgui::Panel *m_pPnlControllerIcon;
	vgui::ImagePanel *m_pImgClearBind;
	vgui::Label *m_pLblDescription;
	vgui::Label *m_pLblNotBound;
	char m_szLabel[256];
	char m_szBind[64];
	bool m_bUseRowLayout;
	bool m_bCapturing;
	static int s_iCursorX;
	static int s_iCursorY;
	CUtlStringList m_AlternateBind;

	CPanelAnimationVar( vgui::HFont, m_hButtonFont, "buttonfont", "GameUIButtonsTinier" );
};

class CRD_VGUI_Settings_Controls : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Controls, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Controls( vgui::Panel *parent, const char *panelName );

	void Activate() override;
	void OnThink() override;
	void OnCommand( const char *command ) override;
	void Paint() override;
	BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnControls; }

	CRD_VGUI_Bind *m_pBindMoveForward;
	CRD_VGUI_Bind *m_pBindMoveLeft;
	CRD_VGUI_Bind *m_pBindMoveBack;
	CRD_VGUI_Bind *m_pBindMoveRight;
	CRD_VGUI_Bind *m_pBindWalk;
	CRD_VGUI_Bind *m_pBindJump;
	vgui::Label *m_pLblLeftStickAction;
	vgui::Label *m_pLblRightStickAction;
	bool m_bMoveStickLeft : 1, m_bMoveStickRight : 1;
	bool m_bLookStickLeft : 1, m_bLookStickRight : 1;

	CRD_VGUI_Option *m_pSettingAutoWalk;
	CRD_VGUI_Option *m_pSettingAutoAttack;
	CRD_VGUI_Option *m_pSettingAimToMovement;
	CRD_VGUI_Option *m_pSettingControllerGlyphs;

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
	CRD_VGUI_Option *m_pSettingDeveloperConsole;

	CPanelAnimationVar( vgui::HFont, m_hButtonFont, "buttonfont", "GameUIButtonsTiny" );
	CPanelAnimationVarAliasType( float, m_flStickTestDistance, "stick_test_distance", "4", "proportional_float" );
};

class CRD_VGUI_Settings_Options_1 : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Options_1, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Options_1( vgui::Panel *parent, const char *panelName );

	void Activate() override;
	void OnCommand( const char *command ) override;
	BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnOptions1; }
	MESSAGE_FUNC_PTR( OnCurrentOptionChanged, "CurrentOptionChanged", panel );

	CRD_VGUI_Option *m_pSettingPlayerNameMode;
	CRD_VGUI_Option *m_pSettingPlayerChatColor;
	CRD_VGUI_Option *m_pSettingPlayerChatNamesUseColors;
	CRD_VGUI_Option *m_pSettingPlayerDeathmatchTeamColorMode;
	CRD_VGUI_Option *m_pSettingPlayerDeathmatchDrawTopScoreboard;

	CRD_VGUI_Option *m_pSettingHintsFailAdvice;
	CRD_VGUI_Option *m_pSettingHintsGameInstructor;
	BaseModUI::BaseModHybridButton *m_pBtnResetGameInstructor;
	CRD_VGUI_Option *m_pSettingHintsDeathmatchRespawn;
	CRD_VGUI_Option *m_pSettingHintsSwarmopediaGrid;
	CRD_VGUI_Option *m_pSettingHintsSwarmopediaUnits;

	CRD_VGUI_Option *m_pSettingDeathCamTakeover;
	CRD_VGUI_Option *m_pSettingDeathCamSlowdown;
	CRD_VGUI_Option *m_pSettingDeathMarineGibs;

	CRD_VGUI_Option *m_pSettingControlsRightClickWireHack;
	CRD_VGUI_Option *m_pSettingControlsSniperSwapWeapons;
	CRD_VGUI_Option *m_pSettingControlsLockMouseToWindow;

	CRD_VGUI_Option *m_pSettingCrosshairMarineLabelDist;
	CRD_VGUI_Option *m_pSettingCrosshairType;
	CRD_VGUI_Option *m_pSettingCrosshairSize;
	CRD_VGUI_Option *m_pSettingCrosshairLaserSight;

	CRD_VGUI_Option *m_pSettingReloadAuto;
	CRD_VGUI_Option *m_pSettingReloadFastUnderMarine;
	CRD_VGUI_Option *m_pSettingReloadFastWide;
	CRD_VGUI_Option *m_pSettingReloadFastTall;
};

class CRD_VGUI_Settings_Options_2 : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Options_2, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Options_2( vgui::Panel *parent, const char *panelName );

	void Activate() override;
	BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnOptions2; }
	MESSAGE_FUNC_PTR( OnCurrentOptionChanged, "CurrentOptionChanged", panel );

	CRD_VGUI_Option *m_pSettingDamageNumbers;
	CRD_VGUI_Option *m_pSettingStrangeRankUp;

	CRD_VGUI_Option *m_pSettingSpeedTimer;
	CRD_VGUI_Option *m_pSettingSpeedTimerColor;
	CRD_VGUI_Option *m_pSettingSpeedObjectivesInChat;
	CRD_VGUI_Option *m_pSettingSpeedAutoRestartMission;

	CRD_VGUI_Option *m_pSettingLeaderboardPrivateStats;
	CRD_VGUI_Option *m_pSettingLeaderboardSend;
	CRD_VGUI_Option *m_pSettingLeaderboardLoading;
	CRD_VGUI_Option *m_pSettingLeaderboardDebrief;

	CRD_VGUI_Option *m_pSettingLoadingMissionIcons;
	CRD_VGUI_Option *m_pSettingLoadingMissionScreens;
	CRD_VGUI_Option *m_pSettingLoadingStatusText;

	CRD_VGUI_Option *m_pSettingAccessibilityTracerTintSelf;
	CRD_VGUI_Option *m_pSettingAccessibilityTracerTintOther;
	CRD_VGUI_Option *m_pSettingAccessibilityHighlightActiveCharacter;
	CRD_VGUI_Option *m_pSettingAccessibilityReduceMotion;
	CRD_VGUI_Option *m_pSettingAccessibilityCameraShake;
	CRD_VGUI_Option *m_pSettingAccessibilityCameraShift;
	CRD_VGUI_Option *m_pSettingAccessibilityMinimapClicks;
	CRD_VGUI_Option *m_pSettingAccessibilityMoveRelativeToAim;

	CRD_VGUI_Option *m_pSettingNetworkInterpolation;
	CRD_VGUI_Option *m_pSettingNetworkRate;
	CRD_VGUI_Option *m_pSettingNetworkAllowRelay;
};

class CRD_VGUI_Settings_Audio : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Audio, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Audio( vgui::Panel *parent, const char *panelName );

	void Activate() override;
	void OnCommand( const char *command ) override;
	BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnAudio; }
	MESSAGE_FUNC_PTR( OnCurrentOptionChanged, "CurrentOptionChanged", panel );

	CRD_VGUI_Option *m_pMixerOverallVolume;
	CRD_VGUI_Option *m_pMixerMusicMenus;
	CRD_VGUI_Option *m_pMixerMusicInGame;
	CRD_VGUI_Option *m_pMixerVoiceDialogue;
	CRD_VGUI_Option *m_pMixerVoicePlayers;
	CRD_VGUI_Option *m_pMixerVoiceAliens;
	CRD_VGUI_Option *m_pMixerInterface;
	CRD_VGUI_Option *m_pMixerEnvironmentAmbient;
	CRD_VGUI_Option *m_pMixerEnvironmentPhysics;
	CRD_VGUI_Option *m_pMixerExplosions;
	CRD_VGUI_Option *m_pMixerCombatDamage;
	CRD_VGUI_Option *m_pMixerCombatWeapons;
	CRD_VGUI_Option *m_pMixerOther;

	CRD_VGUI_Option *m_pSettingSpeakerConfiguration;
	CRD_VGUI_Option *m_pSettingSoundQuality;
	CRD_VGUI_Option *m_pSettingCaptioning;
	CRD_VGUI_Option *m_pSettingStoryDialogue;
	CRD_VGUI_Option *m_pSettingLowHealthSound;

	BaseModUI::BaseModHybridButton *m_pBtnCustomizeCombatMusic;

	CRD_VGUI_Option *m_pSettingVoiceChat;
	CRD_VGUI_Option *m_pSettingVoiceSensitivity;
	CRD_VGUI_Option *m_pSettingVoiceIconPosition;
	CRD_VGUI_Option *m_pSettingBoostMicrophoneGain;
	CRD_VGUI_Microphone_Tester *m_pTestMicrophone;

	CRD_VGUI_Option *m_pSettingHitSoundType;
	CRD_VGUI_Option *m_pSettingHitSoundVolume;

	CRD_VGUI_Option *m_pSettingKillSoundType;
	CRD_VGUI_Option *m_pSettingKillSoundVolume;
};

class CRD_VGUI_Settings_Video : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_Video, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_Video( vgui::Panel *parent, const char *panelName );

	void Activate() override;
	BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnVideo; }
	MESSAGE_FUNC_PTR( OnCurrentOptionChanged, "CurrentOptionChanged", panel );

	CRD_VGUI_Option *m_pSettingScreenResolution;
	CRD_VGUI_Option *m_pSettingDisplayMode;
	CRD_VGUI_Option *m_pSettingScreenBrightness;
	CRD_VGUI_Option *m_pSettingRenderingPipeline;
	CRD_VGUI_Option *m_pSettingVSync;

	CRD_VGUI_Option *m_pSettingEffectDetail;
	CRD_VGUI_Option *m_pSettingShaderDetail;
	CRD_VGUI_Option *m_pSettingTextureDetail;
	CRD_VGUI_Option *m_pSettingAntiAliasing;
	CRD_VGUI_Option *m_pSettingFiltering;

	CRD_VGUI_Option *m_pSettingFilmGrain;
	CRD_VGUI_Option *m_pSettingLocalContrast;
	CRD_VGUI_Option *m_pSettingDepthBlur;
	CRD_VGUI_Option *m_pSettingWeatherEffects;
	CRD_VGUI_Option *m_pSettingBloomScale;
	CRD_VGUI_Option *m_pSettingProjectedTextures;
	CRD_VGUI_Option *m_pSettingFlashlightShadows;
	CRD_VGUI_Option *m_pSettingFlashlightLightSpill;
	CRD_VGUI_Option *m_pSettingHighQualityBeacons;
	CRD_VGUI_Option *m_pSettingMuzzleFlashLights;
	CRD_VGUI_Option *m_pSettingAlienShadows;
	CRD_VGUI_Option *m_pSettingLowHealthEffect;
};

class CRD_VGUI_Settings_About : public CRD_VGUI_Settings_Panel_Base
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Settings_About, CRD_VGUI_Settings_Panel_Base );
public:
	CRD_VGUI_Settings_About( vgui::Panel *parent, const char *panelName );

	void Activate() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;
	BaseModUI::BaseModHybridButton *GetButton( BaseModUI::CRD_VGUI_Settings *pSettings ) override { return pSettings->m_pBtnAbout; }

	vgui::Label *m_pLblBuildID;
	vgui::Label *m_pLblNetworkVersion;
	vgui::Label *m_pLblCurrentBranch;
	vgui::Label *m_pLblWineVersion;
	vgui::ImagePanel *m_pImgSourceEngine;
	BaseModUI::GenericPanelList *m_pCopyrightDisclaimers;
};
