#ifndef __MYAAC_H__
#define __MYAAC_H__

#include <vector>
#include <deque>

class myaac
{
public:
	~myaac(void);
	myaac(int hz, int channal, int bits);

	bool init(void);
	int pcm2aac(const char * pdata, int samples, std::vector<char> &v_aac);

	inline int get_input_samples(void)const{ return m_input_samples; }
	inline std::vector<char> get_decoder_info(void)const{ return m_v_decoder_info; }

private:
	void deinit(void);
	myaac(const myaac&);
	myaac& operator=(const myaac&);

	void* m_h_encoder;
	unsigned long m_max_output_size;	// 输出所需最大空间
	unsigned char* m_p_out_buf;

	int m_channal;		// 声道数
	int m_hz;			// 采样率
	int m_bits;		// 单样本位数(eg:16) 单个采样音频信息位数

	unsigned long m_input_samples;		// 输入样本数
	std::vector<char> m_v_decoder_info;

	std::deque<char> m_pcm_que;
};

#endif