#pragma once

class IConsistency : public IAppSystem
{
public:
	virtual void ConnectClient( CreateInterfaceFn clientFactory ) = 0;
	virtual void ConnectServer( CreateInterfaceFn gameFactory ) = 0;
	virtual uint32_t GetGameVersion() = 0;
};

#define INTERFACEVERSION_ICONSISTENCY_V4 "Interface_Consistency_004"
