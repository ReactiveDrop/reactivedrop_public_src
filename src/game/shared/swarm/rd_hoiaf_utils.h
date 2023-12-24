#pragma once

#include "steam/isteamhttp.h"

class CRD_HoIAF_System : public CAutoGameSystemPerFrame
{
public:
	CRD_HoIAF_System();

	void PostInit() override;
#ifdef CLIENT_DLL
	void Update( float frametime ) override;
#else
	void PreClientUpdate() override;
#endif

	bool CheckIAFIntelUpToDate();
	bool IsRankedServerIP( uint32_t ip ) const;

private:
	void ParseIAFIntel();
	void LoadCachedIAFIntel();
	bool RefreshIAFIntel( bool bOnlyIfExpired = true, bool bForceNewRequest = false );
	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );
	void OnRequestFailed();

	KeyValues::AutoDelete m_pIAFIntel;
	HTTPRequestHandle m_hIAFIntelRefreshRequest{ INVALID_HTTPREQUEST_HANDLE };
	CCallResult<CRD_HoIAF_System, HTTPRequestCompleted_t> m_IAFIntelRefresh;
	int m_iExponentialBackoff{};
	uint32_t m_iBackoffUntil{};

	int64_t m_iExpireAt{};
	CUtlVector<uint32_t> m_RankedServerIPs;
};

CRD_HoIAF_System *HoIAF();
