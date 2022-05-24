#pragma once

class CUtlString;

class CRD_Text_Filtering : CAutoGameSystem
{
public:
	CRD_Text_Filtering();

	virtual void PostInit();

	// Unknown context
	void FilterTextUnknown( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextUnknown( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextUnknown( CUtlString & szText, CSteamID sourceSteamID = CSteamID() );
	// Game content, only legally required filtering is performed
	void FilterTextGameContent( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextGameContent( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextGameContent( CUtlString & szText, CSteamID sourceSteamID = CSteamID() );
	// Chat from another player
	void FilterTextChat( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextChat( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextChat( CUtlString & szText, CSteamID sourceSteamID = CSteamID() );
	// Character or item name
	void FilterTextName( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextName( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID = CSteamID() );
	void FilterTextName( CUtlString & szText, CSteamID sourceSteamID = CSteamID() );

	template<size_t N>
	inline void FilterTextUnknown( wchar_t( &wszText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextUnknown( wszText, N * sizeof( wchar_t ), sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextUnknown( char( &szText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextUnknown( szText, N, sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextGameContent( wchar_t( &wszText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextGameContent( wszText, N * sizeof( wchar_t ), sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextGameContent( char( &szText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextGameContent( szText, N, sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextChat( wchar_t( &wszText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextChat( wszText, N * sizeof( wchar_t ), sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextChat( char( &szText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextChat( szText, N, sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextName( wchar_t( &wszText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextName( wszText, N * sizeof( wchar_t ), sourceSteamID );
	}
	template<size_t N>
	inline void FilterTextName( char( &szText )[N], CSteamID sourceSteamID = CSteamID() )
	{
		FilterTextName( szText, N, sourceSteamID );
	}
};

extern CRD_Text_Filtering g_RDTextFiltering;
