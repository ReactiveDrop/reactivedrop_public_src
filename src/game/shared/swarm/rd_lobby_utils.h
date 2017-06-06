#ifndef RD_LOBBY_UTILS_H__
#define RD_LOBBY_UTILS_H__

#ifdef _WIN32
#pragma once
#endif


#include "steam/steam_api.h"

CSteamID UTIL_RD_GetCurrentLobbyID();
#ifdef CLIENT_DLL
void UTIL_RD_JoinByLobbyID( CSteamID lobby );
#endif
bool UTIL_RD_IsLobbyOwner();
KeyValues *UTIL_RD_LobbyToLegacyKeyValues( CSteamID lobby );
const char *UTIL_RD_GetCurrentLobbyData( const char *pszKey, const char *pszDefaultValue = "" );
void UTIL_RD_UpdateCurrentLobbyData( const char *pszKey, const char *pszValue );
void UTIL_RD_UpdateCurrentLobbyData( const char *pszKey, int iValue );
void UTIL_RD_UpdateCurrentLobbyData( const char *pszKey, uint64 iValue );
void UTIL_RD_RemoveCurrentLobbyData( const char *pszKey );

#ifdef CLIENT_DLL
class CReactiveDropLobbySearch
{
public:
	CReactiveDropLobbySearch( const char *pszDebugName = "unnamed lobby search" );
	~CReactiveDropLobbySearch();

	struct StringFilter_t
	{
		StringFilter_t( const char *pszName, const char *pszValue, ELobbyComparison compareMode = k_ELobbyComparisonEqual ) :
			m_Name( pszName ),
			m_Value( pszValue ),
			m_Compare( compareMode )
		{
		}

		CUtlString m_Name;
		CUtlString m_Value;
		ELobbyComparison m_Compare;

		bool operator==( const StringFilter_t & other )
		{
			return other.m_Name == m_Name && other.m_Value == m_Value && other.m_Compare == m_Compare;
		}
		bool operator!=( const StringFilter_t & other )
		{
			return other.m_Name != m_Name || other.m_Value != m_Value || other.m_Compare != m_Compare;
		}
	};
	CUtlVector<StringFilter_t> m_StringFilters;

	struct NumericalFilter_t
	{
		NumericalFilter_t( const char *pszName, int iValue, ELobbyComparison compareMode = k_ELobbyComparisonEqual ) :
			m_Name( pszName ),
			m_Value( iValue ),
			m_Compare( compareMode )
		{
		}

		CUtlString m_Name;
		int m_Value;
		ELobbyComparison m_Compare;

		bool operator==( const NumericalFilter_t & other )
		{
			return other.m_Name == m_Name && other.m_Value == m_Value && other.m_Compare == m_Compare;
		}
		bool operator!=( const NumericalFilter_t & other )
		{
			return other.m_Name != m_Name || other.m_Value != m_Value || other.m_Compare != m_Compare;
		}
	};
	CUtlVector<NumericalFilter_t> m_NumericalFilters;

	ELobbyDistanceFilter m_DistanceFilter;

	typedef void (*func_t)( const CUtlVector<CSteamID> & lobbies );

	void Clear();
	void StartSearching( bool bForceNow = false );
	void StopSearching();
	void Subscribe( func_t );
	void Unsubscribe( func_t );

private:
	friend class CReactiveDropLobbySearchSystem;
	void Update();
	void UpdateSearch();
	CCallResult<CReactiveDropLobbySearch, LobbyMatchList_t> m_LobbyMatchListResult;
	void LobbyMatchListResult( LobbyMatchList_t *pResult, bool bIOFailure );

	const char *m_pszDebugName;
	float m_flNextUpdate;
	CUtlVector<func_t> m_Subscribers;
};
#endif // CLIENT_DLL

#endif // RD_LOBBY_UTILS_H__
