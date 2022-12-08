#ifndef _INCLUDED_IASW_RANDOM_MISSIONS_H
#define _INCLUDED_IASW_RANDOM_MISSIONS_H
#ifdef _WIN32
#pragma once
#endif

#include "iasw_mission_chooser.h"

namespace vgui
{
	class Panel;
};

class Vector;
class KeyValues;
class IASW_Encounter;

class IASW_Room_Details
{
public:
	// tags
	virtual bool			HasTag( const char *szTag ) = 0;
	virtual int				GetNumTags() = 0;
	virtual const char*		GetTag( int i ) = 0;
	virtual int				GetSpawnWeight() = 0;
	virtual int				GetNumExits() = 0;
	virtual IASW_Room_Details*	GetAdjacentRoom( int nExit ) = 0;
	virtual bool			GetThumbnailName( char* szOut, int iBufferSize ) = 0;
	virtual bool			GetFullRoomName( char* szOut, int iBufferSize ) = 0;
	virtual void			GetSoundscape( char* szOut, int iBufferSize ) = 0;
	virtual void			GetTheme( char* szOut, int iBufferSize ) = 0;
	virtual const Vector&	GetAmbientLight() = 0;
	virtual bool			HasAlienEncounter() = 0;
	virtual int				GetRoomIndex() const = 0;

	// location
	virtual void			GetWorldBounds( Vector *vecWorldMins, Vector *vecWorldMaxs ) = 0;
	virtual const Vector&	WorldSpaceCenter() = 0;
};

class IASW_Random_Missions
{
public:
	virtual vgui::Panel* CreateTileGenFrame( vgui::Panel *parent ) = 0;

	virtual void LevelInitPostEntity( const char *pszMapName ) = 0;
	virtual bool ValidMapLayout() = 0;
	virtual IASW_Room_Details* GetRoomDetails( const Vector &vecPos ) = 0;
	virtual IASW_Room_Details* GetRoomDetails( int iRoomIndex ) = 0;
	virtual IASW_Room_Details* GetStartRoomDetails() = 0;
	virtual int GetNumRooms() = 0;
	virtual void GetMapBounds( Vector *vecWorldMins, Vector *vecWorldMaxs ) = 0;
	virtual KeyValues* GetGenerationOptions() = 0;		// returns the generation options for the currently loaded random map
	virtual int				GetNumEncounters() = 0;
	virtual IASW_Encounter* GetEncounter( int i ) = 0;

	virtual bool CheckAndCleanDirtyLayout( void ) = 0;
};

#endif // _INCLUDED_IASW_RANDOM_MISSIONS_H
