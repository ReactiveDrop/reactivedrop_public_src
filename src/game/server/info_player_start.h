#pragma once

class CBaseStart : public CPointEntity
{
public:
	DECLARE_CLASS( CBaseStart, CPointEntity );

	CBaseStart();
	virtual bool KeyValue( const char* szKeyName, const char* szValue );

	DECLARE_DATADESC();

	int m_nMarineProfile;
	bool m_bUsed;
};