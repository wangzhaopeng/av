
#include <stdio.h>
#include <iostream>

using namespace std;

#include <mp4v2/mp4v2.h>

#include "mymp4.h"


mymp4::mymp4(void)
{
	m_Mp4File = MP4_INVALID_FILE_HANDLE;
}

mymp4::~mymp4(void)
{
	printf("~mymp4\n");
	deinit();
}

bool mymp4::init(const char * pfile)
{
	m_Mp4File = MP4Create( pfile,  0);
	if ( m_Mp4File == MP4_INVALID_FILE_HANDLE)
	{
		printf("open %s fialed.\n",pfile);
		deinit();
		return false;
	}
	return true;
}

void mymp4::deinit(void)
{
	if ( m_Mp4File != MP4_INVALID_FILE_HANDLE)
	{
		MP4Close(m_Mp4File);
		m_Mp4File = MP4_INVALID_FILE_HANDLE;
	}
}

bool mymp4::init_a(int nSampleRate, int m_nInputSamples, unsigned char* faacDecoderInfo, int faacDecoderInfoSize)
{
	m_AudioTrackId  = MP4AddAudioTrack( m_Mp4File, nSampleRate, m_nInputSamples, MP4_MPEG4_AUDIO_TYPE );
	if (m_AudioTrackId == MP4_INVALID_TRACK_ID)
	{
		printf("MP4AddAudioTrack fialed.\n");
		deinit_a();
		return false;
	}

	//set audio level  LC, faac  aacObjectType = LOW;?
	//MP4SetAudioProfileLevel(m_Mp4File, 0x2 );

	bool bOk = MP4SetTrackESConfiguration(m_Mp4File, m_AudioTrackId, faacDecoderInfo, faacDecoderInfoSize );
	if( !bOk )
	{
		printf("MP4SetTrackESConfiguration fialed.\n");
		deinit_a();
		return false;   
	}

	return true;
}

void mymp4::deinit_a(void)
{
	deinit();
}


bool mymp4::write_a(const char *pd, int size)
{
	bool bret;
	bret = MP4WriteSample( m_Mp4File, m_AudioTrackId, (unsigned char*)pd, size);
	if (!bret)
	{
		printf("MP4WriteSample err  audio\n");
	}
	return bret;
}