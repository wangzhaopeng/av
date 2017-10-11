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
	bool init_a(int hz, int m_input_samples, const unsigned char* aac_decoder_info, int aac_decoder_info_size);
	void deinit_a(void);
	bool write_a(const char *pd, int size);

	//video
	bool init_v();
	void deinit_v(void);

private:
	void* m_h_mp4;

	//audio
	unsigned int m_a_track_id; //MP4TrackId use unsigned int

	//video
};

#endif