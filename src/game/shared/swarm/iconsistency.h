#pragma once

class IConsistency : public IAppSystem
{
public:
	virtual void ConnectClient( CreateInterfaceFn clientFactory ) = 0;
	virtual void ConnectServer( CreateInterfaceFn gameFactory ) = 0;
};

#define INTERFACEVERSION_ICONSISTENCY_V2 "Interface_Consistency_002"
