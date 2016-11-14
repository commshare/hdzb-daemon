/*
* header.h
*
*  Created on: 2014��10��8��
*      Author: zz
*/

#ifndef HEADER_H_
#define HEADER_H_

//#define INT64_C
//#define __STDC_CONSTANT_MACROS
#include "../log4z.h"
#include <stdint.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/rational.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
//#include <libavfilter/avfilter.h>
//#include <libavfilter/buffersrc.h>
//#include <libavfilter/buffersink.h>
//#include <libavfilter/avfiltergraph.h>
}

#include <cstdlib>
#include <queue>
#include <list>
#include <iostream>
#include <string>
#include <map>
#include <assert.h>



enum MediaType { mediaAudio, mediaVideo, mediaUnknow };

std::string get_error_text(const int error);

class Packet
{
public:
	Packet();
	~Packet();
	AVPacket* c_pkt(void);// { return &m_pkt; }

	void setStreamIndex(int index);
	int getStreamIndex(void);
	MediaType getMediaType();
	void setMediaType(MediaType type);
	void updateTs(int64_t ts);
	int64_t pts();
	int64_t dts();
	void rescale_ts(AVRational src, AVRational dst);
	void debugTs(const char* tag, MediaType type);
	void copy(Packet* pkt);

private:
	MediaType m_type;
	AVPacket m_pkt;
};



class Frame
{
public:
	Frame();
	~Frame();

	void free();

	AVFrame* c_frame(void);

	bool allocBuffer(int nb_samples, int channel_layout, AVSampleFormat sample_fmt, int sample_rate);

	void debugAudio();

	bool allocBuffer(enum AVPixelFormat pix_fmt, int width, int height);

	void melanism();

	MediaType getMediaType();
	void setMediaType(MediaType type);

	void setFramePts(int64_t ts);

	void copy(Frame* f);
private:
	MediaType m_type;
	AVFrame* m_frame;
};

class AudioFifo
{
public:
	AudioFifo();
	~AudioFifo();


	bool hasAllocated();
	bool alloc(enum AVSampleFormat sample_fmt, int channels, int nb_samples);

	bool realloc(int nb_sample);

	int read(void** data, int nb_sample);

	int write(void** data, int nb_sample);

	int size();

	int space();

private:
	AVAudioFifo* m_af;
public:
	bool m_isAllocated;
};

//class Mutex{
//public:
//	Mutex() :m_mutex(PTHREAD_MUTEX_INITIALIZER){}
//	~Mutex(){ pthread_mutex_destroy(&m_mutex); }
//
//	void lock(void){ pthread_mutex_lock(&m_mutex); }
//
//	void unlock(void){ pthread_mutex_unlock(&m_mutex); }
//private:
//	pthread_mutex_t m_mutex;
//};


#endif /* HEADER_H_ */
