#ifndef RD_CHALLENGE_SELECTION_H__
#define RD_CHALLENGE_SELECTION_H__
#pragma once

#include "basemodui.h"
#include "vgenericpanellist.h"
#include "rd_workshop.h"

class CNB_Header_Footer;
class CNB_Button;

namespace BaseModUI
{
	class ReactiveDropChallengeSelectionListItem : public vgui::EditablePanel, IGenericPanelListItem
	{
		DECLARE_CLASS_SIMPLE( ReactiveDropChallengeSelectionListItem, vgui::EditablePanel );

	public:
		ReactiveDropChallengeSelectionListItem( vgui::Panel *parent, const char *panelName );

		virtual bool IsLabel() { return false; }

		CUtlString m_szChallengeAuthor;
		CUtlString m_szChallengeName;
		CUtlString m_szChallengeDescription;

	protected:
		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual void OnMousePressed( vgui::MouseCode code );
		virtual void OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel );
		virtual void Paint();

	private:
		void PopulateChallenge( const char *szName );
		friend class ReactiveDropChallengeSelection;
		const CReactiveDropWorkshop::WorkshopItem_t &GetWorkshopItem();

		bool m_bCurrentlySelected;
		vgui::HFont	m_hTextFont;

		vgui::Label *m_lblError;
		vgui::ImagePanel *m_imgIcon;
		vgui::Label *m_lblName;
		vgui::Label *m_lblSource;

		PublishedFileId_t m_nWorkshopID;
	};

	class ReactiveDropChallengeSelection : public vgui::EditablePanel
	{
		DECLARE_CLASS_SIMPLE( ReactiveDropChallengeSelection, vgui::EditablePanel );

	public:
		ReactiveDropChallengeSelection( vgui::Panel *parent, const char *panelName, bool bDeathmatch );
		virtual ~ReactiveDropChallengeSelection();

		bool SetSelectedChallenge( const char *szName );

		virtual void OnCommand( const char *command );
		virtual void OnMessage( const KeyValues *params, vgui::VPANEL ifromPanel );

		void UpdateFooter();
		void PopulateChallenges();
		void SetDetailsForChallenge( ReactiveDropChallengeSelectionListItem *pChallenge );

		CNB_Header_Footer *m_pHeaderFooter;
		GenericPanelList *m_gplChallenges;
		CNB_Button *m_pBackButton;

		vgui::Label *m_lblName;
		vgui::ImagePanel *m_imgIcon;
		vgui::Label *m_lblDescription;
		vgui::Label *m_lblAuthor;

		bool m_bIgnoreSelectionChange;
		bool m_bDeathmatch;
	};
}

#endif // RD_CHALLENGE_SELECTION_H__