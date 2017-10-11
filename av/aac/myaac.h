#ifndef __MYAAC_H__
#define __MYAAC_H__

#include <vector>

class myaac
{
public:
	~myaac(void);
	myaac(void);
	bool init(int nSampleRate, int channal, int bitsPerSample);
	void pcm2aac(const char * pdata, int samples, std::vector<char> &v_aac);

	int m_channal;		// ������
	int m_hz;			// ������
	int m_nBitsPerSample;		// ������λ��(eg:16) ����������Ƶ��Ϣλ��

	unsigned long m_nInputSamples;		// ����������
	unsigned long m_nMaxInputBytes;		// �����������ռ�

	unsigned char* faacDecoderInfo;
	unsigned long  faacDecoderInfoSize;

private:
	void deinit(void);

	void* m_hEncoder;
	unsigned long m_nMaxOutputBytes;	// ����������ռ�
	unsigned char* m_pOutAACBuffer;
};

#endif