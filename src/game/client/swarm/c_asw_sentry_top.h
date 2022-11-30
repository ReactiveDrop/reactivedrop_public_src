#ifndef _INCLUDED_C_ASW_SENTRY_TOP_H
#define _INCLUDED_C_ASW_SENTRY_TOP_H

class C_ASW_Sentry_Base;

class C_ASW_Sentry_Top : public C_BaseCombatCharacter
{
public:
	DECLARE_CLASS( C_ASW_Sentry_Top, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_ASW_Sentry_Top();
	virtual			~C_ASW_Sentry_Top();
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void UpdateOnRemove();
	virtual void		ClientThink( void );
	virtual void ProcessMuzzleFlashEvent();
	float GetMuzzleFlashScale();
	int GetMuzzleAttachment();
	virtual void ASWSentryTracer( const Vector &vecEnd );
	virtual bool HasPilotLight() { return false; }

	int GetSentryAngle( void );
	void Scan( void );
	void CreateRadiusBeamEdges( const Vector &vecStart, const Vector &vecDir, int iControlPoint );

	C_ASW_Sentry_Base* GetSentryBase();

protected:
	CUtlReference<CNewParticleEffect> m_hPilotLight;

private:
	C_ASW_Sentry_Top( const C_ASW_Sentry_Top & ) = delete; // not defined, not accessible

	void AdjustRadiusBeamEdges( const Vector &vecStart, const Vector &vecDir, int iControlPoint );

	CNetworkHandle( C_ASW_Sentry_Base, m_hSentryBase );
	CNetworkVar( int, m_iSentryAngle );
	CNetworkVar( float, m_fDeployYaw );
	CNetworkVar( float, m_fCenterAimYaw );
	CNetworkVar( bool, m_bLowAmmo );
	float GetDeployYaw();

	CInterpolatedVar<float> m_iv_fCenterAimYaw;

	bool m_bSpawnedDisplayEffects;
	CUtlReference<CNewParticleEffect> m_hRadiusDisplay;
	CUtlReference<CNewParticleEffect> m_hWarningLight;
};

#endif /* _INCLUDED_C_ASW_SENTRY_TOP_H */
