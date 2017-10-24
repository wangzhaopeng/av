#include "myrtmp.h"

#include <stdio.h>

#include <string.h>
#include <iostream>
#include <vector>
using namespace std;

#include "librtmp/rtmp.h"
#include "librtmp/rtmp_sys.h"
#include "librtmp/amf.h"


#ifdef WIN32
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"winmm.lib")
#endif

int InitSockets()
{
#ifdef WIN32
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else
	return TRUE;
#endif
}


inline void CleanupSockets()
{
#ifdef WIN32
	WSACleanup();
#endif
}

myrtmp::myrtmp(void)
{
	m_p_rtmp = NULL;
	InitSockets();
}

myrtmp::~myrtmp(void)
{
	deinit();
}

void myrtmp::deinit(void)
{
	close();
	CleanupSockets();
}

bool myrtmp::connect(const char* url, int chunk_size)
{
	m_p_rtmp = RTMP_Alloc();
	RTMP_Init((RTMP *)m_p_rtmp);

	int iret;
	iret = RTMP_SetupURL((RTMP *)m_p_rtmp, (char*)url);
	if (iret != 1){
		cout << url << endl;
		cout << "RTMP_SetupURL err" << endl;
		return FALSE;
	}

	RTMP_EnableWrite((RTMP *)m_p_rtmp);

	iret = RTMP_Connect((RTMP *)m_p_rtmp, NULL);
	if (iret != 1){
		cout << url << endl;
		cout << "RTMP_Connect err" << endl;
		return FALSE;
	}

	iret = RTMP_ConnectStream((RTMP *)m_p_rtmp, 0);
	if (iret != 1){
		cout << url << endl;
		cout << "RTMP_ConnectStream err" << endl;
		return FALSE;
	}

	bool bret = change_chunk_size(chunk_size);
	if (!bret){
		return false;
	}

	return TRUE;
}

void myrtmp::close()
{
	if (m_p_rtmp){
		RTMP_Close((RTMP *)m_p_rtmp);
		RTMP_Free((RTMP *)m_p_rtmp);
		m_p_rtmp = NULL;
	}
}

bool myrtmp::send_packet(int packet_type, const char *pd, int size, unsigned int nTimestamp)
{
	if (m_p_rtmp == NULL){
		return FALSE;
	}

	RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	RTMPPacket_Alloc(&packet, size);

	packet.m_packetType = packet_type;
	packet.m_nChannel = 0x04;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nTimeStamp = nTimestamp;
	packet.m_nInfoField2 = ((RTMP *)m_p_rtmp)->m_stream_id;
	packet.m_nBodySize = size;
	memcpy(packet.m_body, pd, size);

	//int nRet = RTMP_SendPacket((RTMP *)m_p_rtmp, &packet, 0);  
	int nRet = RTMP_SendPacket((RTMP *)m_p_rtmp, &packet, 1);

	RTMPPacket_Free(&packet);

	if (nRet == 1){
		return true;
	}else{
		return false;
	}
}

bool myrtmp::change_chunk_size(int size)
{
	((RTMP *)m_p_rtmp)->m_outChunkSize = size;
	size = htonl(size);
	return send_packet(RTMP_PACKET_TYPE_CHUNK_SIZE,(char*)&size,sizeof(size),0);
}

bool myrtmp::init_v(const std::vector<char>&v_sps, const std::vector<char>&v_pps)
{
	char *body = new char[1024 + v_sps.size() + v_pps.size()];

	int i = 0;
	body[i++] = 0x17; // 1:keyframe  7:AVC   
	body[i++] = 0x00; // AVC sequence header   

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00; // fill in 0;   

	// AVCDecoderConfigurationRecord.   
	body[i++] = 0x01; // configurationVersion   
	body[i++] = v_sps[1]; // AVCProfileIndication   
	body[i++] = v_sps[2]; // profile_compatibility   
	body[i++] = v_sps[3]; // AVCLevelIndication    
	body[i++] = (char)0xff; // lengthSizeMinusOne     

	// sps nums   
	body[i++] = (char)0xE1; //&0x1f   
	// sps data length   
	body[i++] = v_sps.size() >> 8;
	body[i++] = v_sps.size() & 0xff;
	// sps data   
	memcpy(&body[i], &v_sps[0], v_sps.size());
	i = i + v_sps.size();

	// pps nums   
	body[i++] = 0x01; //&0x1f   
	// pps data length    
	body[i++] = v_pps.size() >> 8;
	body[i++] = v_pps.size() & 0xff;
	// sps data   
	memcpy(&body[i], &v_pps[0], v_pps.size());
	i = i + v_pps.size();

	bool bret = send_packet(RTMP_PACKET_TYPE_VIDEO, body, i, 0);
	delete[] body;
	return bret;
}

bool myrtmp::send_v(const char *pd, int size, bool key_frame, unsigned int time_stamp)
{
	if (pd == NULL && size<11){
		return false;
	}

	char *body = new char[size + 9];

	int i = 0;
	if (key_frame){
		body[i++] = 0x17;// 1:Iframe  7:AVC   
	}else{
		body[i++] = 0x27;// 2:Pframe  7:AVC   
	}
	body[i++] = 0x01;// AVC NALU   
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	// NALU size   
	body[i++] = size >> 24;
	body[i++] = size >> 16;
	body[i++] = size >> 8;
	body[i++] = size & 0xff;;

	// NALU data   
	memcpy(&body[i], pd, size);

	bool bRet = send_packet(RTMP_PACKET_TYPE_VIDEO, body, i+size, time_stamp);
	delete[] body;
	return bRet;
}

bool myrtmp::init_a(const std::vector<char> aac_info)
{
	char *body = new char[aac_info.size() + 8];
	body[0] = (char)0xAF;
	body[1] = 0x00;
	memcpy(body+2, &aac_info[0], aac_info.size());
	bool bret = send_packet(RTMP_PACKET_TYPE_AUDIO,body, 2+aac_info.size(), 0);
	delete[] body;
	return bret;
}

bool myrtmp::send_a(const char *pd, int size, unsigned int time_stamp)
{
	char *body = new char[size + 8];
	body[0] = (char)0xAF;
	body[1] = 0x01;
	memcpy(body+2, pd, size);
	bool bret = send_packet(RTMP_PACKET_TYPE_AUDIO, body, 2+size, time_stamp);
	delete[] body;
	return bret;
}
