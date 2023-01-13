#include "cbase.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/Tooltip.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static class CRD_Tooltip_Fix final : public CAutoGameSystem
{
	DECLARE_CLASS_SIMPLE( CRD_Tooltip_Fix, CAutoGameSystem );
public:
	CRD_Tooltip_Fix() : BaseClass( "CRD_Tooltip_Fix" )
	{
	}

	void PostInit() override
	{
		// Creating a Tooltip here does two things:
		// - it gives us access to the vtable, and
		// - it initializes the TextEntry that we need to modify
		//
		// We leak the tooltip because normally when every tooltip is deleted,
		// the TextEntry gets deallocated as well.
		vgui::Tooltip *pTooltip = new vgui::Tooltip( NULL );

		// There's only one method in the vtable, so let's just grab it.
		auto pApplySchemeSettings = **reinterpret_cast< void( vgui::Tooltip:: *const *const * )( vgui::IScheme * ) >( pTooltip );

#ifdef _DEBUG
		// This is a thunk, so we need to grab the real one.
		byte *pThunk = *reinterpret_cast< byte *const * >( &pApplySchemeSettings );
		Assert( *pThunk == 0xE9 );
		byte *pRealFunc = pThunk + 5 + *reinterpret_cast< const intptr_t * >( pThunk + 1 );
#else
		// ...except in release builds, where it's not a thunk.
		byte *pRealFunc = *reinterpret_cast< byte *const * >( &pApplySchemeSettings );
#endif
		Assert( *pRealFunc == 0xB9 );

		// Grab the TextEntry handle that is read at the start of the method.
		vgui::DHANDLE<vgui::TextEntry> &hTextEntry = **reinterpret_cast< vgui::DHANDLE<vgui::TextEntry> *const * >( pRealFunc + 1 );
		Assert( hTextEntry );

		vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmScheme" );
		hTextEntry->SetProportional( true );
		hTextEntry->SetScheme( hScheme );
		hTextEntry->MakeReadyForUse();
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( hScheme );
		hTextEntry->SetFont( pScheme->GetFont( "Default", true ) );
	}
} s_RD_Tooltip_Fix;
