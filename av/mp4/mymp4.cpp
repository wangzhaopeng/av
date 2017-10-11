
#include <stdio.h>
#include <iostream>

using namespace std;

#include <mp4v2/mp4v2.h>

#include "mymp4.h"


mymp4::mymp4(void)
{
	m_h_mp4 = MP4_INVALID_FILE_HANDLE;
}

mymp4::~mymp4(void)
{
	//printf("~mymp4\n");
	deinit();
}

bool mymp4::init(const char * pfile)
{
	m_h_mp4 = MP4Create(pfile, 0);
	if (m_h_mp4 == MP4_INVALID_FILE_HANDLE)
	{
		printf("open %s fialed.\n", pfile);
		deinit();
		return false;
	}
	return true;
}

void mymp4::deinit(void)
{
	if (m_h_mp4 != MP4_INVALID_FILE_HANDLE)
	{
		MP4Close(m_h_mp4);
		m_h_mp4 = MP4_INVALID_FILE_HANDLE;
	}
}

bool mymp4::init_a(int hz, int m_input_samples, const unsigned char* aac_decoder_info, int aac_decoder_info_size)
{
	m_a_track_id = MP4AddAudioTrack(m_h_mp4, hz, m_input_samples, MP4_MPEG4_AUDIO_TYPE);
	if (m_a_track_id == MP4_INVALID_TRACK_ID)
	{
		printf("MP4AddAudioTrack fialed.\n");
		deinit_a();
		return false;
	}

	//set audio level  LC, faac  aacObjectType = LOW;?
	//MP4SetAudioProfileLevel(m_h_mp4, 0x2 );

	bool bret = MP4SetTrackESConfiguration(m_h_mp4, m_a_track_id, aac_decoder_info, aac_decoder_info_size);
	if (!bret)
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
	bret = MP4WriteSample(m_h_mp4, m_a_track_id, (unsigned char*)pd, size);
	if (!bret)
	{
		printf("MP4WriteSample err  audio\n");
	}
	return bret;
}