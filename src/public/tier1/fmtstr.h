//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A simple class for performing safe and in-expression sprintf-style
//			string formatting
//
// $NoKeywords: $
//=============================================================================//

#ifndef FMTSTR_H
#define FMTSTR_H

#include <stdarg.h>
#include <stdio.h>
#include "tier0/platform.h"
#include "tier1/strtools.h"

#if defined( _WIN32 )
#pragma once
#endif

//=============================================================================

// using macro to be compatable with GCC
#define FmtStrVSNPrintf( szBuf, nBufSize, ppszFormat, lastArg ) \
	do \
	{ \
		int     result; \
		va_list arg_ptr; \
	\
		va_start(arg_ptr, lastArg); \
		result = Q_vsnprintf((szBuf), (nBufSize)-1, (*(ppszFormat)), arg_ptr); \
		va_end(arg_ptr); \
	\
		(szBuf)[(nBufSize)-1] = 0; \
	} \
	while (0)

//-----------------------------------------------------------------------------
//
// Purpose: String formatter with specified size
//

template <int SIZE_BUF>
class CFmtStrN
{
public:
	CFmtStrN()									
	{ 
		m_szBuf[0] = 0;
	}
	
	// Standard C formatting
	CFmtStrN(const char *pszFormat, ...) FMTFUNCTION( 2, 3 )
	{
		FmtStrVSNPrintf( m_szBuf, SIZE_BUF, &pszFormat, pszFormat );
	}

	// Use this for pass-through formatting
	CFmtStrN( const char **ppszFormat, va_list params )
	{
		V_vsnprintf( m_szBuf, SIZE_BUF, *ppszFormat, params );
		m_szBuf[SIZE_BUF - 1] = 0;
	}

	// Explicit reformat
	const char *sprintf(const char *pszFormat, ...)	FMTFUNCTION( 2, 3 )
	{
		FmtStrVSNPrintf( m_szBuf, SIZE_BUF, &pszFormat, pszFormat );
		return m_szBuf;
	}

	// Use this for pass-through formatting
	void VSprintf( const char **ppszFormat, va_list params )
	{
		V_vsnprintf( m_szBuf, SIZE_BUF, *ppszFormat, params );
		m_szBuf[SIZE_BUF - 1] = 0;
	}

	// Use for access
	operator const char *() const				{ return m_szBuf; }
	char *Access()								{ return m_szBuf; }
	CFmtStrN<SIZE_BUF> & operator=( const char *pchValue ) { sprintf( pchValue ); return *this; }
	CFmtStrN<SIZE_BUF> & operator+=( const char *pchValue ) { Append( pchValue ); return *this; }
	int Length() const { return V_strlen( m_szBuf ); }

	void Clear()								{ m_szBuf[0] = 0; }

	void AppendFormat( const char *pchFormat, ... ) 
	{ 
		int nLength = Length(); 
		char *pchEnd = m_szBuf + nLength; 
		FmtStrVSNPrintf( pchEnd, SIZE_BUF - nLength, &pchFormat, pchFormat ); 
	}

	void AppendFormatV( const char *pchFormat, va_list args );
	void Append( const char *pchValue )
	{
		// This function is close to the metal to cut down on the CPU cost
		// of the previous incantation of Append which was implemented as
		// AppendFormat( pchValue ). This implementation, though not
		// as easy to read, instead does a strcpy from the existing end
		// point of the CFmtStrN. This brings something like a 10-20x speedup
		// in my rudimentary tests. It isn't using V_strncpy because that
		// function doesn't return the number of characters copied, which
		// we need to adjust nLength. Doing the V_strncpy with a V_strlen
		// afterwards took twice as long as this implementations in tests,
		// so V_strncpy's implementation was used to write this method.
		int nLength = Length();
		char *pDest = m_szBuf + nLength;
		const int maxLen = SIZE_BUF - nLength;
		char *pLast = pDest + maxLen - 1;
		while ((pDest < pLast) && (*pchValue != 0))
		{
			*pDest = *pchValue;
			++pDest; ++pchValue;
		}
		*pDest = 0;
	}

	//optimized version of append for just adding a single character
	void Append(char ch)
	{
		int nLength = Length();
		if (nLength < SIZE_BUF - 1)
		{
			m_szBuf[nLength] = ch;
			m_szBuf[nLength+1] = '\0';
		}
	}

	void AppendIndent( uint32 unCount, char chIndent = '\t' );
private:
	char m_szBuf[SIZE_BUF];
};


template< int SIZE_BUF >
void CFmtStrN<SIZE_BUF>::AppendIndent( uint32 unCount, char chIndent )
{
	int nLength = Length();
	if( nLength + unCount >= SIZE_BUF )
		unCount = SIZE_BUF - (1+nLength);
	for ( uint32 x = 0; x < unCount; x++ )
	{
		m_szBuf[ nLength++ ] = chIndent;
	}
	m_szBuf[ nLength ] = '\0';
}

template< int SIZE_BUF >
void CFmtStrN<SIZE_BUF>::AppendFormatV( const char *pchFormat, va_list args )
{
	int nLength = Length();
	V_vsnprintf( m_szBuf+nLength, SIZE_BUF - nLength, pchFormat, args );
}


//-----------------------------------------------------------------------------
//
// Purpose: Default-sized string formatter
//

#define FMTSTR_STD_LEN 256

typedef CFmtStrN<FMTSTR_STD_LEN> CFmtStr;
typedef CFmtStrN<1024> CFmtStr1024;
typedef CFmtStrN<8192> CFmtStrMax;

//=============================================================================

#endif // FMTSTR_H
