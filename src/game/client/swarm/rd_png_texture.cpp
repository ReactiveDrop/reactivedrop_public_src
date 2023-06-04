#include "cbase.h"
#include "rd_png_texture.h"
#include "filesystem.h"
#include "lodepng.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PNG_TEXTURE_VERSION 1

CRD_PNG_Texture::~CRD_PNG_Texture()
{
	if ( m_iTextureID && vgui::surface() )
	{
		vgui::surface()->DestroyTextureID( m_iTextureID );
		m_iTextureID = NULL;
	}
}

void CRD_PNG_Texture::Paint()
{
	if ( !m_iTextureID )
	{
		return;
	}

	vgui::surface()->DrawSetColor( m_Color );
	vgui::surface()->DrawSetTexture( m_iTextureID );
	vgui::surface()->DrawTexturedRect( m_nX, m_nY, m_nX + m_nWide, m_nY + m_nTall );
}

void CRD_PNG_Texture::SetPos( int x, int y )
{
	m_nX = x;
	m_nY = y;
}

void CRD_PNG_Texture::GetContentSize( int &wide, int &tall )
{
	wide = m_nWide;
	tall = m_nTall;
}

void CRD_PNG_Texture::GetSize( int &wide, int &tall )
{
	wide = m_nWide;
	tall = m_nTall;
}

void CRD_PNG_Texture::SetSize( int wide, int tall )
{
	m_nWide = wide;
	m_nTall = tall;
}

void CRD_PNG_Texture::SetColor( Color col )
{
	m_Color = col;
}

bool CRD_PNG_Texture::Evict()
{
	return false;
}

int CRD_PNG_Texture::GetNumFrames()
{
	return m_bReady ? 1 : 0;
}

void CRD_PNG_Texture::SetFrame( int nFrame )
{
}

vgui::HTexture CRD_PNG_Texture::GetID()
{
	return m_iTextureID;
}

void CRD_PNG_Texture::SetRotation( int iRotation )
{
}

bool CRD_PNG_Texture::Init( const char *szDirectory, uint32_t iHash )
{
	Assert( !m_iTextureID );
	Assert( !m_bReady );

	m_DownloadTimer.Start();

	V_snprintf( m_szFileNameVMT, sizeof( m_szFileNameVMT ), "materials/%s/%08x.vmt", szDirectory, iHash );
	V_snprintf( m_szFileNameVTF, sizeof( m_szFileNameVTF ), "materials/%s/%08x.vtf", szDirectory, iHash );

	if ( !g_pFullFileSystem->FileExists( m_szFileNameVMT, "GAME" ) || !g_pFullFileSystem->FileExists( m_szFileNameVTF, "GAME" ) )
		return false;

	CUtlBuffer buf{ 0, 0, CUtlBuffer::TEXT_BUFFER };
	if ( !g_pFullFileSystem->ReadFile( m_szFileNameVMT, "GAME", buf ) )
		return false;

	buf.SeekGet( CUtlBuffer::SEEK_HEAD, strlen( "// version " ) );
	if ( buf.GetInt() == PNG_TEXTURE_VERSION ) // current version number
	{
		m_iTextureID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_iTextureID, m_szFileNameVMT + V_strlen( "materials/" ), true, false );
		vgui::surface()->DrawGetTextureSize( m_iTextureID, m_nWide, m_nTall );
		m_bReady = true;
		m_DownloadTimer.End();

		return true;
	}

	return false;
}

static void SmearAlpha( uint8_t *rgba, int wide, int tall )
{
	uint16_t *pDistanceToColor = new uint16_t[wide * tall];
	bool bAnyOpaque = false, bAnyTransparent = false;

	// first, we set each pixel to either 0 (for pixels with color information) or 1 (for pixels with an alpha value of 0).
	for ( int y = 0; y < tall; y++ )
	{
		for ( int x = 0; x < wide; x++ )
		{
			int i = x + y * wide;

			if ( rgba[i * 4 + 3] == 0 )
			{
				pDistanceToColor[i] = 1;
				bAnyTransparent = true;
			}
			else
			{
				pDistanceToColor[i] = 0;
				bAnyOpaque = true;
			}
		}
	}

	// if we don't have at least one 0 and at least one 1, we can't do anything.
	if ( !bAnyTransparent || !bAnyOpaque )
	{
		delete[] pDistanceToColor;
		return;
	}

	// iteratively convert 1 pixels to instead have the manhattan distance to the nearest 0 pixel.
	bool bKeepGoing = true;
	uint16_t iMaxDistance = 0;
	while ( bKeepGoing )
	{
		iMaxDistance++;
		bKeepGoing = false;

		for ( int y = 0; y < tall; y++ )
		{
			for ( int x = 0; x < wide; x++ )
			{
				int i = x + y * wide;
				uint16_t iDist = pDistanceToColor[i];
				if ( iDist != iMaxDistance )
					continue;

				uint16_t iLeft = x <= 0 ? iDist : pDistanceToColor[i - 1];
				uint16_t iRight = x >= wide - 1 ? iDist : pDistanceToColor[i + 1];
				uint16_t iUp = y <= 0 ? iDist : pDistanceToColor[i - wide];
				uint16_t iDown = y >= tall - 1 ? iDist : pDistanceToColor[i + wide];

				uint16_t iMinNeighbor = MIN( MIN( iLeft, iRight ), MIN( iUp, iDown ) );
				if ( iDist < iMinNeighbor + 1 )
				{
					pDistanceToColor[i]++;
					bKeepGoing = true;
				}
			}
		}
	}

	// loop through and make fully transparent pixels inherit the color of their neighbors.
	for ( uint16_t iCurrentDistance = 1; iCurrentDistance <= iMaxDistance; iCurrentDistance++ )
	{
		for ( int y = 0; y < tall; y++ )
		{
			for ( int x = 0; x < wide; x++ )
			{
				int i = x + y * wide;
				if ( pDistanceToColor[i] != iCurrentDistance )
					continue;

				int r = 0, g = 0, b = 0, iCount = 0;
				if ( x > 0 && pDistanceToColor[i - 1] < iCurrentDistance )
				{
					// left
					r += rgba[( i - 1 ) * 4 + 0];
					g += rgba[( i - 1 ) * 4 + 1];
					b += rgba[( i - 1 ) * 4 + 2];
					iCount++;
				}
				if ( x < wide - 1 && pDistanceToColor[i + 1] < iCurrentDistance )
				{
					// right
					r += rgba[( i + 1 ) * 4 + 0];
					g += rgba[( i + 1 ) * 4 + 1];
					b += rgba[( i + 1 ) * 4 + 2];
					iCount++;
				}
				if ( y > 0 && pDistanceToColor[i - wide] < iCurrentDistance )
				{
					// up
					r += rgba[( i - wide ) * 4 + 0];
					g += rgba[( i - wide ) * 4 + 1];
					b += rgba[( i - wide ) * 4 + 2];
					iCount++;
				}
				if ( y < tall - 1 && pDistanceToColor[i + wide] < iCurrentDistance )
				{
					// down
					r += rgba[( i + wide ) * 4 + 0];
					g += rgba[( i + wide ) * 4 + 1];
					b += rgba[( i + wide ) * 4 + 2];
					iCount++;
				}
				if ( x > 0 && y > 0 && pDistanceToColor[i - wide - 1] < iCurrentDistance )
				{
					// up left
					r += rgba[( i - wide - 1 ) * 4 + 0];
					g += rgba[( i - wide - 1 ) * 4 + 1];
					b += rgba[( i - wide - 1 ) * 4 + 2];
					iCount++;
				}
				if ( x < wide - 1 && y > 0 && pDistanceToColor[i - wide + 1] < iCurrentDistance )
				{
					// up right
					r += rgba[( i - wide + 1 ) * 4 + 0];
					g += rgba[( i - wide + 1 ) * 4 + 1];
					b += rgba[( i - wide + 1 ) * 4 + 2];
					iCount++;
				}
				if ( x > 0 && y < tall - 1 && pDistanceToColor[i + wide - 1] < iCurrentDistance )
				{
					// down left
					r += rgba[( i + wide - 1 ) * 4 + 0];
					g += rgba[( i + wide - 1 ) * 4 + 1];
					b += rgba[( i + wide - 1 ) * 4 + 2];
					iCount++;
				}
				if ( x < wide - 1 && y < tall - 1 && pDistanceToColor[i + wide + 1] < iCurrentDistance )
				{
					// down right
					r += rgba[( i + wide + 1 ) * 4 + 0];
					g += rgba[( i + wide + 1 ) * 4 + 1];
					b += rgba[( i + wide + 1 ) * 4 + 2];
					iCount++;
				}

				Assert( iCount > 0 );
				Assert( rgba[i * 4 + 3] == 0 );

				rgba[i * 4 + 0] = r / iCount;
				rgba[i * 4 + 1] = g / iCount;
				rgba[i * 4 + 2] = b / iCount;
			}
		}
	}

	delete[] pDistanceToColor;
}

void CRD_PNG_Texture::OnPNGDataReady( const void *pData, size_t nDataSize, const char *szIconDebugName )
{
	Assert( !m_iTextureID );
	Assert( !m_bReady );

	m_DownloadTimer.End();

	CFastTimer timer;
	timer.Start();

	uint8_t *rgba = NULL;
	unsigned error = lodepng_decode32( &rgba, ( unsigned * )&m_nWide, ( unsigned * )&m_nTall, ( const unsigned char * )pData, nDataSize );
	if ( error )
	{
		Warning( "Decoding %s: lodepng error %d: %s\n", szIconDebugName, error, lodepng_error_text( error ) );
	}

	SmearAlpha( rgba, m_nWide, m_nTall );

	IVTFTexture *pVTF = CreateVTFTexture();
	pVTF->Init( m_nWide, m_nTall, 1, IMAGE_FORMAT_RGBA8888, TEXTUREFLAGS_EIGHTBITALPHA | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 1 );
	if ( rgba )
		V_memcpy( pVTF->ImageData(), rgba, m_nWide * m_nTall * 4 );
	free( rgba );

	pVTF->ConvertImageFormat( IMAGE_FORMAT_DEFAULT, false );
	VtfProcessingOptions opt = { sizeof( opt ), VtfProcessingOptions::OPT_FILTER_NICE };
	pVTF->SetPostProcessingSettings( &opt );
	pVTF->PostProcess( false );
	pVTF->ConvertImageFormat( IMAGE_FORMAT_DXT5, false );

	CUtlBuffer buf;
	pVTF->Serialize( buf );
	DestroyVTFTexture( pVTF );

	char szStrippedName[MAX_PATH];
	V_strncpy( szStrippedName, m_szFileNameVMT, sizeof( szStrippedName ) );
	V_StripFilename( szStrippedName );
	g_pFullFileSystem->CreateDirHierarchy( szStrippedName, "MOD" );

	g_pFullFileSystem->WriteFile( m_szFileNameVTF, "MOD", buf );

	buf.Clear();
	buf.SetBufferType( true, true );
	buf.PutString( "// version " );
	buf.PutInt( PNG_TEXTURE_VERSION );
	buf.PutString( "\nUnlitGeneric {\n$basetexture ");
	V_StripExtension( m_szFileNameVTF + V_strlen( "materials/" ), szStrippedName, sizeof( szStrippedName ) );
	buf.PutString( szStrippedName );
	buf.PutString( "\n$translucent 1\n$vertexcolor 1\n$vertexalpha $1\n$ignorez 1\n}\n" );

	g_pFullFileSystem->WriteFile( m_szFileNameVMT, "MOD", buf );

	m_iTextureID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iTextureID, szStrippedName, true, false );
	m_bReady = true;

	timer.End();

	Msg( "Recovered from cache miss for PNG texture %s (%s) in %lf+%lf seconds.\n", m_szFileNameVMT, szIconDebugName, m_DownloadTimer.GetDuration().GetSeconds(), timer.GetDuration().GetSeconds() );
}

void CRD_PNG_Texture::OnFailedToLoadData( const char *szReason, const char *szIconDebugName )
{
	Assert( !m_iTextureID );
	Assert( !m_bReady );

	Warning( "Failed to load %s: %s\n", szIconDebugName, szReason );
	m_bReady = true;
}
