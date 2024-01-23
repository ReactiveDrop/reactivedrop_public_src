#pragma once

#include "jpeglib/jpeglib.h"
#include <setjmp.h>

// This struct and the code surrounding it was originally in optionssubmultiplayer.cpp.
// It was copied to vaddons.cpp for Alien Swarm, and then to rd_workshop_preview_image.cpp
// for Alien Swarm: Reactive Drop, and now it's only in one place instead of three or more.
struct ValveJpegErrorHandler_t : public jpeg_error_mgr
{
	// NOTE: Once setjmp is called on this context, no C++ objects with destructors can come into scope while jpeg functions are able to be called.
	// Their destructors will not run if an error occurs, which leads to, best case, a memory leak.
	jmp_buf m_ErrorContext;

	void InitCompress( struct jpeg_compress_struct *jpegInfo );
	void InitDecompress( struct jpeg_decompress_struct *jpegInfo );
};
