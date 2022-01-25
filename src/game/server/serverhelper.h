#ifndef SERVERHELPER_H
#define SERVERHELPER_H

#ifdef _WIN32
#pragma once
#endif

#define INTERFACEVERSION_SERVERHELPERDLL "ServerHelper001"

class IServerHelper {
public:
	virtual unsigned int	GetDataMapOffsetForEdict( edict_t* pEdict, const char* className, const char* property ) = 0;
};

class CServerHelper : public IServerHelper {
public:
	unsigned int	GetDataMapOffsetForEdict( edict_t* pEdict, const char* className, const char* property );
};

#endif