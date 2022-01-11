#pragma once

class IServerHelper {
public:
	virtual unsigned int	GetDataMapOffsetForEdict(edict_t* pEdict, const char* className, const char* property) = 0;
};

class CServerHelper : public IServerHelper {
public:
	unsigned int	GetDataMapOffsetForEdict(edict_t* pEdict, const char* className, const char* property);
};