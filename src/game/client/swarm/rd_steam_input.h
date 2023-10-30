#pragma once

#include "steam/isteaminput.h"

namespace vgui
{
	class IImage;
}

class CRD_Steam_Controller;
class C_ASW_Player;

// CRD_Steam_Input: game system that handles communication with the Steam Input API.
class CRD_Steam_Input final : public CAutoGameSystemPerFrame
{
public:
	CRD_Steam_Input();

	void PostInit() override;
	void Shutdown() override;
	void Update( float frametime ) override;

	CRD_Steam_Controller *FindController( InputHandle_t hController ) const;
	CRD_Steam_Controller *FindOrAddController( InputHandle_t hController );
	int GetJoystickCount();
	const char *Key_LookupBindingEx( const char *pBinding, int iUserId = -1, int iStartCount = 0, int iAllowJoystick = -1 );
	bool IsSteamInputBind( const char *szBinding );
	static const char *OriginPlaceholderString( EInputActionOrigin eOrigin );
	static EInputActionOrigin OriginFromPlaceholderString( const char *szKey );
	static bool IsOriginPlaceholderString( const char *szKey );
	static const char *NameForOrigin( EInputActionOrigin eOrigin );
	static const char *NameForOrigin( const char *szKey );
	vgui::HTexture GlyphForOrigin( EInputActionOrigin eOrigin );
	vgui::IImage *GlyphImageForOrigin( EInputActionOrigin eOrigin );
	void DrawLegacyControllerGlyph( const char *szKey, int x, int y, int iCenterX, int iCenterY, vgui::HFont hFont, int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT(), Color color = Color{ 255, 255, 255, 255 } );
	bool GetGameAxes( int nSlot, float *flMoveX, float *flMoveY, float *flLookX, float *flLookY );
	bool GetMenuNavigateOffset( int nSlot, float *flMenuNavigateX, float *flMenuNavigateY );
	InputActionSetHandle_t DetermineActionSet( CUtlVector<InputActionSetHandle_t> *pLayers, int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT() );

	STEAM_CALLBACK( CRD_Steam_Input, OnSteamInputDeviceConnected, SteamInputDeviceConnected_t );
	STEAM_CALLBACK( CRD_Steam_Input, OnSteamInputDeviceDisconnected, SteamInputDeviceDisconnected_t );
	static void OnActionEvent( SteamInputActionEvent_t *pEvent );

	bool m_bInitialized;
	CUtlVectorAutoPurge<CRD_Steam_Controller *> m_Controllers;
	CUtlMap<EInputActionOrigin, vgui::IImage *> m_GlyphTextures;
	struct
	{
		InputActionSetHandle_t InGame;
		InputActionSetHandle_t Menus;
	} m_ActionSets;
	struct
	{
		InputActionSetHandle_t InGameMenus;
	} m_ActionSetLayers;
	struct
	{
		InputAnalogActionHandle_t Move;
		InputAnalogActionHandle_t Look;
		InputAnalogActionHandle_t MenuNavigate;
	} m_AnalogActions;
	InputHandle_t m_hLastControllerWithEvent{};
};

// CRD_Steam_Controller: a gamepad/controller accessed via the Steam Input API. not necessarily a Steam controller.
class CRD_Steam_Controller final
{
public:
	CRD_Steam_Controller( InputHandle_t hController );
	~CRD_Steam_Controller();

	void OnConnected();
	void OnDisconnected();
	void OnFrame( ISteamInput *pSteamInput );
	void OnDigitalAction( InputDigitalActionHandle_t hAction, bool bState );
	void OnAnalogAction( InputAnalogActionHandle_t hAction, EInputSourceMode mode, float x, float y );

	const InputHandle_t m_hController;
	bool m_bConnected;
	bool m_bJustChangedActionSet;
	InputActionSetHandle_t m_hLastActionSet;
	CUtlVector<InputActionSetHandle_t> m_LastActionSetLayers;
	int m_SplitScreenPlayerIndex;
	Color m_LastPlayerColor;
};

class CRD_Steam_Input_Bind final
{
public:
	CRD_Steam_Input_Bind( const char *szActionName, const char *szBind, const char *szForceActionSet, bool bIgnoreOnActionSetChange = false );

private:
	const char *m_szActionName;
	const char *m_szBind;
	const char *m_szForceActionSet;
	InputDigitalActionHandle_t m_hAction;
	InputActionSetHandle_t m_hForceActionSet;
	bool m_bIgnoreOnActionSetChange;

	CRD_Steam_Input_Bind *m_pNext;
	static CRD_Steam_Input_Bind *s_pBinds;
	static CRD_Steam_Input_Bind *s_pLastBind;

	friend class CRD_Steam_Input;
	friend class CRD_Steam_Controller;
};

#define RD_STEAM_INPUT_BIND( ActionName, ... ) CRD_Steam_Input_Bind SteamInputBind_##ActionName( #ActionName, __VA_ARGS__ )

extern CRD_Steam_Input g_RD_Steam_Input;
