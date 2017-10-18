
#include <iostream>
#include <vector>

using namespace std;

#include <myaac.h>


#include "myrtmp.h"
#include <SpsDecode.h>

void wav_vec(int size, std::vector<vector<char>> &v_slice);

void h264_vec(std::vector<vector<char>> &v_slice);

typedef struct _NaluUnit  
{  
	int type;  
	int size;  
	unsigned char *data;  
}NaluUnit;  


void tst_rtmp(void)
{



	vector<vector<char>> v_slice;
	h264_vec(v_slice);

	
	int a_hz=22050;
	myaac o_aac;
	o_aac.init(a_hz,1,16);
	vector<vector<char>> va_slice;
	wav_vec(o_aac.get_input_samples(),va_slice);
	

	CRTMPStream rtmpSender;
	bool bRet = rtmpSender.connect("rtmp://192.168.5.116/live/zb");

	NaluUnit naluUnit;  

	//// 发送MetaData   
	//rtmpSender.SendMetadata(&metaData);  
	rtmpSender.init_v(v_slice[0],v_slice[1]);  

	//rtmpSender.init_a();
	rtmpSender.init_a(o_aac.get_decoder_info());


	unsigned int tick = 0;  
	//while(ReadOneNaluFromBuf(naluUnit))  
	for(int i = 2; i < v_slice.size(); i++)
	{  
		naluUnit.data = (unsigned char*)&v_slice[i][0];
		naluUnit.size = v_slice[i].size();
		naluUnit.type = naluUnit.data[0]&0x1f;  
		bool bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;  
		// 发送H264数据帧   
		//rtmpSender.SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick);  
		rtmpSender.send_v((char*)naluUnit.data,naluUnit.size,bKeyframe,tick);
		//msleep(40);  
		tick +=46;  


		{
			vector<char> v_aac;
			static int aac_tick = 0;

			//get_aac(v_aac);
			o_aac.pcm2aac(&va_slice[i][0],o_aac.get_input_samples(),v_aac);
			if (v_aac.size()>0)
			{
				rtmpSender.send_a((&v_aac[0])+7,v_aac.size()-7,aac_tick);
				aac_tick += 46;//aac每次1024个采样，在22050hz下需要46ms，上面帧率跟这个也有关
			}

		}
	}  




int a = 0;
a++;
}