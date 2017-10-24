#ifndef __MYRTMP_H__
#define __MYRTMP_H__

#include <vector>

class myrtmp
{
public:
	myrtmp(void);
	~myrtmp(void);
public:
	inline bool init(const char* url){return connect(url);}
	bool connect(const char* url);
	void close();

	/////h264
	bool init_v(const std::vector<char>&v_sps, const std::vector<char>&v_pps);
	bool send_v(const char *pd, int size, bool key_frame, unsigned int time_stamp);

	//aac
	bool init_a(const std::vector<char> aac_info);
	bool send_a(const char *pd, int size, unsigned int time_stamp);

private:
	void deinit(void);
	bool send_packet(int packet_type, const char *pd, int size, unsigned int nTimestamp);

	void* m_p_rtmp;//RTMP* m_p_rtmp;
};

#endif
