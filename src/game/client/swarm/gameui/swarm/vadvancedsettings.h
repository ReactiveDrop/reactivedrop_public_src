#pragma once

#include "basemodui.h"
#include "VFlyoutMenu.h"

class CNB_Header_Footer;

namespace BaseModUI
{
	class BaseModHybridButton;

	class AdvancedSettings : public CBaseModFrame, public FlyoutMenuListener
	{
		DECLARE_CLASS_SIMPLE( AdvancedSettings, CBaseModFrame );

		enum SettingType
		{
			SETTING_INVALID,
			SETTING_SELECT,
		};

		struct SettingOption_t
		{
			const wchar_t *m_Name;
			char m_Value[256];
		};

		struct Setting_t
		{
			Setting_t( const char *szConVar ) : m_ConVar( szConVar )
			{
			}

			SettingType m_Type;
			const wchar_t *m_Name;
			ConVarRef m_ConVar;
			CUtlVectorAutoPurge<ConVarRef *> m_HideUnless;

			CUtlVector<SettingOption_t> m_Options;
		};

		struct Section_t
		{
			const wchar_t *m_Name;
			CUtlVectorAutoPurge<Setting_t *> m_Settings;
		};

	public:
		AdvancedSettings( vgui::Panel *parent, const char *panelName );
		~AdvancedSettings();

		virtual void PerformLayout();
		virtual void OnCommand( const char *command );
		virtual void OnKeyCodePressed( vgui::KeyCode code );
		virtual void OnKeyCodeTyped( vgui::KeyCode code );
		virtual void OnThink();

		//FloutMenuListener
		virtual void OnNotifyChildFocus( vgui::Panel* child );
		virtual void OnFlyoutMenuClose( vgui::Panel* flyTo );
		virtual void OnFlyoutMenuCancelled();

	private:
		void LoadSettingDefinitions();
		void CreateSettingControls();

		CNB_Header_Footer *m_pHeaderFooter;
		CUtlVectorAutoPurge<Section_t *> m_SectionDefs;
		BaseModHybridButton *m_pSectionName;
		int m_iCurrentSection;
		vgui::Button *m_pFlyoutButton;
	};
}
