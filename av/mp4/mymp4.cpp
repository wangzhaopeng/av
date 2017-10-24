#include "mymp4.h"

#include <stdio.h>
#include <iostream>

using namespace std;

#include <mp4v2/mp4v2.h>




mymp4::mymp4(void)
{
	m_h_mp4 = MP4_INVALID_FILE_HANDLE;
}

mymp4::~mymp4(void)
{
	//printf("~mymp4\n");
	deinit();
}

bool mymp4::init(const char * pfile, int time_scale)
{
	m_file_name = pfile;
	m_h_mp4 = MP4Create(pfile, 0);
	if (m_h_mp4 == MP4_INVALID_FILE_HANDLE)
	{
		printf("open %s fialed.\n", pfile);
		deinit();
		return false;
	}
	MP4SetTimeScale(m_h_mp4, time_scale);
	m_time_scale = time_scale;
	return true;
}

void mymp4::deinit(void)
{
	if (m_h_mp4 != MP4_INVALID_FILE_HANDLE)
	{
		MP4Close(m_h_mp4);
		m_h_mp4 = MP4_INVALID_FILE_HANDLE;
	}
	m_file_name = "";
}

bool mymp4::init_a(int hz, int m_input_samples, const std::vector<char>&v_aac_decoder_info)
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

	bool bret = MP4SetTrackESConfiguration(m_h_mp4, m_a_track_id, (unsigned char*)(&v_aac_decoder_info[0]), v_aac_decoder_info.size());
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


bool mymp4::init_v(int width,int height,int frame_rate,const std::vector<char>&v_sps,const std::vector<char>&v_pps)
{
	m_v_width = width;
	m_v_height = height;
	m_v_frame_rate = frame_rate;
	m_v_track_id = MP4AddH264VideoTrack(m_h_mp4,m_time_scale,m_time_scale/m_v_frame_rate,m_v_width,m_v_height,
		v_sps[1],v_sps[2],v_sps[3],3); 
	if (m_v_track_id == MP4_INVALID_TRACK_ID)
	{
		printf("MP4AddH264VideoTrack fialed.\n");
		deinit_v();
		return false;
	}
	//MP4SetVideoProfileLevel(m_h_mp4, 1); 
	MP4AddH264SequenceParameterSet(m_h_mp4,m_v_track_id,(unsigned char*)&v_sps[0],v_sps.size());
	MP4AddH264PictureParameterSet(m_h_mp4,m_v_track_id,(unsigned char*)&v_pps[0],v_pps.size());
	return true;
}

void mymp4::deinit_v(void)
{

}

bool mymp4::write_v(const char *pd, int size)
{
	int datalen = size+4;
	unsigned char *data = new unsigned char[datalen];

	data[0] = size>>24;
	data[1] = size>>16;
	data[2] = size>>8;
	data[3] = size&0xff;

	memcpy(data+4,pd,size);
	if(!MP4WriteSample(m_h_mp4,m_v_track_id, data, datalen,MP4_INVALID_DURATION, 0, 1))
	{
		delete[] data;
		return false;
	}
	delete[] data;

	return true;
}