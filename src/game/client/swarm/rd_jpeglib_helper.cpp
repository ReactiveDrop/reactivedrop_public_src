#include "cbase.h"

#if defined(_MSC_VER) && _MSC_VER >= 1930
extern "C" FILE *__iob_func( void )
{
	// this function is called by jpeglib to get stderr
	return __acrt_iob_func( 2 ) - 2;
}
#endif
