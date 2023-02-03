//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Client side antlion guard. Used to create dlight for the cave guard.
//
//=============================================================================

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "dlight.h"
#include "iefx.h"
#include "c_asw_alien.h"
#include "c_gib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if HL2_EPISODIC
// When enabled, add code to have the antlion bleed profusely as it is badly injured.
#define ANTLIONGUARD_BLOOD_EFFECTS 2
#endif

//ConVar rd_antlionguard_gib_ang_impulse("rd_antlionguard_gib_ang_impulse", "1000", FCVAR_CHEAT, "A random impulse from -value to +value to give to a gib");
//ConVar rd_antlionguard_gib_velocity_min("rd_antlionguard_gib_velocity_min", "400", FCVAR_CHEAT, "A random impulse from -value to +value to give to a gib");
//ConVar rd_antlionguard_gib_velocity_max("rd_antlionguard_gib_velocity_max", "600", FCVAR_CHEAT, "A random impulse from -value to +value to give to a gib");

static void CreateClientGib(const char *model, const Vector &origin)
{
	Vector vecNewVelocity = Vector(0, 0, 1);

	// mix in some noise
	vecNewVelocity.x += random->RandomFloat(-0.25, 0.25);
	vecNewVelocity.y += random->RandomFloat(-0.25, 0.25);
	vecNewVelocity.z += random->RandomFloat(-0.25, 0.25);

	//vecNewVelocity *= random->RandomFloat(rd_antlionguard_gib_velocity_min.GetFloat(), rd_antlionguard_gib_velocity_max.GetFloat());
	//C_Gib::CreateClientsideGib(model, origin, vecNewVelocity, RandomAngularImpulse(-1 * rd_antlionguard_gib_ang_impulse.GetFloat(), rd_antlionguard_gib_ang_impulse.GetFloat()), 5.0f);
	vecNewVelocity *= random->RandomFloat( 400, 600 );
	C_Gib *pGib = C_Gib::CreateClientsideGib(model, origin, vecNewVelocity, RandomAngularImpulse( -1000.0f, 1000.0f ), 5.0f);
	if (pGib)
	{
		pGib->ParticleProp()->Create("shieldBug_legs", PATTACH_ABSORIGIN_FOLLOW);
	}
}

class C_NPC_AntlionGuard : public C_ASW_Alien
{
public:
	C_NPC_AntlionGuard()
		: m_dlight(NULL) 
	{}
	~C_NPC_AntlionGuard()
	{
		if (m_dlight)
		{
			m_dlight->die = gpGlobals->curtime - 1.0f;
			m_dlight = NULL;
		}
	}

	DECLARE_CLASS( C_NPC_AntlionGuard, C_ASW_Alien );
	DECLARE_CLIENTCLASS();
 	DECLARE_DATADESC();

	virtual Class_T Classify() { return (Class_T)CLASS_ASW_ANTLIONGUARD; }
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void ClientThink();
	virtual C_BaseAnimating* BecomeRagdollOnClient(void)
	{
		C_BaseAnimating *result = BaseClass::BecomeRagdollOnClient();

		if (m_nDeathStyle == kDIE_INSTAGIB)
		{
			// make them higher so gibs don't penetrate ground
			Vector gibOrigin = this->GetAbsOrigin() + Vector(0, 0, 64);			
// 			CreateClientGib("models/gib_antlionguard_torso.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_head.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_hand1_p1.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_hand1_p2.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_hand2_p1.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_hand2_p2.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_leg1_p1.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_leg1_p2.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_leg2_p1.mdl", gibOrigin);
// 			CreateClientGib("models/gib_antlionguard_leg2_p2.mdl", gibOrigin);

			ParticleProp()->Create("shieldbug_brain_explode", PATTACH_ABSORIGIN);
			ParticleProp()->Create("mortarbug_death", PATTACH_ABSORIGIN);
		}

		return result;
	}

private:

	bool m_bCavernBreed;
	bool m_bInCavern;
	dlight_t *m_dlight;

#if HL2_EPISODIC
	unsigned char m_iBleedingLevel; //< the version coming from the server
	unsigned char m_iPerformingBleedingLevel; //< the version we're currently performing (for comparison to one above)
	CNewParticleEffect *m_pBleedingFX;

	/// update the hemorrhage particle effect
	virtual void UpdateBleedingPerformance( void );
#endif

	C_NPC_AntlionGuard( const C_NPC_AntlionGuard & );
};


//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC( C_NPC_AntlionGuard )
END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT(C_NPC_AntlionGuard, DT_NPC_AntlionGuard, CNPC_AntlionGuard)
	RecvPropBool( RECVINFO( m_bCavernBreed ) ),
	RecvPropBool( RECVINFO( m_bInCavern ) ),

#if ANTLIONGUARD_BLOOD_EFFECTS
	RecvPropInt(  RECVINFO( m_iBleedingLevel ) ),
#endif
END_RECV_TABLE()


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_AntlionGuard::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( (type == DATA_UPDATE_CREATED) && m_bCavernBreed && m_bInCavern )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}


#if HL2_EPISODIC
	if (m_iBleedingLevel != m_iPerformingBleedingLevel)
	{
		UpdateBleedingPerformance();
	}
#endif

}

#if HL2_EPISODIC
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_AntlionGuard::UpdateBleedingPerformance()
{
	// get my particles
	CParticleProperty * pProp = ParticleProp();

	// squelch the prior effect if it exists
	if (m_pBleedingFX)
	{
		pProp->StopEmission(m_pBleedingFX);
		m_pBleedingFX = NULL;
	}

	// kick off a new effect
	switch (m_iBleedingLevel)
	{
	case 1: // light bleeding
		{
			m_pBleedingFX = pProp->Create( "blood_antlionguard_injured_light", PATTACH_ABSORIGIN_FOLLOW );
			AssertMsg1( m_pBleedingFX, "Particle system couldn't make %s", "blood_antlionguard_injured_light" );
			if ( m_pBleedingFX )
			{
				pProp->AddControlPoint( m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW );
			}
		}
		break;

	case 2: // severe bleeding
		{
			m_pBleedingFX = pProp->Create( "blood_antlionguard_injured_heavy", PATTACH_ABSORIGIN_FOLLOW );
			AssertMsg1( m_pBleedingFX, "Particle system couldn't make %s", "blood_antlionguard_injured_heavy" );
			if ( m_pBleedingFX )
			{
				pProp->AddControlPoint( m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW );
			}

		}
		break;
	}

	m_iPerformingBleedingLevel = m_iBleedingLevel;
}
#endif

// reactivedrop: commented this because it causes strange bug with dlights under the antlion guard 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_AntlionGuard::ClientThink()
{
	// update the dlight. (always done because clienthink only exists for cavernguard)
/*	if (!m_dlight)
	{
		m_dlight = effects->CL_AllocDlight( index );
		m_dlight->color.r = 44;
		m_dlight->color.g = 51;
		m_dlight->color.b = 16;
		m_dlight->radius	= 180;
		//m_dlight->minlight = 128.0 / 256.0f;
		//m_dlight->flags = DLIGHT_NO_MODEL_ILLUMINATION;
	}

	m_dlight->origin	= GetAbsOrigin();
	m_dlight->die = gpGlobals->curtime + 30.0f;
	// dl->die = gpGlobals->curtime + 0.1f;
//*/
	BaseClass::ClientThink();
}
