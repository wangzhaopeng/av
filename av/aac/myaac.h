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

	int m_channal;		// 声道数
	int m_hz;			// 采样率
	int m_nBitsPerSample;		// 单样本位数(eg:16) 单个采样音频信息位数

	unsigned long m_nInputSamples;		// 输入样本数
	unsigned long m_nMaxInputBytes;		// 输入所需最大空间

	unsigned char* faacDecoderInfo;
	unsigned long  faacDecoderInfoSize;

private:
	void deinit(void);

	void* m_hEncoder;
	unsigned long m_nMaxOutputBytes;	// 输出所需最大空间
	unsigned char* m_pOutAACBuffer;
};

#endif