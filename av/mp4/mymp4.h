#ifndef __MYMP4_H__
#define __MYMP4_H__

class mymp4
{
public:
	mymp4(void);
	~mymp4(void);
	bool init(const char * pfile);
	void deinit(void);

	//audio
	bool init_a(int nSampleRate, int m_nInputSamples, unsigned char* faacDecoderInfo, int faacDecoderInfoSize);
	void deinit_a(void);
	bool write_a(const char *pd, int size);

	//video
	bool init_v();
	void deinit_v(void);

private:

	void* m_Mp4File;

	//audio
	int m_AudioTrackId;//MP4TrackId 
};

#endif