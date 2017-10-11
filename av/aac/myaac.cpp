
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <faac.h>
#include "myaac.h"


myaac::myaac(void)
{
	m_hEncoder = NULL;
	m_pOutAACBuffer = NULL;
	faacDecoderInfo = NULL;
}

myaac::~myaac(void)
{
	printf("~aac\n");
	deinit();
}

void myaac::deinit(void)
{
	if (faacDecoderInfo)
	{
		free( faacDecoderInfo );
		faacDecoderInfo = NULL;
	}

	if ( m_pOutAACBuffer)
	{
		delete [] m_pOutAACBuffer;
		m_pOutAACBuffer = NULL;
	}

	faacEncClose( m_hEncoder);
}

bool myaac::init(int nSampleRate, int channal, int bitsPerSample)
{
	int iret=0;
	m_hz = nSampleRate;
	m_channal = channal;
	m_nBitsPerSample = bitsPerSample;

	// init faac
	m_hEncoder = faacEncOpen(nSampleRate, m_channal, &m_nInputSamples, &m_nMaxOutputBytes);

	m_nMaxInputBytes = m_nInputSamples*bitsPerSample*m_channal / 8;

	m_pOutAACBuffer = new unsigned char[m_nMaxOutputBytes];

	// Get current encoding configuration
	faacEncConfigurationPtr pConfiguration = faacEncGetCurrentConfiguration(m_hEncoder);
	if (!pConfiguration)
	{
		printf("faacEncGetCurrentConfiguration error!\n");
		deinit();
		return false;
	}

	//���ð汾,¼��MP4�ļ�ʱҪ��MPEG4
	pConfiguration->version = MPEG4;
	pConfiguration->aacObjectType = LOW; //LC����

	//������������
	pConfiguration->inputFormat = FAAC_INPUT_16BIT;

	// outputFormat (0 = Raw; 1 = ADTS)
	// ¼��MP4�ļ�ʱ��Ҫ��raw������������Ƿ���ȷʱ������Ϊ adts��������
	pConfiguration->outputFormat = 1;

	//˲ʱ��������(temporal noise shaping��TNS)�˲���
	pConfiguration->shortctl = SHORTCTL_NORMAL;

	pConfiguration->useTns = true;
	//pConfiguration->useLfe=false;
	pConfiguration->quantqual = 100;
	pConfiguration->bandWidth = 0;
	pConfiguration->bitRate = 0;

	iret = faacEncSetConfiguration(m_hEncoder, pConfiguration);
	if (iret != 1)
	{
		deinit();
		printf("faacEncSetConfiguration error!\n");
		return false;
	}

	// get decoder info
	if (faacEncGetDecoderSpecificInfo(m_hEncoder, &faacDecoderInfo, &faacDecoderInfoSize))
	{
		deinit();
		printf("faacEncGetDecoderSpecificInfo error!\n");
		return false;
	}

	return true;
}



void myaac::pcm2aac(const char* pdata, int samples, std::vector<char> &v_aac)
{
	int nRet = faacEncEncode(m_hEncoder, (int*)pdata, samples, m_pOutAACBuffer, m_nMaxOutputBytes );
	if ( nRet <= 0 )
	{
		return;
	}

	for (int i = 0; i < nRet; i++)
	{
		v_aac.push_back(*(m_pOutAACBuffer+i));
	}
}