#pragma once

class CRD_GameRulesConVar;

class CRD_GameRulesConVarCollection
{
public:
	CRD_GameRulesConVarCollection( const char *szName );

private:
	void AddVar( CRD_GameRulesConVar *pVar );
	friend class CRD_GameRulesConVar;

	const char *const m_szName;

	CUtlVector<CRD_GameRulesConVar *> m_Vars;
};

#define GameRulesConVar( collection, index, name, defaultValue, helpString ) CRD_GameRulesConVarFloat name( collection, index, #name, #defaultValue, helpString, false, 0, false, 0 )
#define GameRulesConVarMin( collection, index, name, defaultValue, helpString, minValue ) CRD_GameRulesConVarFloat name( collection, index, #name, #defaultValue, helpString, true, minValue, false, 0 )
#define GameRulesConVarMax( collection, index, name, defaultValue, helpString, maxValue ) CRD_GameRulesConVarFloat name( collection, index, #name, #defaultValue, helpString, false, 0, true, maxValue )
#define GameRulesConVarClamp( collection, index, name, defaultValue, helpString, minValue, maxValue ) CRD_GameRulesConVarFloat name( collection, index, #name, #defaultValue, helpString, true, minValue, true, maxValue )
#define GameRulesConVarBool( collection, index, name, defaultValue, helpString ) CRD_GameRulesConVarBool name( collection, index, #name, defaultValue ? "1" : "0", helpString )
#define GameRulesConVarSkill( collection, index, name, defaultBaseValue, defaultStepValue, helpString, skill, subskill ) \
	CRD_GameRulesConVarFloat name##_base( collection, index##_BASE, #name "_base", #defaultBaseValue, helpString, false, 0, false, 0 ); \
	CRD_GameRulesConVarFloat name##_step( collection, index##_STEP, #name "_step", #defaultStepValue, helpString, false, 0, false, 0 )
#define GameRulesConVarSkill5_2( collection, index, name, defaultBaseValue, defaultModerateValue, defaultExpertValue, helpString, skill, subskill ) \
	CRD_GameRulesConVarFloat name##_base( collection, index##_BASE, #name "_base", #defaultBaseValue, helpString " (for 0-1 points)", false, 0, false, 0 ); \
	CRD_GameRulesConVarFloat name##_moderate( collection, index##_MODERATE, #name "_moderate", #defaultModerateValue, helpString " (for 2-3 point)", false, 0, false, 0 ); \
	CRD_GameRulesConVarFloat name##_expert( collection, index##_EXPERT, #name "_expert", #defaultExpertValue, helpString " (for 4-5 points)", false, 0, false, 0 )

class CRD_GameRulesConVar
{
protected:
	CRD_GameRulesConVar( CRD_GameRulesConVarCollection &collection, int index, const char *szName, const char *szDefaultValue, const char *szHelpString, bool bHasMinValue, float flMinValue, bool bHasMaxValue, float flMaxValue );

	CRD_GameRulesConVarCollection &m_Collection;
	const int m_Index;
	ConVar m_Var;

	friend class CRD_GameRulesConVarCollection;

	static void OnChanged( IConVar *pVar, const char *szOldValue, float flOldValue );
};

class CRD_GameRulesConVarFloat : public CRD_GameRulesConVar
{
public:
	CRD_GameRulesConVarFloat( CRD_GameRulesConVarCollection &collection, int index, const char *szName, const char *szDefaultValue, const char *szHelpString, bool bHasMinValue, float flMinValue, bool bHasMaxValue, float flMaxValue );

	float GetFloat();
};


class CRD_GameRulesConVarBool : public CRD_GameRulesConVar
{
public:
	CRD_GameRulesConVarBool( CRD_GameRulesConVarCollection &collection, int index, const char *szName, const char *szDefaultValue, const char *szHelpString );

	bool GetBool();
};
