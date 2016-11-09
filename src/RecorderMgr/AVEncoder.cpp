#include "AVEncoder.h"


AVEncoder::AVEncoder(void):m_stat(false), m_audioCodecCtx(NULL), m_videoCodecCtx(NULL)
{
}

bool AVEncoder::start( EncoderParam& param, OnEncodedPacketCallback callback, void* arg)
{
	m_encoderParam = param;
	m_callback.OnEncodedPacket = callback;
	m_callback.arg = arg;

	if (!OpenCodec()) {
		LOGE("Open codec error.");
		return false;
	}
	
	//m_frames.clear();
	return SimpleThread::start();
}

bool AVEncoder::OpenCodec()
{
	return OpenAudio() && OpenVideo();
}

bool AVEncoder::OpenAudio()
{
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!codec) {
		LOGE("find audio codec failed\n");
		return false;
	}

	m_audioCodecCtx = avcodec_alloc_context3(codec);
	if (!m_audioCodecCtx) {
		LOGE("open audio codec context failed\n");
		return false;
	}

	m_audioCodecCtx->bit_rate 		= m_encoderParam.audio_bitate * 1000;
	m_audioCodecCtx->sample_fmt 	= AV_SAMPLE_FMT_S16;
	m_audioCodecCtx->sample_rate    = m_encoderParam.sample_rate;
	m_audioCodecCtx->channels       = m_encoderParam.channel;
	m_audioCodecCtx->channel_layout = av_get_default_channel_layout(m_encoderParam.channel);

	m_audioCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	if (avcodec_open2(m_audioCodecCtx, codec, NULL /*&options*/)< 0) {
		LOGE("open audio codec failed\n");
		return false;
	}

	return true;
}

bool AVEncoder::OpenVideo()
{
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		LOGE("find video codec failed\n");
		return false;
	}


	m_videoCodecCtx = avcodec_alloc_context3(codec);
	if (!m_videoCodecCtx) {
		LOGE("open video codec context failed\n");
		return false;
	}

	AVDictionary *options = NULL;

	m_videoCodecCtx->bit_rate = m_encoderParam.video_bitrate * 1000;
	m_videoCodecCtx->width 	= m_encoderParam.w;
	m_videoCodecCtx->height = m_encoderParam.h;
	AVRational rational = {1, 15};
	m_videoCodecCtx->time_base = rational; // fps

	m_videoCodecCtx->gop_size = 30;
	//m_videoCodecCtx->max_b_frames = 1;
	m_videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	av_dict_set(&options, "preset", "veryfast", 0);
	av_dict_set(&options, "profile", "baseline", 0);
	av_dict_set(&options, "tune", "zerolatency", 0);

	m_videoCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	if (avcodec_open2(m_videoCodecCtx, codec, &options)< 0) {
		LOGE("open video codec failed\n");
		return false;
	}
	return true;
}

bool AVEncoder::EncodeFrame( Frame* frame, Packet* pkt )
{
	const AVFrame* c_frame = frame->c_frame();
	AVPacket* c_pkt = pkt->c_pkt();

	int ret, got_packet_ptr;
	if (frame->getMediaType() == mediaVideo) {
		ret = avcodec_encode_video2(m_videoCodecCtx, c_pkt, c_frame, &got_packet_ptr);
		pkt->setMediaType(mediaVideo);
		pkt->setStreamIndex(0);
	} else if(frame->getMediaType() == mediaAudio) {
		ret = avcodec_encode_audio2(m_audioCodecCtx, c_pkt, c_frame, &got_packet_ptr);
		pkt->setMediaType(mediaAudio);
		pkt->setStreamIndex(1);
	} else {
		LOGE("unknow type frame when encode frame\n");
		return false;
	}

	if (ret < 0) {
		LOGFMTE("encode %s frame error\n",
			frame->getMediaType() == mediaVideo ? "video":"audio");
		return false;
	}

	if (got_packet_ptr == 0) {
		LOGE("after encode, the packet is empty\n");
		return false;
	}

	return true;
}


void AVEncoder::run()
{
	m_stat = true;
	while(1) {
		Frame* frame = m_frames.popWithCond();
		if (!frame) {
			LOGW("Encoder thread end!");
			break;
		}

		Packet* pkt = new Packet;
		if (EncodeFrame(frame, pkt)) {

#if 0
			AVPacket* c_pkt = pkt->c_pkt();
			static int64_t last_pts[2] = {0};
			LOGI("encodeFrame_pts: {stream_index, pts, dts, diff} = {%d, %lli, %lli, %lli}\n", c_pkt->stream_index, c_pkt->pts, c_pkt->dts, c_pkt->pts - last_pts[c_pkt->stream_index]);
			last_pts[c_pkt->stream_index] = c_pkt->pts;
#endif
			m_callback.OnEncodedPacket(pkt, m_callback.arg);

		} else {
			delete pkt;
		}

		delete frame;
	}
}

bool AVEncoder::stop()
{
	m_stat = false;
	m_frames.pushWithSignal(NULL);
	return true;
}

void AVEncoder::PushFrame( Frame* f )
{
	if (m_stat) m_frames.pushWithSignal(f);
}

AVCodecContext* AVEncoder::GetAudioCodecCtx()
{
	return m_audioCodecCtx;
}

AVCodecContext* AVEncoder::GetVideoCodecCtx()
{
	return m_videoCodecCtx;
}
