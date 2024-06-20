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
