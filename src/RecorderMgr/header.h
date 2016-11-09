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
#include <stdint.h>
#include "../log4z.h"

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

//static string get_error_text(const int error)
//{
//    char error_buffer[255];
//    av_strerror(error, error_buffer, sizeof(error_buffer));
//    return error_buffer;
//}

class Packet
{
public:
	Packet():m_type(mediaUnknow)
	{
		av_init_packet(&m_pkt);
		m_pkt.data = NULL;
		m_pkt.size = 0;
	}
	~Packet() { av_free_packet(&m_pkt); }

	AVPacket* c_pkt(void) { return &m_pkt; }

	void setStreamIndex(int index) { m_pkt.stream_index = index; }
	int getStreamIndex(void) { return m_pkt.stream_index; }
	MediaType getMediaType() { return m_type; }
	void setMediaType(MediaType type) { m_type = type; }
	void updateTs(int64_t ts) { m_pkt.pts = m_pkt.dts = ts; }
	int64_t pts() { return m_pkt.pts; }
	int64_t dts() { return m_pkt.dts; }
	void rescale_ts(AVRational src, AVRational dst)
	{
		m_pkt.pts = av_rescale_q(m_pkt.pts, src, dst);
		m_pkt.dts = av_rescale_q(m_pkt.dts, src, dst);
	}

	void debugTs(const char* tag, MediaType type)
	{
		if (type == m_type)
			LOGFMTI("[%s] %s: stream_index = %d, pts = %lli, dts = %lli, size = %d\n",
			tag, type == mediaAudio ? "audio" : "video", m_pkt.stream_index, m_pkt.pts, m_pkt.dts, m_pkt.size);
	}

	void copy(Packet* pkt)
	{
		av_copy_packet(&m_pkt, pkt->c_pkt());
		m_type = pkt->getMediaType();
	}
private:
	MediaType m_type;
	AVPacket m_pkt;
};

class Frame
{
public:
	Frame() : m_type(mediaUnknow)
	{
		m_frame = av_frame_alloc();
		assert(m_frame);
	}
	~Frame() { free(); }

	void free() { if (m_frame) { av_frame_free(&m_frame); } }

	AVFrame* c_frame(void) { return m_frame; }

	bool allocBuffer(int nb_samples, int channel_layout, AVSampleFormat sample_fmt, int sample_rate)
	{
		m_frame->nb_samples = nb_samples;
		m_frame->channel_layout = channel_layout;
		m_frame->format = sample_fmt;
		m_frame->sample_rate = sample_rate;
		m_frame->channels = av_get_channel_layout_nb_channels(channel_layout);

		/**
		* Allocate the samples of the created frame. This call will make
		* sure that the audio frame can hold as many samples as specified.
		*/
		if (av_frame_get_buffer(m_frame, 0) < 0) {
			av_log(NULL, AV_LOG_ERROR, "allocate audio frame buffer failed\n");
			return false;
		}

		return true;
	}

	void debugAudio()
	{
		LOGFMTI("nb_samples:%d, sample_rate:%d, format:%d, channel_layout:%0x, channels:%d\n",
			m_frame->nb_samples, m_frame->sample_rate, m_frame->format, m_frame->channel_layout, m_frame->channels);
	}

	bool allocBuffer(enum AVPixelFormat pix_fmt, int width, int height)
	{
		m_frame->width = width;
		m_frame->height = height;
		m_frame->format = (AVSampleFormat)pix_fmt;

		if (av_frame_get_buffer(m_frame, 32) < 0) {
			av_log(NULL, AV_LOG_ERROR, "allocate video frame buffer failed\n");
			return false;
		}

		return true;
	}

	void melanism()
	{
		for (int plane = 0; plane < 3; plane++) {
			uint8_t* pdata = m_frame->data[plane];
			int linesize = m_frame->linesize[plane];
			int height = m_frame->height;
			if (plane > 0) {
				height = height / 2;
				memset(pdata, 128, linesize * height);
				//break;
			} else {
				memset(pdata, 16, linesize * height);
			}
		}
	}


	MediaType getMediaType() { return m_type; }
	void setMediaType(MediaType type) { m_type = type; }

	void setFramePts(int64_t ts)
	{
		m_frame->pts = ts;
	}

	void copy(Frame* f)
	{
		av_frame_copy(m_frame, f->c_frame());
		m_type = f->getMediaType();
	}
private:
	MediaType m_type;
	AVFrame* m_frame;
};

class AudioFifo
{
public:
	AudioFifo() :m_af(NULL), m_isAllocated(false) {}
	~AudioFifo()
	{
		if (m_af)
		{ av_audio_fifo_free(m_af); }
	}

	bool hasAllocated() { return m_isAllocated; }
	bool alloc(enum AVSampleFormat sample_fmt, int channels, int nb_samples)
	{
		m_af = av_audio_fifo_alloc(sample_fmt, channels, nb_samples);
		if (m_af) {
			m_isAllocated = true;
			return true;
		} else {
			return false;
		}
	}

	bool realloc(int nb_sample)
	{
		return !av_audio_fifo_realloc(m_af, nb_sample);
	}

	int read(void** data, int nb_sample)
	{
		return av_audio_fifo_read(m_af, data, nb_sample);
	}

	int write(void** data, int nb_sample)
	{
		return av_audio_fifo_write(m_af, data, nb_sample);
	}

	int size()
	{
		return av_audio_fifo_size(m_af);
	}

	int space()
	{
		return av_audio_fifo_space(m_af);
	}

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
