#ifndef _INLCUDE_C_ASW_DRONE_ADVANCED_H
#define _INLCUDE_C_ASW_DRONE_ADVANCED_H

#include "c_asw_alien.h"
#include "interpolatedvar.h"

class C_ASW_Drone_Advanced : public C_ASW_Alien
{
public:
	DECLARE_CLASS( C_ASW_Drone_Advanced, C_ASW_Alien );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_ASW_Drone_Advanced();
	virtual ~C_ASW_Drone_Advanced();

	Class_T Classify( void ) override { return ( Class_T )CLASS_ASW_DRONE; }

	float GetRunSpeed();
	void ClientThink() override;
	void OnDataChanged( DataUpdateType_t updateType ) override;
	void UpdatePoseParams();
	ShadowType_t ShadowCastType() override;
	bool GetShadowCastDistance( float *pDistance, ShadowType_t shadowType ) const override;
	bool GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const override;
	const Vector &GetAimTargetPos( const Vector &vecFiringSrc, bool bWeaponPrefersFlatAiming ) override;
	void GetPoseParameters( CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM] ) override;

	CNetworkHandle( CBaseEntity, m_hAimTarget );

private:
	C_ASW_Drone_Advanced( const C_ASW_Drone_Advanced & ) = delete;

	int m_iJumpSequence{ -2 };
	float m_flCurrentTravelYaw;
	float m_flCurrentTravelSpeed;
	float m_flClientPoseParameter[MAXSTUDIOPOSEPARAM];
	bool m_bWasJumping;
};

#endif /* _INLCUDE_C_ASW_DRONE_ADVANCED_H */