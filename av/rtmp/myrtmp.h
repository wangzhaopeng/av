#ifndef __MYRTMP_H__
#define __MYRTMP_H__


#include "librtmp/rtmp.h"
#include "librtmp/rtmp_sys.h"   
#include "librtmp/amf.h"   
#include <stdio.h>   

#include <vector>


//class CRTMPStream
class myrtmp
{
public:
	myrtmp(void);
	~myrtmp(void);
public:
	// 连接到RTMP Server
	bool connect(const char* url);
	inline bool init(const char* url){return connect(url);}
	// 断开连接   
	void close();

	/////h264
	bool init_v(const std::vector<char>&v_sps, const std::vector<char>&v_pps);
	bool send_v(const char *pd, int size, bool key_frame, unsigned int time_stamp);

	//aac
	bool init_a(std::vector<char> aac_info);
	bool send_a(const char *pd, int size, unsigned int time_stamp);

private:
	void deinit(void);
	// 发送数据   
	bool SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp);
private:
	RTMP* m_pRtmp;
};

#endif