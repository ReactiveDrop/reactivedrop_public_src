#ifndef RD_SWARMOPEDIA_H
#define RD_SWARMOPEDIA_H
#ifdef _WIN32
#pragma once
#endif

#include "gameui/swarm/basemodframe.h"
#include "basemodel_panel.h"

class CNB_Button;
class CNB_Header_Footer;

namespace vgui
{
	class Button;
}

namespace BaseModUI
{
	class GenericPanelList;

	class Swarmopedia_Model_Panel : public CBaseModelPanel
	{
		DECLARE_CLASS_SIMPLE( Swarmopedia_Model_Panel, CBaseModelPanel );

	public:
		Swarmopedia_Model_Panel( vgui::Panel *parent, const char *panelName );
		virtual ~Swarmopedia_Model_Panel();

		void ApplyConfig( KeyValues *pKV );
		virtual void OnPaint3D();

	protected:
		CUtlVector<MDLData_t> m_Models;
	};

	class Swarmopedia : public CBaseModFrame
	{
		DECLARE_CLASS_SIMPLE( Swarmopedia, CBaseModFrame );

	public:
		Swarmopedia( vgui::Panel *parent, const char *panelName );
		virtual ~Swarmopedia();

		void Clear();
		void LoadAllAliens();
		void LoadAliens( KeyValues *pKV );
		void SetSelectedAlien( int iSelection );
		void UpdateFooter();

		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual void PerformLayout();
		virtual void OnOpen();
		virtual void OnCommand( const char *command );

		struct Alien_t
		{
			Alien_t( Swarmopedia *pSwarmopedia, KeyValues *pKV );
			~Alien_t();

			void Merge( Swarmopedia *pSwarmopedia, KeyValues *pKV );

			struct AppearsIn_t
			{
				AppearsIn_t( KeyValues *pKV );

				CUtlString m_szCampaign;
				CUtlString m_szMap;
				CUtlString m_szChallenge;
				int m_iMinDifficulty;
				int m_iMaxDifficulty;
				bool m_bOnslaughtOnly;
			};

			CUtlString m_szID;
			CUtlString m_szName;
			CUtlStringList m_Abilities;
			CUtlVector<Swarmopedia_Model_Panel *> m_DisplayPanel;
			CUtlStringList m_DisplayCaption;
			CUtlVector<AppearsIn_t> m_AppearsIn;
			CUtlStringList m_Paragraphs;
		};

		CNB_Header_Footer *m_pHeaderFooter;
		CNB_Button *m_pModelCaption;
		vgui::Label *m_pLblName;
		vgui::Label *m_pLblAbilities;
		GenericPanelList *m_pGplAliens;
		GenericPanelList *m_pGplParagraphs;
		GenericPanelList *m_pGplAppears;
		CUtlVectorAutoPurge<Alien_t *> m_Aliens;
		int m_iSelectedAlien;
		int m_iSelectedModel;
	};
}

#endif // RD_SWARMOPEDIA_H
