#ifndef _INCLUDED_RD_VGUI_WORKSHOP_DOWNLOAD_PROGRESS
#define _INCLUDED_RD_VGUI_WORKSHOP_DOWNLOAD_PROGRESS

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>

namespace vgui
{
	class Label;
	class ProgressBar;
}

class CRD_VGUI_Workshop_Download_Progress : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Workshop_Download_Progress, vgui::EditablePanel );

public:
	CRD_VGUI_Workshop_Download_Progress( vgui::Panel *parent, const char *panelName );
	virtual ~CRD_VGUI_Workshop_Download_Progress();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

	vgui::Label *m_pLblName;
	vgui::ProgressBar *m_pPrgDownload;
	vgui::Label *m_pLblQueue;
	vgui::ImagePanel *m_pImgPreview;
};

#endif /* _INCLUDED_RD_VGUI_WORKSHOP_DOWNLOAD_PROGRESS */