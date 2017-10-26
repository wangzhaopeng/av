
#include <iostream>
#include <vector>

using namespace std;

#include <myaac.h>

#include "myrtmp.h"

#include <windows.h>

void wav_vec(int size, std::vector<vector<char>> &v_slice);

void h264_vec(std::vector<vector<char>> &v_slice);


static void tst_send(void)
{
	vector<vector<char>> v_slice;
	h264_vec(v_slice);

	int a_hz=22050;
	myaac o_aac;
	o_aac.init(a_hz,1,16);
	vector<vector<char>> va_slice;
	wav_vec(o_aac.get_input_samples(),va_slice);
	double aac_slice_ms = (double)o_aac.get_input_samples()*1000/a_hz;
	
	cout << "tst_rtmp "<<endl;
	myrtmp rtmp_sender;
	bool bret = rtmp_sender.init_send("rtmp://192.168.5.116/live/zb");
	if (!bret){
		return;
	}

	////发送sps pps
	rtmp_sender.init_v(v_slice[0],v_slice[1]);  
	////发送aac decoder info
	rtmp_sender.init_a(o_aac.get_decoder_info());

	double h264_slice_ms = 1000/30.0;/////用的30帧的视频

	int a_f = 0;
	int v_f = 0;
	DWORD ms_b = GetTickCount();
	while(1){
		if (GetTickCount()-ms_b>=a_f*h264_slice_ms){
			int idx = a_f%(v_slice.size()-2)+2;/////2的作用是为了跳过开始的 sps pps
			//int idx = a_f%(300)+2;/////每秒30帧，300帧过后又是I帧 感觉这样效果更好
			int nalu_type = v_slice[idx][0]&0x1F;
			bool bKeyframe  = (nalu_type == 0x05) ? 1 : 0;
			DWORD ms_bsendv = GetTickCount();
			bret = rtmp_sender.send_v((char*)&v_slice[idx][0],v_slice[idx].size(),bKeyframe,GetTickCount());
			if (!bret){
				cout<<"rtmp_sender.send_v false"<<endl;
			}
			//cout << "rtmp_sender.send_v ms "<<GetTickCount()-ms_bsendv<<" size "<<v_slice[idx].size() <<endl;
			a_f++;
		}

		if (GetTickCount()-ms_b>=v_f*aac_slice_ms){
			int idx = v_f%(va_slice.size());
			vector<char> v_aac;
			DWORD ms_pcm2aac = GetTickCount();
			o_aac.pcm2aac(&va_slice[idx][0],o_aac.get_input_samples(),v_aac);
			//cout << "o_aac.pcm2aac ms "<<GetTickCount()-ms_pcm2aac<<" size "<<va_slice[idx].size() <<endl;
			if (v_aac.size()>0){
				DWORD ms_bsenda = GetTickCount();
				bret = rtmp_sender.send_a((&v_aac[0])+7,v_aac.size()-7,GetTickCount());
				if (!bret){
					cout<<"rtmp_sender.send_a false"<<endl;
				}
				//cout << "rtmp_sender.send_a ms "<<GetTickCount()-ms_bsenda<<" size "<<v_aac.size() <<endl;
			}
			v_f++;

			if (v_f>=(int)va_slice.size()){//////是否无限循环
				break;
			}
		}
		Sleep(1);
	}

	return;
}

static void tst_rcv(void)
{
	char *pfile = "rcv_flv.mp4";////这个应该命名为.flv，只是git忽略 .MP4就暂时命名为mp4了
	FILE *fp = NULL;
	errno_t err;

	err = fopen_s(&fp, pfile, "wb");
	if(err != 0){
		cout << "open "<<pfile<<" err\n";
		return;
	}

	cout << "tst_rtmp rcv"<<endl;
	myrtmp rtmp_rcv;
	bool bret = rtmp_rcv.init_rcv("rtmp://192.168.5.116/live/zb");
	//bool bret = rtmp_rcv.init_rcv("rtmp://live.hkstv.hk.lxdns.com/live/hks");
	if (!bret){
		cout<<"rtmp_rcv.init_rcv false"<<endl;
		fclose(fp);
		return;
	}
	
	const int buf_size = 1024*1024*5;
	char *p_buf = new char[buf_size];
	while(1){
		int r_size;
		r_size = rtmp_rcv.rcv(p_buf,buf_size-8);
		if (r_size == 0){
			break;
		}
		fwrite(p_buf,1,r_size,fp);
	}


	delete[] p_buf;


	fclose(fp);
}

void tst_rtmp(void)
{
	tst_send();

	//tst_rcv();
}