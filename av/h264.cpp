
#include <iostream>
#include <vector>
using namespace std;

#include "h264.h"

 

//void h264_vec(std::vector<vector<char>> &v_slice)
//{
//	char *pfile = "test.264";
//	FILE *fp = NULL;
//	static unsigned char h264_data[1024*1024*200]={0};
//	fp = fopen(pfile, "rb");
//	if (fp == NULL){
//		cout << "open "<<pfile<<" err\n";
//		return;
//	}
//	int m_nFileBufSize;
//	m_nFileBufSize = fread(h264_data,1,sizeof(h264_data),fp);
//	fclose(fp);
//
//	NaluUnit nalu;
//	unsigned char *m_pFileBuf = h264_data;
//	int m_nCurPos = 0;
//	int i = m_nCurPos;
//
//	while(i<(int)m_nFileBufSize)
//	{
//		while(i<(int)m_nFileBufSize){
//			if(m_pFileBuf[i++] == 0x00 &&  
//				m_pFileBuf[i++] == 0x00 &&  
//				m_pFileBuf[i++] == 0x00 &&  
//				m_pFileBuf[i++] == 0x01  
//				)  
//			{  
//				int pos = i;  
//				while (pos<(int)m_nFileBufSize)  
//				{  
//					if(m_pFileBuf[pos++] == 0x00 &&  
//						m_pFileBuf[pos++] == 0x00 &&  
//						m_pFileBuf[pos++] == 0x00 &&  
//						m_pFileBuf[pos++] == 0x01  
//						)  
//					{  
//						break;  
//					}  
//				}  
//				if(pos == m_nFileBufSize)  
//				{  
//					nalu.size = pos-i;    
//				}  
//				else  
//				{  
//					nalu.size = (pos-4)-i;  
//				}  
//				nalu.type = m_pFileBuf[i]&0x1f;  
//				nalu.data = &m_pFileBuf[i];  
//
//				m_nCurPos = pos-4;  
//				//return TRUE;
//
//				char *pnalu = (char*)&m_pFileBuf[i];
//				vector<char> vtem(pnalu,pnalu+nalu.size);
//				
//				v_slice.push_back(vtem);
//
//				int a = 0;
//				a ++;
//			}  
//		}
//	}
//}


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

	//NaluUnit nalu;
	unsigned char *m_pFileBuf = h264_data;

	char *p_b=(char*)h264_data;
	char *p_e=(char*)h264_data+m_nFileBufSize;
	char *p_nb=p_b;
	char *p_ne = p_b;

	while(p_nb<p_e && p_ne<p_e){
		if (*p_nb++==0&&*p_nb++==0&&*p_nb++==0&&*p_nb++==1)
		{
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