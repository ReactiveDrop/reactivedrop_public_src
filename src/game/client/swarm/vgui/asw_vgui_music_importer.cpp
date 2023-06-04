#include "cbase.h"
#include "asw_vgui_music_importer.h"
#include "filesystem.h"
#include "strtools.h"
#include "fmtstr.h"
#include "c_asw_jukebox.h"
#include "gamestringpool.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static int ReadID3FrameSize( int nVersion, const byte **ppBuffer, unsigned int unBufferSize )
{
	int nBytesToRead = nVersion < 3 ? 3 : 4;
	int nBitsToShift = nVersion < 4 ? 8 : 7;

	// Read the ridiculous tag size format
	int nTagSize = 0;
	for ( int i = 0; i < nBytesToRead; ++i )
	{
		nTagSize |= *( *ppBuffer + nBytesToRead - i - 1 ) << nBitsToShift * i;
	}

	// Advance the buffer pointer
	*ppBuffer += nBytesToRead;

	return nTagSize;
}

bool ID3Info_t::Deserialize( const byte *pBuffer, unsigned int unBufferSize )
{
	if ( !pBuffer )
		return false;

	// Check for supported ID3v2 format
	const char *pHeader = ( const char * )pBuffer;
	if ( V_strncmp( "ID3", pHeader, 3 ) )
		return false;

	unsigned int unMajorVersion = *( pHeader + 3 );
	if ( unMajorVersion < 2 || unMajorVersion>4 )
		return false;

	// Read the ridiculous tag size format
	int nTagSize = 0;
	for ( int i = 0; i < 3; ++i )
	{
		nTagSize |= *( pHeader + 9 - i ) << 7 * i;
	}

	if ( nTagSize <= 0 )
		return false;

	// Read in each frame
	const byte *pCurr = ( const byte * )pHeader + 10;
	while ( pCurr - pBuffer < nTagSize )
	{
		const int nLabelSize = unMajorVersion < 3 ? 3 : 4;

		// If we're into padding, break
		if ( *pCurr == 0 )
			break;

		// Read the frame header
		const char *szLabel = ( const char * )pCurr;
		pCurr += nLabelSize;
		int nFrameSize = ReadID3FrameSize( unMajorVersion, &pCurr, unBufferSize );

		// Skip the flags bytes (1 for V3 or less, 2 otherwise)
		if ( unMajorVersion >= 3 )
			pCurr += 2;

		// Check for a text frame
		if ( *szLabel == 'T' )
		{
			// Only care about unicode (the first byte in a text frame, all versions)
			if ( *pCurr == 0 )
			{
				if ( !V_strncmp( szLabel, "TP1", 3 ) || !V_strncmp( szLabel, "TOLY", 4 ) || !V_strncmp( szLabel, "TOPE", 4 ) || !V_strncmp( szLabel, "TPE1", 4 ) || !V_strncmp( szLabel, "TPE2", 4 ) )
				{
					// Read the artist
					V_strncpy( szArtistName, ( const char * )( pCurr + 1 ), MIN( nFrameSize + 1, sizeof( szArtistName ) ) );
				}
				else if ( !V_strncmp( szLabel, "TT2", 3 ) || !V_strncmp( szLabel, "TIT2", 4 ) )
				{
					// Read the title
					V_strncpy( szTrackName, ( const char * )( pCurr + 1 ), MIN( nFrameSize + 1, sizeof( szTrackName ) ) );
				}
				else if ( !V_strncmp( szLabel, "TAL", 3 ) )
				{
					// Read the album
					V_strncpy( szAlbumName, ( const char * )( pCurr + 1 ), MIN( nFrameSize + 1, sizeof( szAlbumName ) ) );
				}
			}

		}

		// Advance to the next frame
		pCurr += nFrameSize;
		Assert( pCurr < pBuffer + unBufferSize );
	}

	return true;
}

MusicImporterDialog::MusicImporterDialog( Panel *parent, const char *title, vgui::FileOpenDialogType_t type, KeyValues *pContextKeyValues /*= 0 */ )
: FileOpenDialog(parent, title, type, pContextKeyValues )
{
	AddActionSignalTarget( this );
}

MusicImporterDialog::~MusicImporterDialog()
{
}

void MusicImporterDialog::OnMusicItemSelected( KeyValues *pInfo )
{
	if ( !pInfo )
		return;

	// Parse the keyvalues
	const char *szDirectory = pInfo->GetString( "activedirectory" );
	KeyValues *pSelections = pInfo->FindKey( "selectedfiles" );
	if ( !pSelections || !szDirectory )
		return;

	for ( KeyValues *pSub = pSelections->GetFirstSubKey(); pSub; pSub = pSub->GetNextKey() )
	{
		const char *szAttributes = pSub->GetString( "attributes" );
		if ( !szAttributes )
			continue;

		const char *szFilename = CFmtStr( "%s%s", szDirectory, pSub->GetString( "text" ) );
		//char szOutFilename[MAX_PATH];

		// If the item selected was a directory, search it recursively for music
		if ( strstr( szAttributes, "D" ) )
		{
			ImportAllMusicInDirectory( szFilename );
			continue;
		}

		ImportMusic( pSub->GetString( "text" ), szDirectory );
	}
	PostActionSignal( new KeyValues( "MusicImportComplete" ) );
}

void MusicImporterDialog::ImportMusic( const char *szSrcFilename, const char *szDirectory )
{
	char fn[512];
	V_snprintf( fn, sizeof( fn ), "%s%s", szDirectory, szSrcFilename );

	// Read the whole file.
	CUtlBuffer buf;
	if ( !g_pFullFileSystem->ReadFile( fn, NULL, buf ) )
		return;

	CRC32_t crc = CRC32_ProcessSingleBuffer( buf.Base(), buf.Size() );

	char szHexName[9];
	V_binarytohex( ( const byte * )&crc, sizeof( crc ), szHexName, sizeof( szHexName ) );

	char szDestFileName[512];
	V_snprintf( szDestFileName, sizeof( szDestFileName ), "sound/music/_mp3/%s.mp3", szHexName );
	V_FixSlashes( szDestFileName );

	// Make a local copy
	g_pFullFileSystem->CreateDirHierarchy( "sound/music/_mp3", "MOD" );
	g_pFullFileSystem->WriteFile( szDestFileName, "MOD", buf );

	// Read ID3 header info
	ID3Info_t id3Header;
	id3Header.Deserialize( ( const byte * )buf.Base(), buf.Size() );

	const char *szTrackName = id3Header.szTrackName;
	if ( szTrackName[0] == '\0' )
		szTrackName = szSrcFilename;

	wchar_t wszTrackName[256], wszAlbumName[256], wszArtistName[256];
	V_UTF8ToUnicode( szTrackName, wszTrackName, sizeof( wszTrackName ) );
	V_UTF8ToUnicode( id3Header.szAlbumName, wszAlbumName, sizeof( wszAlbumName ) );
	V_UTF8ToUnicode( id3Header.szArtistName, wszArtistName, sizeof( wszArtistName ) );

	g_ASWJukebox.AddMusicToPlaylist( wszTrackName, szHexName, wszAlbumName, wszArtistName );
}

void MusicImporterDialog::ImportAllMusicInDirectory( const char *szDirectory )
{
	// Check to make sure the specified directory exists
	if ( !g_pFullFileSystem->IsDirectory( szDirectory ) )
		return;

	// Search the directory structure.
	const char *musicwildcard = CFmtStr( "%s%s", szDirectory, "/*.mp3" );

	FileFindHandle_t findHandle;
	for ( char const *findfn = g_pFullFileSystem->FindFirst( musicwildcard, &findHandle ); findfn; findfn = g_pFullFileSystem->FindNext( findHandle ) )
	{
		ImportMusic( findfn, szDirectory );
	}

	g_pFullFileSystem->FindClose( findHandle );
}

vgui::DHANDLE<MusicImporterDialog> g_hMusicImportDialog;

void MusicImporterDialog::OpenImportDialog( Panel *pParent )
{
	if ( g_hMusicImportDialog.Get() == NULL )
	{
		g_hMusicImportDialog = new MusicImporterDialog( NULL, "#asw_music_import_dialog", vgui::FOD_OPEN_MULTIPLE, NULL );
		g_hMusicImportDialog->AddFilter( "*.mp3", "#asw_music_types", true );
	}

	if ( pParent )
		g_hMusicImportDialog->AddActionSignalTarget( pParent );

	g_hMusicImportDialog->DoModal( false );
	g_hMusicImportDialog->Activate();
}

void MusicImporterDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetAlpha( 255 );
}
