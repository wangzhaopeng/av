#ifndef __MYRTMP_H__
#define __MYRTMP_H__

#include <string>
#include <vector>

class myrtmp
{
public:
	myrtmp(void);
	~myrtmp(void);
public:

	bool init_send(const char* url){return connect(url,true);}
	bool init_rcv(const char* url){return connect(url,false);}

	bool connect(const char* url,bool send_flag = true, int chunk_size = 1360);
	void close();

	/////h264
	bool init_v(const std::vector<char>&v_sps, const std::vector<char>&v_pps);
	bool send_v(const char *pd, int size, bool key_frame, unsigned int time_stamp);

	//aac
	bool init_a(const std::vector<char> aac_info);
	bool send_a(const char *pd, int size, unsigned int time_stamp);

	////rcv
	int rcv(char ** pp);
private:
	void deinit(void);
	bool send_packet(int packet_type, const char *pd, int size, unsigned int nTimestamp);
	bool change_chunk_size(int size);

	void* m_p_rtmp;//RTMP* m_p_rtmp;

	////rcv
	static const int m_rcv_buf_size = 5*1024*1024;
	std::vector<char> m_rcv_buf;
};

#endif
