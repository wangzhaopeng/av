
#include <iostream>
#include <vector>
using namespace std;

#include "myrtmp.h"

#ifdef WIN32   
#pragma comment(lib,"WS2_32.lib")   
#pragma comment(lib,"winmm.lib")   
#endif   

#include <string.h>

enum
{
	FLV_CODECID_H264 = 7,
};

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

CRTMPStream::CRTMPStream(void)
{
	m_pRtmp = NULL;
	InitSockets();
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
}

CRTMPStream::~CRTMPStream(void)
{
	close();
	CleanupSockets();
}

bool CRTMPStream::connect(const char* url)
{
	int iret;
	iret = RTMP_SetupURL(m_pRtmp, (char*)url);
	if (iret != 1)
	{
		cout << url << endl;
		cout << "RTMP_SetupURL err" << endl;
		return FALSE;
	}
	RTMP_EnableWrite(m_pRtmp);
	iret = RTMP_Connect(m_pRtmp, NULL);
	if (iret != 1)
	{
		cout << url << endl;
		cout << "RTMP_Connect err" << endl;
		return FALSE;
	}
	iret = RTMP_ConnectStream(m_pRtmp, 0);
	if (iret != 1)
	{
		cout << url << endl;
		cout << "RTMP_ConnectStream err" << endl;
		return FALSE;
	}
	return TRUE;
}

void CRTMPStream::close()
{
	if (m_pRtmp)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
}

bool CRTMPStream::SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp)
{
	if (m_pRtmp == NULL)
	{
		return FALSE;
	}

	RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	RTMPPacket_Alloc(&packet, size);

	packet.m_packetType = nPacketType;
	packet.m_nChannel = 0x04;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nTimeStamp = nTimestamp;
	packet.m_nInfoField2 = m_pRtmp->m_stream_id;
	packet.m_nBodySize = size;
	memcpy(packet.m_body, data, size);

	//int nRet = RTMP_SendPacket(m_pRtmp,&packet,0);  
	int nRet = RTMP_SendPacket(m_pRtmp, &packet, 1);

	RTMPPacket_Free(&packet);

	if (nRet == 1){
		return true;
	}else{
		return false;
	}
}

bool CRTMPStream::init_v(const std::vector<char>&v_sps, const std::vector<char>&v_pps)
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

	bool bret = SendPacket(RTMP_PACKET_TYPE_VIDEO, (unsigned char*)body, i, 0);
	delete[] body;
	return bret;
}

bool CRTMPStream::send_v(const char *pd, int size, bool key_frame, unsigned int time_stamp)
{
	if (pd == NULL && size<11)
	{
		return false;
	}

	unsigned char *body = new unsigned char[size + 9];

	int i = 0;
	if (key_frame)
	{
		body[i++] = 0x17;// 1:Iframe  7:AVC   
	}
	else
	{
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

	bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i + size, time_stamp);
	delete[] body;
	return bRet;
}


bool CRTMPStream::init_a(std::vector<char> aac_info)
{
	char *body = new char[aac_info.size() + 8];
	body[0] = (char)0xAF;
	body[1] = 0x00;
	memcpy(body + 2, &aac_info[0], aac_info.size());
	bool bret = SendPacket(RTMP_PACKET_TYPE_AUDIO, (unsigned char*)body, 4, 0);
	delete[] body;
	return bret;
}

bool CRTMPStream::send_a(const char *pd, int size, unsigned int time_stamp)
{
	unsigned char *body = new unsigned char[size + 8];
	body[0] = 0xAF;
	body[1] = 0x01;
	memcpy(body + 2, pd, size);
	bool bret = SendPacket(RTMP_PACKET_TYPE_AUDIO, body, 2 + size, time_stamp);
	delete[] body;
	return bret;
}