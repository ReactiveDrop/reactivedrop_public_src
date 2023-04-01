#include "cbase.h"
#include "asw_util_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_inhabitable_npc.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_asw_game_resource.h"
#include "clientmode_asw.h"
#include "asw_hudelement.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/TextImage.h>
#define CRD_Hud_Counter C_RD_Hud_Counter
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
#include "asw_inhabitable_npc.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRD_Hud_Counter : public CBaseEntity
{
public:
	DECLARE_CLASS( CRD_Hud_Counter, CBaseEntity );
	DECLARE_NETWORKCLASS();

	CRD_Hud_Counter();
	~CRD_Hud_Counter();

#ifdef CLIENT_DLL
	char m_szLabel[128];
	wchar_t m_wszLabel[128];
	char m_szFormat[128];
	wchar_t m_wszFormat[128];

	float m_flLastChanged;
	int m_iPrevCount;
	int m_iCurCount;

	void OnDataChanged( DataUpdateType_t updateType ) override;
	int GetInterpolatedCount();
#else
	DECLARE_DATADESC();
	CNetworkVar( string_t, m_szLabel );
	CNetworkVar( string_t, m_szFormat );

	bool m_bDisabled;
	int m_iMinCount;
	int m_iMaxCount;

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputSetLimitNPC( inputdata_t &inputdata );
	void InputClearLimitNPC( inputdata_t &inputdata );
	void InputAdd( inputdata_t &inputdata );
	void InputSubtract( inputdata_t &inputdata );
	int UpdateTransmitState() override;
	int ShouldTransmit( const CCheckTransmitInfo *pInfo ) override;
#endif

	CNetworkVar( int, m_iCount );
	CNetworkVar( int, m_iOrder );
	CNetworkVar( float, m_flHoldTime );
	CNetworkVar( float, m_flInterpTime );
	CNetworkHandle( CASW_Inhabitable_NPC, m_hLimitNPC );
	CNetworkVar( bool, m_bLimitByViewNPC );
};

IMPLEMENT_NETWORKCLASS_ALIASED( RD_Hud_Counter, DT_RD_Hud_Counter );

BEGIN_NETWORK_TABLE( CRD_Hud_Counter, DT_RD_Hud_Counter )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iCount ) ),
	RecvPropString( RECVINFO( m_szLabel ) ),
	RecvPropString( RECVINFO( m_szFormat ) ),
	RecvPropInt( RECVINFO( m_iOrder ) ),
	RecvPropFloat( RECVINFO( m_flHoldTime ) ),
	RecvPropFloat( RECVINFO( m_flInterpTime ) ),
	RecvPropEHandle( RECVINFO( m_hLimitNPC ) ),
	RecvPropBool( RECVINFO( m_bLimitByViewNPC ) ),
#else
	SendPropInt( SENDINFO( m_iCount ), -1, SPROP_CHANGES_OFTEN ),
	SendPropStringT( SENDINFO( m_szLabel ) ),
	SendPropStringT( SENDINFO( m_szFormat ) ),
	SendPropInt( SENDINFO( m_iOrder ) ),
	SendPropFloat( SENDINFO( m_flHoldTime ) ),
	SendPropFloat( SENDINFO( m_flInterpTime ) ),
	SendPropEHandle( SENDINFO( m_hLimitNPC ) ),
	SendPropBool( SENDINFO( m_bLimitByViewNPC ) ),
#endif
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
static CUtlVector< CRD_Hud_Counter * > s_pHudCounters;

static int __cdecl SortByOrderAndEntIndex( CRD_Hud_Counter *const *a, CRD_Hud_Counter *const *b )
{
	if ( ( *a )->m_iOrder != ( *b )->m_iOrder )
	{
		return ( *a )->m_iOrder - ( *b )->m_iOrder;
	}

	return ( *a )->entindex() - ( *b )->entindex();
}
#else
LINK_ENTITY_TO_CLASS( rd_hud_counter, CRD_Hud_Counter );

BEGIN_DATADESC( CRD_Hud_Counter )
	DEFINE_INPUT( m_iCount, FIELD_INTEGER, "SetCount" ),
	DEFINE_KEYFIELD( m_iMinCount, FIELD_INTEGER, "MinCount" ),
	DEFINE_KEYFIELD( m_iMaxCount, FIELD_INTEGER, "MaxCount" ),
	DEFINE_KEYFIELD( m_szLabel, FIELD_STRING, "label" ),
	DEFINE_KEYFIELD( m_szFormat, FIELD_STRING, "format" ),
	DEFINE_KEYFIELD( m_iOrder, FIELD_INTEGER, "order" ),
	DEFINE_KEYFIELD( m_flHoldTime, FIELD_FLOAT, "holdtime" ),
	DEFINE_KEYFIELD( m_flInterpTime, FIELD_FLOAT, "interptime" ),
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "SetLimitNPC", InputSetLimitNPC ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ClearLimitNPC", InputClearLimitNPC ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "Add", InputAdd ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "Subtract", InputSubtract ),
	DEFINE_FIELD( m_hLimitNPC, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bLimitByViewNPC, FIELD_BOOLEAN ),
END_DATADESC();
#endif

CRD_Hud_Counter::CRD_Hud_Counter()
{
	m_iCount = 0;
	m_iOrder = 0;
	m_flHoldTime = 0.5f;
	m_flInterpTime = 0.5f;
	m_hLimitNPC = NULL;
	m_bLimitByViewNPC = false;

#ifdef CLIENT_DLL
	m_szLabel[0] = '\0';
	m_wszLabel[0] = L'\0';
	m_szFormat[0] = '\0';
	m_wszFormat[0] = L'\0';
	m_flLastChanged = 0.0f;
	m_iCurCount = m_iPrevCount = m_iCount;

	s_pHudCounters.AddToTail( this );
#else
	m_szLabel = NULL_STRING;
	m_szFormat = AllocPooledStringConstant( "%s1" );
	m_iMinCount = 0;
	m_iMaxCount = 0;
	m_bDisabled = false;
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
#endif
}

CRD_Hud_Counter::~CRD_Hud_Counter()
{
#ifdef CLIENT_DLL
	s_pHudCounters.FindAndRemove( this );
#endif
}

#ifdef CLIENT_DLL
void CRD_Hud_Counter::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		// Optimization: these should not be changing once we see the entity on the client.
		TryLocalize( m_szLabel, m_wszLabel, sizeof( m_wszLabel ) );
		TryLocalize( m_szFormat, m_wszFormat, sizeof( m_wszFormat ) );
		s_pHudCounters.Sort( &SortByOrderAndEntIndex );

		m_iCurCount = m_iPrevCount = m_iCount;
		m_flLastChanged = gpGlobals->curtime;
	}
	else if ( m_iCurCount != m_iCount )
	{
		m_iPrevCount = GetInterpolatedCount();
		m_iCurCount = m_iCount;
		m_flLastChanged = gpGlobals->curtime;
	}
}

int CRD_Hud_Counter::GetInterpolatedCount()
{
	float flSinceChange = gpGlobals->curtime - m_flLastChanged - m_flHoldTime;
	if ( flSinceChange < 0.0f )
	{
		return m_iPrevCount;
	}

	if ( flSinceChange >= m_flInterpTime )
	{
		return m_iCurCount;
	}

	return RemapValClamped( flSinceChange, 0.0f, m_flInterpTime, m_iPrevCount, m_iCurCount );
}
#else
void CRD_Hud_Counter::InputEnable( inputdata_t &inputdata )
{
	m_bDisabled = false;
	DispatchUpdateTransmitState();
}

void CRD_Hud_Counter::InputDisable( inputdata_t &inputdata )
{
	m_bDisabled = true;
	DispatchUpdateTransmitState();
}

void CRD_Hud_Counter::InputSetLimitNPC( inputdata_t &inputdata )
{
	CBaseEntity *pTarget = inputdata.value.Entity();
	if ( pTarget && !pTarget->IsInhabitableNPC() )
	{
		Warning( "#%d:%s: rd_hud_counter->SetLimitNPC: %s is not an inhabitable NPC\n", entindex(), GetDebugName(), pTarget->GetDebugName() );
		return;
	}

	m_bLimitByViewNPC = true;
	m_hLimitNPC = assert_cast< CASW_Inhabitable_NPC * >( pTarget );
	DispatchUpdateTransmitState();
}

void CRD_Hud_Counter::InputClearLimitNPC( inputdata_t &inputdata )
{
	m_bLimitByViewNPC = false;
	m_hLimitNPC = NULL;
	DispatchUpdateTransmitState();
}

void CRD_Hud_Counter::InputAdd( inputdata_t &inputdata )
{
	if ( m_iMinCount == m_iMaxCount )
	{
		Warning( "#%d:%s: rd_hud_counter->Add: min and max count are both %d\n", entindex(), GetDebugName(), m_iMinCount );
	}

	m_iCount = clamp( m_iCount + inputdata.value.Int(), m_iMinCount, m_iMaxCount );
}

void CRD_Hud_Counter::InputSubtract( inputdata_t &inputdata )
{
	if ( m_iMinCount == m_iMaxCount )
	{
		Warning( "#%d:%s: rd_hud_counter->Subtract: min and max count are both %d\n", entindex(), GetDebugName(), m_iMinCount );
	}

	m_iCount = clamp( m_iCount - inputdata.value.Int(), m_iMinCount, m_iMaxCount );
}

int CRD_Hud_Counter::UpdateTransmitState()
{
	if ( m_bDisabled )
	{
		return SetTransmitState( FL_EDICT_DONTSEND );
	}

	if ( m_bLimitByViewNPC )
	{
		return SetTransmitState( FL_EDICT_FULLCHECK );
	}

	return SetTransmitState( FL_EDICT_ALWAYS );
}

int CRD_Hud_Counter::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	if ( !m_bDisabled && !m_bLimitByViewNPC )
	{
		return FL_EDICT_ALWAYS;
	}

	if ( m_bDisabled || ( m_bLimitByViewNPC && !m_hLimitNPC ) )
	{
		return FL_EDICT_DONTSEND;
	}

	CASW_ViewNPCRecipientFilter filter( m_hLimitNPC );
	CBaseEntity *pSendToPlayer = CBaseEntity::Instance( pInfo->m_pClientEnt );
	for ( int i = 0; i < filter.GetRecipientCount(); i++ )
	{
		if ( filter.GetRecipientIndex( i ) == pSendToPlayer->entindex() )
		{
			return FL_EDICT_ALWAYS;
		}
	}

	return FL_EDICT_DONTSEND;
}
#endif

#ifdef CLIENT_DLL
extern ConVar asw_money;

class CASWHudCounters : public CASW_HudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CASWHudCounters, vgui::EditablePanel );

public:
	CASWHudCounters( const char *pElementName );
	void OnThink() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	bool ShouldDraw() override;

	vgui::Label *m_pLabel;
};

DECLARE_HUDELEMENT( CASWHudCounters );

CASWHudCounters::CASWHudCounters( const char *pElementName ) : CASW_HudElement( pElementName ), BaseClass( NULL, "ASWHudCounters" )
{
	SetParent( GetClientMode()->GetViewport() );
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	SetScheme( scheme );

	m_pLabel = new vgui::Label( this, "CounterText", "" );
}

template<int N>
static void AppendFormatCounter( wchar_t( &wszAllCounters )[N], const wchar_t *wszLabel, const wchar_t *wszFormat, int iCount, int iChange )
{
	wchar_t wszRawNum[128]{};

	V_snwprintf( wszRawNum, NELEMS( wszRawNum ), L"%d", iCount );

	wchar_t wszNumber[128]{};

	g_pVGuiLocalize->ConstructString( wszNumber, sizeof( wszNumber ), wszFormat, 1, wszRawNum );

	const char *szChangeSuffix = "";
	if ( iChange > 0 )
	{
		szChangeSuffix = "_increase";
	}
	else if ( iChange < 0 )
	{
		szChangeSuffix = "_decrease";
		iChange = -iChange;
	}

	V_snwprintf( wszRawNum, NELEMS( wszRawNum ), L"%d", iChange );

	const char *szLabelSuffix = "";
	if ( !wszLabel || wszLabel[0] == '\0' )
	{
		szLabelSuffix = "_nolabel";
		wszLabel = L"";
	}

	char szFormatKey[128]{};
	V_snprintf( szFormatKey, sizeof( szFormatKey ), "#asw_hud_counter_format%s%s", szChangeSuffix, szLabelSuffix );

	wchar_t wszCounter[128]{};
	g_pVGuiLocalize->ConstructString( wszCounter, sizeof( wszCounter ), g_pVGuiLocalize->Find( szFormatKey ), 3, wszLabel, wszNumber, wszRawNum );

	if ( wszAllCounters[0] == L'\0' )
	{
		V_snwprintf( wszAllCounters, N, L"%s", wszCounter );
	}
	else
	{
		V_snwprintf( wszAllCounters, N, L"%s\n%s", wszAllCounters, wszCounter );
	}
}

void CASWHudCounters::OnThink()
{
	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pLocalPlayer )
		return;

	C_ASW_Inhabitable_NPC *pViewNPC = pLocalPlayer->GetViewNPC();
	if ( !pViewNPC )
		return;

	wchar_t wszAllCounters[2048]{};

	FOR_EACH_VEC( s_pHudCounters, i )
	{
		C_RD_Hud_Counter *pCounter = s_pHudCounters[i];
		if ( pCounter->m_iOrder >= 0 )
			break;

		if ( pCounter->IsDormant() )
			continue;

		if ( pCounter->m_bLimitByViewNPC && pCounter->m_hLimitNPC.Get() != pViewNPC )
			continue;

		int iCount = pCounter->GetInterpolatedCount();
		int iChange = pCounter->m_iCurCount - iCount;

		AppendFormatCounter( wszAllCounters, pCounter->m_wszLabel, pCounter->m_wszFormat, iCount, iChange );
	}

	if ( ASWGameResource() && asw_money.GetBool() )
	{
		AppendFormatCounter( wszAllCounters, L"", g_pVGuiLocalize->Find( "#asw_money_format" ), ASWGameResource()->GetMoney(), 0 );
	}

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pViewNPC );
	CASW_Marine_Resource *pMR = pMarine ? pMarine->GetMarineResource() : NULL;
	if ( pMR && pMR->m_iScore >= 0 )
	{
		int iScore = pMR->GetInterpolatedScore();
		AppendFormatCounter( wszAllCounters, L"", g_pVGuiLocalize->Find( "#asw_holdout_hud_score" ), iScore, pMR->m_iCurScore - iScore );
	}

	FOR_EACH_VEC( s_pHudCounters, i )
	{
		C_RD_Hud_Counter *pCounter = s_pHudCounters[i];
		if ( pCounter->m_iOrder < 0 )
			continue;

		if ( pCounter->IsDormant() )
			continue;

		if ( pCounter->m_bLimitByViewNPC && pCounter->m_hLimitNPC.Get() != pViewNPC )
			continue;

		int iCount = pCounter->GetInterpolatedCount();
		int iChange = pCounter->m_iCurCount - iCount;

		AppendFormatCounter( wszAllCounters, pCounter->m_wszLabel, pCounter->m_wszFormat, iCount, iChange );
	}

	m_pLabel->SetText( wszAllCounters );
	m_pLabel->GetTextImage()->ResizeImageToContent();

	int iExpectedTall = m_pLabel->GetTextImage()->GetTall();
	int iCurrentTall = m_pLabel->GetTall();
	if ( iExpectedTall != iCurrentTall )
	{
		m_pLabel->SetTall( iExpectedTall );
		int iDiff = iExpectedTall - iCurrentTall;
		SetTall( GetTall() + iDiff );
		int x, y;
		GetPos( x, y );
		SetPos( x, y - iDiff );
	}
}

void CASWHudCounters::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/UI/HudCounters.res" );
}

bool CASWHudCounters::ShouldDraw()
{
	if ( !CASW_HudElement::ShouldDraw() )
		return false;

	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pLocalPlayer )
		return false;

	C_ASW_Inhabitable_NPC *pViewNPC = pLocalPlayer->GetViewNPC();
	if ( !pViewNPC )
		return false;

	if ( asw_money.GetBool() )
		return true;

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pViewNPC );
	CASW_Marine_Resource *pMR = pMarine ? pMarine->GetMarineResource() : NULL;
	if ( pMR && pMR->m_iScore >= 0 )
		return true;

	FOR_EACH_VEC( s_pHudCounters, i )
	{
		if ( s_pHudCounters[i]->IsDormant() )
			continue;

		if ( !s_pHudCounters[i]->m_bLimitByViewNPC )
			return true;

		if ( s_pHudCounters[i]->m_hLimitNPC.Get() == pViewNPC )
			return true;
	}

	return false;
}
#endif
