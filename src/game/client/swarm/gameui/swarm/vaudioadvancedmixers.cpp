#include "cbase.h"
#include "vaudioadvancedmixers.h"
#include "vslidercontrol.h"
#include "engine/IEngineSound.h"
#include "nb_header_footer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

namespace BaseModUI
{
	class AudioMixerSliderControl : public SliderControl
	{
		DECLARE_CLASS_SIMPLE( AudioMixerSliderControl, SliderControl );
	public:
		AudioMixerSliderControl( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
		{
			m_szSoundName[0] = '\0';
			m_hPlayingSound = 0;
		}
		virtual ~AudioMixerSliderControl()
		{
			StopSound();
		}

		virtual void NavigateTo()
		{
			BaseClass::NavigateTo();
			StartSound();
		}

		virtual void NavigateFrom()
		{
			StopSound();
			BaseClass::NavigateFrom();
		}

		virtual void ApplySettings( KeyValues *inResourceData )
		{
			BaseClass::ApplySettings( inResourceData );
			V_strncpy( m_szSoundName, inResourceData->GetString( "previewsound" ), sizeof( m_szSoundName ) );

			Assert( m_szSoundName[0] );
			if ( m_szSoundName[0] )
			{
				enginesound->PrecacheSound( m_szSoundName, true, true );
				Assert( enginesound->IsLoopingSound( m_szSoundName ) );
			}

			Reset();
		}

		void StartSound()
		{
			if ( m_szSoundName[0] && !m_hPlayingSound )
			{
				Assert( enginesound->IsLoopingSound( m_szSoundName ) );

				enginesound->EmitAmbientSound( m_szSoundName, 1.0f );
				m_hPlayingSound = enginesound->GetGuidForLastSoundEmitted();
				Assert( m_hPlayingSound );
			}
		}
		void StopSound()
		{
			if ( m_hPlayingSound )
			{
				enginesound->StopSoundByGuid( m_hPlayingSound );
				m_hPlayingSound = 0;
			}
		}

		char m_szSoundName[255];
		int m_hPlayingSound;
	};
}
DECLARE_BUILD_FACTORY( AudioMixerSliderControl );

AudioAdvancedMixers::AudioAdvancedMixers( Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName, false, true )
{
	GameUI().PreventEngineHideGameUI();

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 60, 310 );
}

AudioAdvancedMixers::~AudioAdvancedMixers()
{
	GameUI().AllowEngineHideGameUI();
}

void AudioAdvancedMixers::OnCommand( const char *command )
{
	if ( !V_stricmp( "Back", command ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}
