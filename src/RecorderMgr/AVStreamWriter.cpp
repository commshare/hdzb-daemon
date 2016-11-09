#include "AVStreamWriter.h"
#include "AVEncoder.h"


AVStreamWriter::AVStreamWriter(void)
{
	m_videoStream = NULL;
	m_audioStream = NULL;
	m_fmtCtx = NULL;
	m_encoder = NULL;
}

bool AVStreamWriter::OpenStream()
{
	int error = avformat_alloc_output_context2(&m_fmtCtx, NULL, "flv", m_url.c_str());
	if (error < 0) {
		LOGFMTE("alloc %s failed\n", m_url.c_str());
		return false;
	}


	m_videoStream = avformat_new_stream(m_fmtCtx, NULL /*istream->codec->codec*/);
	m_audioStream = avformat_new_stream(m_fmtCtx, NULL /*istream->codec->codec*/);
	if (!m_audioStream || !m_videoStream) {
		LOGFMTE("[%s] add stream failed\n", m_url.c_str());
		return false;
	}

	AVCodecContext* videoCodecCtx = m_encoder->GetVideoCodecCtx();
	avcodec_copy_context(m_videoStream->codec, videoCodecCtx);
	m_videoStream->time_base = videoCodecCtx->time_base;

	AVCodecContext* audioCodecCtx = m_encoder->GetAudioCodecCtx();
	avcodec_copy_context(m_audioStream->codec, audioCodecCtx);
	m_audioStream->time_base = audioCodecCtx->time_base;

	if (m_fmtCtx->oformat->flags & AVFMT_GLOBALHEADER) {
		m_videoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		m_audioStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}


	if ((error = avio_open(&m_fmtCtx->pb, m_url.c_str(), AVIO_FLAG_WRITE)) < 0) {
		LOGFMTE("[%s] avio_open failed\n", m_url.c_str());
		return false;
	}

	if ((error = avformat_write_header(m_fmtCtx, NULL)) < 0) {
		LOGFMTE("[%s] write header failed!\n", m_url.c_str()/*, get_error_text(error).c_str()*/);
		return false;
	}

	av_dump_format(m_fmtCtx, 0, m_url.c_str(), 1);
	return true;
}

bool AVStreamWriter::CloseStream()
{
	if (m_fmtCtx) {
		if (m_fmtCtx->pb) avio_close(m_fmtCtx->pb);
		avformat_free_context(m_fmtCtx);
		m_fmtCtx = NULL;
	}
	return true;
}

void AVStreamWriter::run()
{
	int ret = false;
	while(1) {
		while(ret) {
			CloseStream();
			ret = OpenStream();
			LOGE("send packet failed, reopen after 3s\n");
			m_pkts.clear();
		}

		Packet* pkt = m_pkts.popWithCond();
		if (!pkt) {
			LOGFMTW("%s stream end.", m_url.c_str());
			break;
		}

		ret = SendPacket(pkt);
		delete pkt;
	}
	CloseStream();
}

bool AVStreamWriter::SendPacket( Packet* pkt )
{
	if (pkt->getMediaType() == mediaAudio) {
		pkt->setStreamIndex(1);
	} else if (pkt->getMediaType() == mediaVideo) {
		pkt->setStreamIndex(0);
	} else {
		LOGE("unknow packet when output!!\n");
	}

#if 0
	AVPacket* c_pkt = pkt->c_pkt();
	static int64_t last_pts[2] = {0};
	LOGI("writeFrame: {stream_index, pts, dts, diff} = {%d, %lli, %lli, %lli}\n", c_pkt->stream_index, c_pkt->pts, c_pkt->dts, c_pkt->pts - last_pts[c_pkt->stream_index]);
	last_pts[c_pkt->stream_index] = c_pkt->pts;
#endif

	if(av_interleaved_write_frame(m_fmtCtx, pkt->c_pkt()) < 0) {
		LOGFMTE("[%s] write frame error!!\n", m_url.c_str());
		return false;
	}

	return true;
}
