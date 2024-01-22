#ifndef __QUICKJOIN_PUBLIC_H__
#define __QUICKJOIN_PUBLIC_H__

#include "VQuickJoin.h"

namespace BaseModUI
{
	class QuickJoinPublicPanel : public QuickJoinPanel
	{
		DECLARE_CLASS_SIMPLE( QuickJoinPublicPanel, QuickJoinPanel );

	public:
		QuickJoinPublicPanel( vgui::Panel *parent, const char *panelName );
		~QuickJoinPublicPanel();

		void OnMousePressed( vgui::MouseCode code ) override;
		void OnCommand( const char *command ) override;
		bool ShouldAlwaysBeVisible() const override { return true; }

	protected:
		void AddServersToList( void ) override;
		const char *GetTitle( void ) override { return "#L4D360UI_MainMenu_PublicLobbies"; }

		void RefreshQuery();
	};
};

#endif	// __QUICKJOIN_PUBLIC_H__
