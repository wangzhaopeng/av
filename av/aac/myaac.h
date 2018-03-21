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
	unsigned long m_max_output_size;	// ����������ռ�
	unsigned char* m_p_out_buf;

	int m_channal;		// ������
	int m_hz;			// ������
	int m_bits;		// ������λ��(eg:16) ����������Ƶ��Ϣλ��

	unsigned long m_input_samples;		// ����������
	std::vector<char> m_v_decoder_info;

	std::deque<char> m_pcm_que;
};

#endif