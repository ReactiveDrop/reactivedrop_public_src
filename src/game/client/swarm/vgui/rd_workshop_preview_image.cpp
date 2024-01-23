#include "cbase.h"
#include "rd_workshop_preview_image.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "rd_image_utils.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

CReactiveDropWorkshopPreviewImage::CReactiveDropWorkshopPreviewImage( const CUtlBuffer & buf )
{
	m_iTextureID = -1;
	m_nX = 0;
	m_nY = 0;
	m_flWideScale = 1;
	m_flTallScale = 1;

	if ( TryParseImage( buf ) )
	{
		m_SavedBuffer.Put( buf.Base(), buf.TellPut() );
	}
}

CReactiveDropWorkshopPreviewImage::~CReactiveDropWorkshopPreviewImage()
{
	if ( m_iTextureID != -1 && vgui::surface() )
	{
		vgui::surface()->DestroyTextureID( m_iTextureID );
	}
}

void CReactiveDropWorkshopPreviewImage::Paint()
{
	if ( m_iTextureID == -1 )
	{
		return;
	}

	CheckTextureID();

	vgui::surface()->DrawSetColor( m_Color );
	vgui::surface()->DrawSetTexture( m_iTextureID );
	vgui::surface()->DrawTexturedSubRect( m_nX, m_nY, m_nX + m_nWide, m_nY + m_nTall, 0, 0, m_flWideScale, m_flTallScale );
}

void CReactiveDropWorkshopPreviewImage::InitFromRGBA( const byte *rgba, int width, int height )
{
	if ( m_iTextureID != -1 )
	{
		vgui::surface()->DestroyTextureID( m_iTextureID );
	}
	int nWide = SmallestPowerOfTwoGreaterOrEqual( width );
	int nTall = SmallestPowerOfTwoGreaterOrEqual( height );
	m_iTextureID = vgui::surface()->CreateNewTextureID( true );
	vgui::surface()->DrawSetTextureRGBA( m_iTextureID, rgba, nWide, nTall );
	m_flWideScale = float( width ) / float( nWide );
	m_flTallScale = float( height ) / float( nTall );
	m_nWide = width;
	m_nTall = height;
	m_Color = Color( 255, 255, 255, 255 );
}

bool CReactiveDropWorkshopPreviewImage::TryParseImage( const CUtlBuffer & buf )
{
	if ( TryParseJPEG( buf ) )
	{
		return true;
	}

	// TODO: more image formats
	return false;
}

bool CReactiveDropWorkshopPreviewImage::TryParseJPEG( const CUtlBuffer & buf )
{
	jpeg_decompress_struct jpegInfo;
	ValveJpegErrorHandler_t jerr;

	JSAMPROW row_pointer[1];
	int row_stride;
	int cur_row = 0;

	// image attributes
	int image_height;
	int image_width;

	jerr.InitDecompress( &jpegInfo );

	if ( setjmp( jerr.m_ErrorContext ) )
	{
		DevWarning( "Parsing image as JPEG failed: an error should be shown above this message\n" );
		// Get here if there is any error
		jpeg_destroy_decompress( &jpegInfo );
		return false;
	}

	jpeg_mem_src( &jpegInfo, ( const unsigned char * )buf.Base(), buf.TellPut() );

	if ( jpeg_read_header( &jpegInfo, 1 ) != JPEG_HEADER_OK )
	{
		DevWarning( "Parsing image as JPEG failed: jpeg_read_header failed\n" );
		jpeg_destroy_decompress( &jpegInfo );
		return false;
	}

	jpegInfo.scale_num = 1;

	if ( jpegInfo.image_width >= 2048 || jpegInfo.image_height >= 2048 )
	{
		jpegInfo.scale_denom = 8;
	}
	else if ( jpegInfo.image_width >= 1024 || jpegInfo.image_height >= 1024 )
	{
		jpegInfo.scale_denom = 4;
	}
	else if ( jpegInfo.image_width >= 512 || jpegInfo.image_height >= 512 )
	{
		jpegInfo.scale_denom = 2;
	}
	else
	{
		jpegInfo.scale_denom = 1;
	}

	jpeg_calc_output_dimensions( &jpegInfo );

	if ( jpeg_start_decompress( &jpegInfo ) != 1 )
	{
		DevWarning( "Parsing image as JPEG failed: jpeg_start_decompress failed\n" );
		jpeg_destroy_decompress( &jpegInfo );
		return false;
	}
	// now that we've started the decompress with the jpeg lib, we have the attributes of the
	// image ready to be read out of the decompress struct.
	row_stride = SmallestPowerOfTwoGreaterOrEqual( jpegInfo.output_width ) * 4;
	image_height = jpegInfo.image_height;
	image_width = jpegInfo.image_width;
	int mem_required = SmallestPowerOfTwoGreaterOrEqual( image_height ) * row_stride;

	// allocate the memory to read the image data into.
	unsigned char *rgba = new unsigned char[mem_required]();

	// read in all the scan lines of the image into our image data buffer.
	bool working = true;
	while ( working && ( jpegInfo.output_scanline < jpegInfo.output_height ) )
	{
		row_pointer[0] = &(rgba[cur_row * row_stride]);
		if ( jpeg_read_scanlines( &jpegInfo, row_pointer, 1 ) != 1 )
		{
			DevWarning( "Parsing image as JPEG failed: jpeg_read_scanlines failed for row %d\n", cur_row );
			working = false;
		}
		for ( int x0 = jpegInfo.output_width * 4 - 4, x1 = ( jpegInfo.output_width - 1 ) * jpegInfo.output_components; x0 >= 0; x0 -= 4, x1 -= jpegInfo.output_components )
		{
			memcpy( row_pointer[0] + x0, row_pointer[0] + x1, jpegInfo.output_components );
			memset( row_pointer[0] + x0 + jpegInfo.output_components, 255, 4 - jpegInfo.output_components );
		}
		++cur_row;
	}

	if ( !working )
	{
		delete[] rgba;
		jpeg_destroy_decompress( &jpegInfo );
		return false;
	}

	jpeg_finish_decompress( &jpegInfo );

	jpeg_destroy_decompress( &jpegInfo );

	InitFromRGBA( rgba, jpegInfo.output_width, jpegInfo.output_height );
	delete[] rgba;

	return true;
}

void CReactiveDropWorkshopPreviewImage::CheckTextureID()
{
	if ( m_iTextureID == -1 )
	{
		return;
	}

	if ( vgui::surface()->IsTextureIDValid( m_iTextureID ) )
	{
		return;
	}

	DevMsg( "Workshop preview image texture ID %d is invalid. Re-parsing image.\n", m_iTextureID );

	if ( !TryParseImage( m_SavedBuffer ) )
	{
		Assert( !"Workshop preview icon texture re-parsing failed unexpectedly" );
	}
}
