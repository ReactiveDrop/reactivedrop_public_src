#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


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

	float flScale = assert_cast< ConVar * >( var )->GetFloat();

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

void RDMixerInit()
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

	if ( ConVar *pLegacyMusicVolume = g_pCVar->FindVar( "snd_musicvolume" ) )
	{
		// avoid players setting this in configs and causing the UI music sliders to break
		pLegacyMusicVolume->AddFlags( FCVAR_DEVELOPMENTONLY );
	}

	Assert( !s_bIgnoreMusicVolumeChange );
	s_bIgnoreMusicVolumeChange = true;
	for ( int i = 0; i < NELEMS( s_MixerVars ); i++ )
	{
		MixerVolumeMultiplierChanged( &s_MixerVars[i].var, "1", 1.0f );
	}
	s_bIgnoreMusicVolumeChange = false;
}

CON_COMMAND_F( _rd_mixer_init, "Deferred mixer volume initialization.", FCVAR_HIDDEN )
{
	RDMixerInit();
}
