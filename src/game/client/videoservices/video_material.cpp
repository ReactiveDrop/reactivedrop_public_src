//===========================================================================//
//
// Purpose: Webm capable video_services replacement
// Written by: Noodles
// Utilising Błażej Szczygieł's libsimplewebm
//
//===========================================================================//


#include "cbase.h"

#include "video_material.h"
#include "video_services.h"
#include "materialsystem/imaterial.h"
#include "filesystem.h"
#include "tier0/platform.h"
#include "tier1/KeyValues.h"
#include "tier1/utlbuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// about a second
#define BUFFER_SIZE 196608
// at minimim accomdate about an update every 125ms
#define BUFFER_FILLED_MIN 4096 * 16
#define FREEZE_TIME 0.125


//=============================================================================
// 
// Video texture regenerator
// 
// Notes: Bik shader only supports YUV420, I might eventually support more 
//			video colour formats but I will need to write the shaders
// 
//=============================================================================

template <YUVChannel_t Channel>
void CYUVTextureRegenerator<Channel>::RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
{
	unsigned char *imageData = pVTFTexture->ImageData();
	int rowSize = pVTFTexture->RowSizeInBytes( 0 );

	if ( m_decodedImage && m_decodedImage->chromaShiftW == 1 && m_decodedImage->chromaShiftH == 1 )
	{
		unsigned char *pixels = m_decodedImage->planes[ Channel ];
		int lineSize = m_decodedImage->linesize[ Channel ];
		for ( int y = 0; y < m_videoHeight; ++y )
		{
			Q_memcpy( imageData, pixels, m_videoWidth );
			imageData += rowSize;
			pixels += lineSize;
		}
		m_decodedImage = nullptr;
	}

}

//=============================================================================
// 
// Video material
// 
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: a paranoid amount of initialisation
//-----------------------------------------------------------------------------
CVideoMaterial::CVideoMaterial()
{
	m_mkvReader = nullptr;
	m_demuxer = nullptr;
	m_videoDecoder = nullptr;
	m_audioDecoder = nullptr;
	m_audioFrame = new WebMFrame();
	m_image = new VPXDecoder::Image();
	m_pcm = nullptr;

	m_videoWidth = 0;
	m_videoHeight = 0;
	m_textureWidth = 0;
	m_textureHeight = 0;

	m_videoReady = false;
	m_videoPlaying = false;
	m_videoStarted = false;
	m_videoLooping = false;
	m_videoStopped = false;
	m_videoEnded = false;

	m_soundKilled = false;

	m_videoPath[ 0 ] = '\0';

	m_volume = 1.0f;
	m_videoTime = 0.0;
	m_curTime = 0.0;
	m_prevTicks = 0;

	m_currentFrame = 0;

	m_yTextureRegen = nullptr;
	m_crTextureRegen = nullptr;
	m_cbTextureRegen = nullptr;

	m_nAudioBufferWriteOffset = 0;
	m_nAudioBufferReadOffset = 0;

	m_nAudioBufferSize = 0;
	m_nBytesPerSample = 0;
	m_nAudioBufferFilledSize = 0;

	m_pAudioDevice = nullptr;
	m_pAudioBuffer = nullptr;
#ifdef _WIN32
	m_directSoundNotify = nullptr;
	m_endEventHandle = nullptr;
	m_halfwayEventHandle = nullptr;
	m_videoOverEventHandle = nullptr;
#endif
}

CVideoMaterial::~CVideoMaterial()
{
	m_videoEnded = true;
	m_videoStopped = true;
	m_videoFrames.Purge();

	DestroySoundBuffer();

	// Often the same video material is used over and over, so unless you completely rid of it issues arise. 
	// I don't know how much of this is necessary anymore now that it actually dies, but better safe than sorry

	if ( m_crTexture.IsValid() )
	{
		m_crTexture->SetTextureRegenerator( nullptr );
		m_crTexture.Shutdown( true );
	}
	if ( m_cbTexture.IsValid() )
	{
		m_cbTexture->SetTextureRegenerator( nullptr );
		m_cbTexture.Shutdown( true );
	}
	if ( m_yTexture.IsValid() )
	{
		m_yTexture->SetTextureRegenerator( nullptr );
		m_yTexture.Shutdown( true );
	}

	//delete m_crTexture;
	//delete m_cbTexture;
	//delete m_yTexture;

	delete m_yTextureRegen;
	delete m_crTextureRegen;
	delete m_cbTextureRegen;

	IMaterial *material = m_videoMaterial;
	m_videoMaterial.Shutdown();
	// Removes any material that might reference the video texture
	if ( materials )
		materials->UncacheUnusedMaterials();

	// kill it if it remains
	if ( material )
		material->DeleteIfUnreferenced();

#ifdef _WIN32
	if ( m_endEventHandle )
		CloseHandle( m_endEventHandle );
	if ( m_halfwayEventHandle )
		CloseHandle( m_halfwayEventHandle );
	if ( m_videoOverEventHandle )
		CloseHandle( m_videoOverEventHandle );
#endif

	delete m_pcm;
	delete m_image;
	delete m_audioDecoder;
	delete m_videoDecoder;
	delete m_demuxer;
	delete m_mkvReader;

	delete m_audioFrame;
	delete m_pAudioBuffer;
}

bool CVideoMaterial::LoadVideo( const char *pMaterialName, const char *pVideoFileName, void *pSoundDevice )
{
	Q_strncpy( m_videoPath, pVideoFileName, sizeof( m_videoPath ) );
	m_mkvReader = new MkvReader( m_videoPath );
	if ( !m_mkvReader )
		return false;

	m_demuxer = new WebMDemuxer( m_mkvReader );
	if ( !m_demuxer || !m_demuxer->isOpen() )
	{
		if ( m_demuxer )
		{
			delete m_mkvReader;
			delete m_demuxer;
			m_mkvReader = nullptr;
			m_demuxer = nullptr;
		}
		return false;
	}

	// assign the decoder a reasonable number of threads
	const CPUInformation &cpuInfo = GetCPUInformation();
	unsigned int numthreads = clamp( cpuInfo.m_nLogicalProcessors - 2, 1, 8 );
	m_videoDecoder = new VPXDecoder( *m_demuxer, numthreads );
	m_audioDecoder = new OpusVorbisDecoder( *m_demuxer );
	m_pcm = m_audioDecoder->isOpen() ? new short[ m_audioDecoder->getBufferSamples() * m_demuxer->getChannels() ] : NULL;
	m_videoWidth = m_demuxer->getWidth();
	m_videoHeight = m_demuxer->getHeight();
	// This is a guessed framerate from the first 50 frames 
	m_frameRate.SetFPS( m_demuxer->getFrameRate() );

	CreateSoundBuffer( pSoundDevice );
	CreateVideoMaterial( pMaterialName );
	return true;
}

bool CVideoMaterial::CreateSoundBuffer( void *pSoundDevice )
{
	if ( !m_audioDecoder->isOpen() )
		return false;

	if ( !pSoundDevice )
	{
		DevMsg( "No sound device!\n" );
		return false;
	}

#ifdef _WIN32
	m_pAudioDevice = ( IDirectSound * )pSoundDevice;

	WAVEFORMATEX waveFormat;
	Q_memset( &waveFormat, 0, sizeof( WAVEFORMATEX ) );
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = m_demuxer->getChannels();
	waveFormat.nSamplesPerSec = m_demuxer->getSampleRate();
	waveFormat.wBitsPerSample = 16; // S16
	waveFormat.nBlockAlign = ( waveFormat.nChannels * waveFormat.wBitsPerSample ) / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	DSBUFFERDESC dsbd;
	Q_memset( &dsbd, 0, sizeof( DSBUFFERDESC ) );
	dsbd.dwSize = sizeof( DSBUFFERDESC );
	dsbd.dwBufferBytes = BUFFER_SIZE;
	dsbd.lpwfxFormat = &waveFormat;

	// if we have the losefocus cvar determine if we want to remove the global focus flag
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS;
	ConVarRef snd_mute_losefocus( "snd_mute_losefocus" );
	if ( snd_mute_losefocus.GetBool() )
		dsbd.dwFlags = dsbd.dwFlags & ~( DSBCAPS_GLOBALFOCUS );

	m_nAudioBufferSize = BUFFER_SIZE;
	m_nBytesPerSample = waveFormat.nBlockAlign;

	IDirectSoundBuffer *tempBuffer = nullptr;
	if ( FAILED( IDirectSound_CreateSoundBuffer( m_pAudioDevice, &dsbd, &tempBuffer, NULL ) ) )
		return false;

	if ( FAILED( IDirectSoundBuffer_QueryInterface( tempBuffer, IID_IDirectSoundBuffer, ( LPVOID * )&m_pAudioBuffer ) ) )
		return false;

	m_pAudioBuffer->AddRef();
	tempBuffer->Release();

	if ( FAILED( IDirectSoundBuffer_QueryInterface( m_pAudioBuffer, IID_IDirectSoundNotify, ( LPVOID * )&m_directSoundNotify ) ) )
		return false;

	DSBPOSITIONNOTIFY posNotify[ 2 ];
	Q_memset( &posNotify, 0, sizeof( posNotify ) );

	// create nofitication
	m_endEventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_halfwayEventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_videoOverEventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );

	// notifcations at end and halfway mark
	posNotify[ 0 ].dwOffset = BUFFER_SIZE - 1;
	posNotify[ 0 ].hEventNotify = m_endEventHandle;
	posNotify[ 1 ].dwOffset = ( BUFFER_SIZE / 2.0f ) - 1;
	posNotify[ 1 ].hEventNotify = m_halfwayEventHandle;

	IDirectSoundNotify_SetNotificationPositions( m_directSoundNotify, 2, posNotify );

	m_hBufferThreadHandle = CreateSimpleThread( HandleBufferUpdates, this );
#endif
	m_soundKilled = false;
	return true;
}

void CVideoMaterial::CreateVideoMaterial( const char *pMaterialName )
{
	// ---------------------------
	// texture
	char ytexture[ MAX_PATH ];
	Q_snprintf( ytexture, MAX_PATH, "%s_y", pMaterialName );
	char crtexture[ MAX_PATH ];
	Q_snprintf( crtexture, MAX_PATH, "%s_cr", pMaterialName );
	char cbtexture[ MAX_PATH ];
	Q_snprintf( cbtexture, MAX_PATH, "%s_cb", pMaterialName );

	const int tex_flags = TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT |
		TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_SINGLECOPY;

	m_textureWidth = ALIGN_VALUE( m_videoWidth, 8 );
	m_textureHeight = ALIGN_VALUE( m_videoHeight, 8 );

	// create the textures
	m_yTexture.InitProceduralTexture( ytexture, "VideoCacheTextures", m_textureWidth, m_textureHeight, IMAGE_FORMAT_I8, tex_flags );
	// CB and CR are half the size of the Y (the brightness)
	m_cbTexture.InitProceduralTexture( cbtexture, "VideoCacheTextures", m_textureWidth >> 1, m_textureHeight >> 1, IMAGE_FORMAT_I8, tex_flags );
	m_crTexture.InitProceduralTexture( crtexture, "VideoCacheTextures", m_textureWidth >> 1, m_textureHeight >> 1, IMAGE_FORMAT_I8, tex_flags );

	m_yTextureRegen = new CYUVTextureRegenerator<YUVCHANNEL_Y>( m_textureWidth, m_textureHeight );
	m_cbTextureRegen = new CYUVTextureRegenerator<YUVCHANNEL_CB>( m_textureWidth >> 1, m_textureHeight >> 1 );
	m_crTextureRegen = new CYUVTextureRegenerator<YUVCHANNEL_CR>( m_textureWidth >> 1, m_textureHeight >> 1 );
	m_yTexture->SetTextureRegenerator( m_yTextureRegen );
	m_crTexture->SetTextureRegenerator( m_crTextureRegen );
	m_cbTexture->SetTextureRegenerator( m_cbTextureRegen );

	// update the procedural texture with the first frame of the video
	WebMFrame video_frame;
	VPXDecoder::Image image;
	while ( m_demuxer->readFrame( &video_frame, nullptr ) )
	{
		if ( !m_videoDecoder->decode( video_frame ) )
			continue;

		if ( m_videoDecoder->getImage( image ) == VPXDecoder::IMAGE_ERROR::NO_IMAGE_ERROR )
		{

			m_yTextureRegen->m_decodedImage = &image;
			m_crTextureRegen->m_decodedImage = &image;
			m_cbTextureRegen->m_decodedImage = &image;

			m_yTexture->Download();
			m_crTexture->Download();
			m_cbTexture->Download();

			break;
		}
	}
	m_demuxer->resetVideo();

	// ---------------------------
	// material
	// Use the Bik shader as it deals with YUV420
	KeyValues *pVMTKeyValues = new KeyValues( "Bik" );
	pVMTKeyValues->SetString( "$ytexture", ytexture );
	pVMTKeyValues->SetString( "$cbtexture", cbtexture );
	pVMTKeyValues->SetString( "$crtexture", crtexture );
	pVMTKeyValues->SetString( "$basetexture", m_yTexture->GetName() );
	pVMTKeyValues->SetInt( "$nobasetexture", 1 );
	pVMTKeyValues->SetInt( "$nolod", 1 );
	pVMTKeyValues->SetInt( "$nomip", 1 );
	pVMTKeyValues->SetInt( "$nofog", 1 );
	pVMTKeyValues->SetInt( "$translucent", 1 );
	pVMTKeyValues->SetInt( "$vertexcolor", 1 );
	pVMTKeyValues->SetInt( "$vertexalpha", 1 );
	pVMTKeyValues->SetInt( "$gammacolorread", 0 );
	pVMTKeyValues->SetInt( "$spriteorientation", 3 );

	m_videoMaterial.Init( pMaterialName, pVMTKeyValues );
	// Refresh the material vars because apparently init doesn't do this
	// and retains the previous video's frame
	m_videoMaterial->Refresh();

	m_videoReady = true;
	m_videoStarted = false;
}

const char *CVideoMaterial::GetVideoFileName()
{
	return m_videoPath;
}

VideoResult_t CVideoMaterial::GetLastResult()
{
	return VideoResult_t::SYSTEM_NOT_AVAILABLE;
}

VideoFrameRate_t &CVideoMaterial::GetVideoFrameRate()
{
	return m_frameRate;
}

// Video playback state functions
bool CVideoMaterial::IsVideoReadyToPlay()
{
	return m_videoReady;
}

bool CVideoMaterial::IsVideoPlaying()
{
	return m_videoPlaying;
}

bool CVideoMaterial::IsNewFrameReady()
{
	return false;
}

bool CVideoMaterial::IsFinishedPlaying()
{
	return m_videoEnded;
}

#if _WIN32
//-----------------------------------------------------------------------------
// Purpose: Threaded function that pauses the sound buffer if it hasn't been updated in a while
//-----------------------------------------------------------------------------
unsigned int CVideoMaterial::HandleBufferUpdates( void *params )
{
	CVideoMaterial *m = ( CVideoMaterial * )params;

	HANDLE hEvents[ 3 ];
	hEvents[ 0 ] = m->m_endEventHandle;
	hEvents[ 1 ] = m->m_halfwayEventHandle;
	hEvents[ 2 ] = m->m_videoOverEventHandle;
	while ( true )
	{
		DWORD result = WaitForMultipleObjects( 3, hEvents, FALSE, INFINITE );
		m->m_mutex.Lock();
		switch ( result )
		{
		case WAIT_OBJECT_0:
		case WAIT_OBJECT_0 + 1:
		{
			unsigned int curTicks = Plat_MSTime();
			double timepassed = ( double )( curTicks - m->m_prevTicks ) / 1000.0;
			if ( timepassed > FREEZE_TIME )
				IDirectSoundBuffer_Stop( m->m_pAudioBuffer );
			break;
		}
		case WAIT_OBJECT_0 + 2:
			m->m_mutex.Unlock();
			return 0;

		default:
			break;
		}
		m->m_mutex.Unlock();
	}

	return 0;
}
#endif

void CVideoMaterial::DestroySoundBuffer()
{
	if ( !m_pAudioBuffer )
		return;

#ifdef _WIN32

	if ( m_hBufferThreadHandle )
	{
		SetEvent( m_videoOverEventHandle );
		ThreadJoin( m_hBufferThreadHandle );
		ReleaseThreadHandle( m_hBufferThreadHandle );
	}

	if ( !m_soundKilled )
	{
		IDirectSound *pDSInterface = nullptr;
		IDirectSoundBuffer *pDSBufferInterface = nullptr;
		if ( SUCCEEDED( m_pAudioDevice->QueryInterface( IID_IDirectSound, ( void ** )&pDSInterface ) ) &&
			SUCCEEDED( m_pAudioBuffer->QueryInterface( IID_IDirectSoundBuffer, ( void ** )&pDSBufferInterface ) ) )
		{
			IDirectSoundBuffer_Stop( m_pAudioBuffer );
			IDirectSoundBuffer_Release( m_pAudioBuffer );
		}
		if ( pDSBufferInterface )
			pDSBufferInterface->Release();
		if ( pDSInterface )
			pDSInterface->Release();
	}
	m_pAudioBuffer = nullptr;
	m_soundKilled = true;
#endif
}

bool CVideoMaterial::StartVideo()
{
	if ( m_videoStarted )
		return true;

	m_videoStarted = true;
	m_videoPlaying = true;
	m_videoStopped = false;

	m_currentFrame = 0;
	m_videoTime = 0.0;
	m_curTime = 0.0;

	m_prevTicks = Plat_MSTime();

	return true;
}

bool CVideoMaterial::StopVideo()
{
	if ( !m_videoStarted )
		return true;

	m_videoStopped = true;
	m_videoStarted = false;
	DestroySoundBuffer();

	return true;
}

void CVideoMaterial::SetLooping( bool bLoopVideo )
{
	m_videoLooping = bLoopVideo;
}

bool CVideoMaterial::IsLooping()
{
	return m_videoLooping;
}

void CVideoMaterial::SetPaused( bool bPauseState )
{
	if ( m_videoStarted )
	{
		// Unpause
		if ( !m_videoPlaying && !bPauseState )
		{
#ifdef _WIN32
			if ( m_pAudioBuffer )
				IDirectSoundBuffer_Play( m_pAudioBuffer, 0, 0, DSBPLAY_LOOPING );
#endif
			m_prevTicks = Plat_MSTime();
		}
#ifdef _WIN32
		// Pause sound buffer
		else if ( m_videoPlaying && bPauseState )
		{
			if ( m_pAudioBuffer )
				IDirectSoundBuffer_Stop( m_pAudioBuffer );
		}
#endif
	}

	m_videoPlaying = !bPauseState;
}

bool CVideoMaterial::IsPaused()
{
	return !m_videoPlaying;
}

float CVideoMaterial::GetVideoDuration()
{
	if ( m_demuxer )
		return m_demuxer->getLength();
	return 0.0f;
}

int CVideoMaterial::GetFrameCount()
{
	return 0;
}

bool CVideoMaterial::SetFrame( int FrameNum )
{
	return false;
}

int	CVideoMaterial::GetCurrentFrame()
{
	return m_currentFrame;
}

bool CVideoMaterial::SetTime( float flTime )
{
	return false;
}

float CVideoMaterial::GetCurrentVideoTime()
{
	return 0.0f;
}

bool CVideoMaterial::NeedNewFrame( double curtime )
{
	if ( m_videoFrames.Count() == 0 )
		return true;

	if ( m_videoFrames.Tail()->time <= curtime )
		return true;

	if ( m_pAudioBuffer && m_nAudioBufferFilledSize < BUFFER_FILLED_MIN )
		return true;

	return false;
}

void CVideoMaterial::RestartVideo()
{
	m_currentFrame = 0;
	m_demuxer->resetVideo();
	m_curTime = m_videoTime = 0.0;
	m_prevTicks = Plat_MSTime();
#ifdef _WIN32
	if ( m_pAudioBuffer && !m_soundKilled )
		m_pAudioBuffer->SetCurrentPosition( 0 );
#endif
}

bool CVideoMaterial::Update()
{
	if ( !StartVideo() )
		return false;

	// the video has stopped, there is nothing more to do
	if ( m_videoStopped )
		return false;

	// we're not stopped, but we're paused
	if ( !m_videoPlaying )
		return true;

	// Update time
	unsigned int curTicks = Plat_MSTime();
	double timepassed = ( double )( curTicks - m_prevTicks ) / 1000.0;
	m_curTime += timepassed;
	m_prevTicks = curTicks;

#ifdef _WIN32
	if ( m_pAudioBuffer )
	{
		// kinda rough but it'll do
		m_nAudioBufferFilledSize -= m_demuxer->getSampleRate() * m_nBytesPerSample * timepassed;
		if ( m_nAudioBufferFilledSize < 0 )
			m_nAudioBufferFilledSize = 0;
	}
#endif

	if ( m_curTime < m_videoTime )
	{
		if ( m_pAudioBuffer )
		{
			if ( m_nAudioBufferFilledSize > BUFFER_FILLED_MIN )
				return true;
		}
		else
			return true;
	}

	// Has the stream ended?
	if ( m_demuxer->isEOS() )
	{
		// Noodles; this might be stupid
		if ( m_videoFrames.Count() == 0 )
		{
			if ( m_videoLooping )
			{
				RestartVideo();
			}
			else
			{
				m_videoEnded = true;
				StopVideo();
				return false;
			}
		}
	}

	// if we have audio check if we should update the buffer and make sure we're playing
	bool bNeedUpdate = false;
#ifdef _WIN32
	DWORD status = NULL;
	if ( !m_demuxer->isEOS() && m_pAudioBuffer )
	{
		m_pAudioBuffer->GetStatus( &status );
		if ( !( status & DSBSTATUS_PLAYING ) )
		{
			IDirectSoundBuffer_SetCurrentPosition( m_pAudioBuffer, m_nAudioBufferWriteOffset );
			IDirectSoundBuffer_Play( m_pAudioBuffer, 0, 0, DSBPLAY_LOOPING );
		}
	}
#endif
	// Read until we've filled the buffer or got enough frames
	while ( NeedNewFrame( m_curTime ) || bNeedUpdate )
	{
		WebMFrame *video_frame = new WebMFrame();
		// did we reach the EOS
		if ( !m_demuxer->readFrame( video_frame, m_audioFrame ) )
		{
			bNeedUpdate = false;
			delete video_frame;
			break;
		}
		else if ( !video_frame->isValid() )
		{
			delete video_frame;
		}
		else
		{
			m_videoFrames.Insert( video_frame );
		}

		bNeedUpdate = false;
		if ( m_audioFrame->isValid() && m_pAudioBuffer )
		{
			int numOutSamples = 0;

			m_audioDecoder->getPCMS16( *m_audioFrame, m_pcm, numOutSamples );
			if ( numOutSamples == 0 )
				continue;

			int nBytesRead = numOutSamples * m_nBytesPerSample;
#ifdef _WIN32
			int nPCMOverflowSize = 0;
			int nPCMOverflowOffset = 0;
			m_nAudioBufferFilledSize += nBytesRead;

			// can't fit the whole thing at the end so it needs to be split
			if ( ( m_nAudioBufferWriteOffset + nBytesRead ) >= m_nAudioBufferSize )
			{
				// save amount gone over
				nPCMOverflowSize = ( m_nAudioBufferWriteOffset + nBytesRead ) - m_nAudioBufferSize;
				nPCMOverflowOffset = nBytesRead - nPCMOverflowSize;
				nBytesRead -= nPCMOverflowSize;
				bNeedUpdate = true;
			}

			void *pAudioPtr = NULL;
			DWORD dwAudioBytes1;
			IDirectSoundBuffer_Lock( m_pAudioBuffer, m_nAudioBufferWriteOffset, nBytesRead, &pAudioPtr, &dwAudioBytes1, NULL, NULL, 0 );

			Q_memcpy( pAudioPtr, m_pcm, nBytesRead );
			m_nAudioBufferWriteOffset += nBytesRead;

			IDirectSoundBuffer_Unlock( m_pAudioBuffer, pAudioPtr, dwAudioBytes1, NULL, NULL );


			if ( m_nAudioBufferWriteOffset == m_nAudioBufferSize )
			{
				m_nAudioBufferWriteOffset = 0;
				IDirectSoundBuffer_Lock( m_pAudioBuffer, 0, nBytesRead, &pAudioPtr, &dwAudioBytes1, NULL, NULL, 0 );

				Q_memcpy( pAudioPtr, ( char * )( m_pcm )+nPCMOverflowOffset, nPCMOverflowSize );
				m_nAudioBufferWriteOffset += nPCMOverflowSize;

				IDirectSoundBuffer_Unlock( m_pAudioBuffer, pAudioPtr, dwAudioBytes1, NULL, NULL );
			}
#endif

			// if our timer is waayyy ahead set it back to the audio time
			if ( m_curTime > m_audioFrame->time )
			{
				m_curTime = m_audioFrame->time;
			}
		}
	}

	// roll back for videos with no audio
	if ( !m_demuxer->isEOS() && !m_audioDecoder->isOpen() )
	{
		// if our current time is out, roll it back
		// Noodles; I feel this will cause issues, but it seems fine right now
		double frameDur = 1.0 / m_frameRate.GetFPS();
		if ( m_videoFrames.Count() > 0 && ( m_curTime - m_videoFrames.Head()->time ) > ( frameDur * 6.0 ) )
		{
			m_curTime = m_videoTime - frameDur;
		}
	}

	while ( m_videoFrames.Count() > 0 && m_curTime >= m_videoTime )
	{
		if ( m_videoFrames.Head()->isValid() )
		{
			m_videoDecoder->decode( *m_videoFrames.Head() );

			VPXDecoder::IMAGE_ERROR err;
			if ( ( err = m_videoDecoder->getImage( *m_image ) ) != VPXDecoder::NO_FRAME )
			{
				if ( err == VPXDecoder::IMAGE_ERROR::NO_IMAGE_ERROR )
				{

					m_yTextureRegen->m_decodedImage = m_image;
					m_crTextureRegen->m_decodedImage = m_image;
					m_cbTextureRegen->m_decodedImage = m_image;

					m_yTexture->Download();
					m_crTexture->Download();
					m_cbTexture->Download();
				}
			}
			m_videoTime = m_videoFrames.Head()->time;
			m_currentFrame++;
		}

		WebMFrame *frame = m_videoFrames.RemoveAtHead();
		if ( frame )
			delete frame;
	}

	return true;
}

// Material / Texture Info functions
IMaterial *CVideoMaterial::GetMaterial()
{
	return m_videoMaterial;
}

// Where the video is actually is within the texture
void CVideoMaterial::GetVideoTexCoordRange( float *pMaxU, float *pMaxV )
{
	*pMaxU = ( float )m_videoWidth / ( float )m_textureWidth;
	*pMaxV = ( float )m_videoHeight / ( float )m_textureHeight;
}

void CVideoMaterial::GetVideoImageSize( int *pWidth, int *pHeight )
{
	*pWidth = m_videoWidth;
	*pHeight = m_videoHeight;
}

// Audio Functions
bool CVideoMaterial::HasAudio()
{
	return m_audioDecoder && m_audioDecoder->isOpen();
}

bool CVideoMaterial::SetVolume( float fVolume )
{
	m_volume = min( 1.0f, max( 0.0f, fVolume ) );
	if ( !m_pAudioBuffer )
		return false;

#ifdef _WIN32
	// TODO figure out what fucking value I'm supposed to use
	float log_volume = pow( m_volume, 0.2 );
	IDirectSoundBuffer_SetVolume( m_pAudioBuffer, ( LONG )( -10000 * ( 1.0f - log_volume ) ) );
	return true;
#else
	return false;
#endif
}

float CVideoMaterial::GetVolume()
{
	return m_volume;
}

void CVideoMaterial::SetMuted( bool bMuteState )
{
}

bool CVideoMaterial::IsMuted()
{
	return false;
}

VideoResult_t CVideoMaterial::SoundDeviceCommand( VideoSoundDeviceOperation_t operation, void *pDevice, void *pData )
{
#ifdef _WIN32
	if ( operation == VideoSoundDeviceOperation_t::SET_DIRECT_SOUND_DEVICE )
	{
		// if we had a sound buffer before, kill it
		m_soundKilled = true;
		DestroySoundBuffer();
		CreateSoundBuffer( pDevice );
		return VideoResult_t::SUCCESS;
	}
#endif
	return VideoResult_t::SYSTEM_NOT_AVAILABLE;
}
