#ifndef __MYAAC_H__
#define __MYAAC_H__

#include <vector>

class myaac
{
public:
	~myaac(void);
	myaac(void);
	bool init(int hz, int channal, int bits);
	void pcm2aac(const char * pdata, int samples, std::vector<char> &v_aac);

	unsigned long m_input_samples;		// ����������
	std::vector<char> m_v_decoder_info;

private:
	void deinit(void);

	void* m_h_encoder;
	unsigned long m_max_output_size;	// ����������ռ�
	unsigned char* m_p_out_buf;

	int m_channal;		// ������
	int m_hz;			// ������
	int m_bits;		// ������λ��(eg:16) ����������Ƶ��Ϣλ��
};

#endif