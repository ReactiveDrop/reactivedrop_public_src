#ifndef _INLCUDE_C_ASW_BUZZER_H
#define _INLCUDE_C_ASW_BUZZER_H

#include "c_asw_alien.h"
#include "asw_shareddefs.h"

class CNewParticleEffect;

// Buzzer is our flying poisoning alien (based on the hl2 manhack code)

class C_ASW_Buzzer : public C_ASW_Inhabitable_NPC
{
public:
	C_ASW_Buzzer();
	virtual ~C_ASW_Buzzer();

	DECLARE_CLASS( C_ASW_Buzzer, C_ASW_Inhabitable_NPC );
	DECLARE_CLIENTCLASS();

	// Purpose: Start the buzzer's engine sound.
	void OnDataChanged( DataUpdateType_t type ) override;
	void UpdateOnRemove() override;
	void OnRestore() override;
	float GetInterpolationAmount( int flags ) override;

	Class_T Classify( void ) override { return (Class_T) CLASS_ASW_BUZZER; }
	bool IsAlien( void ) const override { return true; }

	float GetRadius() override { return 18; }
	bool GetShadowCastDistance( float *pDistance, ShadowType_t shadowType ) const override;
	bool GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const override;
	ShadowType_t ShadowCastType() override;

private:
	C_ASW_Buzzer( const C_ASW_Buzzer & ) = delete;

	// Purpose: Start + stop the buzzer's engine sound.
	void SoundInit( void );
	void SoundShutdown( void );

	CSoundPatch *m_pEngineSound1;

	int m_nEnginePitch1;
	float m_flEnginePitch1Time;

	CUtlReference<CNewParticleEffect> m_pTrailEffect;
};

#endif /* _INLCUDE_C_ASW_BUZZER_H */