#pragma once

struct HUDGlowHelper_t
{
	uint16_t m_iGlow{};
	uint16_t m_iSustain{};

	uint8_t Update( bool bShouldGlow );
	uint8_t Get() const;
};
