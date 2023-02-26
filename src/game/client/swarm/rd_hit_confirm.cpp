#include "cbase.h"
#include "c_asw_player.h"
#include "c_asw_inhabitable_npc.h"
#include "c_user_message_register.h"
#include "fmtstr.h"
#include "asw_util_shared.h"
#include "asw_hud_floating_number.h"
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


constexpr const char *const s_DingalingSounds[] =
{
	"DefaultDing",
	"Electro",
	"Notes",
	"Percussion",
	"Retro",
	"Space",
	"Beepo",
	"Vortex",
	"Squasher",
};

// ConVar names, default values, descriptions, and sound effects stolen from TF2.
ConVar tf_dingalingaling[2]
{
	{ "tf_dingalingaling", "0", FCVAR_ARCHIVE, "If set to 1, play a sound everytime you injure an enemy.The sound can be customized by replacing the 'sound/ui/hitsound.wav' file." },
	{ "tf_dingalingaling_lasthit", "0", FCVAR_ARCHIVE, "If set to 1, play a sound whenever one of your attacks kills an enemy.The sound can be customized by replacing the 'sound/ui/killsound.wav' file." },
};
ConVar tf_dingaling_volume[2]
{
	{ "tf_dingaling_volume", "0.75", FCVAR_ARCHIVE, "Desired volume of the hit sound.", true, 0, true, 1 },
	{ "tf_dingaling_lasthit_volume", "0.75", FCVAR_ARCHIVE, "Desired volume of the last hit sound.", true, 0, true, 1 },
};
ConVar tf_dingaling_pitchmindmg[2]
{
	{ "tf_dingaling_pitchmindmg", "100", FCVAR_ARCHIVE, "Desired pitch of the hit sound when a minimal damage hit (<= 10 health) is done.", true, 1, true, 255 },
	{ "tf_dingaling_lasthit_pitchmindmg", "100", FCVAR_ARCHIVE, "Desired pitch of the last hit sound when a minimal damage hit (<= 10 health) is done.", true, 1, true, 255 },
};
ConVar tf_dingaling_pitchmaxdmg[2]
{
	{ "tf_dingaling_pitchmaxdmg", "100", FCVAR_ARCHIVE, "Desired pitch of the hit sound when a maximum damage hit (>= 150 health) is done.", true, 1, true, 255 },
	{ "tf_dingaling_lasthit_pitchmaxdmg", "100", FCVAR_ARCHIVE, "Desired pitch of the last hit sound when a maximum damage hit (>= 150 health) is done.", true, 1, true, 255 },
};
ConVar tf_dingaling_pitch_override[2]
{
	{ "tf_dingaling_pitch_override", "-1", FCVAR_NONE, "If set, pitch for all hit sounds." },
	{ "tf_dingaling_lasthit_pitch_override", "-1", FCVAR_NONE, "If set, pitch for last hit sounds." },
};
ConVar tf_dingalingaling_effect[2]
{
	{ "tf_dingalingaling_effect", "0", FCVAR_ARCHIVE, "Which Dingalingaling sound is used", true, 0, true, NELEMS( s_DingalingSounds ) - 1 },
	{ "tf_dingalingaling_last_effect", "0", FCVAR_ARCHIVE, "Which final hit sound to play when the target expires.", true, 0, true, NELEMS( s_DingalingSounds ) - 1 },
};
ConVar tf_dingalingaling_repeat_delay( "tf_dingalingaling_repeat_delay", "0.0", FCVAR_ARCHIVE, "Desired repeat delay of the hit sound.Set to 0 to play a sound for every instance of damage dealt.", true, 0, false, 0 );
extern ConVar asw_floating_number_type;
ConVar asw_floating_number_combine( "asw_floating_number_combine", "0", FCVAR_ARCHIVE );

void __MsgFunc_RDHitConfirm( bf_read &msg )
{
	int entindex = msg.ReadShort();
	int targetent = msg.ReadShort();
	Vector vecDamagePosition;
	msg.ReadBitVec3Coord( vecDamagePosition );
	bool bKilled = msg.ReadOneBit();
	bool bDamageOverTime = msg.ReadOneBit();
	bool bBlastDamage = msg.ReadOneBit();
	int iDisposition = msg.ReadUBitLong( 3 );
	float flDamage = msg.ReadFloat();

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer || !pPlayer->GetNPC() || pPlayer->GetNPC()->entindex() != entindex )
		return;

	bool bDoHitSound = tf_dingalingaling[bKilled].GetBool();
	if ( bDoHitSound && !bKilled )
	{
		static float s_flNextHitSound = 0.0f;
		if ( s_flNextHitSound > gpGlobals->curtime + tf_dingalingaling_repeat_delay.GetFloat() )
			s_flNextHitSound = 0;

		if ( s_flNextHitSound > gpGlobals->curtime )
			bDoHitSound = false;
		else
			s_flNextHitSound = gpGlobals->curtime + tf_dingalingaling_repeat_delay.GetFloat();
	}

	if ( bDoHitSound )
	{
		CFmtStr szSoundName{ bKilled ? "Player.KillSound%s" : "Player.HitSound%s", s_DingalingSounds[tf_dingalingaling_effect[bKilled].GetInt()] };

		EmitSound_t soundparams;
		soundparams.m_nChannel = CHAN_STATIC;
		soundparams.m_SoundLevel = SNDLVL_NONE;
		soundparams.m_pSoundName = soundemitterbase->GetWavFileForSound( szSoundName, GENDER_NONE );
		soundparams.m_flVolume = tf_dingaling_volume[bKilled].GetFloat();
		soundparams.m_nPitch = tf_dingaling_pitch_override[bKilled].GetFloat() > 0 ?
			tf_dingaling_pitch_override[bKilled].GetFloat() :
			RemapValClamped( flDamage, 10, 150,
				tf_dingaling_pitchmindmg[bKilled].GetFloat(),
				tf_dingaling_pitchmaxdmg[bKilled].GetFloat() );
		soundparams.m_bWarnOnDirectWaveReference = false;
		soundparams.m_bEmitCloseCaption = false;
		soundparams.m_pOrigin = &vecDamagePosition;

		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, -1 /*SOUND_FROM_LOCAL_PLAYER*/, soundparams );
	}

	if ( asw_floating_number_type.GetInt() == 1 )
	{
		floating_number_params_t params;
		params.bShowPlus = false;
		params.flMoveDuration = 0.9f;
		params.flFadeStart = 0.6f;
		params.flFadeDuration = 0.3f;
		params.rgbColor = Color( 200, 200, 200, 255 );
		params.alignment = vgui::Label::a_center;
		params.bWorldSpace = true;
		params.vecPos = vecDamagePosition;

		new CFloatingNumber( flDamage, params, GetClientMode()->GetViewport() );
	}
	else if ( asw_floating_number_type.GetInt() == 2 )
	{
		C_BaseEntity *pAttacker = ClientEntityList().GetBaseEntity( entindex );
		C_ASW_Inhabitable_NPC *pTarget = dynamic_cast< C_ASW_Inhabitable_NPC * >( ClientEntityList().GetBaseEntity( targetent ) );
		if ( pTarget && !bBlastDamage && !bDamageOverTime )
		{
			if ( gpGlobals->curtime - pTarget->m_flLastDamageNumberTime < asw_floating_number_combine.GetFloat() )
			{
				flDamage += pTarget->m_flAccumulatedDamage;
				CNewParticleEffect *pParticle = pTarget->m_hDamageNumberParticle.GetObject();
				if ( pParticle )
				{
					pAttacker->ParticleProp()->StopEmissionAndDestroyImmediately( pParticle );
				}
			}

			pTarget->m_flLastDamageNumberTime = gpGlobals->curtime;
			pTarget->m_flAccumulatedDamage = flDamage;
		}

		HPARTICLEFFECT hParticle = UTIL_ASW_ParticleDamageNumber( pAttacker, vecDamagePosition, flDamage, iDisposition == 3 /*D_LI*/ ? DAMAGE_FLAG_CRITICAL : 0, bDamageOverTime ? 0.5f : 1.0f, bBlastDamage || bDamageOverTime );
		if ( pTarget && !bBlastDamage && !bDamageOverTime )
		{
			pTarget->m_hDamageNumberParticle = hParticle;
		}
	}
}
USER_MESSAGE_REGISTER( RDHitConfirm );
