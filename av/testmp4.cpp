
#include <iostream>
#include <vector>

using namespace std;

#include "myaac.h"
#include "mymp4.h"

void wav_vec(int size, std::vector<vector<char>> &v_slice);

////mp4 封装aac
static void amp4(void)
{
	myaac o_aac;
	o_aac.init(22050,1,16);

	mymp4 o_mp4;
	o_mp4.init("a.mp4");
	o_mp4.init_a(o_aac.m_hz,o_aac.m_nInputSamples,o_aac.faacDecoderInfo,o_aac.faacDecoderInfoSize);

	vector<vector<char>> v_slice;
	wav_vec(o_aac.m_nInputSamples,v_slice);

	for(int i = 0; i< (int)v_slice.size(); i++)
	{
		vector<char> v_aac;
		//o_aac.pcm2aac(&v_slice[i][0],v_aac);
		o_aac.pcm2aac(&v_slice[i][0],o_aac.m_nInputSamples,v_aac);
		if (v_aac.size())
		{
			o_mp4.write_a(&v_aac[0+7],v_aac.size()-7);
		}
	}

	////清理faac缓存，发现清理过后faac就不能正常工作了
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



void tst_mp4(void)
{


	amp4();



int a = 0;
a++;
}