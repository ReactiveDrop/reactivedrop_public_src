#pragma once

class ASW_Alien_Class_Entry
{
public:
	ASW_Alien_Class_Entry( const char *szClass, int nHullType, Vector vecRealHullMins, Vector vecRealHullMaxs, const char *szHordeSound = "Spawner.Horde" );

	const char *m_pszAlienClass;
	mutable string_t m_iszAlienClass;
	int m_nHullType;
	Vector m_vecRealHullMins;
	Vector m_vecRealHullMaxs;
	const char *m_szHordeSound;
};

#ifdef RD_7A_ENEMIES
extern const ASW_Alien_Class_Entry g_Aliens[38];
#else
extern const ASW_Alien_Class_Entry g_Aliens[31];
#endif
extern const ASW_Alien_Class_Entry g_NonSpawnableAliens[5];

int GetAlienClassIndex( CBaseEntity *pAlien );

const ASW_Alien_Class_Entry *GetAlienClass( int index );
const char *GetAlienClassname( int index );

inline const char *GetAlienClassname( CBaseEntity *pAlien )
{
	return GetAlienClassname( GetAlienClassIndex( pAlien ) );
}

extern const int g_nDroneClassEntry;
extern const int g_nDroneJumperClassEntry;
