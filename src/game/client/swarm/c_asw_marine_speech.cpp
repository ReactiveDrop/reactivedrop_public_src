#include "cbase.h"
#include "c_te_effect_dispatch.h"
#include "engine/IEngineSound.h"
#include "shareddefs.h"
#include "soundemittersystem/isoundemittersystembase.h"
#include "c_asw_marine.h"
#include "c_asw_player.h"
#include "asw_marine_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar asw_client_chatter_rate("asw_client_chatter_rate", "1.0", FCVAR_NONE, "The time to wait before the next allowed marine chatter.");

bool ShouldPlayChatterDirectly( int iChatterType )
{
	return ( iChatterType == CHATTER_PAIN_LARGE ||
			 iChatterType == CHATTER_PAIN_SMALL ||
			 iChatterType == CHATTER_DIE ||
			 iChatterType == CHATTER_MARINE_DOWN );
}

void ASW_SpeechCallback(const CEffectData& data)
{
	//Vector vecPosition;
	//vecPosition = data.m_vOrigin;

	C_BaseEntity* pEnt = data.GetEntity();
	if ( pEnt && pEnt->Classify() == CLASS_ASW_MARINE )
	{
		C_ASW_Marine* pSpeaker = assert_cast<C_ASW_Marine*>(pEnt);
		if (!pSpeaker->GetMarineProfile())
			return;

		// damage type var holds which wav to play
		const char* soundbase = pSpeaker->GetMarineProfile()->m_Chatter[data.m_nDamageType];
		char soundname[128];
		Q_snprintf(soundname, sizeof(soundname), "%s%d", soundbase, data.m_nMaterial);

		// special case for playing death sounds loudly and directly to the controlling player
		if (ShouldPlayChatterDirectly(data.m_nDamageType) && pSpeaker->IsInhabited())
		{
			FOR_EACH_VALID_SPLITSCREEN_PLAYER(hh)
			{
				ACTIVE_SPLITSCREEN_PLAYER_GUARD(hh);
				C_ASW_Player* pPlayer = C_ASW_Player::GetLocalASWPlayer();
				if (pPlayer && pSpeaker->GetCommander() == pPlayer)
				{
					CLocalPlayerFilter filter;
					C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, soundname);
					return;
				}
			}
		}

		if (!pSpeaker->IsInhabited())
			pSpeaker->EmitSound(soundname);
		else
		{
			C_ASW_Player* pPlayer = C_ASW_Player::GetLocalASWPlayer();
			if (pPlayer && pSpeaker->GetCommander() == pPlayer)
			{
				pSpeaker->EmitSound(soundname);
			}
			else
			{
				if (gpGlobals->curtime > pSpeaker->m_flNextChatter)
				{
					pSpeaker->m_flNextChatter = gpGlobals->curtime + asw_client_chatter_rate.GetFloat();
					pSpeaker->EmitSound(soundname);
				}
			}
		}
	}
}

DECLARE_CLIENT_EFFECT( ASWSpeech, ASW_SpeechCallback );