#define DLL_IMPLEMENT   

#include "RecoderMgr.h"
#include <tchar.h>

static const int max_person_size = 4;

static void copyVideoFrame(VideoFrame* src, VideoFrame* dst)
{
	uint8* pData = dst->data;
	*dst = *src;
	dst->data = pData;
	memcpy(dst->data, src->data, dst->data_size);
}


static void _mix16(const int16_t* pInBufferPtr, float dInBufferVolume, int16_t* pOutBufferPtr, size_t nSizeToMixInBytes)
{
	register uint32_t i;
	float mixedSample;
	size_t nSizeToMixInSamples = (nSizeToMixInBytes >>1);
	for(i = 0; i<nSizeToMixInSamples; ++i)
	{
		mixedSample = ((float)pOutBufferPtr[i]/32768.0f) + (((float)pInBufferPtr[i]/32768.0f) * dInBufferVolume);
		if(mixedSample < -1.0f) mixedSample=-1.0f;
		if(mixedSample > +1.0f) mixedSample=+1.0f;
		pOutBufferPtr[i] = (int16_t)(mixedSample  * 32768.0f);
	}
}


RecoderMgr::RecoderMgr(void)
{
	m_picMode = 0;
	m_recordStat = false;
	m_rawVideoFrames.resize(max_person_size);
	m_rawAudioFifo.resize(max_person_size);

	//HMODULE hMoude = ::LoadLibrary(_T("avcodec-56.dll"));
	//hMoude = ::LoadLibrary(_T("avformat-56.dll"));
	//hMoude = ::LoadLibrary(_T("avutil-54.dll"));
	//hMoude = ::LoadLibrary(_T("swscale-3.dll"));

}

void RecoderMgr::OnRemoteVideoFrame( VideoFrame *pFrameData )
{
	string& id = pFrameData->identifier;

	int index = m_id2index.Get(id);

	if (index < 1 || index >= m_rawVideoFrames.size())
		return;
	
	if (!m_rawVideoFrames[index]) {
		m_rawVideoFrames[index] = new VideoFrame;
	}

	m_rawVideoFrameLocks[index].Lock();
	copyVideoFrame(pFrameData, m_rawVideoFrames[index]);
	m_rawVideoFrameLocks[index].Unlock();
}

void RecoderMgr::mixVideoFrame( int index )
{
	// lock
	// resize
	
	if (!m_reusedFrame) {
		m_reusedFrame = new Frame;
		m_reusedFrame->allocBuffer(AV_PIX_FMT_YUV420P, m_encoderParam.w/2, m_encoderParam.h/2);
	}

	Frame* outFrame = new Frame;
	outFrame->allocBuffer(AV_PIX_FMT_YUV420P, m_encoderParam.w, m_encoderParam.h);

	AVFrame* tmpFrame = m_reusedFrame->c_frame();
	AVFrame* avOutFrame = outFrame->c_frame();

	for(size_t i = 0; i < m_rawVideoFrames.size(); i++) {

		uint8_t *data[4];
		int src_linesize[4];
		

		m_rawVideoFrameLocks[i].Lock();
		VideoFrame* pFrameData = m_rawVideoFrames[i];
		src_linesize[0] = pFrameData->desc.width;
		src_linesize[1] = src_linesize[0] / 2;
		src_linesize[2] = src_linesize[0] / 2;
		data[0] = pFrameData->data;
		data[1] = pFrameData->data + pFrameData->desc.width * pFrameData->desc.height;
		data[2] = data[1] + pFrameData->desc.width * pFrameData->desc.height / 4;

		SwsContext* ctx = sws_getCachedContext(m_swsCtx, 
			pFrameData->desc.width, pFrameData->desc.height, AV_PIX_FMT_YUV420P,
			tmpFrame->width, tmpFrame->height, (AVPixelFormat)tmpFrame->pict_type,
			SWS_BILINEAR, NULL, NULL, NULL);
		sws_scale(ctx, data, src_linesize, 0, pFrameData->desc.height,
			tmpFrame->data, tmpFrame->linesize);
		m_rawVideoFrameLocks[i].Unlock();


	
		memcpy(data, avOutFrame->data, sizeof(data));

		switch(i) {
		case 1:
			avOutFrame->data[0] += avOutFrame->linesize[0]/2;
			avOutFrame->data[1] += avOutFrame->linesize[1]/2;
			avOutFrame->data[2] += avOutFrame->linesize[1]/2;
			break;
		case 2:
			avOutFrame->data[0] += avOutFrame->linesize[0] * avOutFrame->height;
			avOutFrame->data[1] += avOutFrame->linesize[1] * avOutFrame->height;
			avOutFrame->data[2] += avOutFrame->linesize[1] * avOutFrame->height;
			break;
		case 3:
			avOutFrame->data[0] += avOutFrame->linesize[0] * avOutFrame->height + avOutFrame->linesize[0]/2;
			avOutFrame->data[1] += avOutFrame->linesize[1] * avOutFrame->height + avOutFrame->linesize[1]/2;
			avOutFrame->data[2] += avOutFrame->linesize[1] * avOutFrame->height + avOutFrame->linesize[1]/2;
			break;
		default:
			break;
		}

		av_picture_copy((AVPicture*)avOutFrame, (AVPicture*)tmpFrame, AV_PIX_FMT_YUV420P, tmpFrame->width, tmpFrame->height);
		memcpy(avOutFrame->data, data, sizeof(data));
	}

	OnRecvMixedFrame(outFrame);
}



void RecoderMgr::OnAudioFrame( AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type )
{
	// localstream 以本地音频为基准
	if(AVAudioCtrl::AudioDataSourceType::AUDIO_DATA_SOURCE_SEND == src_type) {
		if (!m_rawAudioFifo[0]) {
			m_rawAudioFifo[0] = new AudioFifo;
			m_rawAudioFifo[0]->alloc(AV_SAMPLE_FMT_S16, 1, 4096);
		}

		m_rawAudioFifo[0]->write((void**)audio_frame->data, audio_frame->data_size / audio_frame->desc.bits / 8);

		mixAudioFrame();

	} else if(AVAudioCtrl::AudioDataSourceType::AUDIO_DATA_SOURCE_NETSTREM == src_type) {
		
		string& id = audio_frame->identifier;
		int index = m_id2index.Get(id);

		if (!m_rawAudioFifo[index]) {
			m_rawAudioFifo[index] = new AudioFifo;
			m_rawAudioFifo[index]->alloc(AV_SAMPLE_FMT_S16, 1, 4096);
		}

		m_rawAudioFifo[index]->write((void**)audio_frame->data, audio_frame->data_size / audio_frame->desc.bits / 8);

	} else {
		// ignore
	}
}

// 混音策略： 本地缓存满（>= 3072）, 强制混音; ...
void RecoderMgr::mixAudioFrame()
{
	// 1. check to mix
	bool toMix = false;
	int size = m_rawAudioFifo[0]->size();
	
	if (size >= 3072) {
		toMix = true;
	} else if (size >= 2048) {
		for (size_t i = 1; i < m_rawAudioFifo.size(); i++) {
			if (m_rawAudioFifo[i] && m_rawAudioFifo[i]->size() >= 1024) 
				toMix = true;	
		}
	} else if (size >= 1024) {
		toMix = true; 
		for (size_t i = 1; i < m_rawAudioFifo.size(); i++) {
			if (m_rawAudioFifo[i] && m_rawAudioFifo[i]->size() < 1024)  
				toMix = false;	
		}
	}

	if (!toMix) return;

	// 2. mix 
	while (m_rawAudioFifo[0]->size() >= 1024) {
		Frame* audioFrame = new Frame;
		audioFrame->allocBuffer(1024, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, 16000);
		uint8_t* outBuffer = audioFrame->c_frame()->data[0];
		m_rawAudioFifo[0]->read((void**)outBuffer, 1024);
		for(size_t i = 1; i <= m_rawAudioFifo.size(); i++) {
			if (m_rawAudioFifo[i] && m_rawAudioFifo[i]->size() >= 1024) {
				uint8_t* data[1024];
				m_rawAudioFifo[i]->read((void**)data, 1024);
				_mix16((const int16_t*)data, 1, (int16_t*)outBuffer, 1024 * 2);
			}	
		}
		
		// TODO: new a audio frame, reset the buffer;
		OnRecvMixedFrame(audioFrame);
	}
}

void RecoderMgr::OnRecvMixedFrame( Frame* frame )
{
	// TODO: calcuate timestamp
	

	// push to encoder.
	m_encoder.PushFrame(frame);
}

void RecoderMgr::OnRecvEncodedPacket( Packet* pkt )
{
	// send to packet.
	assert(m_writers.size() == 1);

	// TODO: 目前只录一个， 多个录像时，需复制 pkt;
	for(size_t i = 0; m_writers.size(); i++) {
		m_writers[i]->PushPacket(pkt);
	}
}

void RecoderMgr::OnRecvEncodedPacket( Packet* pkt, void* arg )
{
	RecoderMgr* mgr = reinterpret_cast<RecoderMgr*>(arg);
	mgr->OnRecvEncodedPacket(pkt);
}

void RecoderMgr::Start()
{
	bool ret = m_encoder.start(m_encoderParam, RecoderMgr::OnRecvEncodedPacket, reinterpret_cast<void*>(this));
	if (ret) {
		LOGE("start encoder error.");
		return;
	}

	AVStreamWriter* writer = new AVStreamWriter;
	writer->SetAVEncoder(&m_encoder);
	writer->SetOuturl("");
	m_writers.push_back(writer);

	m_recordStat = true;
}

void RecoderMgr::Stop()
{
	m_recordStat = false;
	m_encoder.stop();
}

void RecoderMgr::SetAVParam( EncoderParam& param )
{
	m_encoderParam = param;
}

void RecoderMgr::OnLocalVideoFrame( VideoFrame* pFrameData )
{
	copyVideoFrame(pFrameData, 0);
	mixVideoFrame(0);
}

void RecoderMgr::UpdateRemoteViewsIndex( const vector<string>& ids )
{
	if (ids.size() > 3) {
		LOGFMTE("max remote person is 3, but require is %d", ids.size());
		return;
	}
	
	m_id2index.Set(ids, 1); 
}
