#include "cbase.h"
#include "rd_image_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void ValveJpegErrorHandler( j_common_ptr cinfo )
{
	ValveJpegErrorHandler_t *pError = static_cast< ValveJpegErrorHandler_t * >( cinfo->err );

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	( *cinfo->err->format_message )( cinfo, buffer );

	Warning( "jpeg error: %s\n", buffer );

	// Bail
	longjmp( pError->m_ErrorContext, 1 );
}

void ValveJpegErrorHandler_t::InitCompress( struct jpeg_compress_struct *jpegInfo )
{
	jpegInfo->err = jpeg_std_error( this );
	jpegInfo->err->error_exit = &ValveJpegErrorHandler;
	jpeg_create_compress( jpegInfo );
}
void ValveJpegErrorHandler_t::InitDecompress( struct jpeg_decompress_struct *jpegInfo )
{
	jpegInfo->err = jpeg_std_error( this );
	jpegInfo->err->error_exit = &ValveJpegErrorHandler;
	jpeg_create_decompress( jpegInfo );
}
