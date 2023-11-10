#pragma once

namespace zbalermorna
{
	void PaintText( float x0, float y0, const char *szText, float flTextSize, const Color &color = Color{ 255, 255, 255, 255 } );
	void MeasureText( const char *szText, float flTextSize, float &flWide, float &flTall, bool bLastLineWide = false );
}
