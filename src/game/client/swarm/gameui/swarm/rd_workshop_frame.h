#ifndef INCLUDED_RD_WORKSHOP_FRAME
#define INCLUDED_RD_WORKSHOP_FRAME

#ifdef _WIN32
#pragma once
#endif

// There's something weird going on with the other includes in this header, and something is changing the size of some type if rd_workshop.h doesn't go first.
#include "rd_workshop.h"

#include "gameui/swarm/basemodframe.h"
#include "gameui/swarm/vgenericpanellist.h"

class CNB_Header_Footer;
class CNB_Button;

namespace BaseModUI
{
	class ReactiveDropWorkshop;

	class ReactiveDropWorkshopListItem : public vgui::EditablePanel, IGenericPanelListItem
	{
		DECLARE_CLASS_SIMPLE( ReactiveDropWorkshopListItem, vgui::EditablePanel );
	public:
		ReactiveDropWorkshopListItem( vgui::Panel *parent, const char *panelName );
		virtual ~ReactiveDropWorkshopListItem();

		CReactiveDropWorkshop::WorkshopItem_t GetDetails();
		CReactiveDropWorkshopPreviewImage *GetPreviewImage();

		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual bool IsLabel() { return false; }
		virtual void OnMousePressed( vgui::MouseCode code );

		MESSAGE_FUNC( OnPanelUnSelected, "PanelUnSelected" );
		MESSAGE_FUNC( OnPanelSelected, "PanelSelected" );

	private:
		friend class ReactiveDropWorkshop;
		PublishedFileId_t m_nPublishedFileID;

		void UpdateDetails();

		vgui::EditablePanel *m_pPnlHighlight;
		vgui::Label *m_pLblHighlight;
		vgui::ImagePanel *m_pImgPreview;
		vgui::Label *m_pLblName;
	};

	class ReactiveDropWorkshop : public CBaseModFrame
	{
		DECLARE_CLASS_SIMPLE( ReactiveDropWorkshop, CBaseModFrame );
	public:
		ReactiveDropWorkshop( vgui::Panel *parent, const char *panelName );
		virtual ~ReactiveDropWorkshop();

		virtual void Activate();
		virtual void PerformLayout();
		virtual void OnCommand( const char *command );
		virtual void OnThink();
		virtual void OnClose();

		void OnWorkshopPreviewReady( PublishedFileId_t nFileID, CReactiveDropWorkshopPreviewImage *pPreviewImage );
		MESSAGE_FUNC_CHARPTR( OnItemSelected, "OnItemSelected", panelName );
		MESSAGE_FUNC_CHARPTR_CHARPTR( OnFileSelected, "FileSelected", fullpath, filterinfo );

	private:
		void InitReady();
		void InitWait();
		void InitEdit( CReactiveDropWorkshop::WorkshopItem_t item, CReactiveDropWorkshopPreviewImage *pPreviewImage );
		ReactiveDropWorkshopListItem *AddWorkshopItem( PublishedFileId_t nFileID );
		void RequestSingleItem( PublishedFileId_t nPublishedFileID );

		CNB_Header_Footer *m_pHeaderFooter;
		vgui::Label *m_pLblWaiting;
		vgui::ProgressBar *m_pPrgWaiting;
		CNB_Button *m_pBtnCreateWorkshopItem;
		GenericPanelList *m_pGplWorkshopItems;

		bool m_bEditing;
		PublishedFileId_t m_nEditingWorkshopID;
		UGCUpdateHandle_t m_hUpdate;
		CUtlString m_szPreviewImage;
		CNB_Button *m_pBtnEditContent;
		vgui::Label *m_pLblEditingPreview;
		vgui::ImagePanel *m_pImgEditingPreview;
		CNB_Button *m_pBtnEditPreview;
		vgui::Label *m_pLblEditingTitle;
		vgui::TextEntry *m_pTxtEditingTitle;
		vgui::Label *m_pLblEditingDescription;
		vgui::TextEntry *m_pTxtEditingDescription;
		CUtlStringList m_aszAutoTags;
		vgui::Label *m_pLblEditingTags;
		vgui::TextEntry *m_pTxtEditingTags;
		vgui::Label *m_pLblEditingChangeDescription;
		vgui::TextEntry *m_pTxtEditingChangeDescription;
		CNB_Button *m_pBtnEdit;
		CNB_Button *m_pBtnCancel;
		CNB_Button *m_pBtnSubmit;
		CNB_Button *m_pBtnOpen;

		CUtlReference<CReactiveDropWorkshopPreviewImage> m_pFreePreviewImage;

		template<typename Result_t>
		bool OnAPIResult( const Result_t *pResult, bool bIOFailure, const char *szCallName );

		CCallResult<ReactiveDropWorkshop, SteamUGCQueryCompleted_t> m_RequestSingleItemCall;
		void RequestSingleItemCall( SteamUGCQueryCompleted_t *pResult, bool bIOFailure );
		UGCQueryHandle_t m_hSingleItemQuery;
		CCallResult<ReactiveDropWorkshop, CreateItemResult_t> m_CreateItemCall;
		void CreateItemCall( CreateItemResult_t *pResult, bool bIOFailure );
		CCallResult<ReactiveDropWorkshop, SubmitItemUpdateResult_t> m_SubmitItemUpdateCall;
		void SubmitItemUpdateCall( SubmitItemUpdateResult_t *pResult, bool bIOFailure );
	};
}

#endif /* INCLUDED_RD_WORKSHOP_FRAME */
