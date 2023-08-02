#pragma once
#include "cbase.h"
#include "rd_lock.h"
#include "util.h"
#include <Windows.h>

void CreateAndLockExclusive()
{
	static bool locked;

	if ( locked ) return;
	
	// check if there isn't another instance running from this folder
	// server instances need to be exclusive, and will interfere with each other
	// resulting in weird behavior like ugc content (workshop items) not being updated

	ConMsg("Creating exclusive lock on game folder\n");

	char szDir[MAX_PATH];
	UTIL_GetModDir( szDir, sizeof(szDir) );

	char szLockfile[MAX_PATH];
	V_ComposeFileName( szDir, "lockfile", szLockfile, sizeof(szLockfile) );

	CreateFile( szLockfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL );
	uint iError = GetLastError();

	if ( iError > 0 && iError != ERROR_ALREADY_EXISTS ) 
	{
		Error( "Cannot create lockfile (%d), is there another instance running?\n", iError );
	}
	
	locked = true;
}

