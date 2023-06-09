#include "cbase.h"
#include "vcontracts.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

Contracts::Contracts( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );



	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

Contracts::~Contracts()
{
}
