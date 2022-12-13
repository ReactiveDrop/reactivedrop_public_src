#pragma once

#include "steam/isteaminput.h"

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
	bool IsOriginPlaceholderString( const char *szKey );
	const char *NameForOrigin( EInputActionOrigin eOrigin );
	const char *NameForOrigin( const char *szKey );
	vgui::HTexture GlyphForOrigin( EInputActionOrigin eOrigin );
	void DrawLegacyControllerGlyph( const char *szKey, int x, int y, int iCenterX, int iCenterY, vgui::HFont hFont, int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT() );
	bool GetGameAxes( int nSlot, float *flMoveX, float *flMoveY, float *flLookX, float *flLookY );

	STEAM_CALLBACK( CRD_Steam_Input, OnSteamInputDeviceConnected, SteamInputDeviceConnected_t );
	STEAM_CALLBACK( CRD_Steam_Input, OnSteamInputDeviceDisconnected, SteamInputDeviceDisconnected_t );
	static void OnActionEvent( SteamInputActionEvent_t *pEvent );

	bool m_bInitialized;
	CUtlVectorAutoPurge<CRD_Steam_Controller *> m_Controllers;
	CUtlMap<EInputActionOrigin, vgui::HTexture> m_GlyphTextures;
	struct
	{
		InputActionSetHandle_t InGame;
	} m_ActionSets;
	struct
	{
	} m_ActionSetLayers;
	struct
	{
		InputAnalogActionHandle_t Move;
		InputAnalogActionHandle_t Look;
	} m_AnalogActions;
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
	int m_SplitScreenPlayerIndex;
};

class CRD_Steam_Input_Bind final
{
public:
	CRD_Steam_Input_Bind( const char *szActionName, const char *szBind );

private:
	const char *m_szActionName;
	const char *m_szBind;
	InputDigitalActionHandle_t m_hAction;

	CRD_Steam_Input_Bind *m_pNext;
	static CRD_Steam_Input_Bind *s_pBinds;

	friend class CRD_Steam_Input;
	friend class CRD_Steam_Controller;
};

#define RD_STEAM_INPUT_BIND( ActionName, szCommand ) CRD_Steam_Input_Bind SteamInputBind_##ActionName( #ActionName, szCommand )

extern CRD_Steam_Input g_RD_Steam_Input;
