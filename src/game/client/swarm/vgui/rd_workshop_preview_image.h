#ifndef RD_WORKSHOP_PREVIEW_IMAGE_H__
#define RD_WORKSHOP_PREVIEW_IMAGE_H__

#ifdef _WIN32
#pragma once
#endif

#include <utlobjectreference.h>
#include <utlbuffer.h>
#include <vgui/IImage.h>

class CReactiveDropWorkshopPreviewImage : public vgui::IImage
{
public:
	CReactiveDropWorkshopPreviewImage( const CUtlBuffer & buf );
	virtual ~CReactiveDropWorkshopPreviewImage();

	// Call to Paint the image
	// Image will draw within the current panel context at the specified position
	virtual void Paint();

	// Set the position of the image
	virtual void SetPos( int x, int y ) { m_nX = x; m_nY = y; }

	// Gets the size of the content
	virtual void GetContentSize( int &wide, int &tall ) { wide = m_nWide; tall = m_nTall; }

	// Get the size the image will actually draw in (usually defaults to the content size)
	virtual void GetSize( int &wide, int &tall ) { wide = m_nWide; tall = m_nTall; }

	// Sets the size of the image
	virtual void SetSize( int wide, int tall ) { m_nWide = wide; m_nTall = tall; }

	// Set the draw color 
	virtual void SetColor( Color col ) { m_Color = col; }

	// not for general purpose use
	// evicts the underlying image from memory if refcounts permit, otherwise ignored
	// returns true if eviction occurred, otherwise false
	virtual bool Evict() { return false; }

	virtual int GetNumFrames() { return 1; }
	virtual void SetFrame( int nFrame ) {}
	virtual vgui::HTexture GetID() { CheckTextureID(); return m_iTextureID; }

	virtual void SetRotation( int iRotation ) {}

private:
	Color m_Color;
	int m_iTextureID;
	int m_nX, m_nY, m_nWide, m_nTall;
	float m_flWideScale, m_flTallScale;
	void InitFromRGBA( const byte *rgba, int width, int height );
	bool TryParseImage( const CUtlBuffer & buf );
	bool TryParseJPEG( const CUtlBuffer & buf );
	void CheckTextureID();
	CUtlBuffer m_SavedBuffer;

	DECLARE_REFERENCED_CLASS( CReactiveDropWorkshopPreviewImage );
};

#endif // RD_WORKSHOP_PREVIEW_IMAGE_H__
