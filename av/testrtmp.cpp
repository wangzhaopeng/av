
#include <iostream>
#include <vector>

using namespace std;

#include <myaac.h>


#include "myrtmp.h"
#include <SpsDecode.h>

void wav_vec(int size, std::vector<vector<char>> &v_slice);

void h264_vec(std::vector<vector<char>> &v_slice);


void tst_rtmp(void)
{
	vector<vector<char>> v_slice;
	h264_vec(v_slice);

	int a_hz=22050;
	myaac o_aac;
	o_aac.init(a_hz,1,16);
	vector<vector<char>> va_slice;
	wav_vec(o_aac.get_input_samples(),va_slice);
	double aac_slice_ms = (double)o_aac.get_input_samples()*1000/a_hz;
	
	myrtmp rtmp_sender;
	bool bret = rtmp_sender.connect("rtmp://192.168.5.116/live/zb");
	if (!bret){
		return;
	}

	////发送sps pps
	rtmp_sender.init_v(v_slice[0],v_slice[1]);  
	////发送aac decoder info
	rtmp_sender.init_a(o_aac.get_decoder_info());

	unsigned int tick = 0;
	for(int i = 2; i < (int)v_slice.size(); i++){  
		int nalu_type = v_slice[i][0]&0x1F;
		bool bKeyframe  = (nalu_type == 0x05) ? 1 : 0;

		rtmp_sender.send_v((char*)&v_slice[i][0],v_slice[i].size(),bKeyframe,tick);
		//msleep(40);
		tick += (int)aac_slice_ms;

		{
			vector<char> v_aac;
			static int aac_tick = 0;
			o_aac.pcm2aac(&va_slice[i][0],o_aac.get_input_samples(),v_aac);
			if (v_aac.size()>0){
				rtmp_sender.send_a((&v_aac[0])+7,v_aac.size()-7,aac_tick);
				aac_tick += (int)aac_slice_ms;//aac每次1024个采样，在22050hz下需要46ms，上面帧率跟这个也有关
			}
		}
	}  

int a = 0;
a++;
}