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

	m_rcv_buf.resize(m_rcv_buf_size);
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

bool myrtmp::connect(const char* url, bool send_flag, int chunk_size)
{
	m_p_rtmp = RTMP_Alloc();
	RTMP_Init((RTMP *)m_p_rtmp);

	//RTMP *tem_rtmp = (RTMP *)m_p_rtmp;

	((RTMP *)m_p_rtmp)->Link.timeout=5;
	int iret;
	iret = RTMP_SetupURL((RTMP *)m_p_rtmp, (char*)url);
	if (iret != 1){
		cout << url << endl;
		cout << "RTMP_SetupURL false" << endl;
		deinit();
		return FALSE;
	}

	if (send_flag){
		RTMP_EnableWrite((RTMP *)m_p_rtmp);
	}

	//((RTMP *)m_p_rtmp)->Link.lFlags|=RTMP_LF_LIVE;
	//RTMP_SetBufferMS((RTMP *)m_p_rtmp, 3600*1000);

	iret = RTMP_Connect((RTMP *)m_p_rtmp, NULL);
	if (iret != 1){
		cout << url << endl;
		cout << "RTMP_Connect false" << endl;
		deinit();
		return FALSE;
	}

	iret = RTMP_ConnectStream((RTMP *)m_p_rtmp, 0);
	if (iret != 1){
		cout << url << endl;
		cout << "RTMP_ConnectStream false" << endl;
		deinit();
		return FALSE;
	}

	bool bret = change_chunk_size(chunk_size);
	if (!bret){
		cout << url << endl;
		cout << "change_chunk_size false " << chunk_size << endl;
		deinit();
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

int myrtmp::rcv(char ** pp)
{
	char* p_b = &m_rcv_buf[0];
	int rcv_size;
	rcv_size = RTMP_Read((RTMP *)m_p_rtmp,p_b,m_rcv_buf_size);
	*pp = p_b;
	return rcv_size;
}

void myrtmp::flv2mp4(const char*pd,int size,s_rcv_data *p_rcv_data)
{
	const char *pd_cur;
	pd_cur = pd;

	s_rcv_data tem_data = {0};
	*p_rcv_data = tem_data;

	if (*pd_cur == 'F'){
		////头三个字节为"FLV"
		pd_cur+=9;////跳过FLV Header
		pd_cur+=4;////跳过previousTagSizen
	}

	int iret;
	while(pd_cur<pd+size){
		s_rtmp_chunk s_chunk={0};
		pd_cur=get_chunk(pd_cur,size-(pd_cur-pd),&s_chunk);
		if (s_chunk.type == RTMP_PACKET_TYPE_AUDIO){
			iret = get_aac(&s_chunk, p_rcv_data);
			if (iret == -1){
				p_rcv_data->err_flag = 1;
				return;
			}
		}else if (s_chunk.type == RTMP_PACKET_TYPE_VIDEO){
			iret = get_h264(&s_chunk,p_rcv_data);
			if (iret == -1){
				p_rcv_data->err_flag = 1;
				return;
			}
		}else if (s_chunk.type == RTMP_PACKET_TYPE_INFO){

			static char buf[1024];
			memcpy(buf,s_chunk.p_d,s_chunk.size);
			int a = 0;
			a ++;
		}else{
			cout<<"err rcv not AUDIO VIDEO TYPE_INFO "<<endl;
			p_rcv_data->err_flag = 1;
			return;
		}
	}
}

char* myrtmp::get_chunk(const char*pd,int size,s_rtmp_chunk* p_chunk)
{
	const unsigned char *pd_cur;
	pd_cur = (unsigned char *)pd;

	if (*pd_cur != RTMP_PACKET_TYPE_AUDIO && *pd_cur != RTMP_PACKET_TYPE_VIDEO && *pd_cur != RTMP_PACKET_TYPE_INFO){
		cout<<"err rcv not AUDIO VIDEO TYPE_INFO "<<endl;
		return (char*)pd+size;
	}

	if(size<11+4){
		cout<<"err size need min 11+4 "<<endl;
		return (char*)pd+size;
	}

	p_chunk->type = *pd_cur++;

	p_chunk->size += (*pd_cur++<<16);
	p_chunk->size += (*pd_cur++<<8);
	p_chunk->size += (*pd_cur++<<0);

	if (size < (int)(11+4+p_chunk->size)){
		cout<<"err size < 11+4+s_chunk.size "<<endl;
		return (char*)pd+size;
	}

	p_chunk->time_stamp += (*pd_cur++<<24);
	p_chunk->time_stamp += (*pd_cur++<<16);
	p_chunk->time_stamp += (*pd_cur++<<8);
	p_chunk->time_stamp += (*pd_cur++<<0);

	pd_cur += 3;
	p_chunk->p_d = (char*)pd_cur;
	pd_cur += p_chunk->size;
	pd_cur += 4;

	return (char*)pd_cur;
}

/////返回-1错误   1有数据   0 aac info
int myrtmp::get_aac(const s_rtmp_chunk* p_chunk,s_rcv_data *p_rcv_data)
{
	const unsigned char *pd_cur;
	pd_cur = (unsigned char *)p_chunk->p_d;
	if (*pd_cur!=0xaf){
		cout<<"err get_aac *pd_cur!=0xaf "<<endl;
		return -1;
	}
	if (p_chunk->size<4){
		////aac info 2字节 加头 为4字节
		cout<<"err get_aac s_chunk.size<4"<<endl;
		return -1;
	}

	if (*(pd_cur+1)==0x1){
		////aac 声音数据
		p_rcv_data->p_aac = (char*)pd_cur+2;
		p_rcv_data->aac_size = p_chunk->size - 2;

		return 1;
	}else if (*(pd_cur+1)==0x0){
		////aac info
		if (p_chunk->size>64){
			/////限定 aac info 不会超过64字节 或者小于4-2字节
			cout<<"err get_aac aac info > 64 "<<endl;
			return -1;
		}else{
			p_rcv_data->p_aac_info = (char*)pd_cur+2;
			p_rcv_data->aac_info_size = p_chunk->size - 2;

			return 0;
		}
	}else {
		cout<<"err get_aac *pd_cur!=0 1 "<<endl;
		return -1;
	}
}

/////返回-1错误   1有数据   0 sps pps
int myrtmp::get_h264(const s_rtmp_chunk* p_chunk,s_rcv_data *p_rcv_data)
{
	const unsigned char *pd_cur;
	pd_cur = (unsigned char *)p_chunk->p_d;
	if (*pd_cur!=0x17 && *pd_cur!=0x27){
		cout<<"err get_h264 *pd_cur!=0x17 || *pd_cur!=0x27 "<<endl;
		return -1;
	}

	if ((*pd_cur==0x17||*pd_cur==0x27) && *(pd_cur+1)==0x1){
		////视频帧

		if (p_chunk->size<9){
			cout << "err get_h264 s_chunk.size<9"<<endl;
			return -1;
		}

		p_rcv_data->p_h264 = (char*)pd_cur + 9;
		p_rcv_data->h264_size = p_chunk->size - 9;
		return 1;
	}else if (*pd_cur==0x17 && *(pd_cur+1)==0x0){
		////sps pps
		if (p_chunk->size<13){
			cout << "err sps pps s_chunk.size<13"<<endl;
			return -1;
		}
		int sps_len = (pd_cur[11]<<8)+pd_cur[12];

		if ((int)p_chunk->size<13+sps_len+3){
			cout<< "err sps pps s_chunk.size<13+sps_len"<<endl;
			return -1;
		}
		if (sps_len>256){
			cout<< "err sps_len>256"<<endl;
			return -1;
		}

		p_rcv_data->p_sps = (char*)&pd_cur[13];
		p_rcv_data->sps_size = sps_len;

		int pps_len = (pd_cur[13+sps_len+1]<<8)+pd_cur[13+sps_len+2];

		if ((int)p_chunk->size<13+sps_len+3+pps_len){
			cout<< "err s_chunk.size<13+sps_len+3+pps_len"<<endl;
			return -1;
		}
		if (pps_len>256){
			cout<< "err pps_len>256"<<endl;
			return -1;
		}

		p_rcv_data->p_pps = (char *)&pd_cur[13+sps_len+3];
		p_rcv_data->pps_size = pps_len;

		return 0;
	}else {
		cout<<"err get_h264 err "<<endl;
		return -1;
	}
}
