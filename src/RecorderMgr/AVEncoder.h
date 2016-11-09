#pragma once
#include "../Util.h"
#include "header.h"

struct EncoderParam {
	int w, h, pic_format, video_bitrate;
	int sample_rate, channel, format, audio_bitate;
};



class AVEncoder: public SimpleThread
{
public:
	AVEncoder(void);

	typedef void (*OnEncodedPacketCallback)(Packet* pkt, void* arg);

	bool start(EncoderParam& param, OnEncodedPacketCallback, void* arg);
	bool stop();

	void PushFrame(Frame* f);

	AVCodecContext* GetAudioCodecCtx();
	AVCodecContext* GetVideoCodecCtx();
private:
	bool OpenCodec();
	bool OpenAudio();
	bool OpenVideo();

	bool EncodeFrame(Frame* frame, Packet* pkt);
private:
	virtual void run();

	bool m_stat;
	AVCodecContext* m_audioCodecCtx;
	AVCodecContext* m_videoCodecCtx;
	SimpleQueue<Frame*> m_frames;
	EncoderParam m_encoderParam;

	struct EncodedCallback {
		OnEncodedPacketCallback OnEncodedPacket;
		void* arg;
	} m_callback;
};

