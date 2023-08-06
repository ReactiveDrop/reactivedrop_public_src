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

int UTIL_RD_PingLobby( CSteamID lobby );

struct RD_Lobby_Scoreboard_Entry_t
{
	char Name[k_cchPersonaNameMax];
	int Score;
	float Connected;
};
void UTIL_RD_ReadLobbyScoreboard( CSteamID lobby, CUtlVector<RD_Lobby_Scoreboard_Entry_t> &scoreboard );

#ifdef CLIENT_DLL
// without __single_inheritance, if this file is included in anything vgui-related, this class becomes 12 bytes larger and breaks everything
class __single_inheritance CReactiveDropLobbySearch
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

	void Clear();
	void WantUpdatedLobbyList( bool bForce = false );
	CUtlVector<CSteamID> m_MatchingLobbies;

private:
	void UpdateSearch();
	CCallResult<CReactiveDropLobbySearch, LobbyMatchList_t> m_LobbyMatchListResult;
	void LobbyMatchListResult( LobbyMatchList_t *pResult, bool bIOFailure );

	const char *m_pszDebugName;
	float m_flNextUpdate;
};

class CReactiveDropServerListHelper : public ISteamMatchmakingServerListResponse
{
public:
	~CReactiveDropServerListHelper();

	enum Mode_t
	{
		MODE_INTERNET,
		MODE_LAN,
		MODE_FRIENDS,
		MODE_FAVORITES,
		MODE_HISTORY,
		MODE_SPECTATOR,
	};
	Mode_t m_eMode{ MODE_INTERNET };

	CUtlVectorAutoPurge< MatchMakingKeyValuePair_t * > m_Filters;

	void WantUpdatedServerList();
	float m_flSoonestServerListRequest{};

	int Count() const;
	gameserveritem_t *GetDetails( int iServer ) const;
	bool IsHoIAFServer( int iServer ) const;
	bool IsVACSecure( int iServer ) const;
	bool HasPassword( int iServer ) const;
	int GetPing( int iServer ) const;
	const char *GetName( int iServer ) const;
	CSteamID GetSteamID( int iServer ) const;

	// ISteamMatchmakingServerListResponse implementation
	void ServerResponded( HServerListRequest hRequest, int iServer ) override;
	void ServerFailedToRespond( HServerListRequest hRequest, int iServer ) override;
	void RefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response ) override;
	HServerListRequest m_hServerListRequest{};
	HServerListRequest m_hServerListRequestNext{};
};

class CReactiveDropServerList
{
public:
	CReactiveDropServerList();

	bool IsHoIAFServerIP( uint32_t ip );
	CUtlVector<uint32_t> m_ParticipatingServers;

	CReactiveDropLobbySearch m_PublicLobbies;

	CReactiveDropServerListHelper m_PublicServers;   // servers with players or HoIAF servers
	CReactiveDropServerListHelper m_InternetServers; // unfiltered public servers
	CReactiveDropServerListHelper m_FavoriteServers; // servers the player has bookmarked
	CReactiveDropServerListHelper m_LANServers;      // servers on the local network
};

extern CReactiveDropServerList g_ReactiveDropServerList;
#endif // CLIENT_DLL

#endif // RD_LOBBY_UTILS_H__
