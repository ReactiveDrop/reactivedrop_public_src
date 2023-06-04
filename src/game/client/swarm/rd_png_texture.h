#pragma once

#include <vgui_controls/Image.h>

class CRD_PNG_Texture : public vgui::IImage
{
public:
	~CRD_PNG_Texture();

	void Paint() override;
	void SetPos( int x, int y ) override;
	void GetContentSize( int &wide, int &tall ) override;
	void GetSize( int &wide, int &tall ) override;
	void SetSize( int wide, int tall ) override;
	void SetColor( Color col ) override;
	bool Evict() override;
	int GetNumFrames() override;
	void SetFrame( int nFrame ) override;
	vgui::HTexture GetID() override;
	void SetRotation( int iRotation ) override;

protected:
	// Init should be called in the constructor by derived classes.
	//
	// If Init returns false, the caller should fetch the PNG data and call either OnPNGDataReady or OnFailedToLoadData.
	bool Init( const char *szDirectory, uint32_t iHash );

	void OnPNGDataReady( const unsigned char *pData, size_t nDataSize, const char *szIconDebugName = "icon" );
	void OnFailedToLoadData( const char *szReason, const char *szIconDebugName = "icon" );

private:
	vgui::HTexture m_iTextureID{ NULL };
	Color m_Color{ 255, 255, 255, 255 };
	int m_nX{ 0 }, m_nY{ 0 };
	int m_nWide{ 512 }, m_nTall{ 512 };
	bool m_bReady{ false };

	char m_szFileNameVMT[MAX_PATH]{};
	char m_szFileNameVTF[MAX_PATH]{};
};
