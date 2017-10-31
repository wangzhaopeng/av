#ifndef __MYRTMP_H__
#define __MYRTMP_H__

#include <string>
#include <vector>

struct s_rtmp_chunk//////11+rtmp数据 字节
{
	unsigned char type;/////1个字节
	unsigned int size;////3个字节，下面有效数据的长度
	unsigned int time_stamp;////4个字节
	//unsigned int field;/////3字节  填充？
	char*p_d;/////指向数据的指针
	//unsigned int end;////4字节
};

struct s_rcv_data
{
	char *p_sps,*p_pps,*p_h264;
	int sps_size,pps_size,h264_size;

	char *p_aac_info,*p_aac;
	int aac_info_size,aac_size;

	int err_flag;
};

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
	void flv2mp4(const char*pd,int size,s_rcv_data *p_rcv_data);

private:
	void deinit(void);
	bool send_packet(int packet_type, const char *pd, int size, unsigned int nTimestamp);
	bool change_chunk_size(int size);

	void* m_p_rtmp;//RTMP* m_p_rtmp;

	////rcv
	static const int m_rcv_buf_size = 5*1024*1024;
	std::vector<char> m_rcv_buf;

	char* get_chunk(const char*pd,int size,s_rtmp_chunk* p_chunk);
	int get_aac(const s_rtmp_chunk* p_chunk,s_rcv_data *p_rcv_data);
	int get_h264(const s_rtmp_chunk* p_chunk,s_rcv_data *p_rcv_data);
};
#endif
