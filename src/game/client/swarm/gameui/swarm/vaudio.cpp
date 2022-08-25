//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "VAudio.h"
#include "VFooterPanel.h"
#include "VDropDownMenu.h"
#include "VSliderControl.h"
#include "VHybridButton.h"
#include "EngineInterface.h"
#include "gameui_util.h"
#include "vgui/ISurface.h"
#include "VGenericConfirmation.h"
#include "ivoicetweak.h"
#include "materialsystem/materialsystem_config.h"
#include "cdll_util.h"
#include "nb_header_footer.h"
#include "vgui_controls/ImagePanel.h"

#ifdef _X360
#include "xbox/xbox_launch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define VIDEO_LANGUAGE_COMMAND_PREFIX "_language"


using namespace vgui;
using namespace BaseModUI;


void SetFlyoutButtonText( const char *pchCommand, FlyoutMenu *pFlyout, const char *pchNewText );

static void MixerVolumeMultiplierChanged( IConVar *var, const char *pOldValue, float flOldValue );
static void MusicVolumeMultiplierChanged( IConVar *var, const char *pOldValue, float flOldValue );

ConVar rd_mixer_volume_music_menus( "rd_mixer_volume_music_menus", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for music played on menus.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_music_ingame( "rd_mixer_volume_music_ingame", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for music played during missions.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar _rd_mixer_volume_music_both_internal( "_rd_mixer_volume_music_both_internal", "1", FCVAR_HIDDEN, "Internal convar. Get->Average music volume, Set->Set both music volumes", true, 0, false, 0, MusicVolumeMultiplierChanged );
ConVar rd_mixer_volume_voice_dialogue( "rd_mixer_volume_voice_dialogue", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for NPC and marine dialogue.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_voice_players( "rd_mixer_volume_voice_players", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for player voice chat.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_voice_aliens( "rd_mixer_volume_voice_aliens", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for alien vocalizations.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_interface( "rd_mixer_volume_interface", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for UI sounds.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_environment_ambient( "rd_mixer_volume_environment_ambient", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for ambient sounds.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_environment_physics( "rd_mixer_volume_environment_physics", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for physics/impact sound effects.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_combat_damage( "rd_mixer_volume_combat_damage", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for damage sound effects.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_combat_weapons( "rd_mixer_volume_combat_weapons", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for weapon sounds.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_explosions( "rd_mixer_volume_explosions", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for explosions.", true, 0, false, 0, MixerVolumeMultiplierChanged );
ConVar rd_mixer_volume_other( "rd_mixer_volume_other", "1", FCVAR_ARCHIVE, "Mixer volume multiplier for sounds not matched by another mixer category.", true, 0, false, 0, MixerVolumeMultiplierChanged );

static struct MixerVar_t
{
	ConVar &var;
	CSplitString mixers;
	CUtlVector<float> defaultVolumes;
} s_MixerVars[] =
{
	{
		rd_mixer_volume_music_menus,
		{"Briefing,Shell", ","},
		{},
	},
	{
		rd_mixer_volume_music_ingame,
		{"TimorMusic,Music", ","},
		{},
	},
	{
		rd_mixer_volume_voice_dialogue,
		{"Command,VO", ","},
		{},
	},
	{
		rd_mixer_volume_voice_players,
		{"voip", ","},
		{},
	},
	{
		rd_mixer_volume_voice_aliens,
		{"AlienVoc,AlienVocBig", ","},
		{},
	},
	{
		rd_mixer_volume_interface,
		{"UI,UI2", ","},
		{},
	},
	{
		rd_mixer_volume_environment_ambient,
		{"Ambient,AmbientLoud,Soundscapes", ","},
		{},
	},
	{
		rd_mixer_volume_environment_physics,
		{"Physics,AlienPhysics", ","},
		{},
	},
	{
		rd_mixer_volume_combat_damage,
		{"Damage,Player_Damage", ","},
		{},
	},
	{
		rd_mixer_volume_combat_weapons,
		{"Weapons,Player_Weapons", ","},
		{},
	},
	{
		rd_mixer_volume_explosions,
		{"Explosions,Nuke", ","},
		{},
	},
	{
		rd_mixer_volume_other,
		{"All", ","},
		{},
	},
};

static bool InitDefaultMixLevels()
{
	if ( s_MixerVars[0].defaultVolumes.Count() != 0 )
	{
		return true;
	}

	// BenLubar: Bear with me here, because we're about to dive deep into the guts of the engine and do something naughty. [Puts DFHack hat on.]
	ConCommand *pCmd = g_pCVar->FindCommand( "snd_getmixer" );
	Assert( pCmd );
	if ( !pCmd )
	{
		Warning( "Failed to init sound mixer default volumes! Mixer volume multipliers will not be applied.\n" );
		return false;
	}

	// Get m_fnCommandCallback; we now have a pointer to an engine function.
	const byte *pCallback = *reinterpret_cast< const byte *const * >( reinterpret_cast< const byte * >( pCmd ) + sizeof( ConCommandBase ) );

	// The function we are looking at goes like this:
	// 8b 4c 24 04                     get concommand arg count
	// 83 39 02                        test if argument count equals 2
	// 75 24                           if wrong argument count, skip ahead to end of function
	// a1 xx xx xx xx                  get current mixer index
	// 85 c0                           check if mixer index is negative
	// 7c 1b                           if bad mixer index, skip ahead to end of function
	// 8b 89 0c 04 00 00               get first concommand arg
	// 69 c0 80 0a 00 00               multiply mixer index by size of mixer structure
	// 05 yy yy yy yy                  add mixer structure offset to global mixer array pointer
	// 50 51                           push arguments for next function call onto the stack
	// e8 zz zz zz zz                  call the next function we need to steal pointers from
	// 83 c4 08                        pop arguments from stack
	// c3                              return
	//
	// The X and Y pointers are absolute, and Z pointer is relative. (CPUs are weird.)

	Assert( pCallback[0] == 0x8b );
	Assert( pCallback[1] == 0x4c );
	Assert( pCallback[2] == 0x24 );
	Assert( pCallback[3] == 0x04 );
	Assert( pCallback[4] == 0x83 );
	Assert( pCallback[5] == 0x39 );
	Assert( pCallback[6] == 0x02 );
	Assert( pCallback[7] == 0x75 );
	Assert( pCallback[8] == 0x24 );
	Assert( pCallback[9] == 0xa1 );

	const int *pCurrentMixerIndex = *reinterpret_cast< const int *const * >( pCallback + 10 );

	Assert( pCallback[14] == 0x85 );
	Assert( pCallback[15] == 0xc0 );
	Assert( pCallback[16] == 0x7c );
	Assert( pCallback[17] == 0x1b );
	Assert( pCallback[18] == 0x8b );
	Assert( pCallback[19] == 0x89 );
	Assert( pCallback[20] == 0x0c );
	Assert( pCallback[21] == 0x04 );
	Assert( pCallback[22] == 0x00 );
	Assert( pCallback[23] == 0x00 );
	Assert( pCallback[24] == 0x69 );
	Assert( pCallback[25] == 0xc0 );
	Assert( pCallback[26] == 0x80 );
	Assert( pCallback[27] == 0x0a );
	Assert( pCallback[28] == 0x00 );
	Assert( pCallback[29] == 0x00 );
	Assert( pCallback[30] == 0x05 );

	struct GlobalMixer_t
	{
		byte pad0[128];             // padding and data we don't need
		float m_flGroupVolume[128]; // mixer group id to volume multiplier mapping
		byte pad1[2048];            // more data we don't need
	};

	const GlobalMixer_t *pGlobalMixersTable = *reinterpret_cast< const GlobalMixer_t *const * >( pCallback + 31 );

	Assert( pCallback[35] == 0x50 );
	Assert( pCallback[36] == 0x51 );
	Assert( pCallback[37] == 0xe8 );

	const byte *pCallback2 = reinterpret_cast< const byte * >( uintptr_t( pCallback ) + 42 + *reinterpret_cast< const uintptr_t * >( pCallback + 38 ) );

	Assert( pCallback[42] == 0x83 );
	Assert( pCallback[43] == 0xc4 );
	Assert( pCallback[44] == 0x08 );
	Assert( pCallback[45] == 0xc3 );

	// The next function we're dissecting is longer, but we don't need all of it:
	// 55 8b ec 83 e4 c0 83 ec 34      align the stack to a multiple of 128 bytes
	// 83 3d xx xx xx xx 00            check the number of defined mixer groups
	// 53 56 57                        push some registers onto the stack
	// c7 44 24 28 00 00 00 00         set a loop counter on the stack to 0
	// 0f 8e b0 00 00 00               if number of defined mixer groups is not positive, bail
	// 8b 75 0c                        get mixer sent as an argument to this function
	// 8b 1d yy yy yy yy               get the address of the DevMsg function
	// bf zz zz zz zz                  get a pointer to the global mixer group list
	// 90                              nop
	// (the function continues, but we don't need any more data from it)

	Assert( pCallback2[0] == 0x55 );
	Assert( pCallback2[1] == 0x8b );
	Assert( pCallback2[2] == 0xec );
	Assert( pCallback2[3] == 0x83 );
	Assert( pCallback2[4] == 0xe4 );
	Assert( pCallback2[5] == 0xc0 );
	Assert( pCallback2[6] == 0x83 );
	Assert( pCallback2[7] == 0xec );
	Assert( pCallback2[8] == 0x34 );
	Assert( pCallback2[9] == 0x83 );
	Assert( pCallback2[10] == 0x3d );

	const int *pMixerGroupCount = *reinterpret_cast< const int *const * >( pCallback2 + 11 );

	Assert( pCallback2[15] == 0x00 );
	Assert( pCallback2[16] == 0x53 );
	Assert( pCallback2[17] == 0x56 );
	Assert( pCallback2[18] == 0x57 );
	Assert( pCallback2[19] == 0xc7 );
	Assert( pCallback2[20] == 0x44 );
	Assert( pCallback2[21] == 0x24 );
	Assert( pCallback2[22] == 0x28 );
	Assert( pCallback2[23] == 0x00 );
	Assert( pCallback2[24] == 0x00 );
	Assert( pCallback2[25] == 0x00 );
	Assert( pCallback2[26] == 0x00 );
	Assert( pCallback2[27] == 0x0f );
	Assert( pCallback2[28] == 0x8e );
	Assert( pCallback2[29] == 0xb0 );
	Assert( pCallback2[30] == 0x00 );
	Assert( pCallback2[31] == 0x00 );
	Assert( pCallback2[32] == 0x00 );
	Assert( pCallback2[33] == 0x8b );
	Assert( pCallback2[34] == 0x75 );
	Assert( pCallback2[35] == 0x0c );
	Assert( pCallback2[36] == 0x8b );
	Assert( pCallback2[37] == 0x1d );

	// don't need the DevMsg function pointer

	Assert( pCallback2[42] == 0xbf );

	struct GlobalMixerGroup_t
	{
		char m_szName[32]; // mixer group name
		int m_ID;          // mixer group index
		byte pad0[84];     // other data we don't need
	};

	const GlobalMixerGroup_t *pGlobalMixerGroupTable = *reinterpret_cast< const GlobalMixerGroup_t *const * >( pCallback2 + 43 );

	Assert( pCallback2[47] == 0x90 );

	// Alright, we're all set up to grab the default volume settings.
	// There's only one thing that can stop us, and that's if we don't have mixers loaded yet:
	if ( *pCurrentMixerIndex < 0 )
	{
		// Bail, but don't write any error messages.
		return false;
	}

	const GlobalMixer_t &mixer = pGlobalMixersTable[*pCurrentMixerIndex];

	for ( int i = 0; i < NELEMS( s_MixerVars ); i++ )
	{
		static const float flNegativeOne = -1.0f;
		s_MixerVars[i].defaultVolumes.AddMultipleToTail( s_MixerVars[i].mixers.Count(), &flNegativeOne );
	}

	for ( int i = 0; i < *pMixerGroupCount; i++ )
	{
		int id = pGlobalMixerGroupTable[i].m_ID;
		const char *szName = pGlobalMixerGroupTable[i].m_szName;

		float flDefaultVolume = mixer.m_flGroupVolume[id];
		if ( flDefaultVolume < 0 )
		{
			continue;
		}

		bool bFound = false;
		for ( int j = 0; j < NELEMS( s_MixerVars ); j++ )
		{
			FOR_EACH_VEC( s_MixerVars[j].mixers, k )
			{
				if ( V_strcmp( s_MixerVars[j].mixers[k], szName ) )
				{
					continue;
				}

				s_MixerVars[j].defaultVolumes[k] = flDefaultVolume;
				bFound = true;
				break;
			}

			if ( bFound )
			{
				break;
			}
		}

		Assert( bFound );
		if ( !bFound )
		{
			Assert( &s_MixerVars[NELEMS( s_MixerVars ) - 1].var == &rd_mixer_volume_other );

			char *szNameCopy = new char[32];
			V_strcpy( szNameCopy, szName );

			s_MixerVars[NELEMS( s_MixerVars ) - 1].mixers.AddToTail( szNameCopy );
			s_MixerVars[NELEMS( s_MixerVars ) - 1].defaultVolumes.AddToTail( flDefaultVolume );
		}
	}

	for ( int i = 0; i < NELEMS( s_MixerVars ); i++ )
	{
		FOR_EACH_VEC_BACK( s_MixerVars[i].defaultVolumes, j )
		{
			Assert( s_MixerVars[i].defaultVolumes[j] >= 0 );
			if ( s_MixerVars[i].defaultVolumes[j] < 0 )
			{
				delete[] s_MixerVars[i].mixers[j];
				s_MixerVars[i].mixers.Remove( j );
				s_MixerVars[i].defaultVolumes.Remove( j );
			}
		}
	}

	return true;
}

static bool s_bIgnoreMusicVolumeChange = false;

static void MixerVolumeMultiplierChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( !InitDefaultMixLevels() )
	{
		return;
	}

	float flScale = assert_cast<ConVar *>( var )->GetFloat();

	if ( !s_bIgnoreMusicVolumeChange && ( var == &rd_mixer_volume_music_ingame || var == &rd_mixer_volume_music_menus ) )
	{
		s_bIgnoreMusicVolumeChange = true;
		_rd_mixer_volume_music_both_internal.SetValue( ( rd_mixer_volume_music_ingame.GetFloat() + rd_mixer_volume_music_menus.GetFloat() ) / 2 );
		s_bIgnoreMusicVolumeChange = false;
	}

	for ( int i = 0; i < NELEMS( s_MixerVars ); i++ )
	{
		if ( &s_MixerVars[i].var == var )
		{
			FOR_EACH_VEC( s_MixerVars[i].mixers, j )
			{
				engine->SetMixGroupOfCurrentMixer( s_MixerVars[i].mixers[j], "vol", s_MixerVars[i].defaultVolumes[j] * flScale, 0 );
			}

			return;
		}
	}

	Assert( !"Unhandled mixer convar - make sure it's added to the table!" );
}

static void MusicVolumeMultiplierChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( s_bIgnoreMusicVolumeChange )
	{
		return;
	}

	float flTarget = _rd_mixer_volume_music_both_internal.GetFloat();
	rd_mixer_volume_music_ingame.SetValue( flTarget );
	rd_mixer_volume_music_menus.SetValue( flTarget );
}

CON_COMMAND_F( _rd_mixer_init, "Deferred mixer volume initialization.", FCVAR_HIDDEN )
{
	if ( !InitDefaultMixLevels() )
	{
		Assert( !"_rd_mixer_init called too early!" );
		return;
	}

	ConVarRef snd_musicvolume( "snd_musicvolume" );
	Assert( snd_musicvolume.IsValid() );
	if ( snd_musicvolume.IsValid() && snd_musicvolume.GetFloat() < 1.0f && _rd_mixer_volume_music_both_internal.GetFloat() == 1.0f )
	{
		Msg( "Converting old ingame music volume setting to new mixer system.\n" );
		_rd_mixer_volume_music_both_internal.SetValue( snd_musicvolume.GetFloat() );
		snd_musicvolume.SetValue( "1.0" );
		engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
	}
	else if ( snd_musicvolume.IsValid() && snd_musicvolume.GetFloat() < 1.0f )
	{
		Warning( "Both new (rd_mixer_volume_music_ingame/rd_mixer_volume_music_menus) and old (snd_musicvolume) music volume settings are set. Music volume will be scaled twice!\n" );
	}

	Assert( !s_bIgnoreMusicVolumeChange );
	s_bIgnoreMusicVolumeChange = true;
	for ( int i = 0; i < NELEMS( s_MixerVars ); i++ )
	{
		MixerVolumeMultiplierChanged( &s_MixerVars[i].var, "1", 1.0f );
	}
	s_bIgnoreMusicVolumeChange = false;
}

// This member is static so that the updated audio language can be referenced during shutdown
const char* Audio::m_pchUpdatedAudioLanguage = "";

extern ConVar ui_gameui_modal;

//=============================================================================
Audio::Audio(Panel *parent, const char *panelName):
BaseClass(parent, panelName)
{
	if ( ui_gameui_modal.GetBool() )
	{
		GameUI().PreventEngineHideGameUI();
	}

#if !defined( NO_VOICE )
	m_pVoiceTweak = engine->GetVoiceTweakAPI();
#else
	m_pVoiceTweak = NULL;
#endif

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 60, 340 );

	SetDeleteSelfOnClose(true);

	SetProportional( true );

	SetUpperGarnishEnabled(true);
	SetLowerGarnishEnabled(true);

	m_sldGameVolume = NULL;
	m_sldMusicVolume = NULL;
	m_btnAdvancedMixers = NULL;
	m_drpSpeakerConfiguration = NULL;
	m_drpSoundQuality = NULL;
	m_drpLanguage = NULL;
	m_drpCaptioning = NULL;
	m_drpVoiceCommunication = NULL;
	m_sldRecieveVolume = NULL;
	m_drpBoostMicrophoneGain = NULL;
	m_btnTestMicrophone = NULL;
	m_sldVoiceThreshold = NULL;
	m_drpVoiceCommunicationStyle = NULL;

	m_pMicMeter = NULL;
	m_pMicMeter2 = NULL;
	m_pMicMeterIndicator = NULL;

	m_btn3rdPartyCredits = NULL;

	m_nSelectedAudioLanguage = k_Lang_None;
	m_nCurrentAudioLanguage = k_Lang_None;

	m_nNumAudioLanguages = 0;
}

//=============================================================================
Audio::~Audio()
{
	GameUI().AllowEngineHideGameUI();

	EndTestMicrophone();

	UpdateFooter( false );

	if ( m_pchUpdatedAudioLanguage[ 0 ] != '\0' )
	{
		PostMessage( &(CBaseModPanel::GetSingleton()), new KeyValues( "command", "command", "RestartWithNewLanguage" ), 0.01f );
	}
}

//=============================================================================
void Audio::Activate()
{
	BaseClass::Activate();

	if ( m_sldGameVolume )
	{
		m_sldGameVolume->Reset();
	}

	if ( m_sldMusicVolume )
	{
		m_sldMusicVolume->Reset();
	}

	if ( m_sldVoiceThreshold )
	{
		m_sldVoiceThreshold->Reset();
	}

	if ( m_drpSpeakerConfiguration )
	{
		CGameUIConVarRef snd_surround_speakers("Snd_Surround_Speakers");

		switch ( snd_surround_speakers.GetInt() )
		{
		case 2:
			m_drpSpeakerConfiguration->SetCurrentSelection( "#GameUI_2Speakers" );
			break;
		case 4:
			m_drpSpeakerConfiguration->SetCurrentSelection( "#GameUI_4Speakers" );
			break;
		case 5:
			m_drpSpeakerConfiguration->SetCurrentSelection( "#GameUI_5Speakers" );
			break;
		case 0:
		default:
			m_drpSpeakerConfiguration->SetCurrentSelection( "#GameUI_Headphones" );
			break;
		}

		FlyoutMenu *pFlyout = m_drpSpeakerConfiguration->GetCurrentFlyout();
		if ( pFlyout )
		{
			pFlyout->SetListener( this );
		}
	}

	if ( m_drpSoundQuality )
	{
		CGameUIConVarRef Snd_PitchQuality("Snd_PitchQuality");
		CGameUIConVarRef dsp_slow_cpu("dsp_slow_cpu");

		int quality = 0;
		if ( dsp_slow_cpu.GetBool() == false )
		{
			quality = 1;
		}
		if ( Snd_PitchQuality.GetBool() )
		{
			quality = 2;
		}

		switch ( quality )
		{
		case 1:
			m_drpSoundQuality->SetCurrentSelection( "#GameUI_Medium" );
			break;
		case 2:
			m_drpSoundQuality->SetCurrentSelection( "#GameUI_High" );
			break;
		case 0:
		default:
			m_drpSoundQuality->SetCurrentSelection( "#GameUI_Low" );
			break;
		}

		FlyoutMenu *pFlyout = m_drpSoundQuality->GetCurrentFlyout();
		if ( pFlyout )
		{
			pFlyout->SetListener( this );
		}
	}

	if ( m_drpCaptioning )
	{
		CGameUIConVarRef closecaption("closecaption");
		CGameUIConVarRef cc_subtitles("cc_subtitles");

		if ( !closecaption.GetBool() )
		{
			m_drpCaptioning->SetCurrentSelection( "#L4D360UI_AudioOptions_CaptionOff" );
		}
		else
		{
			if ( cc_subtitles.GetBool() )
			{
				m_drpCaptioning->SetCurrentSelection( "#L4D360UI_AudioOptions_CaptionSubtitles" );
			}
			else
			{
				m_drpCaptioning->SetCurrentSelection( "#L4D360UI_AudioOptions_CaptionOn" );
			}
		}

		FlyoutMenu *pFlyout = m_drpCaptioning->GetCurrentFlyout();
		if ( pFlyout )
		{
			pFlyout->SetListener( this );
		}
	}

	if ( m_drpLanguage )
	{
		char szCurrentLanguage[50] = "";
		char szAvailableLanguages[512] = "";
		szAvailableLanguages[0] = '\0';

		// Fallback to current engine language
		engine->GetUILanguage( szCurrentLanguage, sizeof( szCurrentLanguage ));

		// In a Steam environment we get the current language 
#if !defined( NO_STEAM )
		// When Steam isn't running we can't get the language info... 
		if ( SteamApps() )
		{
			Q_strncpy( szCurrentLanguage, SteamApps()->GetCurrentGameLanguage(), sizeof(szCurrentLanguage) );
			Q_strncpy( szAvailableLanguages, SteamApps()->GetAvailableGameLanguages(), sizeof(szAvailableLanguages) );
		}
#endif

		// Get the spoken language and store it for comparison purposes
		m_nCurrentAudioLanguage = PchLanguageToELanguage( szCurrentLanguage );

		// Set up the base string for each button command
		char szCurrentButton[ 32 ];
		Q_strncpy( szCurrentButton, VIDEO_LANGUAGE_COMMAND_PREFIX, sizeof( szCurrentButton ) );

		int iCommandNumberPosition = Q_strlen( szCurrentButton );
		szCurrentButton[ iCommandNumberPosition + 1 ] = '\0';

		// Check to see if we have a list of languages from Steam
		if ( szAvailableLanguages[0] != '\0' )
		{
			int iSelectedLanguage = 0;

			FlyoutMenu *pLanguageFlyout = m_drpLanguage->GetCurrentFlyout();

			// Loop through all the languages
			CSplitString languagesList( szAvailableLanguages, "," );

			for ( m_nNumAudioLanguages = 0; m_nNumAudioLanguages < languagesList.Count() && m_nNumAudioLanguages < MAX_DYNAMIC_AUDIO_LANGUAGES; ++m_nNumAudioLanguages )
			{
				szCurrentButton[ iCommandNumberPosition ] = m_nNumAudioLanguages + '0';
				m_nAudioLanguages[ m_nNumAudioLanguages ].languageCode = PchLanguageToELanguage( languagesList[ m_nNumAudioLanguages ] );

				SetFlyoutButtonText( szCurrentButton, pLanguageFlyout, GetLanguageVGUILocalization( m_nAudioLanguages[ m_nNumAudioLanguages ].languageCode ) );

				if ( m_nCurrentAudioLanguage == m_nAudioLanguages[ m_nNumAudioLanguages ].languageCode )
				{
					iSelectedLanguage = m_nNumAudioLanguages;
				}
			}

			// Change the height to fit the active items
			pLanguageFlyout->SetBGTall( m_nNumAudioLanguages * 20 + 5 );

			// Disable the remaining possible choices
			for ( int i = m_nNumAudioLanguages; i < MAX_DYNAMIC_AUDIO_LANGUAGES; ++i )
			{
				szCurrentButton[ iCommandNumberPosition ] = i + '0';

				Button *pButton = pLanguageFlyout->FindChildButtonByCommand( szCurrentButton );
				if ( pButton )
				{
					pButton->SetVisible( false );
				}
			}

			// Set the current selection
			szCurrentButton[ iCommandNumberPosition ] = iSelectedLanguage + '0';

			m_drpLanguage->SetCurrentSelection( szCurrentButton );

			m_nSelectedAudioLanguage = m_nAudioLanguages[ iSelectedLanguage ].languageCode;
		}

		// reactivedrop: don't show language selection drop down in audio settings window
		// because it cannot hold all of our supported languages
		// audio language can be choosen in game's properties window in Steam
		// this is same how it works in Left 4 Dead 2
		//m_drpLanguage->SetVisible( m_nNumAudioLanguages > 1 );
		m_drpLanguage->SetVisible( false );
	}

	if ( !m_pVoiceTweak )
	{
		SetControlVisible( "DrpVoiceCommunication", false );
		SetControlVisible( "SldVoiceReceiveVolume", false );
		SetControlVisible( "DrpBoostMicrophone", false );
		SetControlVisible( "BtnTestMicrophone", false );
		SetControlVisible( "MicMeter", false );
		SetControlVisible( "MicMeter2", false );
	}
	else
	{
		CGameUIConVarRef voice_modenable("voice_modenable");
		CGameUIConVarRef voice_enable("voice_enable");

		bool bVoiceEnabled = voice_enable.GetBool() && voice_modenable.GetBool();

		if ( m_drpVoiceCommunication )
		{
			if ( bVoiceEnabled )
			{
				m_drpVoiceCommunication->SetCurrentSelection( "#L4D360UI_Enabled" );
			}
			else
			{
				m_drpVoiceCommunication->SetCurrentSelection( "#L4D360UI_Disabled" );
			}

			FlyoutMenu *pFlyout = m_drpVoiceCommunication->GetCurrentFlyout();
			if ( pFlyout )
			{
				pFlyout->SetListener( this );
			}
		}

		CGameUIConVarRef voice_vox("voice_vox");

		if ( m_drpVoiceCommunicationStyle )
		{
			m_drpVoiceCommunicationStyle->SetEnabled( bVoiceEnabled );

			if ( voice_vox.GetBool() )
			{
				m_drpVoiceCommunicationStyle->SetCurrentSelection( "#L4D360UI_OpenMic" );
				SetControlEnabled( "SldVoiceVoxThreshold", bVoiceEnabled );
			}
			else
			{
				m_drpVoiceCommunicationStyle->SetCurrentSelection( "#L4D360UI_PushToTalk" );
				SetControlEnabled( "SldVoiceVoxThreshold", false );
			}

			FlyoutMenu *pFlyout = m_drpVoiceCommunicationStyle->GetCurrentFlyout();
			if ( pFlyout )
			{
				pFlyout->SetListener( this );
			}
		}

		if ( m_sldRecieveVolume )
		{
			float flRecVolume = m_pVoiceTweak->GetControlFloat( OtherSpeakerScale );
			m_sldRecieveVolume->Reset();
			m_sldRecieveVolume->SetCurrentValue( flRecVolume );
			m_sldRecieveVolume->SetEnabled( bVoiceEnabled );
		}

		if ( m_drpBoostMicrophoneGain )
		{
			float fMicBoost = m_pVoiceTweak->GetControlFloat( MicBoost );

			if ( fMicBoost != 0.0f )
			{
				m_drpBoostMicrophoneGain->SetCurrentSelection( "#L4D360UI_Enabled" );
			}
			else
			{
				m_drpBoostMicrophoneGain->SetCurrentSelection( "#L4D360UI_Disabled" );
			}

			m_drpBoostMicrophoneGain->SetEnabled( bVoiceEnabled );

			FlyoutMenu *pFlyout = m_drpBoostMicrophoneGain->GetCurrentFlyout();
			if ( pFlyout )
			{
				pFlyout->SetListener( this );
			}
		}

		if ( m_pMicMeter )
		{
			m_pMicMeter->SetVisible( bVoiceEnabled );
		}

		if ( m_pMicMeter2 )
		{
			m_pMicMeter2->SetVisible( false );
		}

		if ( m_pMicMeterIndicator )
		{
			m_pMicMeterIndicator->SetVisible( false );
		}
	}

	UpdateFooter( true );

	if ( m_sldGameVolume )
	{
		if ( m_ActiveControl )
			m_ActiveControl->NavigateFrom( );
		m_sldGameVolume->NavigateTo();
		m_ActiveControl = m_sldGameVolume;
	}
}

void Audio::OnThink()
{
	BaseClass::OnThink();

	bool needsActivate = false;

	if ( !m_sldGameVolume )
	{
		m_sldGameVolume = dynamic_cast< SliderControl * >( FindChildByName( "SldGameVolume" ) );
		Assert( m_sldGameVolume );
		needsActivate = true;
	}

	if ( !m_sldMusicVolume )
	{
		m_sldMusicVolume = dynamic_cast< SliderControl * >( FindChildByName( "SldMusicVolume" ) );
		Assert( m_sldMusicVolume );
		needsActivate = true;
	}

	if ( !m_btnAdvancedMixers )
	{
		m_btnAdvancedMixers = dynamic_cast< BaseModHybridButton * >( FindChildByName( "BtnAdvancedMixers" ) );
		Assert( m_btnAdvancedMixers );
		needsActivate = true;
	}

	if ( !m_sldVoiceThreshold )
	{
		m_sldVoiceThreshold = dynamic_cast< SliderControl * >( FindChildByName( "SldVoiceVoxThreshold" ) );
		Assert( m_sldVoiceThreshold );
		needsActivate = true;
	}

	if ( !m_drpSpeakerConfiguration )
	{
		m_drpSpeakerConfiguration = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpSpeakerConfiguration" ) );
		Assert( m_drpSpeakerConfiguration );
		needsActivate = true;
	}

	if ( !m_drpSoundQuality )
	{
		m_drpSoundQuality = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpSoundQuality" ) );
		Assert( m_drpSoundQuality );
		needsActivate = true;
	}

	if ( !m_drpLanguage )
	{
		m_drpLanguage = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpLanguage" ) );
		Assert( m_drpLanguage );
		needsActivate = true;
	}

	if ( !m_drpCaptioning )
	{
		m_drpCaptioning = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpCaptioning" ) );
		Assert( m_drpCaptioning );
		needsActivate = true;
	}

	if ( !m_drpVoiceCommunication )
	{
		m_drpVoiceCommunication = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpVoiceCommunication" ) );
		Assert( m_drpVoiceCommunication );
		needsActivate = true;
	}

	if ( !m_drpVoiceCommunicationStyle )
	{
		m_drpVoiceCommunicationStyle = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpVoiceCommunicationStyle" ) );
		Assert( m_drpVoiceCommunicationStyle );
		needsActivate = true;
	}

	if ( !m_sldRecieveVolume )
	{
		m_sldRecieveVolume = dynamic_cast< SliderControl * >( FindChildByName( "SldVoiceReceiveVolume" ) );
		Assert( m_sldRecieveVolume );
		needsActivate = true;
	}

	if ( !m_drpBoostMicrophoneGain )
	{
		m_drpBoostMicrophoneGain = dynamic_cast< DropDownMenu * >( FindChildByName( "DrpBoostMicrophone" ) );
		Assert( m_drpBoostMicrophoneGain );
		needsActivate = true;
	}

	if ( !m_btnTestMicrophone )
	{
		m_btnTestMicrophone = dynamic_cast< BaseModHybridButton * >( FindChildByName( "BtnTestMicrophone" ) );
		Assert( m_btnTestMicrophone );
		needsActivate = true;
	}

	if ( !m_pMicMeter )
	{
		m_pMicMeter = dynamic_cast< ImagePanel * >( FindChildByName( "MicMeter" ) );
		Assert( m_pMicMeter );
		needsActivate = true;
	}

	if ( !m_pMicMeter2 )
	{
		m_pMicMeter2 = dynamic_cast< ImagePanel * >( FindChildByName( "MicMeter2" ) );
		Assert( m_pMicMeter2 );
		needsActivate = true;
	}

	if ( !m_pMicMeterIndicator )
	{
		m_pMicMeterIndicator = dynamic_cast< ImagePanel * >( FindChildByName( "MicMeterIndicator" ) );
		Assert( m_pMicMeterIndicator );
		needsActivate = true;
	}

	if ( !m_btn3rdPartyCredits )
	{
		m_btn3rdPartyCredits = dynamic_cast< BaseModHybridButton * >( FindChildByName( "Btn3rdPartyCredits" ) );
		Assert( m_btn3rdPartyCredits );
		needsActivate = true;
	}

	if ( needsActivate )
	{
		Activate();
	}

	if ( m_pVoiceTweak && m_pMicMeter && m_pMicMeter2 && m_pMicMeterIndicator && m_pMicMeter2->IsVisible() )
	{
		if ( !m_pVoiceTweak->IsStillTweaking() )
		{
			DevMsg( 1, "Lost Voice Tweak channels, resetting\n" );
			EndTestMicrophone();
		}
		else
		{
			int wide, tall;
			m_pMicMeter->GetSize( wide, tall );

			int indicatorWide, indicatorTall;
			m_pMicMeterIndicator->GetSize( indicatorWide, indicatorTall );

			int iXPos, iYPos;
			m_pMicMeter2->GetPos( iXPos, iYPos );

			int iFinalPos = iXPos - ( indicatorWide / 2 ) + static_cast< float >( wide ) * m_pVoiceTweak->GetControlFloat( SpeakingVolume );

			m_pMicMeterIndicator->GetPos( iXPos, iYPos );

			iFinalPos = Approach( iFinalPos, iXPos, vgui::scheme()->GetProportionalScaledValue( 4 ) );

			m_pMicMeterIndicator->SetPos( iFinalPos, iYPos );
		}
	}
}

void Audio::PerformLayout()
{
	BaseClass::PerformLayout();

	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );
}

void Audio::OnKeyCodePressed(KeyCode code)
{
	int joystick = GetJoystickForCode( code );
	int userId = CBaseModPanel::GetSingleton().GetLastActiveUserId();
	if ( joystick != userId || joystick < 0 )
	{	
		return;
	}

	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		if ( m_nSelectedAudioLanguage != m_nCurrentAudioLanguage && m_drpLanguage && m_drpLanguage->IsVisible() )
		{
			UseSelectedLanguage();

			// Pop up a dialog to confirm changing the language
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

			GenericConfirmation* confirmation = 
				static_cast< GenericConfirmation* >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#GameUI_ChangeLanguageRestart_Title";
			data.pMessageText = "#GameUI_ChangeLanguageRestart_Info";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptLanguageChangeCallback;
			data.bCancelButtonEnabled = true;
			data.pfnCancelCallback = &CancelLanguageChangeCallback;

			// WARNING! WARNING! WARNING!
			// The nature of Generic Confirmation is that it will be silently replaced
			// with another Generic Confirmation if a system event occurs
			// e.g. user unplugs controller, user changes storage device, etc.
			// If that happens neither OK nor CANCEL callbacks WILL NOT BE CALLED
			// The state machine cannot depend on either callback advancing the
			// state because in some situations neither callback can fire and the
			// confirmation dismissed/closed/replaced.
			// State machine must implement OnThink and check if the required
			// confirmation box is still present!
			// This code implements no fallback - it will result in minor UI
			// bug that the language box will be changed, but the title not restarted.
			// Vitaliy -- 9/26/2009
			//
			confirmation->SetUsageData(data);
		}
		else
		{
			// Ready to write that data... go ahead and nav back
			BaseClass::OnKeyCodePressed(code);
		}
		break;

	default:
		BaseClass::OnKeyCodePressed(code);
		break;
	}
}

//=============================================================================
void Audio::OnCommand(const char *command)
{
	if( Q_stricmp( "#GameUI_Headphones", command ) == 0 )
	{
		CGameUIConVarRef snd_surround_speakers("Snd_Surround_Speakers");
		snd_surround_speakers.SetValue( "0" );

		UpdateEnhanceStereo();
	}
	else if( Q_stricmp( "#GameUI_2Speakers", command ) == 0 )
	{
		CGameUIConVarRef snd_surround_speakers("Snd_Surround_Speakers");
		snd_surround_speakers.SetValue( "2" );

		// headphones at high quality get enhanced stereo turned on
		CGameUIConVarRef dsp_enhance_stereo( "dsp_enhance_stereo" );
		dsp_enhance_stereo.SetValue( 0 );
	}
	else if( Q_stricmp( "#GameUI_4Speakers", command ) == 0 )
	{
		CGameUIConVarRef snd_surround_speakers("Snd_Surround_Speakers");
		snd_surround_speakers.SetValue( "4" );

		// headphones at high quality get enhanced stereo turned on
		CGameUIConVarRef dsp_enhance_stereo( "dsp_enhance_stereo" );
		dsp_enhance_stereo.SetValue( 0 );
	}
	else if( Q_stricmp( "#GameUI_5Speakers", command ) == 0 )
	{
		CGameUIConVarRef snd_surround_speakers("Snd_Surround_Speakers");
		snd_surround_speakers.SetValue( "5" );

		// headphones at high quality get enhanced stereo turned on
		CGameUIConVarRef dsp_enhance_stereo( "dsp_enhance_stereo" );
		dsp_enhance_stereo.SetValue( 0 );
	}
	else if( Q_stricmp( "#GameUI_High", command ) == 0 )
	{
		CGameUIConVarRef Snd_PitchQuality( "Snd_PitchQuality" );
		CGameUIConVarRef dsp_slow_cpu( "dsp_slow_cpu" );
		dsp_slow_cpu.SetValue(false);
		Snd_PitchQuality.SetValue(true);

		UpdateEnhanceStereo();
	}
	else if( Q_stricmp( "#GameUI_Medium", command ) == 0 )
	{
		CGameUIConVarRef Snd_PitchQuality( "Snd_PitchQuality" );
		CGameUIConVarRef dsp_slow_cpu( "dsp_slow_cpu" );
		dsp_slow_cpu.SetValue(false);
		Snd_PitchQuality.SetValue(false);

		// headphones at high quality get enhanced stereo turned on
		CGameUIConVarRef dsp_enhance_stereo( "dsp_enhance_stereo" );
		dsp_enhance_stereo.SetValue( 0 );
	}
	else if( Q_stricmp( "#GameUI_Low", command ) == 0 )
	{
		CGameUIConVarRef Snd_PitchQuality( "Snd_PitchQuality" );
		CGameUIConVarRef dsp_slow_cpu( "dsp_slow_cpu" );
		dsp_slow_cpu.SetValue(true);
		Snd_PitchQuality.SetValue(false);

		// headphones at high quality get enhanced stereo turned on
		CGameUIConVarRef dsp_enhance_stereo( "dsp_enhance_stereo" );
		dsp_enhance_stereo.SetValue( 0 );
	}
	else if( Q_stricmp( "#L4D360UI_AudioOptions_CaptionOff", command ) == 0 )
	{
		CGameUIConVarRef closecaption("closecaption");
		CGameUIConVarRef cc_subtitles("cc_subtitles");
		closecaption.SetValue( 0 );
		cc_subtitles.SetValue( 0 );
	}
	else if( Q_stricmp( "#L4D360UI_AudioOptions_CaptionSubtitles", command ) == 0 )
	{
		CGameUIConVarRef closecaption("closecaption");
		CGameUIConVarRef cc_subtitles("cc_subtitles");
		closecaption.SetValue( 1 );
		cc_subtitles.SetValue( 1 );
	}
	else if( Q_stricmp( "#L4D360UI_AudioOptions_CaptionOn", command ) == 0 )
	{
		CGameUIConVarRef closecaption("closecaption");
		CGameUIConVarRef cc_subtitles("cc_subtitles");
		closecaption.SetValue( 1 );
		cc_subtitles.SetValue( 0 );
	}
	else if ( StringHasPrefix( command, VIDEO_LANGUAGE_COMMAND_PREFIX ) )
	{
		int iCommandNumberPosition = Q_strlen( VIDEO_LANGUAGE_COMMAND_PREFIX );
		int iSelectedLanguage = clamp( command[ iCommandNumberPosition ] - '0', 0, m_nNumAudioLanguages - 1 );

		m_nSelectedAudioLanguage = m_nAudioLanguages[ iSelectedLanguage ].languageCode;
	}
	else if( Q_stricmp( "VoiceCommunicationEnabled", command ) == 0 )
	{
		CGameUIConVarRef voice_modenable("voice_modenable");
		CGameUIConVarRef voice_vox("voice_vox");
		CGameUIConVarRef voice_enable("voice_enable");
		voice_modenable.SetValue( 1 );
		voice_enable.SetValue( 1 );

		bool bMicVolumeFound = m_pVoiceTweak->IsControlFound( MicrophoneVolume );

		if ( voice_vox.GetBool() )
		{
			voice_vox.SetValue( 0 );
			voice_vox.SetValue( 1 );
		}

		SetControlEnabled( "SldVoiceTransmitVolume", bMicVolumeFound );
		SetControlEnabled( "SldVoiceReceiveVolume", true );
		SetControlEnabled( "DrpBoostMicrophone", true );
		SetControlVisible( "MicMeter", true );
		SetControlVisible( "MicMeter2", false );
		SetControlEnabled( "BtnTestMicrophone", true );
		SetControlEnabled( "DrpVoiceCommunicationStyle", true );
		SetControlEnabled( "SldVoiceVoxThreshold", voice_vox.GetBool() );
	}
	else if( Q_stricmp( "VoiceCommunicationDisabled", command ) == 0 )
	{
		CGameUIConVarRef voice_modenable("voice_modenable");
		CGameUIConVarRef voice_enable("voice_enable");
		voice_modenable.SetValue( 0 );
		voice_enable.SetValue( 0 );

		SetControlEnabled( "SldVoiceTransmitVolume", false );
		SetControlEnabled( "SldVoiceReceiveVolume", false );
		SetControlEnabled( "DrpBoostMicrophone", false );
		SetControlVisible( "MicMeter", false );
		SetControlVisible( "MicMeter2", false );
		SetControlEnabled( "BtnTestMicrophone", false );
		SetControlEnabled( "DrpVoiceCommunicationStyle", false );
		SetControlEnabled( "SldVoiceVoxThreshold", false);
	
		if ( m_drpBoostMicrophoneGain )
		{
			m_drpBoostMicrophoneGain->CloseDropDown();
			m_drpBoostMicrophoneGain->SetEnabled( false );
		}
	}
	else if( Q_stricmp( "VoiceCommunicationPushToTalk", command ) == 0 )
	{
		CGameUIConVarRef voice_vox("voice_vox");
		voice_vox.SetValue( 0 );

		SetControlEnabled( "SldVoiceVoxThreshold", false );
	}
	else if( Q_stricmp( "VoiceCommunicationOpenMic", command ) == 0 )
	{
		CGameUIConVarRef voice_vox("voice_vox");
		voice_vox.SetValue( 1 );

		SetControlEnabled( "SldVoiceVoxThreshold", true );
	}
	else if( Q_stricmp( "BoostMicrophoneEnabled", command ) == 0 )
	{
		if ( m_pVoiceTweak )
		{
			m_pVoiceTweak->SetControlFloat( MicBoost, 1.0f );
		}
	}
	else if( Q_stricmp( "BoostMicrophoneDisabled", command ) == 0 )
	{
		if ( m_pVoiceTweak )
		{
			m_pVoiceTweak->SetControlFloat( MicBoost, 0.0f );
		}
	}
	else if( Q_stricmp( "TestMicrophone", command ) == 0 )
	{
		FlyoutMenu::CloseActiveMenu();
		if ( m_pVoiceTweak && m_pMicMeter2 )
		{
			if ( !m_pMicMeter2->IsVisible() )
			{
				StartTestMicrophone();
			}
			else
			{
				EndTestMicrophone();
			}
		}
	}
	else if( !Q_strcmp( command, "Jukebox" ) )
	{
		if ( m_pVoiceTweak && m_pMicMeter2 )
		{
			if ( m_pMicMeter2->IsVisible() )
			{
				EndTestMicrophone();
			}
		}

		CBaseModPanel::GetSingleton().OpenWindow( WT_JUKEBOX, this, true );
	}
	else if ( !Q_strcmp( command, "AdvancedMixers" ) )
	{
		if ( m_pVoiceTweak && m_pMicMeter2 )
		{
			if ( m_pMicMeter2->IsVisible() )
			{
				EndTestMicrophone();
			}
		}

		CBaseModPanel::GetSingleton().OpenWindow( WT_AUDIOADVANCEDMIXERS, this, true );
	}
	else if( Q_stricmp( "Back", command ) == 0 )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if( Q_stricmp( "3rdPartyCredits", command ) == 0 )
	{
		if ( m_pVoiceTweak && m_pMicMeter2 )
		{
			if ( m_pMicMeter2->IsVisible() )
			{
				EndTestMicrophone();
			}
		}

		EndTestMicrophone();
		OpenThirdPartySoundCreditsDialog();
		FlyoutMenu::CloseActiveMenu();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void Audio::UpdateEnhanceStereo( void )
{
	// headphones at high quality get enhanced stereo turned on
	CGameUIConVarRef Snd_PitchQuality( "Snd_PitchQuality" );
	CGameUIConVarRef dsp_slow_cpu( "dsp_slow_cpu" );
	CGameUIConVarRef snd_surround_speakers("Snd_Surround_Speakers");
	CGameUIConVarRef dsp_enhance_stereo( "dsp_enhance_stereo" );

	if ( !dsp_slow_cpu.GetBool() && Snd_PitchQuality.GetBool() && snd_surround_speakers.GetInt() == 0 )
	{
		dsp_enhance_stereo.SetValue( 1 );
	}
	else
	{
		dsp_enhance_stereo.SetValue( 0 );
	}
}

void Audio::OnNotifyChildFocus( vgui::Panel* child )
{
}

void Audio::UpdateFooter( bool bEnableCloud )
{
	if ( !BaseModUI::CBaseModPanel::GetSingletonPtr() )
		return;

	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		footer->SetButtons( FB_ABUTTON | FB_BBUTTON, FF_AB_ONLY, false );
		footer->SetButtonText( FB_ABUTTON, "#L4D360UI_Select" );
		footer->SetButtonText( FB_BBUTTON, "#L4D360UI_Controller_Done" );

		footer->SetShowCloud( bEnableCloud );
	}
}

void Audio::OnFlyoutMenuClose( vgui::Panel* flyTo )
{
	UpdateFooter( true );
}

void Audio::OnFlyoutMenuCancelled()
{
}

//=============================================================================
Panel* Audio::NavigateBack()
{
	if ( m_pVoiceTweak && m_sldRecieveVolume )
	{
		float flVal = m_sldRecieveVolume->GetCurrentValue();
		m_pVoiceTweak->SetControlFloat( OtherSpeakerScale, flVal );
	}

	engine->ClientCmd_Unrestricted( VarArgs( "host_writeconfig_ss %d", XBX_GetPrimaryUserId() ) );

	return BaseClass::NavigateBack();
}

void Audio::UseSelectedLanguage()
{
	m_pchUpdatedAudioLanguage = GetLanguageName( m_nSelectedAudioLanguage );
}

void Audio::ResetLanguage()
{
	m_pchUpdatedAudioLanguage = "";
}

void Audio::StartTestMicrophone()
{
	if ( m_pVoiceTweak && m_pVoiceTweak->StartVoiceTweakMode() )
	{
		if ( m_pMicMeter )
		{
			m_pMicMeter->SetVisible( false );
		}

		if ( m_pMicMeter2 )
		{
			m_pMicMeter2->SetVisible( true );
		}

		if ( m_pMicMeterIndicator )
		{
			m_pMicMeterIndicator->SetVisible( true );
		}

		SetControlEnabled( "SldVoiceTransmitVolume", false );
		SetControlEnabled( "SldVoiceReceiveVolume", false );

		if ( m_drpVoiceCommunication )
		{
			m_drpVoiceCommunication->CloseDropDown();
			m_drpVoiceCommunication->SetEnabled( false );
		}

		if ( m_drpBoostMicrophoneGain )
		{
			m_drpBoostMicrophoneGain->CloseDropDown();
			m_drpBoostMicrophoneGain->SetEnabled( false );
		}
	}
}

void Audio::EndTestMicrophone()
{
	if ( m_pVoiceTweak && m_pVoiceTweak->IsStillTweaking() )
	{
		m_pVoiceTweak->EndVoiceTweakMode();

		CGameUIConVarRef voice_vox("voice_vox");

		if ( voice_vox.GetBool() )
		{
			voice_vox.SetValue( 0 );
			voice_vox.SetValue( 1 );
		}
	}

	if ( m_pMicMeter )
	{
		CGameUIConVarRef voice_modenable("voice_modenable");
		CGameUIConVarRef voice_enable("voice_enable");
		m_pMicMeter->SetVisible( voice_enable.GetBool() && voice_modenable.GetBool() );
	}

	if ( m_pMicMeter2 )
	{
		m_pMicMeter2->SetVisible( false );
	}

	if ( m_pMicMeterIndicator )
	{
		m_pMicMeterIndicator->SetVisible( false );
	}

	bool bMicVolumeFound = m_pVoiceTweak->IsControlFound( MicrophoneVolume );

	SetControlEnabled( "DrpVoiceCommunication", true );
	SetControlEnabled( "SldVoiceTransmitVolume", bMicVolumeFound );
	SetControlEnabled( "SldVoiceReceiveVolume", true );
	SetControlEnabled( "DrpBoostMicrophone", true );
}

void Audio::OpenThirdPartySoundCreditsDialog()
{
	if (!m_OptionsSubAudioThirdPartyCreditsDlg.Get())
	{
		m_OptionsSubAudioThirdPartyCreditsDlg = new COptionsSubAudioThirdPartyCreditsDlg(GetVParent());
	}
	m_OptionsSubAudioThirdPartyCreditsDlg->Activate();
}

void Audio::AcceptLanguageChangeCallback() 
{
	Audio *self = static_cast< Audio * >( CBaseModPanel::GetSingleton().GetWindow( WT_AUDIO ) );
	if( self )
	{
		self->BaseClass::OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
}

//=============================================================================
void Audio::CancelLanguageChangeCallback()
{
	Audio *self = static_cast< Audio * >( CBaseModPanel::GetSingleton().GetWindow( WT_AUDIO ) );
	if( self )
	{
		self->ResetLanguage();
	}
}

void Audio::PaintBackground()
{
	//BaseClass::DrawDialogBackground( "#GameUI_Audio", NULL, "#L4D360UI_AudioVideo_Desc", NULL, NULL, true );
}

void Audio::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// required for new style
	SetPaintBackgroundEnabled( true );
	SetupAsDialogStyle();
}
