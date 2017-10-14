
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <faac.h>
#include "myaac.h"


myaac::myaac(void)
{
	m_h_encoder = NULL;
	m_p_out_buf = NULL;
}

myaac::~myaac(void)
{
	//printf("~aac\n");
	deinit();
}

void myaac::deinit(void)
{
	if (m_p_out_buf)
	{
		delete[] m_p_out_buf;
		m_p_out_buf = NULL;
	}

	if (m_h_encoder)
	{
		faacEncClose(m_h_encoder);
	}
}

bool myaac::init(int hz, int channal, int bits)
{
	int iret = 0;
	m_hz = hz;
	m_channal = channal;
	m_bits = bits;

	// init faac
	m_h_encoder = faacEncOpen(m_hz, m_channal, &m_input_samples, &m_max_output_size);

	//m_nMaxInputBytes = m_input_samples*m_bits*m_channal / 8;

	m_p_out_buf = new unsigned char[m_max_output_size];

	// Get current encoding configuration
	faacEncConfigurationPtr p_cfg = faacEncGetCurrentConfiguration(m_h_encoder);
	if (!p_cfg)
	{
		printf("faacEncGetCurrentConfiguration error!\n");
		deinit();
		return false;
	}

	//设置版本,录制MP4文件时要用MPEG4
	p_cfg->version = MPEG4;
	p_cfg->aacObjectType = LOW; //LC编码

	//输入数据类型
	p_cfg->inputFormat = FAAC_INPUT_16BIT;

	// outputFormat (0 = Raw; 1 = ADTS)
	// 录制MP4文件时，要用raw流。检验编码是否正确时可设置为 adts传输流，
	p_cfg->outputFormat = 1;

	//瞬时噪声定形(temporal noise shaping，TNS)滤波器
	p_cfg->shortctl = SHORTCTL_NORMAL;

	p_cfg->useTns = true;
	//p_cfg->useLfe=false;
	p_cfg->quantqual = 100;
	p_cfg->bandWidth = 0;
	p_cfg->bitRate = 0;

	iret = faacEncSetConfiguration(m_h_encoder, p_cfg);
	if (iret != 1)
	{
		deinit();
		printf("faacEncSetConfiguration error!\n");
		return false;
	}

	// get decoder info
	unsigned char* p_decoder_info;
	unsigned long  decoder_info_size;
	if (faacEncGetDecoderSpecificInfo(m_h_encoder, &p_decoder_info, &decoder_info_size))
	{
		deinit();
		printf("faacEncGetDecoderSpecificInfo error!\n");
		return false;
	}
	std::vector<char> v_decoder_info(p_decoder_info,p_decoder_info+decoder_info_size);
	m_v_decoder_info = v_decoder_info;
	free(p_decoder_info);

	return true;
}



void myaac::pcm2aac(const char* pdata, int samples, std::vector<char> &v_aac)
{
	int iret = faacEncEncode(m_h_encoder, (int*)pdata, samples, m_p_out_buf, m_max_output_size);
	if (iret <= 0)
	{
		return;
	}

	for (int i = 0; i < iret; i++)
	{
		v_aac.push_back(*(m_p_out_buf + i));
	}
}