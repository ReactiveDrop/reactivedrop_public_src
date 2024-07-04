#ifndef _INLCUDE_C_ASW_PARASITE_H
#define _INLCUDE_C_ASW_PARASITE_H

#include "c_asw_alien.h"

class C_ASW_Parasite : public C_ASW_Alien
{
public:
	DECLARE_CLASS( C_ASW_Parasite, C_ASW_Alien );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_ASW_Parasite();
	virtual ~C_ASW_Parasite();

	void OnRestore() override;
	void UpdateOnRemove() override;
	void SoundInit();
	void SoundShutdown();
	void OnDataChanged( DataUpdateType_t updateType ) override;
	const char *GetBigDeathParticleEffectName() override { return "drone_death_sml"; }
	float GetRadius() override { return 12; }
	Class_T Classify( void ) override { return (Class_T) CLASS_ASW_PARASITE; }
	bool IsAimTarget() override;
	ShadowType_t ShadowCastType() override;
	bool GetShadowCastDistance( float *pDistance, ShadowType_t shadowType ) const override;
	bool GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const override;

	bool IsEggIdle() const { return m_bDoEggIdle; }
	bool IsJumping() const;

private:
	C_ASW_Parasite( const C_ASW_Parasite & ) = delete;
	CSoundPatch *m_pLoopingSound;
	bool m_bStartIdleSound;
	bool m_bDoEggIdle;
	bool m_bInfesting;
};

#endif /* _INLCUDE_C_ASW_PARASITE_H */