#include "cbase.h"
#include "c_asw_player.h"
#include "c_asw_inhabitable_npc.h"
#include "c_user_message_register.h"
#include "fmtstr.h"
#include "asw_util_shared.h"
#include "asw_hud_floating_number.h"
#include "iclientmode.h"
#include "asw_input.h"

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
	"Swarm",
};

// ConVar names, default values, descriptions, and sound effects stolen from TF2.
ConVar tf_dingalingaling[2]
{
	{ "tf_dingalingaling", "0", FCVAR_ARCHIVE, "If set to 1, play a sound every time you injure an enemy.The sound can be customized by replacing the 'sound/ui/hitsound.wav' file." },
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
ConVar asw_floating_number_combine_window( "asw_floating_number_combine_window", "0.5", FCVAR_NONE );

static struct RD_Floating_Damage_Number_t
{
	float m_flAccumulatedDamage{};
	float m_flLastDamageNumberTime{};
	HPARTICLEFFECT m_hDamageNumberParticle{};
} s_RD_Floating_Damage_Numbers[MAX_EDICTS];

void UpdateHitConfirmRotation()
{
	// rotate damage numbers to whatever our current camera angle is

	QAngle vecAngles;
	vecAngles[PITCH] = 0.0f;
	vecAngles[YAW] = ASWInput()->ASW_GetCameraYaw() - 90;
	vecAngles[ROLL] = ASWInput()->ASW_GetCameraPitch();

	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

	for ( int i = 0; i < MAX_EDICTS; i++ )
	{
		HPARTICLEFFECT &hEffect = s_RD_Floating_Damage_Numbers[i].m_hDamageNumberParticle;
		if ( !hEffect )
			continue;

		hEffect->SetControlPointOrientation( 5, vecForward, vecRight, vecUp );
	}
}

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
	short weaponindex = msg.ReadShort();

	Assert( entindex >= -1 && entindex < MAX_EDICTS );
	Assert( targetent >= -1 && targetent < MAX_EDICTS );
	Assert( weaponindex >= -1 && weaponindex < MAX_EDICTS );

	C_BaseEntity *pAttacker = C_BaseEntity::Instance( entindex );
	C_BaseEntity *pTarget = C_BaseEntity::Instance( targetent );
	C_BaseEntity *pWeapon = C_BaseEntity::Instance( weaponindex );

	Assert( vecDamagePosition != vec3_origin );
	if ( vecDamagePosition == vec3_origin && pTarget )
		vecDamagePosition = pTarget->WorldSpaceCenter();

	if ( pTarget )
	{
		Vector vecOffset = vecDamagePosition - pTarget->WorldSpaceCenter();
		float flMaxDistFromCenter = pTarget->BoundingRadius() * 0.75f;
		if ( vecOffset.LengthSqr() > Square( flMaxDistFromCenter ) )
		{
			vecOffset.NormalizeInPlace();
			vecOffset *= flMaxDistFromCenter;
			vecDamagePosition = vecOffset + pTarget->WorldSpaceCenter();
		}
	}

	ReactiveDropInventory::OnHitConfirm( pAttacker, pTarget, vecDamagePosition, bKilled, bDamageOverTime, bBlastDamage, iDisposition, flDamage, pWeapon );

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer || !pPlayer->GetViewNPC() )
		return;

	bool bMyHit = pPlayer->GetViewNPC()->entindex() == entindex;
	bool bHitMe = pPlayer->GetViewNPC()->entindex() == targetent;

	if ( !bMyHit && !bHitMe )
		return;

	bool bDoHitSound = bMyHit && !bHitMe && tf_dingalingaling[bKilled].GetBool();
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
		bool bIsAccumulated = false;
		if ( targetent >= 0 && targetent < MAX_EDICTS && !bBlastDamage && !bDamageOverTime )
		{
			RD_Floating_Damage_Number_t &accumulator = s_RD_Floating_Damage_Numbers[targetent];
			if ( gpGlobals->curtime - accumulator.m_flLastDamageNumberTime < ( asw_floating_number_combine.GetBool() ? asw_floating_number_combine_window.GetFloat() : 0.0f ) )
			{
				bIsAccumulated = true;
				flDamage += accumulator.m_flAccumulatedDamage;
				CNewParticleEffect *pParticle = accumulator.m_hDamageNumberParticle.GetObject();
				if ( pParticle )
				{
					pAttacker->ParticleProp()->StopEmissionAndDestroyImmediately( pParticle );
				}
			}

			accumulator.m_flLastDamageNumberTime = gpGlobals->curtime;
			accumulator.m_flAccumulatedDamage = flDamage;
		}

		int iDmgCustom = 0;
		// don't render punctuation if we're not going to render a number
		if ( flDamage >= 0.5f )
		{
			if ( iDisposition == 3 ) // D_LI
				iDmgCustom |= DAMAGE_FLAG_CRITICAL;
			if ( bHitMe )
				iDmgCustom |= DAMAGE_FLAG_WEAKSPOT;
		}

		HPARTICLEFFECT hParticle = UTIL_ASW_ParticleDamageNumber( pAttacker, vecDamagePosition, flDamage, iDmgCustom, bDamageOverTime ? 0.5f : 1.0f, bBlastDamage || bDamageOverTime, bIsAccumulated );
		if ( targetent >= 0 && targetent < MAX_EDICTS && !bBlastDamage && !bDamageOverTime )
		{
			s_RD_Floating_Damage_Numbers[targetent].m_hDamageNumberParticle = hParticle;
		}
	}
}
USER_MESSAGE_REGISTER( RDHitConfirm );
