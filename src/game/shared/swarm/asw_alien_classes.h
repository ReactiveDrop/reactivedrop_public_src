#pragma once

class ASW_Alien_Class_Entry
{
public:
	ASW_Alien_Class_Entry( const char *szClass, int nHullType );

	const char *m_pszAlienClass;
	string_t m_iszAlienClass;
	int m_nHullType;
};

extern ASW_Alien_Class_Entry g_Aliens[23];
int GetAlienClassIndex( CBaseEntity *pAlien );
extern const int g_nDroneClassEntry;
extern const int g_nDroneJumperClassEntry;
