#include "cbase.h"
#include "asw_weapon_stim_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#endif
#include "asw_gamerules.h"
#include "asw_marine_skills.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Stim, DT_ASW_Weapon_Stim )

BEGIN_NETWORK_TABLE( CASW_Weapon_Stim, DT_ASW_Weapon_Stim )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Stim )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_stim, CASW_Weapon_Stim );
PRECACHE_WEAPON_REGISTER(asw_weapon_stim);

#ifndef CLIENT_DLL

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Stim )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Stim::CASW_Weapon_Stim()
{
	m_fMinRange1	= 0;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;
}


CASW_Weapon_Stim::~CASW_Weapon_Stim()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CASW_Weapon_Stim::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}

bool CASW_Weapon_Stim::OffhandActivate()
{	
	if (!GetMarine() || GetMarine()->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
		return false;

	if (m_flNextPrimaryAttack < gpGlobals->curtime)
		PrimaryAttack();

	return true;
}

void CASW_Weapon_Stim::PrimaryAttack( void )
{
	if (!ASWGameRules())
		return;
	float fStimEndTime = ASWGameRules()->GetStimEndTime();
	if ( fStimEndTime <= gpGlobals->curtime + 1.0f )
	{
		InjectStim();
	}
}

void CASW_Weapon_Stim::InjectStim()
{
	CASW_Marine *pMarine = GetMarine();

	if ( m_iClip1 <= 0 )
		return;

	if (pMarine)		// firing from a marine
	{
		//make the proper weapon sound
		WeaponSound(SINGLE);
		// sets the animation on the weapon model iteself
		SendWeaponAnim( GetPrimaryAttackActivity() );

		// sets the animation on the marine holding this weapon
		//pMarine->SetAnimation( PLAYER_ATTACK1 );
#ifndef CLIENT_DLL

		// make it cause the slow time
		float fDuration = MarineSkills()->GetBestSkillValue( ASW_MARINE_SKILL_DRUGS, ASW_MARINE_SUBSKILL_STIM_DURATION );
		if (fDuration < 0)
			fDuration = 5;

		//CALL_ATTRIB_HOOK_FLOAT( fDuration, mod_duration );

		ASWGameRules()->StartStim( fDuration, pMarine->GetCommander() );

		pMarine->OnWeaponFired( this, 1 );
#endif
		// decrement ammo
		m_iClip1 -= 1;

		m_flNextPrimaryAttack = gpGlobals->curtime + 4.0f;

#ifdef GAME_DLL
		DestroyIfEmpty( true );
#endif
	}
}

void CASW_Weapon_Stim::Precache()
{
	BaseClass::Precache();
}

int CASW_Weapon_Stim::ASW_SelectWeaponActivity(int idealActivity)
{
	// we just use the normal 'no weapon' anims for this
	return idealActivity;
}
