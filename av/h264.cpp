
#include <iostream>
#include <vector>
using namespace std;

#include "h264.h"

void h264data2vec(char*pd, int size, std::vector<vector<char>> &v_slice)
{
	char *p_b=(char*)pd;
	char *p_e=(char*)pd+size;
	char *p_nb=p_b;
	char *p_ne = p_b;

	while(p_nb<p_e){
		if (*p_nb++==0&&*p_nb++==0&&*p_nb++==0&&*p_nb++==1){
			p_ne = p_nb;
			while(p_ne<p_e){
				if (*p_ne++==0&&*p_ne++==0&&*p_ne++==0&&*p_ne++==1){
					p_ne -=4;
					break;
				}
			}

			vector<char> v_tem(p_nb,p_ne);
			v_slice.push_back(v_tem);
			p_nb = p_ne;
		}
	}
}

void h264_vec(std::vector<vector<char>> &v_slice)
{
	char *pfile = "test.264";
	FILE *fp = NULL;
	static unsigned char h264_data[1024*1024*200]={0};
	fp = fopen(pfile, "rb");
	if (fp == NULL){
		cout << "open "<<pfile<<" err\n";
		return;
	}
	int m_nFileBufSize;
	m_nFileBufSize = fread(h264_data,1,sizeof(h264_data),fp);
	fclose(fp);

	return h264data2vec((char*)h264_data,m_nFileBufSize,v_slice);
}