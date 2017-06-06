#include "cbase.h"
#include "rd_workshop_preview_image.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "jpeglib/jpeglib.h"

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
		m_SavedBuffer.Put( buf.Base(), buf.TellMaxPut() );
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

// mostly copied from optionssubmultiplayer.cpp:

struct RdValveJpegErrorHandler_t
{
	// The default manager
	struct jpeg_error_mgr	m_Base;
	// For handling any errors
	jmp_buf					m_ErrorContext;
};

static void ValveJpegErrorHandler( j_common_ptr cinfo )
{
	RdValveJpegErrorHandler_t *pError = reinterpret_cast< RdValveJpegErrorHandler_t * >( cinfo->err );

	char buffer[ JMSG_LENGTH_MAX ];

	/* Create the message */
	( *cinfo->err->format_message )( cinfo, buffer );

	Warning( "%s\n", buffer );

	// Bail
	longjmp( pError->m_ErrorContext, 1 );
}

struct RdJpegSource_t
{
	RdJpegSource_t( const CUtlBuffer & buf ) : m_Buf( buf.Base(), buf.TellMaxPut(), CUtlBuffer::READ_ONLY )
	{
		m_Base.init_source = &InitSource;
		m_Base.fill_input_buffer = &FillInputBuffer;
		m_Base.skip_input_data = &SkipInputData;
		m_Base.resync_to_restart = &ResyncToRestart;
		m_Base.term_source = &TermSource;
	}

	// The default manager
	struct jpeg_source_mgr	m_Base;

	CUtlBuffer m_Buf;

	static RdJpegSource_t *Get( j_decompress_ptr cinfo )
	{
		return reinterpret_cast<RdJpegSource_t *>( cinfo->src );
	}

	static void InitSource( j_decompress_ptr cinfo )
	{
		RdJpegSource_t *pSource = Get( cinfo );
		pSource->m_Base.next_input_byte = reinterpret_cast<const JOCTET *>( pSource->m_Buf.Base() );
		pSource->m_Base.bytes_in_buffer = pSource->m_Buf.Size();
	}

	static boolean FillInputBuffer( j_decompress_ptr cinfo )
	{
		return FALSE;
	}

	static void SkipInputData( j_decompress_ptr cinfo, long num_bytes )
	{
		RdJpegSource_t *pSource = Get( cinfo );
		Assert( num_bytes < (long)pSource->m_Base.bytes_in_buffer );
		pSource->m_Base.bytes_in_buffer -= num_bytes;
		pSource->m_Base.next_input_byte += num_bytes;
	}

	static boolean ResyncToRestart( j_decompress_ptr cinfo, int desired )
	{
		return jpeg_resync_to_restart( cinfo, desired );
	}

	static void TermSource( j_decompress_ptr cinfo )
	{
		// no cleanup needed
	}
};

bool CReactiveDropWorkshopPreviewImage::TryParseJPEG( const CUtlBuffer & buf )
{
	jpeg_decompress_struct jpegInfo;
	RdValveJpegErrorHandler_t jerr;
	jpegInfo.err = jpeg_std_error( &jerr.m_Base );
	jpegInfo.err->error_exit = &ValveJpegErrorHandler;

	JSAMPROW row_pointer[1];
	int row_stride;
	int cur_row = 0;

	// image attributes
	int image_height;
	int image_width;

	jpeg_create_decompress( &jpegInfo );

	RdJpegSource_t jsrc( buf );
	jpegInfo.src = &jsrc.m_Base;

	if ( setjmp( jerr.m_ErrorContext ) )
	{
		DevWarning( "Parsing image as JPEG failed: an error should be shown above this message\n" );
		// Get here if there is any error
		jpeg_destroy_decompress( &jpegInfo );
		return false;
	}

	if ( jpeg_read_header( &jpegInfo, 1 ) != JPEG_HEADER_OK )
	{
		DevWarning( "Parsing image as JPEG failed: jpeg_read_header failed\n" );
		jpeg_destroy_decompress( &jpegInfo );
		return false;
	}
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

	InitFromRGBA( rgba, image_width, image_height );
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
