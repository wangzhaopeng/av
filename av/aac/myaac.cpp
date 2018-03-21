#include "myaac.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <faac.h>

#include <iostream>

using namespace std;

myaac::myaac(int hz, int channal, int bits)
{
	m_hz = hz;
	m_channal = channal;
	m_bits = bits;
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


bool myaac::init(void){
	int iret = 0;

	// init faac
	m_h_encoder = faacEncOpen(m_hz, m_channal, &m_input_samples, &m_max_output_size);

	//m_nMaxInputBytes = m_input_samples*m_bits*m_channal / 8;

	m_p_out_buf = new unsigned char[m_max_output_size];

	// Get current encoding configuration
	faacEncConfigurationPtr p_cfg = faacEncGetCurrentConfiguration(m_h_encoder);
	if (!p_cfg){
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
	if (iret != 1){
		deinit();
		printf("faacEncSetConfiguration error!\n");
		return false;
	}

	// get decoder info
	unsigned char* p_decoder_info;
	unsigned long  decoder_info_size;
	if (faacEncGetDecoderSpecificInfo(m_h_encoder, &p_decoder_info, &decoder_info_size)){
		deinit();
		printf("faacEncGetDecoderSpecificInfo error!\n");
		return false;
	}
	std::vector<char> v_decoder_info(p_decoder_info,p_decoder_info+decoder_info_size);
	m_v_decoder_info = v_decoder_info;
	free(p_decoder_info);

	return true;
}

////return -1 err     0 ok v_aac为输出的aac数据 为空代表没有aac数据
int myaac::pcm2aac(const char* pdata, int samples, std::vector<char> &v_aac){
	v_aac.clear();

	int sample_size = m_bits/8;
	int enc_size = sample_size*m_input_samples;
	m_pcm_que.insert(m_pcm_que.end(),pdata,pdata+samples*sample_size);

	std::vector<char> v_enc_samples;
	if ((int)m_pcm_que.size()>=enc_size){
		v_enc_samples.insert(v_enc_samples.begin(),m_pcm_que.begin(),m_pcm_que.begin()+enc_size);
		m_pcm_que.erase(m_pcm_que.begin(), m_pcm_que.begin() + enc_size);

		//int iret = faacEncEncode(m_h_encoder, (int*)pdata, samples, m_p_out_buf, m_max_output_size);
		int iret = faacEncEncode(m_h_encoder, (int*)&v_enc_samples[0], m_input_samples, m_p_out_buf, m_max_output_size);
		if(iret <0){
			cout<<" faacEncEncode err "<<endl;
			return -1;
		}else if (iret == 0){

		}else{
			v_aac.insert(v_aac.end(),m_p_out_buf,m_p_out_buf+iret);
		}
	}
	return 0;
}