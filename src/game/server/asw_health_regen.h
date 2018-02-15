#ifndef asw_health_regen_h__
#define asw_health_regen_h__

class CASW_Health_Regen : public CLogicalEntity 
{
public:
	DECLARE_CLASS( CASW_Health_Regen, CLogicalEntity );
	CASW_Health_Regen();
	~CASW_Health_Regen();

	virtual void Spawn();
	//virtual void Think();

private:
	void HealthIncreaseThink( void );
	void HealthDecreaseThink( void );
};

#endif // asw_health_regen_h__