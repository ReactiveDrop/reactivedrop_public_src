#include "cbase.h"
#include "c_asw_level_exploder.h"
#include "shake.h"
#include "ivieweffects.h"
#include "c_asw_objective_countdown.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_EXPLODER_INITIAL_DELAY  3.2f
#define ASW_EXPLODER_LIFETIME 16.0f
#define ASW_EXPLODER_INTERVAL 0.5f

C_ASW_Level_Exploder::C_ASW_Level_Exploder( void )
{
	m_bFlashed = false;
	m_fDieTime = 0;
}

C_ASW_Level_Exploder *C_ASW_Level_Exploder::CreateClientsideLevelExploder( const CBaseHandle & hObj )
{
	C_ASW_Objective_Countdown *pObj = assert_cast< C_ASW_Objective_Countdown * >( hObj.Get() );
	C_ASW_Level_Exploder *pExploder = new C_ASW_Level_Exploder;

	if (!pExploder)
		return NULL;

	if (!pExploder->InitializeAsClientEntity( NULL, false ))
	{
		pExploder->Release();
		return NULL;
	}

	pExploder->m_FadeToColor = pObj->m_FailColor;
	pExploder->m_fDieTime = gpGlobals->curtime + ASW_EXPLODER_LIFETIME;
	pExploder->SetNextClientThink(gpGlobals->curtime + ASW_EXPLODER_INITIAL_DELAY);

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, pObj->m_szSoundFail );
	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, pObj->m_szSoundFailLF );

	return pExploder;
}

void C_ASW_Level_Exploder::ClientThink()
{
	if (!m_bFlashed)		// flash should come after explosions?
	{
		m_bFlashed = true;
		ScreenFade_t fade;

		fade.duration = (float)(1<<SCREENFADE_FRACBITS) * 2.0f;
		fade.holdTime = (float)(1<<SCREENFADE_FRACBITS) * 5.0f;
		fade.fadeFlags = FFADE_IN;
		fade.r = m_FadeToColor.r;
		fade.g = m_FadeToColor.g;
		fade.b = m_FadeToColor.b;
		fade.a = m_FadeToColor.a;

		GetViewEffects()->Fade( fade );
	}
	else
	{
		// todo: spawn an explosion effect somewhere within the player's camera view
	}

	if (gpGlobals->curtime >= m_fDieTime)
	{
		Release();
	}
	else
	{
		SetNextClientThink(gpGlobals->curtime + ASW_EXPLODER_INTERVAL);
	}
}