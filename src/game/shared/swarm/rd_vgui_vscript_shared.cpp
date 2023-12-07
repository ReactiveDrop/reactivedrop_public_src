#include "cbase.h"
#include "rd_vgui_vscript_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_inhabitable_npc.h"
#include "c_asw_player.h"
#include "asw_input.h"
#include "rd_steam_input.h"
#include "inputsystem/iinputsystem.h"
#include "controller_focus.h"
#include <vgui_controls/Panel.h>
#else
#include "asw_inhabitable_npc.h"
#include "asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
CUtlVector<CRD_VGui_VScript *> CRD_VGui_VScript::s_InteractiveHUDEntities;

BEGIN_PREDICTION_DATA( CRD_VGui_VScript )
	DEFINE_PRED_FIELD( m_hDataEntity, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY( m_szDataString, FIELD_CHARACTER, 256, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY( m_flDataFloat, FIELD_FLOAT, 32, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY( m_iDataInt, FIELD_INTEGER, 64, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA();
#else
LINK_ENTITY_TO_CLASS( rd_vgui_vscript, CRD_VGui_VScript );

BEGIN_DATADESC( CRD_VGui_VScript )
END_DATADESC();
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( RD_VGui_VScript, DT_RD_VGui_VScript );

BEGIN_NETWORK_TABLE( CRD_VGui_VScript, DT_RD_VGui_VScript )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iRandomCheck ) ),
	RecvPropEHandle( RECVINFO( m_hInteracter ) ),
#else
	SendPropInt( SENDINFO( m_iRandomCheck ) ),
	SendPropEHandle( SENDINFO( m_hInteracter ) ),
#endif
END_NETWORK_TABLE();

BEGIN_ENT_SCRIPTDESC( CRD_VGui_VScript, CRD_HUD_VScript, "Alien Swarm: Reactive Drop scriptable interactive UI" )
#ifdef CLIENT_DLL
	DEFINE_SCRIPTFUNC( SendInput, "Sends a numeric input to the server." )
	DEFINE_SCRIPTFUNC( SetEntity, "Predict a change to the value of an entity parameter. (Only during Input.)" )
	DEFINE_SCRIPTFUNC( SetInt, "Predict a change to the value of an integer parameter. (Only during Input.)" )
	DEFINE_SCRIPTFUNC( SetFloat, "Predict a change to the value of a float parameter. (Only during Input.)" )
	DEFINE_SCRIPTFUNC( SetString, "Predict a change to the value of a string parameter. (Only during Input.)" )
	DEFINE_SCRIPTFUNC( CreateButton, "Creates a button that can be targeted by both the mouse and the controller." )
#else
	DEFINE_SCRIPTFUNC( SetInteracter, "Sets the character who is allowed to interact with this screen." )
#endif
	DEFINE_SCRIPTFUNC( GetInteracter, "Gets the character who is allowed to interact with this screen." )
END_SCRIPTDESC();

#ifdef CLIENT_DLL
class CRD_VGui_VScript_Button_Panel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CRD_VGui_VScript_Button_Panel, vgui::Panel );
public:
	CRD_VGui_VScript_Button_Panel( vgui::Panel *parent, const char *panelName, CRD_VGui_VScript_Button *pDef ) : BaseClass( parent, panelName )
	{
		Assert( !pDef->m_hPanel );
		pDef->m_hPanel = this;

		m_pDef = pDef;

		GetControllerFocus()->AddToFocusList( this );

		SetKeyBoardInputEnabled( false );
	}

	~CRD_VGui_VScript_Button_Panel()
	{
		Assert( m_pDef->m_hPanel.Get() == this );
		m_pDef->m_hPanel = NULL;

		GetControllerFocus()->RemoveFromFocusList( this );
	}

	void PerformLayout() override
	{
		BaseClass::PerformLayout();

		SetBounds( m_pDef->m_x, m_pDef->m_y, m_pDef->m_wide, m_pDef->m_tall );
	}

	void CursorThink( int x, int y, bool bOverParent )
	{
		int localX = x - m_pDef->m_x;
		int localY = y - m_pDef->m_y;

		bool bLastOver = m_bLastCursorOver;
		bool bMoved = x != m_iLastX || y != m_iLastY;
		bool bOver = bOverParent && localX >= 0 && localY >= 0 && localX < m_pDef->m_wide && localY < m_pDef->m_tall;

		m_iLastX = x;
		m_iLastY = y;
		m_bLastCursorOver = bOver;

		if ( bLastOver && !bOver )
		{
			m_pDef->OnCursorExited();
		}
		else if ( bOver && !bLastOver )
		{
			m_pDef->OnCursorEntered();
		}

		if ( bMoved && bOver )
		{
			m_pDef->OnCursorMoved( x, y );
		}
	}

	void OnMousePressed( vgui::MouseCode code ) override
	{
		BaseClass::OnMousePressed( code );

		m_pDef->OnMousePressed( code == MOUSE_RIGHT );
	}

	CRD_VGui_VScript_Button *m_pDef;
	int m_iLastX{ 0 }, m_iLastY{ 0 };
	bool m_bLastCursorOver{ false };
};

static const struct VScriptAllowedButton_t
{
	ButtonCode_t code;
	const char *name;
} s_VGuiVScriptButtons[] =
{
	{ MOUSE_LEFT, "mouse_left" },
	{ MOUSE_RIGHT, "mouse_right" },
	{ KEY_LEFT, "key_left" },
	{ KEY_RIGHT, "key_right" },
	{ KEY_UP, "key_up" },
	{ KEY_DOWN, "key_down" },
	{ KEY_SPACE, "key_space" },
	{ KEY_ENTER, "key_enter" },
	{ KEY_XSTICK1_LEFT, "controller_left" },
	{ KEY_XSTICK1_RIGHT, "controller_right" },
	{ KEY_XSTICK1_UP, "controller_up" },
	{ KEY_XSTICK1_DOWN, "controller_down" },
	{ KEY_XBUTTON_A, "controller_a" },
	{ KEY_XBUTTON_B, "controller_b" },
};
#endif

CRD_VGui_VScript::CRD_VGui_VScript()
{
	m_hInteracter = NULL;

#ifdef CLIENT_DLL
	s_InteractiveHUDEntities.AddToTail( this );
	SetPredictionEligible( true );
#else
	// random check number to make it slightly harder to write a script that interacts with custom screens directly
	m_iRandomCheck = RandomInt( 0, INT32_MAX );
#endif
}

CRD_VGui_VScript::~CRD_VGui_VScript()
{
#ifdef CLIENT_DLL
	s_InteractiveHUDEntities.FindAndRemove( this );

	if ( m_hButtonPanelParent )
	{
		ClearButtonPanels();
	}
#endif

	if ( m_ScriptScope.IsInitialized() )
	{
#ifdef CLIENT_DLL
		g_pScriptVM->ReleaseValue( m_hControlTable );
		m_ScriptScope.ReleaseFunction( m_hControlFunc );
#endif
		m_ScriptScope.ReleaseFunction( m_hInputFunc );
	}
}

#ifdef CLIENT_DLL
void CRD_VGui_VScript::SendInput( int value )
{
	if ( !m_bIsControlling )
	{
		Warning( "%s (%s): SendInput can only be called from the Control function.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( !AllowedToInteract() )
	{
		Warning( "%s (%s): called SendInput, but the current player is not allowed to interact with this screen.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	engine->ServerCmd( VarArgs( "cl_vgui_vscript_input %d %d %d\n", entindex(), m_iRandomCheck.Get(), value ) );
	m_QueuedInputsForPrediction.AddToTail( value );
}

void CRD_VGui_VScript::SetEntity( int i, HSCRIPT entity )
{
	if ( !m_bIsPredicting )
	{
		Warning( "%s (%s): SetEntity can only be called on the client from the Input function.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( i != 0 )
	{
		Warning( "Entity index %d is not allowed for %s (%s)\n", i, GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_hDataEntity.Set( ToEnt( entity ) );
}

void CRD_VGui_VScript::SetInt( int i, int value )
{
	if ( !m_bIsPredicting )
	{
		Warning( "%s (%s): SetInt can only be called on the client from the Input function.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( i < 0 || i >= m_iDataInt.Count() )
	{
		Warning( "Integer index %d is not allowed for %s (%s)\n", i, GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_iDataInt.Set( i, value );
}

void CRD_VGui_VScript::SetFloat( int i, float value )
{
	if ( !m_bIsPredicting )
	{
		Warning( "%s (%s): SetFloat can only be called on the client from the Input function.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( i < 0 || i >= m_flDataFloat.Count() )
	{
		Warning( "Float index %d is not allowed for %s (%s)\n", i, GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_flDataFloat.Set( i, value );
}

void CRD_VGui_VScript::SetString( int i, const char *string )
{
	if ( !m_bIsPredicting )
	{
		Warning( "%s (%s): SetString can only be called on the client from the Input function.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( i != 0 )
	{
		Warning( "String index %d is not allowed for %s (%s)\n", i, GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( V_strcmp( m_szDataString.Get(), string ) )
	{
		V_strncpy( m_szDataString.GetForModify(), string, sizeof( m_szDataString ) );
	}
}

void CRD_VGui_VScript::OnDataChanged( DataUpdateType_t type )
{
	bool bWantScriptInit = !m_ScriptScope.IsInitialized() && m_szClientVScript.Get()[0] != '\0';

	BaseClass::OnDataChanged( type );

	if ( bWantScriptInit && m_ScriptScope.IsInitialized() )
	{
		g_pScriptVM->CreateTable( m_hControlTable );
		m_hControlFunc = m_ScriptScope.LookupFunction( "Control" );
		m_hInputFunc = m_ScriptScope.LookupFunction( "Input" );

		if ( !m_hControlFunc || m_hControlFunc == INVALID_HSCRIPT )
		{
			Warning( "%s (%s) does not have a Control function in its script scope.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		}
		if ( !m_hInputFunc || m_hInputFunc == INVALID_HSCRIPT )
		{
			Warning( "%s (%s) does not have an Input function in its client script scope. (This function should match on the client and the server.)\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		}
	}
}

bool CRD_VGui_VScript::ShouldPredict()
{
	C_BasePlayer *pOwner = GetPredictionOwner();

	return pOwner && pOwner->IsLocalPlayer();
}

C_BasePlayer *CRD_VGui_VScript::GetPredictionOwner()
{
	CASW_Inhabitable_NPC *pInteracter = m_hInteracter;
	if ( !pInteracter || !pInteracter->IsInhabited() )
		return NULL;

	return pInteracter->GetCommander();
}

void CRD_VGui_VScript::PhysicsSimulate()
{
	if ( ShouldPredict() && m_QueuedInputsForPrediction.Count() && m_hInputFunc && m_hInputFunc != INVALID_HSCRIPT )
	{
		Assert( !m_bIsPredicting );
		Assert( !m_bIsInput );
		m_bIsPredicting = true;
		m_bIsInput = true;

		FOR_EACH_VEC( m_QueuedInputsForPrediction, i )
		{
			g_pScriptVM->Call( m_hInputFunc, m_ScriptScope, true, NULL, m_QueuedInputsForPrediction[i] );
		}
		m_QueuedInputsForPrediction.RemoveAll();

		m_bIsPredicting = false;
		m_bIsInput = false;

		if ( m_hUpdateFunc && m_hUpdateFunc != INVALID_HSCRIPT )
		{
			g_pScriptVM->Call( m_hUpdateFunc, m_ScriptScope );
		}
	}

	BaseClass::PhysicsSimulate();
}

bool CRD_VGui_VScript::AllowedToInteract()
{
	ASSERT_LOCAL_PLAYER_RESOLVABLE();

	C_BaseEntity *pOwner = GetPredictionOwner();

	return pOwner && pOwner == C_BasePlayer::GetLocalPlayer();
}

bool CRD_VGui_VScript::InterceptButtonPress( ButtonCode_t iButton )
{
	ASSERT_LOCAL_PLAYER_RESOLVABLE();
	if ( !AllowedToInteract() || !m_hControlFunc || m_hControlFunc == INVALID_HSCRIPT )
		return false;

	for ( int i = 0; i < NELEMS( s_VGuiVScriptButtons ); i++ )
	{
		if ( s_VGuiVScriptButtons[i].code == iButton )
		{
			RunControlFunction( iButton );
			return true;
		}
	}

	return false;
}

void CRD_VGui_VScript::RunControlFunction( ButtonCode_t iButton )
{
	if ( !AllowedToInteract() )
		return;

	if ( ( iButton == MOUSE_LEFT || iButton == MOUSE_RIGHT ) && m_iControllerFocusIndex != -1 )
		m_ButtonDefs[m_iControllerFocusIndex]->OnMousePressed( iButton == MOUSE_RIGHT );

	if ( !m_hControlFunc || m_hControlFunc == INVALID_HSCRIPT )
		return;

	UpdateControlTable( iButton );

	Assert( !m_bIsControlling );
	m_bIsControlling = true;
	g_pScriptVM->Call( m_hControlFunc, m_ScriptScope, true, NULL, m_hControlTable );
	m_bIsControlling = false;
}

void CRD_VGui_VScript::UpdateControlTable( ButtonCode_t iButton )
{
	CASWInput *pInput = ASWInput();
	Assert( pInput );
	if ( !pInput )
		return;

#ifdef DBGFLAG_ASSERT
	if ( iButton != BUTTON_CODE_NONE )
	{
		bool bAny = false;
		for ( int i = 0; i < NELEMS( s_VGuiVScriptButtons ); i++ )
		{
			bAny = s_VGuiVScriptButtons[i].code == iButton;
			if ( bAny )
				break;
		}

		Assert( bAny );
	}
#endif

	g_pScriptVM->SetValue( m_hControlTable, "mouse", !pInput->ControllerModeActiveMouse() );
	g_pScriptVM->SetValue( m_hControlTable, "keyboard", !pInput->ControllerModeActiveKeyboard() );
	g_pScriptVM->SetValue( m_hControlTable, "button", iButton );

	for ( int i = 0; i < NELEMS( s_VGuiVScriptButtons ); i++ )
	{
		g_pScriptVM->SetValue( m_hControlTable, s_VGuiVScriptButtons[i].name, g_pInputSystem->IsButtonDown( s_VGuiVScriptButtons[i].code ) );
	}

	int x, y;
	pInput->GetSimulatedFullscreenMousePos( &x, &y );
	g_pScriptVM->SetValue( m_hControlTable, "mouse_x", x );
	g_pScriptVM->SetValue( m_hControlTable, "mouse_y", y );

	float flMoveX, flMoveY, flLookX, flLookY;
	g_RD_Steam_Input.GetGameAxes( GET_ACTIVE_SPLITSCREEN_SLOT(), &flMoveX, &flMoveY, &flLookX, &flLookY );
	g_pScriptVM->SetValue( m_hControlTable, "move_x", flMoveX / MAX_BUTTONSAMPLE );
	g_pScriptVM->SetValue( m_hControlTable, "move_y", flMoveY / MAX_BUTTONSAMPLE );
	g_pScriptVM->SetValue( m_hControlTable, "look_x", flLookX / MAX_BUTTONSAMPLE );
	g_pScriptVM->SetValue( m_hControlTable, "look_y", flLookY / MAX_BUTTONSAMPLE );

	g_pScriptVM->SetValue( m_hControlTable, "focus", m_iControllerFocusIndex == -1 ? SCRIPT_VARIANT_NULL : m_ButtonDefs[m_iControllerFocusIndex]->m_hThis );
}

void CRD_VGui_VScript::InitButtonPanels( vgui::Panel *pParent )
{
	Assert( pParent );
	Assert( !m_hButtonPanelParent );
	Assert( !m_ButtonPanels.Count() );

	if ( m_hButtonPanelParent && m_ButtonPanels.Count() == m_ButtonDefs.Count() )
	{
		ClearButtonPanels();
	}

	m_hButtonPanelParent = pParent;

	m_ButtonPanels.SetCount( m_ButtonDefs.Count() );
	FOR_EACH_VEC( m_ButtonDefs, i )
	{
		m_ButtonPanels[i] = new CRD_VGui_VScript_Button_Panel( pParent, "VScriptButtonPanel", m_ButtonDefs[i] );
	}
}

void CRD_VGui_VScript::ClearButtonPanels()
{
	Assert( m_hButtonPanelParent );
	Assert( m_ButtonPanels.Count() == m_ButtonDefs.Count() );

	m_hButtonPanelParent = NULL;
	FOR_EACH_VEC( m_ButtonPanels, i )
	{
		vgui::Panel *pPanel = m_ButtonPanels[i];
		Assert( pPanel );
		if ( pPanel )
		{
			pPanel->MarkForDeletion();
		}
	}
	m_ButtonPanels.Purge();
}

void CRD_VGui_VScript::CursorThink( int x, int y, bool bOverParent )
{
	FOR_EACH_VEC( m_ButtonPanels, i )
	{
		assert_cast< CRD_VGui_VScript_Button_Panel * >( m_ButtonPanels[i].Get() )->CursorThink( x, y, bOverParent );
	}
}

HSCRIPT CRD_VGui_VScript::CreateButton()
{
	CRD_VGui_VScript_Button *pButton = new CRD_VGui_VScript_Button( this );
	int i = m_ButtonDefs.AddToTail( pButton );

	if ( m_hButtonPanelParent )
	{
		int j = m_ButtonPanels.AddToTail();
		Assert( i == j );
		m_ButtonPanels[j] = new CRD_VGui_VScript_Button_Panel( m_hButtonPanelParent, "VScriptButtonPanel", pButton );
	}

	return pButton->m_hThis;
}
#else
void CRD_VGui_VScript::OnInput( int value )
{
	if ( !m_hInputFunc || m_hInputFunc == INVALID_HSCRIPT )
		return;

	Assert( !m_bIsInput );
	m_bIsInput = true;
	g_pScriptVM->Call( m_hInputFunc, m_ScriptScope, true, NULL, value );
	m_bIsInput = false;
}

void CRD_VGui_VScript::SetInteracter( HSCRIPT interacter )
{
	if ( !AllowSetInteracter() )
	{
		Warning( "%s (%s) cannot call SetInteracter.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	CBaseEntity *pInteracter = ToEnt( interacter );
	if ( !pInteracter )
	{
		m_hInteracter = NULL;
		return;
	}

	if ( !pInteracter->IsInhabitableNPC() )
	{
		Warning( "%s (%s) called SetInteracter with unexpected entity type %s.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ), pInteracter->GetClassname() );
		return;
	}

	m_hInteracter = assert_cast< CASW_Inhabitable_NPC * >( pInteracter );
}

void CRD_VGui_VScript::RunVScripts()
{
	BaseClass::RunVScripts();

	m_hInputFunc = m_ScriptScope.IsInitialized() ? m_ScriptScope.LookupFunction( "Input" ) : INVALID_HSCRIPT;
	if ( !m_hInputFunc || m_hInputFunc == INVALID_HSCRIPT )
	{
		Warning( "%s (%s) does not have an Input function in its server script scope. (This function should match on the client and the server.)\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
	}
}
#endif

HSCRIPT CRD_VGui_VScript::GetInteracter()
{
	return ToHScript( m_hInteracter );
}

#ifdef CLIENT_DLL
BEGIN_SCRIPTDESC_ROOT( CRD_VGui_VScript_Button, "Alien Swarm: Reactive Drop scriptable interactive UI button" )
	DEFINE_SCRIPTFUNC( GetX, "Gets the x position of the button." )
	DEFINE_SCRIPTFUNC( GetY, "Gets the x position of the button." )
	DEFINE_SCRIPTFUNC( GetWide, "Gets the width of the button." )
	DEFINE_SCRIPTFUNC( GetTall, "Gets the height of the button." )
	DEFINE_SCRIPTFUNC( SetX, "Sets the y position of the button." )
	DEFINE_SCRIPTFUNC( SetY, "Sets the y position of the button." )
	DEFINE_SCRIPTFUNC( SetWide, "Sets the width of the button." )
	DEFINE_SCRIPTFUNC( SetTall, "Sets the height of the button." )
	DEFINE_SCRIPTFUNC( SetPos, "Sets the x and y position of the button." )
	DEFINE_SCRIPTFUNC( SetSize, "Sets the width and height of the button." )
	DEFINE_SCRIPTFUNC( SetBounds, "Sets the x and y position and width and height of the button." )
	DEFINE_SCRIPTFUNC( SetOnCursorMoved, "Sets a function that will be called when the cursor is moved over this button." )
	DEFINE_SCRIPTFUNC( SetOnCursorEntered, "Sets a function that will be called when the cursor moves onto this button." )
	DEFINE_SCRIPTFUNC( SetOnCursorExited, "Sets a function that will be called when the cursor moves off of this button." )
	DEFINE_SCRIPTFUNC( SetOnMousePressed, "Sets a function that will be called when a mouse button is pressed on this button." )
END_SCRIPTDESC();

CRD_VGui_VScript_Button::CRD_VGui_VScript_Button( CRD_VGui_VScript *pOwner )
{
	m_hThis = g_pScriptVM->RegisterInstance( this );
	m_pOwner = pOwner;
	m_x = 0;
	m_y = 0;
	m_wide = YRES( 30 );
	m_tall = YRES( 10 );
}

CRD_VGui_VScript_Button::~CRD_VGui_VScript_Button()
{
	if ( m_hThis )
	{
		g_pScriptVM->RemoveInstance( m_hThis );
	}
	m_hThis = NULL;
}

void CRD_VGui_VScript_Button::SetX( int x )
{
	m_x = x;

	if ( m_hPanel )
		m_hPanel->InvalidateLayout();
}

void CRD_VGui_VScript_Button::SetY( int y )
{
	m_y = y;

	if ( m_hPanel )
		m_hPanel->InvalidateLayout();
}

void CRD_VGui_VScript_Button::SetWide( int wide )
{
	m_wide = wide;

	if ( m_hPanel )
		m_hPanel->InvalidateLayout();
}

void CRD_VGui_VScript_Button::SetTall( int tall )
{
	m_tall = tall;

	if ( m_hPanel )
		m_hPanel->InvalidateLayout();
}

void CRD_VGui_VScript_Button::SetOnCursorMoved( HSCRIPT callback )
{
	char buf[256];
	g_pScriptVM->GenerateUniqueKey( "ButtonOnCursorMoved", buf, sizeof( buf ) );
	g_pScriptVM->SetValue( m_pOwner->m_ScriptScope, buf, callback );
	ScriptVariant_t ref;
	g_pScriptVM->GetValue( m_pOwner->m_ScriptScope, buf, &ref );
	m_hCursorMovedCallback = ref;
}

void CRD_VGui_VScript_Button::SetOnCursorEntered( HSCRIPT callback )
{
	char buf[256];
	g_pScriptVM->GenerateUniqueKey( "ButtonOnCursorEntered", buf, sizeof( buf ) );
	g_pScriptVM->SetValue( m_pOwner->m_ScriptScope, buf, callback );
	ScriptVariant_t ref;
	g_pScriptVM->GetValue( m_pOwner->m_ScriptScope, buf, &ref );
	m_hCursorEnteredCallback = ref;
}

void CRD_VGui_VScript_Button::SetOnCursorExited( HSCRIPT callback )
{
	char buf[256];
	g_pScriptVM->GenerateUniqueKey( "ButtonOnCursorExited", buf, sizeof( buf ) );
	g_pScriptVM->SetValue( m_pOwner->m_ScriptScope, buf, callback );
	ScriptVariant_t ref;
	g_pScriptVM->GetValue( m_pOwner->m_ScriptScope, buf, &ref );
	m_hCursorExitedCallback = ref;
}

void CRD_VGui_VScript_Button::SetOnMousePressed( HSCRIPT callback )
{
	char buf[256];
	g_pScriptVM->GenerateUniqueKey( "ButtonOnMousePressed", buf, sizeof( buf ) );
	g_pScriptVM->SetValue( m_pOwner->m_ScriptScope, buf, callback );
	ScriptVariant_t ref;
	g_pScriptVM->GetValue( m_pOwner->m_ScriptScope, buf, &ref );
	m_hMousePressedCallback = ref;
}

void CRD_VGui_VScript_Button::OnCursorMoved( int x, int y )
{
	if ( !m_hCursorMovedCallback || m_hCursorMovedCallback == INVALID_HSCRIPT )
		return;

	Assert( !m_pOwner->m_bIsControlling );
	m_pOwner->m_bIsControlling = true;
	g_pScriptVM->Call( m_hCursorMovedCallback, m_pOwner->m_ScriptScope, true, NULL, x, y );
	m_pOwner->m_bIsControlling = false;
}

void CRD_VGui_VScript_Button::OnCursorEntered()
{
	int i = m_pOwner->m_ButtonDefs.Find( this );
	Assert( m_pOwner->m_iControllerFocusIndex != i );
	m_pOwner->m_iControllerFocusIndex = i;
	Assert( m_pOwner->m_iControllerFocusIndex != -1 );

	if ( !m_hCursorEnteredCallback || m_hCursorEnteredCallback == INVALID_HSCRIPT )
		return;

	Assert( !m_pOwner->m_bIsControlling );
	m_pOwner->m_bIsControlling = true;
	g_pScriptVM->Call( m_hCursorEnteredCallback, m_pOwner->m_ScriptScope );
	m_pOwner->m_bIsControlling = false;
}

void CRD_VGui_VScript_Button::OnCursorExited()
{
	int i = m_pOwner->m_ButtonDefs.Find( this );
	if ( m_pOwner->m_iControllerFocusIndex == i )
		m_pOwner->m_iControllerFocusIndex = -1;

	if ( !m_hCursorExitedCallback || m_hCursorExitedCallback == INVALID_HSCRIPT )
		return;

	Assert( !m_pOwner->m_bIsControlling );
	m_pOwner->m_bIsControlling = true;
	g_pScriptVM->Call( m_hCursorExitedCallback, m_pOwner->m_ScriptScope );
	m_pOwner->m_bIsControlling = false;
}

void CRD_VGui_VScript_Button::OnMousePressed( bool right )
{
	if ( !m_hMousePressedCallback || m_hMousePressedCallback == INVALID_HSCRIPT )
		return;

	Assert( !m_pOwner->m_bIsControlling );
	m_pOwner->m_bIsControlling = true;
	g_pScriptVM->Call( m_hMousePressedCallback, m_pOwner->m_ScriptScope, true, NULL, right );
	m_pOwner->m_bIsControlling = false;
}
#endif
