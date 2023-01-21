#pragma once

class ASW_Alien_Class_Entry
{
public:
	ASW_Alien_Class_Entry( const char *szClass, int nHullType, Vector vecRealHullMins, Vector vecRealHullMaxs );

	const char *m_pszAlienClass;
	mutable string_t m_iszAlienClass;
	int m_nHullType;
	Vector m_vecRealHullMins;
	Vector m_vecRealHullMaxs;
};

extern const ASW_Alien_Class_Entry g_Aliens[31];
int GetAlienClassIndex( CBaseEntity *pAlien );
extern const int g_nDroneClassEntry;
extern const int g_nDroneJumperClassEntry;
