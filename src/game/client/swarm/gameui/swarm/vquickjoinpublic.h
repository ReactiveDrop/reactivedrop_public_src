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
		virtual ~QuickJoinPublicPanel();

		virtual void OnMousePressed( vgui::MouseCode code );
		virtual void OnCommand(const char *command);

	protected:
		virtual void AddServersToList( void );
		virtual const char *GetTitle( void ) { return "#L4D360UI_MainMenu_PublicLobbies"; }

		void RefreshQuery();
	};
};

#endif	// __QUICKJOIN_PUBLIC_H__
