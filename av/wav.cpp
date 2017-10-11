
#include <iostream>
#include <vector>
using namespace std;


////wo.wav 22050hz  2Channal  16bitsPerSample
/// ×ª»¯Îª 22050hz 1Channal  16bitsPerSample
void wav_vec(int samples, std::vector<vector<char>> &v_slice)
{
	char *pfile = "wo.wav";
	FILE *fp = NULL;
	static unsigned char wav_data[1024*1024*200]={0};
	fp = fopen(pfile, "rb");
	if (fp == NULL){
		cout << "open "<<pfile<<" err\n";
		//return -1;
	}
	int read_size;
	read_size = fread(wav_data,1,sizeof(wav_data),fp);
	fclose(fp);

	int idx = 0x50;
	while(1)
	{
		if (idx >= read_size)
		{
			break;
		}
		vector<char> vtem;
		int ifr;
		for (ifr = 0; ifr <samples*2; ifr+=2)
		{
			vtem.push_back(*(wav_data+idx + ifr*2));
			vtem.push_back(*(wav_data+idx + ifr*2+1));
		}
		v_slice.push_back(vtem);
		idx += samples*2*2;
	}
}