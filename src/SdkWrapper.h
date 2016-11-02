#pragma once
#include "avsdk/av_sdk.h"
#include "Util.h"
#include <vector>
#include <map>
#include <fstream>


using namespace std;
using namespace tencent::av;

#define VIDEO_RENDER_BIG_VIEW_WIDTH 640
#define VIDEO_RENDER_BIG_VIEW_HEIGHT 480
#define VIDEO_RENDER_SMALL_VIEW_WIDTH 160
#define VIDEO_RENDER_SMALL_VIEW_HEIGHT 120

class SdkWrapper:public AVRoomMulti::Delegate
{
public:
	SdkWrapper(void);
	~SdkWrapper(void);


	const char* GetVersion();

	void EnableCrashReport(bool enable);

	// 连接服务
	int CreateContext();
	int DestroyContext();
	int StartContext(const AVContext::StartParam& param);
	int StopContext();

	//房间
	int EnterRoom(AVRoomMulti::EnterParam& param);
	int ExitRoom();
	int ChangeAuthority(uint64 authBits,const std::string& strPrivilegeMap);
	//AVEndpoint* GetEndpointByIndex(int index);
	AVEndpoint* GetEndpointById(string identifier);
	int GetEndpointList(AVEndpoint** endpointList[]);

	UINT GetRoomId();
	int ChangeRole(string role);

	//房间成员
	int MuteAudio(std::string identifier, bool is_mute = false);	
	int RequestViewList(std::vector<std::string> identifierList, std::vector<View> viewList);
	int CancelAllView();

	//自监听开关
	bool EnableLoopback(bool isEnable);

	//语音设备
	//麦克风
	int OpenMic();
	int CloseMic();
	int SetSelectedMicId(std::string micId);
	std::string GetSelectedMicId();
	int GetMicList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &micList);
	void SetMicVolume(uint32 value);
	uint32 GetMicVolume();

	//播放器(扬声器/耳机)
	int OpenPlayer();
	int ClosePlayer();
	int SetSelectedPlayerId(std::string playerId);
	std::string GetSelectedPlayerId();
	int GetPlayerList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &playerList);
	void SetPlayerVolume(uint32 value);
	uint32 GetPlayerVolume();

	//伴奏
	int StartAccompany(std::string playerPath, std::string mediaFilePath, AVAccompanyDevice::SourceType sourceType);
	int StopAccompany();
	void SetAccompanyVolume(uint32 value);
	uint32 GetAccompanyVolume();

	//输入混音
	int StartMicAndAccompanyForMix(std::vector<std::string> localDeviceList);
	int StopMicAndAccompanyForMix(std::vector<std::string> localDeviceList);

	//视频设备
	//摄像头
	int OpenCamera();
	int CloseCamera();
	int SetSelectedCameraId(std::string cameraId);
	std::string GetSelectedCameraId();
	int GetCameraList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &cameraList);

	//屏幕分享
	int OpenScreenShareSend(uint32 left, uint32 top, uint32 right, uint32 bottom, uint32 fps);
	int CloseScreenShareSend();
	void GetScreenCaptureParam(uint32 &left, uint32 &top, uint32 &right, uint32 &bottom, uint32 &fps);
	void EnableScreenShareHDMode(bool isEnable);

	int SetExternalCamAbility(CameraInfo* pCamInfo);
	int OpenExternalCapture();
	int CloseExternalCapture();
	int FillExternalCaptureFrame(VideoFrame* pFrame);

	void SetVideoParam(AVSupportVideoPreview::PreviewParam);
	void ClearVideoParam();


	//设备管理
	void GetAudioInputDeviceList();

	//其他
	std::string GetQualityTips();

	//查询运行时参数
	std::string GetRoomStatParam();

	//水印
	int AddWaterMark(int preset, int *pARGBData, int width, int height);

	void UpdateRequestViewIdentifierList(std::vector<std::string> identifierList){m_curRequestViewIdentifierList = identifierList;/*TODO 这边未加保护，可能会导致使用到该变量的逻辑异常*/}
	std::vector<std::string> GetRequestViewIdentifierList(){return m_curRequestViewIdentifierList;}
	void UpdateRequestViewParamList(std::vector<View> viewList){m_curRequestViewParamList = viewList;/*TODO 这边未加保护，可能会导致使用到该变量的逻辑异常*/}
	std::vector<View> GetRequestViewParamList(){return m_curRequestViewParamList;}
	void SetDefaultCameraState(bool isEnable){m_isSelectedCameraEnable = isEnable;};
	bool GetDefaultCameraState(){return m_isSelectedCameraEnable;};
	void SetDefaultScreenShareSendState(bool isEnable){m_isSelectedScreenShareSendEnable = isEnable;};
	bool GetDefaultScreenShareSendState(){return m_isSelectedScreenShareSendEnable;};
	void SetDefaultMicState(bool isEnable){m_isSelectedMicEnable = isEnable;};
	bool GetDefaultMicState(){return m_isSelectedMicEnable;};
	void SetDefaultPlayerState(bool isEnable){m_isSelectedPlayerEnable = isEnable;};
	bool GetDefaultPlayerState(){return m_isSelectedPlayerEnable;};
	
	void EnableHostMode();
public:
	
	virtual void OnStartContextComplete(int result);
	virtual void OnStopContextComplete();
	virtual void OnEnterRoomComplete(int result);
	virtual void OnExitRoomComplete();
	virtual void OnExitRoomComplete(int reason);
	virtual void OnRoomDisconnect(int reason);
	virtual void OnChangeAuthority(int32 ret_code);
	virtual void OnChangeRole(int32 ret_code);

	//virtual void OnEndpointsEnterRoom(int endpointCount, AVEndpoint *endPointList[]);
	//virtual void OnEndpointsExitRoom(int endpointCount, AVEndpoint *endPointList[]);
	virtual void OnEndpointsUpdateInfo(AVRoomMulti::EndpointEventId eventid, std::vector<std::string> updatelist);
	virtual void OnEndpointsUpdateInfo();
	virtual void OnPrivilegeDiffNotify(int32 privilege);
	virtual void OnDeviveDetecNotify(AVDeviceMgr* device_mgr,DetectedDeviceInfo& info, bool*pbUnselect, void* custom_data);
	//DeviceTest
	int StartDeviceTest();
	int StopDeviceTest();
	void DeviceTestSetSelectedMicId(std::string deviceId);
	std::string DeviceTestGetSelectedMicId(){return m_deviceTestSelectedMicId;}
	int DeviceTestOpenMic();
	int DeviceTestCloseMic();
	int DeviceTestGetMicList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &deviceId);
	void DeviceTestSetMicVolume(uint32 value);
	uint32 DeviceTestGetMicVolume();
	uint32 DeviceTestGetMicDynamicVolume();
	
	void DeviceTestSetSelectedPlayerId(std::string deviceId);
	std::string DeviceTestGetSelectedPlayerId(){return m_deviceTestSelectedPlayerId;}
	int DeviceTestOpenPlayer();
	int DeviceTestClosePlayer();
	int DeviceTestGetPlayerList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &deviceId);
	void DeviceTestSetPlayerVolume(uint32 value);
	uint32 DeviceTestGetPlayerVolume();
	uint32 DeviceTestGetPlayerDynamicVolume();
	
	void DeviceTestSetSelectedCameraId(std::string deviceId);
	std::string DeviceTestGetSelectedCameraId(){return m_deviceTestSelectedCameraId;}
	int DeviceTestOpenCamera();
	int DeviceTestCloseCamera();
	int DeviceTestGetCameraList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &deviceId);

	static void OnStartDeviceTestCompleteCallback(int result, void *pCustomData);
	static void OnStopDeviceTestCompleteCallback(void *pCustomData);
	static void OnDeviceTestLocalVideo(VideoFrame *pFrameData, void *pCustomData);
	static void OnDeviceTestDeviceOperationCallback(AVDeviceTest *pDevTest, AVDevice::DeviceOperation oper, const std::string &deviceId, int retCode, void *pCustomData);
	static void OnDeviceTestDeviceDetectCallback(AVDeviceTest *pDeviceTest, void *pCustomData);
	virtual void OnSemiAutoRecvCameraVideo(std::vector<std::string> identifierList);
	virtual void OnCameraSettingNotify(int width, int height, int fps);

	int StartInputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type, std::string path);
	int StopInputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type);

	int StartOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type);
	int StopOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type);

	int StartInOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type);
	int StopInOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type);

	int EnableAudioData(AVAudioCtrl::AudioDataSourceType src_type, bool enable);
	bool IsEnableAudioData(AVAudioCtrl::AudioDataSourceType src_type);

	int SetAudioDataFormat(AVAudioCtrl::AudioDataSourceType src_type, AudioFrameDesc audio_desc);
	int GetAudioDataFormat(AVAudioCtrl::AudioDataSourceType src_type, AudioFrameDesc& audio_desc);
	int SetAudioDataVolume(AVAudioCtrl::AudioDataSourceType src_type, float volume);
	int GetAudioDataVolume(AVAudioCtrl::AudioDataSourceType src_type, float* volume);

	int SetAudioDataDBVolume(AVAudioCtrl::AudioDataSourceType src_type, int volume);
	int GetAudioDataDBVolume(AVAudioCtrl::AudioDataSourceType src_type, int* volume);

	void Set1v1VideoRenderDlg(void *p1v1VideoRenderDlg);

	AVContext* GetAVContext() {return m_pContext;};

	// 音视频数据回调
	virtual void OnRemoteVideoCallback(VideoFrame *pFrameData) {}
	virtual void OnLocalVideoCallback(VideoFrame *pFrameData) {}

	virtual int AudioDataCallback(AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type);
private:
	static int AudioDataCallback(AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type, void* custom_data);
private:
	void OnStartDeviceTestCompleteCallbackInternal(int result);
	void OnStopDeviceTestCompleteCallbackInternal();

protected:
	static void OnStartContextCompleteCallback(int result, void *pCustomData);

	static void OnRequestViewListCompleteCallback(std::vector<std::string> identifierList, std::vector<View> viewList, int32 result, void *pCustomData);
	static void OnCancelAllViewCompleteCallback(int result, void *pCustomData);
	static void OnAudioDeviceOperationCallback(AVDeviceMgr *pAudMgr, AVDevice::DeviceOperation oper, const std::string &deviceId, int retCode, void *pCustomData);
	static void OnVideoDeviceOperationCallback(AVDeviceMgr *pVidMgr, AVDevice::DeviceOperation oper, const std::string &deviceId, int retCode, void *pCustomData);
	static void OnVideoDeviceChangeCallback(AVDeviceMgr *pVidMgr, void *pCustomData);	
	static void OnAudioDeviveDetectNotify(AVDeviceMgr* device_mgr,DetectedDeviceInfo& info, bool*pbUnselect, void* custom_data);
	static void OnRemoteVideoCallback(VideoFrame *pFrameData, void *pCustomData);
	static void OnLocalVideoCallback(VideoFrame *pFrameData, void *pCustomData);
	static void OnPreTreatmentFun(VideoFrame *pFrameData, void *pCustomData);
	static void OnChangeAuthorityCallback(int32 ret_code, void *pCustomData);
	static void OnChangeRoleCallback(int32 ret_code, void *pCustomData);

private:
	//void *m_pMainDlg;	
	//void *m_p1v1VideoRenderDlg;
	AVContext *m_pContext;
	
	AVRoomMulti *m_pRoom;
	AVAudioCtrl *m_pAudCtrl;
	AVVideoCtrl *m_pVidCtrl;
	AVDeviceMgr *m_pAudMgr;
	AVDeviceMgr *m_pVidMgr;
	AVDeviceTest *m_pDeviceTest;

	AVSupportVideoPreview::PreviewParam m_cameraParam;
	AVSupportVideoPreview::PreviewParam m_remoteVideoParam;
	AVSupportVideoPreview::PreviewParam m_screenSendParam;
	AVSupportVideoPreview::PreviewParam m_screenRecvParam;

	std::vector<std::string> m_curRequestViewIdentifierList;
	std::vector<View> m_curRequestViewParamList;

	std::vector<std::pair<std::string/*id*/, std::string/*name*/> > m_micList;
	std::vector<std::pair<std::string/*id*/, std::string/*name*/> > m_playerList;
	std::vector<std::pair<std::string/*id*/, std::string/*name*/> > m_cameraList;
	std::string m_selectedMicId;
	std::string m_selectedPlayerId;
	std::string m_selectedCameraId;
	bool m_isSelectedMicEnable;
	bool m_isSelectedPlayerEnable;
	bool m_isSelectedCameraEnable;
	bool m_isSelectedScreenShareSendEnable;

	std::map<AVAudioCtrl::AudioDataSourceType, FILE*> m_mapAudioData;
	std::map<AVAudioCtrl::AudioDataSourceType, std::string> m_mapAudioDataName;
	std::map<std::string/*id*/, FILE*> m_mapAudioDataNetStreams;

	bool m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_END];
	AudioFrameDesc m_AudioDataDesc[AVAudioCtrl::AUDIO_DATA_SOURCE_END];
	SimpleLock m_AudioDataLock[AVAudioCtrl::AUDIO_DATA_SOURCE_END];
	SimpleLock m_AudioDataMapLock;

	std::string m_strMixToSendPath;
	std::string m_strMixToPlayPath;

	HMODULE m_hMoude;
	PROC_AVAPI_GetVersion m_funcGetVersion;
	PROC_AVAPI_CreateContext m_funcCreateContext;	
	PROC_AVAPI_EnableCrashReport m_funcEnableCrashReport;
	//DeviceTest
	std::string m_deviceTestSelectedMicId;
	std::string m_deviceTestSelectedPlayerId;
	std::string m_deviceTestSelectedCameraId;
};

