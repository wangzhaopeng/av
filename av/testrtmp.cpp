
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

#include <myaac.h>
#include "mymp4.h"
#include "myrtmp.h"

#include <windows.h>

void wav_vec(int size, std::vector<vector<char>> &v_slice);

void h264_vec(std::vector<vector<char>> &v_slice);


static string hex2str(const char*p,int size)
{
	ostringstream oss;
	oss << "size "<<size<<":\t\t";
	string str_hex;

	str_hex += oss.str();

	const int buf_size = size*8;
	char *char_buf = new char[buf_size+8]();
	int idx = 0;
	for(int i = 0; i < size; i++){
		//idx += sprintf(char_buf+idx,"0x%02x,",(unsigned char)*(p+i));/////有警告
		//idx += sprintf_s(char_buf+idx,buf_size-idx,"0x%02x,",(unsigned char)*(p+i));/////太慢
		unsigned char tem_v = (unsigned char)*(p+i);
		char_buf[idx++] = '0';
		char_buf[idx++] = 'x';
		if ((tem_v>>4)<10){
			char_buf[idx++] = (tem_v>>4)+'0';
		}else{
			char_buf[idx++] = (tem_v>>4)-10+'a';
		}
		if ((tem_v&0xf)<10){
			char_buf[idx++] = (tem_v&0xf)+'0';
		}else{
			char_buf[idx++] = (tem_v&0xf)-10+'a';
		}
		
		char_buf[idx++] = ',';
	}

	str_hex = str_hex+char_buf+"\n";
	delete[] char_buf;
	return str_hex;
}

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

	string str_send_hex;
	str_send_hex += hex2str(&v_slice[0][0],v_slice[0].size());
	str_send_hex += hex2str(&v_slice[1][0],v_slice[1].size());
	str_send_hex += hex2str(&(o_aac.get_decoder_info()[0]),(o_aac.get_decoder_info().size()));

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
			bret = rtmp_sender.send_v(&v_slice[idx][0],v_slice[idx].size(),bKeyframe,GetTickCount());
			if (!bret){
				cout<<"rtmp_sender.send_v false"<<endl;
			}
			cout << "rtmp_sender.send_v ms "<<GetTickCount()-ms_bsendv<<" size "<<v_slice[idx].size() <<endl;

			str_send_hex += hex2str(&v_slice[idx][0],v_slice[idx].size());

			a_f++;
			if (a_f>=(int)v_slice.size()){//////是否无限循环
				break;
			}
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
				cout << "rtmp_sender.send_a ms "<<GetTickCount()-ms_bsenda<<" size "<<v_aac.size()-7 <<endl;

				str_send_hex += hex2str((&v_aac[0])+7,v_aac.size()-7);
			}

			v_f++;
			if (v_f>=(int)va_slice.size()){//////是否无限循环
				break;
			}
		}
		Sleep(1);
	}

	{
		ofstream ofile;
		ofile.open("send_rtmp_hex.txt");
		ofile<<str_send_hex;
		ofile.close();
	}

	return;
}

static void tst_rcv(void)
{
	cout << "tst_rtmp rcv"<<endl;

	ofstream of_flv;
	of_flv.open("rtmp_rcv.flv",ios::binary);

	mymp4 o_mp4;
	o_mp4.init("rtmp_rcv.mp4");
	int aac_info_flag = 0;
	int sps_pps_flag = 0;

	myrtmp rtmp_rcv;
	bool bret = rtmp_rcv.init_rcv("rtmp://192.168.5.116/live/zb");
	//bool bret = rtmp_rcv.init_rcv("rtmp://live.hkstv.hk.lxdns.com/live/hks");
	if (!bret){
		cout<<"rtmp_rcv.init_rcv false"<<endl;
		return;
	}

	string str_rcv_hex;

	while(1){
		int r_size;
		char *p_buf;
		r_size = rtmp_rcv.rcv(&p_buf);

		if (r_size == 0){
			break;
		}
		cout << "rcv size "<<r_size<<endl;

		of_flv.write(p_buf,r_size);

		str_rcv_hex += hex2str(p_buf,r_size);

		{
			/////把收到的flv数据转为h264 aac 存为mp4
			s_rcv_data rcv_data;
			rtmp_rcv.flv2mp4(p_buf,r_size,&rcv_data);
			if (rcv_data.p_aac_info&&aac_info_flag==0){
				aac_info_flag = 1;
				////还不知道怎么从aac中得到 下面的22050 1024
				o_mp4.init_a(22050,1024,
					vector<char>(rcv_data.p_aac_info,rcv_data.p_aac_info+rcv_data.aac_info_size));
			}
			if (rcv_data.p_pps&&rcv_data.p_sps,sps_pps_flag==0){
				sps_pps_flag = 1;
				//////从sps中能得到视频分辨率，帧率不知怎么得到
				o_mp4.init_v(1280,720,30,
					vector<char>(rcv_data.p_sps,rcv_data.p_sps+rcv_data.sps_size),
					vector<char>(rcv_data.p_pps,rcv_data.p_pps+rcv_data.pps_size));
			}
			if (aac_info_flag&&rcv_data.p_aac){
				o_mp4.write_a(rcv_data.p_aac, rcv_data.aac_size);
			}
			if (sps_pps_flag&&rcv_data.p_h264){
				o_mp4.write_v(rcv_data.p_h264, rcv_data.h264_size);
			}
		}
	}

	{
		ofstream ofile;
		ofile.open("rcv_rtmp_hex.txt");
		ofile<<str_rcv_hex;
		//ofile.close();
	}

	return;
}


void tst_rtmp(void)
{
	//tst_send();

	tst_rcv();
}
