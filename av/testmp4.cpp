
#include <iostream>
#include <vector>

using namespace std;

#include "myaac.h"
#include "mymp4.h"

void wav_vec(int size, std::vector<vector<char>> &v_slice);

void h264_vec(std::vector<vector<char>> &v_slice);


void tst_mp4(void)
{
	mymp4 o_mp4;
	o_mp4.init("av.mp4");

	////�����Ƶ����
	{
		vector<vector<char>> v_slice;
		h264_vec(v_slice);

		////��֪��h264���� 1280*720 30fps
		o_mp4.init_v(1280,720,30,v_slice[0],v_slice[1]);
		for(int i = 2; i < (int)v_slice.size(); i++)
		{
			o_mp4.write_v(&v_slice[i][0], v_slice[i].size());
		}
	}

	////�����Ƶ����
	{
		int a_hz=22050;
		myaac o_aac(a_hz,1,16);
		o_aac.init();

		vector<vector<char>> v_slice;
		wav_vec(o_aac.get_input_samples(),v_slice);

		o_mp4.init_a(a_hz,o_aac.get_input_samples(),o_aac.get_decoder_info());

		for(int i = 0; i< (int)v_slice.size(); i++)
		{
			vector<char> v_aac;
			o_aac.pcm2aac(&v_slice[i][0],o_aac.get_input_samples(),v_aac);
			if (v_aac.size())
			{
				o_mp4.write_a(&v_aac[0+7],v_aac.size()-7);
			}
		}


		////����faac���棬�����������faac�Ͳ�������������
		//while(1)
		//{
		//	vector<char> v_aac;
		//	o_aac.pcm2aac(&v_slice[0][0],0,v_aac);
		//	if (v_aac.size())
		//	{
		//		o_mp4.write_a(&v_aac[0+7],v_aac.size()-7);
		//	}else
		//	{
		//		break;
		//	}
		//}
	}



int a = 0;
a++;
}