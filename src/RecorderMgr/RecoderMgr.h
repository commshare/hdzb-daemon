#pragma once

#include "header.h"
#include "tim.h"
#include "../log4z.h"
#include "../gci-json.h"
#include "../CustomWinMsg.h"
#include "AVEncoder.h"
#include "AVStreamWriter.h"
#include "../SdkWrapper.h"

using namespace imcore;
using namespace ggicci;

// Frame --> buffer --> mix --> encoder --> onavpacket --> send.


#ifdef DLL_IMPLEMENT  
#define DLL_API __declspec(dllexport)  
#else  
#define DLL_API __declspec(dllimport)  
#endif  

class DLL_API RecoderMgr
{
public:
	RecoderMgr(void);

	//************************************
	// Method:    SetPicMode
	// FullName:  RecoderMgr::SetPicMode
	// Access:    public
	// Returns:   void
	// Qualifier: 设置图像的拼接方式，单画面，双画面，三画面，四画面。
	// Parameter: size_t mode  取值为 1 ~ 4
	//************************************
	void SetPicMode(size_t mode) { m_picMode = mode; }
	void SetAVParam(EncoderParam& param);

	void Start();
	void Stop();

	void OnRemoteVideoFrame(VideoFrame* pFrameData);
	void OnLocalVideoFrame(VideoFrame* pFrameData);
	void OnAudioFrame(AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type);

	void UpdateRemoteViewsIndex(const vector<string>& ids);

private:
	size_t m_picMode;
	bool m_recordStat;
	
	class IdsMap {
		std::map<string, int> m_id2index;
		SimpleLock m_lock;
	public:
		int Get(const string& id ) 
		{ 
			SimpleAutoLock lock(&m_lock);
			if (m_id2index.count(id))
				return m_id2index.count(id);
			else
				return -1;
		}

		void Set(const string& id, int index)
		{
			SimpleAutoLock lock(&m_lock);
			m_id2index[id] = index;
		}

		void Set(const std::vector<string> ids, int baseIndex)
		{
			SimpleAutoLock lock(&m_lock);
			for (size_t i = 0; i < ids.size(); i++) {
				m_id2index[ids[i]] = baseIndex + i;
			}
		}
	} m_id2index;
	
	
	// 缓存原始音视频数据
	std::vector<SimpleLock>   m_rawVideoFrameLocks;
	std::vector<VideoFrame*>  m_rawVideoFrames; // av_image_copy
	std::vector<AudioFifo*>	  m_rawAudioFifo;

	
	AVEncoder m_encoder;
	std::vector<AVStreamWriter*> m_writers;

	SwsContext* m_swsCtx;
	EncoderParam m_encoderParam;

	Frame* m_reusedFrame;
private:
	void mixVideoFrame(int index);
	void mixAudioFrame();
	void OnRecvMixedFrame(Frame* frame); // --> encoder
	void OnRecvEncodedPacket(Packet* pkt); // --> rtmp/flv
	static void OnRecvEncodedPacket(Packet* pkt, void* arg);
};


