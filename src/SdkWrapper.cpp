#include "SdkWrapper.h"
#include "Util.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include "log4z.h"
//#include <afx.h>  // MFC

SdkWrapper::SdkWrapper(void)
{
	//m_pMainDlg = NULL;
	//m_p1v1VideoRenderDlg = NULL;

	m_pContext = NULL;
	m_pRoom = NULL;
	m_pAudCtrl = NULL;
	m_pVidCtrl = NULL;
	m_pAudMgr = NULL;
	m_pVidMgr = NULL;

	m_cameraParam.device_id = "";/*不指定id，代表每路视频都共用这组参数*/
	m_cameraParam.width = 0;//VIDEO_RENDER_BIG_VIEW_WIDTH;
	m_cameraParam.height = 0;//VIDEO_RENDER_BIG_VIEW_HEIGHT;
	m_cameraParam.color_format = COLOR_FORMAT_RGB24;//SDK1.3版本只支持COLOR_FORMAT_RGB24和COLOR_FORMAT_I420
	m_cameraParam.src_type = VIDEO_SRC_TYPE_CAMERA;//SDK1.3版本只支持VIDEO_SRC_TYPE_CAMERA

	m_remoteVideoParam.device_id = "";/*不指定id，代表每路视频都共用这组参数*/
	m_remoteVideoParam.width = 0;//VIDEO_RENDER_BIG_VIEW_WIDTH;
	m_remoteVideoParam.height = 0;//VIDEO_RENDER_BIG_VIEW_HEIGHT;
	m_remoteVideoParam.color_format = COLOR_FORMAT_RGB24;//SDK1.3版本只支持COLOR_FORMAT_RGB24和COLOR_FORMAT_I420
	m_remoteVideoParam.src_type = VIDEO_SRC_TYPE_CAMERA;//SDK1.3版本只支持VIDEO_SRC_TYPE_CAMERA

	m_screenSendParam.device_id = "";/*不指定id，代表每路视频都共用这组参数*/
	m_screenSendParam.width = 0;//VIDEO_RENDER_BIG_VIEW_WIDTH;
	m_screenSendParam.height = 0;//VIDEO_RENDER_BIG_VIEW_HEIGHT;
	m_screenSendParam.color_format = COLOR_FORMAT_RGB24;//SDK1.6版本只支持COLOR_FORMAT_RGB24和COLOR_FORMAT_I420
	m_screenSendParam.src_type = VIDEO_SRC_TYPE_SCREEN;//SDK1.6版本只支持VIDEO_SRC_TYPE_SCREEN

	m_screenRecvParam.device_id = "";/*不指定id，代表每路视频都共用这组参数*/
	m_screenRecvParam.width = 0;//VIDEO_RENDER_BIG_VIEW_WIDTH;
	m_screenRecvParam.height = 0;//VIDEO_RENDER_BIG_VIEW_HEIGHT;
	m_screenRecvParam.color_format = COLOR_FORMAT_RGB24;//SDK1.6版本只支持COLOR_FORMAT_RGB24和COLOR_FORMAT_I420
	m_screenRecvParam.src_type = VIDEO_SRC_TYPE_SCREEN;//SDK1.6版本只支持VIDEO_SRC_TYPE_SCREEN

	m_selectedMicId = "";
	m_selectedPlayerId = "";
	m_selectedCameraId = "";
	m_isSelectedMicEnable = false;
	m_isSelectedPlayerEnable = false;
	m_isSelectedCameraEnable = false;
	m_isSelectedScreenShareSendEnable = false;

	m_deviceTestSelectedMicId = "";
	m_deviceTestSelectedPlayerId = "";
	m_deviceTestSelectedCameraId = "";

	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_MIC] = false;
	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOSEND] = false;
	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_SEND] = false;
	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOPLAY] = false;
	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_PLAY] = false;
	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_NETSTREM] = false;
	m_mapAudioDataEnable[AVAudioCtrl::AUDIO_DATA_SOURCE_VOICEDISPOSE] = false;

	m_mapAudioDataName[AVAudioCtrl::AUDIO_DATA_SOURCE_MIC] = "Mic";
	m_mapAudioDataName[AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOSEND] = "MixToSend";
	m_mapAudioDataName[AVAudioCtrl::AUDIO_DATA_SOURCE_SEND] = "Send";
	m_mapAudioDataName[AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOPLAY] = "MixToPlay";
	m_mapAudioDataName[AVAudioCtrl::AUDIO_DATA_SOURCE_PLAY] = "Play";
	m_mapAudioDataName[AVAudioCtrl::AUDIO_DATA_SOURCE_NETSTREM] = "NetStream";

	m_hMoude = ::LoadLibrary(_T("qavsdk.dll"));
	m_funcGetVersion = (PROC_AVAPI_GetVersion)::GetProcAddress(m_hMoude, "AVAPI_GetVersion");
	m_funcCreateContext = (PROC_AVAPI_CreateContext)::GetProcAddress(m_hMoude, "AVAPI_CreateContext");
	m_funcEnableCrashReport = (PROC_AVAPI_EnableCrashReport)::GetProcAddress(m_hMoude, "AVAPI_EnableCrashReport");
}

SdkWrapper::~SdkWrapper(void)
{
	if (m_pContext) {
		m_pContext->Destroy();
		m_pContext = NULL;
	}
}

const char* SdkWrapper::GetVersion()
{
	if (!m_hMoude) return 0;
	if (!m_funcGetVersion) return 0;

	return (*m_funcGetVersion)();
}

void SdkWrapper::EnableCrashReport(bool enable)
{
	return (*m_funcEnableCrashReport)(enable, true);
}

int SdkWrapper::CreateContext()
{
	if (m_pContext == NULL) {
		AVContext* pContext = (AVContext*)(*m_funcCreateContext)();
		m_pContext = pContext;
	}
	return AV_OK;
}

int SdkWrapper::DestroyContext()
{
	if (m_pContext) {
		m_pContext->Destroy();
		m_pContext = NULL;
	}
	return AV_OK;
}

int SdkWrapper::StartContext(const AVContext::StartParam& param)
{
	
	if (!m_hMoude) return AV_ERR_FAILED;
	if (!m_funcGetVersion) return AV_ERR_FAILED;
	if (!m_funcCreateContext) return AV_ERR_FAILED;
	if (!m_funcEnableCrashReport) return AV_ERR_FAILED;

	if (m_pContext == NULL)return AV_ERR_FAILED;


	return m_pContext->Start(param, &SdkWrapper::OnStartContextCompleteCallback, this);
}

int SdkWrapper::StopContext()
{
	if (m_pContext == NULL)return AV_ERR_FAILED;
	int result = m_pContext->Stop();
	if (AV_OK == result)
		OnStopContextComplete();
	return result;
}

//房间
int SdkWrapper::EnterRoom(AVRoomMulti::EnterParam& param)
{
	if (m_pContext == NULL)return AV_ERR_FAILED;
	return m_pContext->EnterRoom(this, param);
}

int SdkWrapper::ExitRoom()
{
	if (m_pContext == NULL) return AV_ERR_FAILED;
	return m_pContext->ExitRoom();
}

int SdkWrapper::ChangeAuthority(uint64 authBits, const std::string& strPrivilegeMap)
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return NULL;
	return m_pRoom->ChangeAuthority(authBits, strPrivilegeMap, SdkWrapper::OnChangeAuthorityCallback, this);
}

int SdkWrapper::ChangeRole(string role)
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;
	return m_pRoom->ChangeAVControlRole(role.c_str(), SdkWrapper::OnChangeRoleCallback, this);
}

AVEndpoint* SdkWrapper::GetEndpointById(string identifier)
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return NULL;
	if (identifier == "") return NULL;
	return m_pRoom->GetEndpointById(identifier);
}

int SdkWrapper::GetEndpointList(AVEndpoint** endpointList[])
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return 0;
	return m_pRoom->GetEndpointList(endpointList);
}

//房间成员
int SdkWrapper::MuteAudio(std::string identifier, bool is_mute)
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;
	if (identifier == "") return AV_ERR_FAILED;

	AVEndpoint *pEndpoint = m_pRoom->GetEndpointById(identifier);
	if (pEndpoint) {
		bool ret = pEndpoint->MuteAudio(is_mute);
		return ret ? AV_OK : AV_ERR_FAILED;
	} else {
		return AV_ERR_FAILED;
	}

	return AV_OK;
}

/*
RequestViewList用于同时请求多路画面。如果要同一时刻看到多路画面，可以使用该接口。
注意：
. 每次请求多路画面时，必须都带上想要查看的成员的id。如第一次要同时看A,B和C的画面，就必须带上这三个人的id；
	如果接下去要同时看A和B的画面，就必须带上这两个人的id；如果接下去又要同时看A,B,C和D的画面，就必须带上这四个人的id。
*/
int SdkWrapper::RequestViewList(std::vector<std::string> identifierList, std::vector<View> viewList)
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr) return AV_ERR_FAILED;
	if (identifierList.size() == 0 || identifierList.size() != viewList.size()) return AV_ERR_INVALID_ARGUMENT;

	/*
	注意:
	1. 画面大小可以根据业务层实际需要及硬件能力决定。
	2. 如果是手机，建议只有其中一路是大画面，其他都是小画面，这样硬件更容易扛得住，同时减少流量。
	3. 这边把320×240及以上大小的画面认为是大画面；反之，认为是小画面。
	4. 实际上请求到的画面大小，由发送方决定。如A传的画面是小画面，即使这边即使想请求它的大画面，也只能请求到的小画面。
	5. 发送方传的画面大小，是否同时有大画面和小画面，由其所设置的编解码参数、场景、硬件、网络等因素决定。
	*/

	return m_pRoom->RequestViewList(identifierList, viewList, OnRequestViewListCompleteCallback, this);
}

/*
CancelAllView用于取消所有的画面。
*/
int SdkWrapper::CancelAllView()
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	if (m_curRequestViewIdentifierList.size() == 0) {
		return AV_ERR_FAILED;
	}

	m_curRequestViewIdentifierList.clear();
	m_curRequestViewParamList.clear();
	return m_pRoom->CancelAllView(OnCancelAllViewCompleteCallback, this);
}

//自监听开关
bool SdkWrapper::EnableLoopback(bool isEnable)
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return false;
	return m_pAudCtrl->EnableLoopback(isEnable);
}

//麦克风
int SdkWrapper::OpenMic()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	if (m_selectedMicId == "")return AV_ERR_DEVICE_NOT_EXIST;
	return m_pAudMgr->SelectInputDevice(m_selectedMicId, true);
}

int SdkWrapper::CloseMic()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	if (m_selectedMicId == "")return AV_ERR_DEVICE_NOT_EXIST;
	return m_pAudMgr->SelectInputDevice(m_selectedMicId, false);
}

int SdkWrapper::SetSelectedMicId(std::string micId)
{
	m_selectedMicId = micId;
	return AV_ERR_DEVICE_NOT_EXIST;
}

std::string SdkWrapper::GetSelectedMicId()
{
	return m_selectedMicId;
}

int SdkWrapper::GetMicList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &micList)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	AVDevice **ppMicList = NULL;
	int micCount = m_pAudMgr->GetDeviceByType(DEVICE_MIC, &ppMicList);//获取麦克风列表
	if (ppMicList == NULL)return AV_ERR_DEVICE_NOT_EXIST;

	if (micCount == 0) {
		return AV_ERR_DEVICE_NOT_EXIST;
	}

	for (int i = 0; i < micCount; i++) {
		std::pair<std::string/*id*/, std::string/*name*/> mic;
		mic.first = ppMicList[i]->GetId();
		mic.second = ppMicList[i]->GetInfo().name;
		micList.push_back(mic);
	}
	if (ppMicList)delete[]ppMicList;//业务侧负责释放这个数组。
	return AV_OK;
}

void SdkWrapper::SetMicVolume(uint32 value)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return;

	AVDevice **ppMicList = NULL;
	int micCount = m_pAudMgr->GetDeviceByType(DEVICE_MIC, &ppMicList);//获取麦克风列表
	if (ppMicList == NULL)return;

	if (micCount == 0) {
		return;
	} else if (micCount == 1) {
		((AVMicDevice *)ppMicList[0])->SetVolume(value);
		return;
	} else//多个麦克风
	{
		//由于默认打开的是第一个设备，这样也就操作第一个设备。
		((AVMicDevice*)ppMicList[0])->SetVolume(value);
	}
	if (ppMicList)delete[]ppMicList;//业务侧负责释放这个数组。
}


uint32 SdkWrapper::GetMicVolume()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return MIN_AUDIO_DEVICE_VOLUME;

	AVDevice **ppMicList = NULL;
	int micCount = m_pAudMgr->GetDeviceByType(DEVICE_MIC, &ppMicList);//获取麦克风列表
	if (ppMicList == NULL)return MIN_AUDIO_DEVICE_VOLUME;

	if (micCount == 0) {
		return MIN_AUDIO_DEVICE_VOLUME;
	} else if (micCount == 1) {
		uint32 volume = ((AVMicDevice *)ppMicList[0])->GetVolume();
		if (ppMicList)delete[]ppMicList;//业务侧负责释放这个数组。
		return volume;
	} else//多个麦克风
	{
		//由于默认打开的是第一个设备，这样也就操作第一个设备。		
		uint32 volume = ((AVMicDevice*)ppMicList[0])->GetVolume();
		if (ppMicList)delete[]ppMicList;//业务侧负责释放这个数组。
		return volume;
	}
}


//播放器
int SdkWrapper::OpenPlayer()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	if (m_selectedPlayerId == "")return AV_ERR_DEVICE_NOT_EXIST;
	return m_pAudMgr->SelectOutputDevice(m_selectedPlayerId, true);
}

int SdkWrapper::ClosePlayer()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	if (m_selectedPlayerId == "")return AV_ERR_DEVICE_NOT_EXIST;
	return m_pAudMgr->SelectOutputDevice(m_selectedPlayerId, false);
}

int SdkWrapper::SetSelectedPlayerId(std::string playerId)
{
	m_selectedPlayerId = playerId;
	return AV_ERR_DEVICE_NOT_EXIST;
}

std::string SdkWrapper::GetSelectedPlayerId()
{
	return m_selectedPlayerId;
}

int SdkWrapper::GetPlayerList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &playerList)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	AVDevice **ppPlayerList = NULL;
	int playerCount = m_pAudMgr->GetDeviceByType(DEVICE_PLAYER, &ppPlayerList);//获取播放器列表
	if (ppPlayerList == NULL)return AV_ERR_DEVICE_NOT_EXIST;

	if (playerCount == 0) {
		return AV_ERR_DEVICE_NOT_EXIST;
	}

	for (int i = 0; i < playerCount; i++) {
		std::pair<std::string/*id*/, std::string/*name*/> player;
		player.first = ppPlayerList[i]->GetId();
		player.second = ppPlayerList[i]->GetInfo().name;
		playerList.push_back(player);
	}
	if (ppPlayerList)delete[]ppPlayerList;//业务侧负责释放这个数组。
	return AV_OK;
}

void SdkWrapper::SetPlayerVolume(uint32 value)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return;

	AVDevice **ppPlayerList = NULL;
	int playerCount = m_pAudMgr->GetDeviceByType(DEVICE_PLAYER, &ppPlayerList);//获取播放器列表
	if (ppPlayerList == NULL)return;

	if (playerCount == 0) {
		return;
	} else if (playerCount == 1) {
		((AVPlayerDevice *)ppPlayerList[0])->SetVolume(value);
		if (ppPlayerList)delete[]ppPlayerList;//业务侧负责释放这个数组。
		return;
	} else//多个播放器
	{
		//由于默认打开的是第一个设备，这样也就操作第一个设备。		
		((AVPlayerDevice*)ppPlayerList[0])->SetVolume(value);
		if (ppPlayerList)delete[]ppPlayerList;//业务侧负责释放这个数组。
	}
}

uint32 SdkWrapper::GetPlayerVolume()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return MIN_AUDIO_DEVICE_VOLUME;

	AVDevice **ppPlayerList = NULL;
	int playerCount = m_pAudMgr->GetDeviceByType(DEVICE_PLAYER, &ppPlayerList);//获取播放器列表
	if (ppPlayerList == NULL)return MIN_AUDIO_DEVICE_VOLUME;

	if (playerCount == 0) {
		return MIN_AUDIO_DEVICE_VOLUME;
	} else if (playerCount == 1) {
		uint32 volume = ((AVPlayerDevice *)ppPlayerList[0])->GetVolume();
		if (ppPlayerList)delete[]ppPlayerList;//业务侧负责释放这个数组。
		return volume;
	} else//多个播放器
	{
		//由于默认打开的是第一个设备，这样也就操作第一个设备。		
		uint32 volume = ((AVPlayerDevice*)ppPlayerList[0])->GetVolume();
		if (ppPlayerList)delete[]ppPlayerList;//业务侧负责释放这个数组。
		return volume;
	}
}

//伴奏
int SdkWrapper::StartAccompany(std::string playerPath, std::string mediaFilePath, AVAccompanyDevice::SourceType sourceType)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	std::string deviceId = DEVICE_ACCOMPANY;//默认只有一个伴奏设备，所以这个伴奏设备的id设置为DEVICE_ACCOMPANY；
	AVAccompanyDevice *pAccompany = (AVAccompanyDevice *)m_pAudMgr->GetDeviceById(deviceId);
	if (pAccompany == NULL)return AV_ERR_DEVICE_NOT_EXIST;

	pAccompany->SetSource(playerPath, mediaFilePath, sourceType);
	return m_pAudMgr->SelectInputDevice(deviceId, true);
}

int SdkWrapper::StopAccompany()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	std::string deviceId = DEVICE_ACCOMPANY;//默认只有一个伴奏设备，所以这个伴奏设备的id设置为DEVICE_ACCOMPANY；
	AVAccompanyDevice *pAccompany = (AVAccompanyDevice *)m_pAudMgr->GetDeviceById(deviceId);
	if (pAccompany == NULL)return AV_ERR_DEVICE_NOT_EXIST;

	return m_pAudMgr->SelectInputDevice(deviceId, false);
}

void SdkWrapper::SetAccompanyVolume(uint32 value)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return;

	std::string deviceId = DEVICE_ACCOMPANY;
	AVAccompanyDevice *pAccompany = (AVAccompanyDevice *)m_pAudMgr->GetDeviceById(deviceId);
	if (pAccompany == NULL)return;

	pAccompany->SetVolume(value);
}

uint32 SdkWrapper::GetAccompanyVolume()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return MIN_AUDIO_DEVICE_VOLUME;

	std::string deviceId = DEVICE_ACCOMPANY;
	AVAccompanyDevice *pAccompany = (AVAccompanyDevice *)m_pAudMgr->GetDeviceById(deviceId);
	if (pAccompany == NULL)return MIN_AUDIO_DEVICE_VOLUME;

	return pAccompany->GetVolume();
}

//输入混音
int SdkWrapper::StartMicAndAccompanyForMix(std::vector<std::string> localDeviceList)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;

	for (size_t i = 0; i < localDeviceList.size(); i++) {
		std::string deviceId = localDeviceList.at(i);

		if (deviceId == DEVICE_MIC) {
			int retCode = m_pAudMgr->SelectInputDevice(deviceId, true);//打开麦克风
		} else if (deviceId == DEVICE_ACCOMPANY) {
			int retCode = StartAccompany("", "", AVAccompanyDevice::AV_ACCOMPANY_SOURCE_TYPE_SYSTEM);//系统伴奏
		}
	}

	return AV_OK;
}

int SdkWrapper::StopMicAndAccompanyForMix(std::vector<std::string> localDeviceList)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return AV_ERR_FAILED;
	if (localDeviceList.size() == 0)return AV_ERR_FAILED;

	for (size_t i = 0; i < localDeviceList.size(); i++) {
		std::string deviceId = localDeviceList.at(i);

		if (deviceId == DEVICE_MIC) {
			int retCode = m_pAudMgr->SelectInputDevice(deviceId, false);
		} else if (deviceId == DEVICE_ACCOMPANY) {
			int retCode = StopAccompany();
		}
	}

	return AV_OK;
}

//摄像头
int SdkWrapper::OpenCamera()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return AV_ERR_FAILED;

	if (m_selectedCameraId == "") return AV_ERR_DEVICE_NOT_EXIST;

	AVCameraDevice *pCamera = (AVCameraDevice*)m_pVidMgr->GetDeviceById(m_selectedCameraId);
	if (pCamera == NULL) return AV_ERR_DEVICE_NOT_EXIST;


	//视频渲染前，先设定渲染画面大小和颜色格式。这些参数设置只影响到渲染，不影响到编解码。
	m_cameraParam.color_format = COLOR_FORMAT_I420;
	pCamera->SetPreviewParam(m_cameraParam.device_id, m_cameraParam.width, m_cameraParam.height, m_cameraParam.color_format);
	pCamera->SetPreviewCallback(&SdkWrapper::OnLocalVideoCallback, this);//获取设备的视频流
	pCamera->SetPreTreatmentFun(&SdkWrapper::OnPreTreatmentFun, this);//设置预处理回调
	return m_pVidMgr->SelectInputDevice(m_selectedCameraId, true);
}

int SdkWrapper::CloseCamera()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return AV_ERR_FAILED;

	if (m_selectedCameraId == "")return AV_ERR_DEVICE_NOT_EXIST;
	return m_pVidMgr->SelectInputDevice(m_selectedCameraId, false);
}

int SdkWrapper::SetSelectedCameraId(std::string cameraId)
{
	for (size_t i = 0; i < m_cameraList.size(); i++) {
		if (cameraId == m_cameraList[i].first) {
			m_selectedCameraId = cameraId;
			return AV_OK;
		}
	}

	return AV_ERR_DEVICE_NOT_EXIST;
}

std::string SdkWrapper::GetSelectedCameraId()
{
	return m_selectedCameraId;
}

int SdkWrapper::GetCameraList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &cameraList)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return AV_ERR_FAILED;

	AVDevice **ppCameraList = NULL;
	int cameraCount = m_pVidMgr->GetDeviceByType(DEVICE_CAMERA, &ppCameraList);
	if (ppCameraList == NULL || cameraCount == 0) {
		return AV_ERR_DEVICE_NOT_EXIST;
	}

	for (int i = 0; i < cameraCount; i++) {
		std::pair<std::string/*id*/, std::string/*name*/> camera;
		camera.first = ppCameraList[i]->GetId();
		camera.second = ppCameraList[i]->GetInfo().name;
		cameraList.push_back(camera);
	}

	if (ppCameraList)delete[]ppCameraList;//业务侧负责释放这个数组。
	return AV_OK;
}

//屏幕分享
int SdkWrapper::OpenScreenShareSend(uint32 left, uint32 top, uint32 right, uint32 bottom, uint32 fps)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return AV_ERR_FAILED;

	AVLocalScreenVideoDevice *pScreenShareSend = (AVLocalScreenVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_LOCAL_SCREEN_VIDEO);
	if (pScreenShareSend == NULL)return AV_ERR_DEVICE_NOT_EXIST;
	//视频渲染前，先设定渲染画面大小和颜色格式。这些参数设置只影响到渲染，不影响到编解码。
	pScreenShareSend->SetPreviewParam(m_screenSendParam.device_id, m_screenSendParam.width, m_screenSendParam.height, m_screenSendParam.color_format);
	pScreenShareSend->SetPreviewCallback(&SdkWrapper::OnLocalVideoCallback, this);//获取设备的视频流
	//pScreenShareSend->SetPreTreatmentFun(&SdkWrapper::OnPreTreatmentFun, this);//设置预处理回调
	pScreenShareSend->SetScreenCaptureParam(left, top, right, bottom, fps);
	return m_pVidMgr->SelectInputDevice(DEVICE_LOCAL_SCREEN_VIDEO, true);
}

int SdkWrapper::CloseScreenShareSend()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return AV_ERR_FAILED;

	return m_pVidMgr->SelectInputDevice(DEVICE_LOCAL_SCREEN_VIDEO, false);
}

void SdkWrapper::GetScreenCaptureParam(uint32 &left, uint32 &top, uint32 &right, uint32 &bottom, uint32 &fps)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return;

	AVLocalScreenVideoDevice *pScreenShareSend = (AVLocalScreenVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_LOCAL_SCREEN_VIDEO);
	if (pScreenShareSend == NULL)return;

	pScreenShareSend->GetScreenCaptureParam(left, top, right, bottom, fps);
}

void SdkWrapper::EnableScreenShareHDMode(bool isEnable)
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr) return;

	AVLocalScreenVideoDevice *pScreenShareSend = (AVLocalScreenVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_LOCAL_SCREEN_VIDEO);
	if (pScreenShareSend == NULL)return;
	pScreenShareSend->EnableHDMode(isEnable);
}

//设备管理
void SdkWrapper::GetAudioInputDeviceList()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr) return;

	int deviceCount = m_pAudMgr->GetInputDeviceCount();
	for (int i = 0; i < deviceCount; i++) {
		AVDevice *pAudioDevice = m_pAudMgr->GetInputDevice(i);
		if (pAudioDevice) {
			std::string name = pAudioDevice->GetInfo().name;
			std::string id = pAudioDevice->GetInfo().string_id;
		}
	}
}

void SdkWrapper::OnRemoteVideoCallback(VideoFrame *pFrameData, void *pCustomData)
{

#if 1
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	pSdkWrapper->OnRemoteVideoCallback(pFrameData);
#else
	std::vector<std::string> identifierList = pSdkWrapper->GetRequestViewIdentifierList();
	bool bHasRequestView = false;
	for (int i = 0; i < identifierList.size(); i++) {
		if (identifierList[i] == pFrameData->identifier) {
			bHasRequestView = true;
			break;
		}
	}

	if (!bHasRequestView) {
		return;
	}

	DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)pSdkWrapper->m_pMainDlg;
	if (pFrameData->desc.color_format == COLOR_FORMAT_RGB24) {
		if (pFrameData->desc.src_type == VIDEO_SRC_TYPE_CAMERA) {
			if (pMainDlg->m_viewRemoteVideoSmallRender1.m_identifier == pFrameData->identifier)pMainDlg->m_viewRemoteVideoSmallRender1.DoRender(pFrameData);
			else if (pMainDlg->m_viewRemoteVideoSmallRender2.m_identifier == pFrameData->identifier)pMainDlg->m_viewRemoteVideoSmallRender2.DoRender(pFrameData);
			else if (pMainDlg->m_viewRemoteVideoSmallRender3.m_identifier == pFrameData->identifier)pMainDlg->m_viewRemoteVideoSmallRender3.DoRender(pFrameData);
			else if (pMainDlg->m_viewRemoteVideoSmallRender4.m_identifier == pFrameData->identifier)pMainDlg->m_viewRemoteVideoSmallRender4.DoRender(pFrameData);
		} else if (pFrameData->desc.src_type == VIDEO_SRC_TYPE_SCREEN) {
			if (pMainDlg->m_viewScreenShareRender.m_identifier == pFrameData->identifier)pMainDlg->m_viewScreenShareRender.DoRender(pFrameData);
		}

		if (pMainDlg->m_viewBigVideoRender.m_identifier == pFrameData->identifier
			&& pMainDlg->m_viewBigVideoRender.m_videoSrcType == pFrameData->desc.src_type) {
			Dialog1v1VideoRender *p1v1VideoRenderDlg = (Dialog1v1VideoRender*)pSdkWrapper->m_p1v1VideoRenderDlg;
			if (p1v1VideoRenderDlg) {
				p1v1VideoRenderDlg->UpdateWindowPos(pFrameData->desc.width, pFrameData->desc.height);
				p1v1VideoRenderDlg->m_view1v1VideoRender.m_identifier = pMainDlg->m_viewBigVideoRender.m_identifier;
				p1v1VideoRenderDlg->m_view1v1VideoRender.m_videoSrcType = pMainDlg->m_viewBigVideoRender.m_videoSrcType;
				p1v1VideoRenderDlg->m_view1v1VideoRender.DoRender(pFrameData);
			} else {
				pMainDlg->m_viewBigVideoRender.DoRender(pFrameData);
			}
		}
	} else {
		if (pMainDlg->m_viewBigVideoRender.m_identifier == pFrameData->identifier)pMainDlg->SaveRemoteYuvFrame(pFrameData);
	}
#endif
}

void SdkWrapper::OnLocalVideoCallback(VideoFrame *pFrameData, void *pCustomData)
{
#if 1
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	pSdkWrapper->OnLocalVideoCallback(pFrameData);
#else
	if (pFrameData->desc.src_type == VIDEO_SRC_TYPE_CAMERA)// && pSdkWrapper->GetDefaultCameraState())//自己有打开摄像头，才去渲染
	{
		DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)pSdkWrapper->m_pMainDlg;

		if (pMainDlg->m_viewLocalVideoRender.m_identifier == pMainDlg->m_contextStartParam.identifier &&
			pMainDlg->m_viewLocalVideoRender.m_videoSrcType == pFrameData->desc.src_type) {
			pMainDlg->m_viewLocalVideoRender.DoRender(pFrameData);
		}

		if (pMainDlg->m_viewBigVideoRender.m_identifier == pMainDlg->m_contextStartParam.identifier &&
			pMainDlg->m_viewBigVideoRender.m_videoSrcType == pFrameData->desc.src_type) {
			Dialog1v1VideoRender *p1v1VideoRenderDlg = (Dialog1v1VideoRender*)pSdkWrapper->m_p1v1VideoRenderDlg;
			if (p1v1VideoRenderDlg) {
				p1v1VideoRenderDlg->UpdateWindowPos(pFrameData->desc.width, pFrameData->desc.height);
				p1v1VideoRenderDlg->m_view1v1VideoRender.m_identifier = pMainDlg->m_viewBigVideoRender.m_identifier;
				p1v1VideoRenderDlg->m_view1v1VideoRender.m_videoSrcType = pMainDlg->m_viewBigVideoRender.m_videoSrcType;
				p1v1VideoRenderDlg->m_view1v1VideoRender.DoRender(pFrameData);
			} else {
				pMainDlg->m_viewBigVideoRender.DoRender(pFrameData);
			}
		}
	} else if (pFrameData->desc.src_type == VIDEO_SRC_TYPE_SCREEN && pSdkWrapper->GetDefaultScreenShareSendState())//自己有打开屏幕分享，才去渲染
	{
		DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)pSdkWrapper->m_pMainDlg;

		if (pMainDlg->m_viewScreenShareRender.m_identifier == pMainDlg->m_contextStartParam.identifier &&
			pMainDlg->m_viewScreenShareRender.m_videoSrcType == pFrameData->desc.src_type) {
			pMainDlg->m_viewScreenShareRender.DoRender(pFrameData);
		}

		if (pMainDlg->m_viewBigVideoRender.m_identifier == pMainDlg->m_contextStartParam.identifier &&
			pMainDlg->m_viewBigVideoRender.m_videoSrcType == pFrameData->desc.src_type) {
			Dialog1v1VideoRender *p1v1VideoRenderDlg = (Dialog1v1VideoRender*)pSdkWrapper->m_p1v1VideoRenderDlg;
			if (p1v1VideoRenderDlg) {
				p1v1VideoRenderDlg->UpdateWindowPos(pFrameData->desc.width, pFrameData->desc.height);
				p1v1VideoRenderDlg->m_view1v1VideoRender.m_identifier = pMainDlg->m_viewBigVideoRender.m_identifier;
				p1v1VideoRenderDlg->m_view1v1VideoRender.m_videoSrcType = pMainDlg->m_viewBigVideoRender.m_videoSrcType;
				p1v1VideoRenderDlg->m_view1v1VideoRender.DoRender(pFrameData);
			} else {
				pMainDlg->m_viewBigVideoRender.DoRender(pFrameData);
			}
		}
	}
#endif
}

std::string SdkWrapper::GetQualityTips()
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return "";
	std::string tipsAll = "";
	std::string tipsAudio = "";
	std::string tipsVideo = "";
	std::string tipsRoom = "";

	tipsAudio = m_pAudCtrl->GetQualityTips();
	tipsVideo = m_pVidCtrl->GetQualityTips();
	tipsRoom = m_pRoom->GetQualityTips();

	if (tipsRoom != "") {
		tipsAll.append(tipsRoom);
		tipsAll.append("\r\n");
	}

	if (tipsAudio != "") {
		tipsAll.append(tipsAudio);
		tipsAll.append("\r\n");
	}

	if (tipsVideo != "") {
		tipsAll.append(tipsVideo);
	}

	return tipsAll;
}

std::string SdkWrapper::GetRoomStatParam()
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return "";
	std::string AllParas = "";

	RoomStatParam avParas;
	memset(&avParas, 0, sizeof(RoomStatParam));

	bool result = m_pRoom->GetQualityParam(&avParas);
#if 0
	if (result) {

		CString sysparas;
		sysparas.Format(_T("系统参数：开始时间本地=%u, 结束时间本地=%u,系统CPU使用率=%u,进程CPU使用率=%u\r\n"),
			avParas.tick_count_begin, avParas.tick_count_end, avParas.network_param.cpu_rate_sys / 100, avParas.network_param.cpu_rate_app / 100);
		std::string SysParas = StrWToStrA(sysparas);

		if (SysParas != "") {
			AllParas.append(SysParas);
			AllParas.append("\r\n");
		}

		CString netparas;
		netparas.Format(_T("网络参数：发包速率=%u,发包个数=%u,上行丢包率=%u,上行平均连续丢包个数=%u,收包速率=%u,收包个数=%u,下行丢包率=%u,下行平均连续丢包个数=%u,往返时延=%u\r\n"),
			avParas.network_param.kbps_send, avParas.network_param.packet_send, avParas.network_param.loss_rate_send, avParas.network_param.loss_model_send,
			avParas.network_param.kbps_recv, avParas.network_param.packet_recv, avParas.network_param.loss_rate_recv, avParas.network_param.loss_model_recv, avParas.network_param.rtt);

		std::string NetParas = StrWToStrA(netparas);

		if (NetParas != "") {
			AllParas.append(NetParas);
			AllParas.append("\r\n");
		}

		CString ctrlparas;
		ctrlparas.Format(_T("流控下发：音频：采样率=%d, 通道数=%d, 编码类型=%d, 码率=%d，视频（大画面）：分辨率=%d * %d，码率=%d，帧率=%d，视频（小画面）：分辨率=%d * %d，码率=%d，帧率=%d\r\n"), avParas.audio_param.qos_param.sample_rate, avParas.audio_param.qos_param.channel_count, avParas.audio_param.qos_param.codec_type, avParas.audio_param.qos_param.bitrate,
			avParas.video_param.qos_param_big.width, avParas.video_param.qos_param_big.height, avParas.video_param.qos_param_big.bitrate, avParas.video_param.qos_param_big.fps, avParas.video_param.qos_param_small.width, avParas.video_param.qos_param_small.height, avParas.video_param.qos_param_small.bitrate, avParas.video_param.qos_param_small.fps);

		std::string Ctrlparas = StrWToStrA(ctrlparas);

		if (Ctrlparas != "") {
			AllParas.append(Ctrlparas);
			AllParas.append("\r\n");
		}

		CString videoencparas;
		std::string VideoEncparas;

		int i;

		for (i = 0; i < avParas.video_param.encode_count; i++) {
			videoencparas.Format(_T("视频编码信息\r\n画面类型=%u，编码宽=%u，编码高=%u，编码实时帧率=%u，编码码率（无包头）=%u\r\n"), avParas.video_param.encode_params[i].view_type, avParas.video_param.encode_params[i].width, avParas.video_param.encode_params[i].height,
				avParas.video_param.encode_params[i].fps, avParas.video_param.encode_params[i].bitrate);
			VideoEncparas = StrWToStrA(videoencparas);
			if (VideoEncparas != "") {
				AllParas.append(VideoEncparas);
				AllParas.append("\r\n");
			}
		}

		CString videodecparas;
		std::string VideoDecparas;
		for (i = 0; i < avParas.video_param.decode_count; i++) {
			videodecparas.Format(_T("视频解码信息\r\n用户UIN =%u, 画面类型=%u，解码宽=%u，解码高=%u，解码后帧率=%u，解码后码率（无包头）=%u\r\n"), avParas.video_param.decode_params[i].tiny_id, avParas.video_param.decode_params[i].view_type, avParas.video_param.decode_params[i].width,
				avParas.video_param.decode_params[i].height, avParas.video_param.decode_params[i].fps, avParas.video_param.decode_params[i].bitrate);
			VideoDecparas = StrWToStrA(videodecparas);
			if (VideoDecparas != "") {
				AllParas.append(VideoDecparas);
				AllParas.append("\r\n");
			}

		}

		CString capture;
		capture.Format(_T("采集分辨率=%d * %d\r\n\r\n"), avParas.video_param.capture_param.width, avParas.video_param.capture_param.height);
		std::string CapParas = StrWToStrA(capture);



		if (CapParas != "") {
			AllParas.append(CapParas);
			AllParas.append("\r\n");
		}

	} else 
		AllParas.append("getparas failed");

#endif
	return AllParas;

}

int SdkWrapper::AddWaterMark(int preset, int *pARGBData, int width, int height)
{
	if (!m_pVidCtrl) {
		return AV_ERR_FAILED;
	}
	return m_pVidCtrl->AddWaterMark(preset, pARGBData, width, height);
}

void SdkWrapper::OnStartContextComplete(int result)
{
	if (result != AV_OK) {
		//::PostMessage(GetMainHWnd(), WM_ON_START_CONTEXT, (WPARAM)result, 0);
		return;
	}

	m_pAudCtrl = dynamic_cast<AVAudioCtrl*>(m_pContext->GetAudioCtrl());
	m_pVidCtrl = dynamic_cast<AVVideoCtrl*>(m_pContext->GetVideoCtrl());

	if (!m_pAudCtrl || !m_pVidCtrl) {
		//::PostMessage(GetMainHWnd(), WM_ON_START_CONTEXT, (WPARAM)AV_ERR_FAILED, 0);
		return;
	}

	//::SendMessage(GetMainHWnd(), WM_ON_START_CONTEXT, (WPARAM)result, 0);
}

void SdkWrapper::OnStopContextComplete()
{
	m_pAudCtrl = NULL;
	m_pVidCtrl = NULL;

	//::PostMessage(GetMainHWnd(), WM_ON_STOP_CONTEXT, (WPARAM)AV_OK, 0);
}

void SdkWrapper::OnEnterRoomComplete(int result)
{
	if (result != AV_OK) {
		//::PostMessage(GetMainHWnd(), WM_ON_ENTER_ROOM, (WPARAM)result, 0);
		return;
	}

	m_pAudMgr = dynamic_cast<AVDeviceMgr*>(m_pContext->GetAudioDeviceMgr());
	m_pVidMgr = dynamic_cast<AVDeviceMgr*>(m_pContext->GetVideoDeviceMgr());

	if (!m_pAudMgr || !m_pAudMgr) {
		//::PostMessage(GetMainHWnd(), WM_ON_ENTER_ROOM, (WPARAM)AV_ERR_FAILED, 0);
		return;
	}

	m_pVidMgr->SetDeviceOperationCallback(OnVideoDeviceOperationCallback, this);
	m_pVidMgr->SetDeviceChangeCallback(OnVideoDeviceChangeCallback, this);
	//创建房间后，设置远程视频回调
	AVRemoteVideoDevice *pVideoDevice = (AVRemoteVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_REMOTE_VIDEO);
	if (pVideoDevice) {
		//视频渲染前，先设定渲染画面大小和颜色格式。这些参数设置只影响到渲染，不影响到编解码。
		m_pVidMgr->SelectOutputDevice(DEVICE_REMOTE_VIDEO, true);
		pVideoDevice->SetPreviewParam(m_remoteVideoParam.device_id, m_remoteVideoParam.width, m_remoteVideoParam.height, m_remoteVideoParam.color_format);
		pVideoDevice->SetPreviewCallback(&SdkWrapper::OnRemoteVideoCallback, this);
	}

	AVRemoteScreenVideoDevice *pScreenShareRecvDevice = (AVRemoteScreenVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_REMOTE_SCREEN_VIDEO);
	if (pScreenShareRecvDevice) {
		//视频渲染前，先设定渲染画面大小和颜色格式。这些参数设置只影响到渲染，不影响到编解码。
		m_pVidMgr->SelectOutputDevice(DEVICE_REMOTE_SCREEN_VIDEO, true);
		pScreenShareRecvDevice->SetPreviewParam(m_screenRecvParam.device_id, m_screenRecvParam.width, m_screenRecvParam.height, m_screenRecvParam.color_format);
		pScreenShareRecvDevice->SetPreviewCallback(&SdkWrapper::OnRemoteVideoCallback, this);
	}

	int retCode = AV_OK;
	retCode = GetCameraList(m_cameraList);
	if (m_selectedCameraId == "") {
		m_selectedCameraId = retCode == AV_OK ? m_cameraList[0].first : "";
	}

	m_pAudMgr->SetDeviceOperationCallback(OnAudioDeviceOperationCallback, this);
	m_pAudMgr->SetDeviceDetectNotify(OnAudioDeviveDetectNotify, this);

	retCode = GetMicList(m_micList);
	m_selectedMicId = retCode == AV_OK ? m_micList[0].first : "";

	retCode = GetPlayerList(m_playerList);
	m_selectedPlayerId = retCode == AV_OK ? m_playerList[0].first : "";

	m_pRoom = dynamic_cast<AVRoomMulti*>(m_pContext->GetRoom());
	if (!m_pRoom) {
		//::PostMessage(GetMainHWnd(), WM_ON_ENTER_ROOM, (WPARAM)AV_ERR_FAILED, 0);
		return;
	}
	//::PostMessage(GetMainHWnd(), WM_ON_ENTER_ROOM, (WPARAM)result, 0);
}

void SdkWrapper::OnExitRoomComplete()
{
	OnExitRoomComplete(AV_OK);
}
void SdkWrapper::OnRoomDisconnect(int reason)
{
	OnExitRoomComplete(reason);
}

void SdkWrapper::OnExitRoomComplete(int result)
{
	m_pRoom = NULL;
	m_curRequestViewIdentifierList.clear();
	m_curRequestViewParamList.clear();

	m_pVidMgr = NULL;
	m_cameraList.clear();

	m_selectedCameraId = "";
	m_isSelectedCameraEnable = false;
	m_isSelectedScreenShareSendEnable = false;

	m_pAudMgr = NULL;
	m_micList.clear();
	m_playerList.clear();

	m_selectedMicId = "";
	m_selectedPlayerId = "";
	m_isSelectedMicEnable = false;
	m_isSelectedPlayerEnable = false;

	//::PostMessage(GetMainHWnd(), WM_ON_EXIT_ROOM, (WPARAM)result, 0);
}

void SdkWrapper::OnChangeAuthority(int32 ret_code)
{
	//DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)m_pMainDlg;
	//pMainDlg->ChangeAuthComplete(ret_code);
}

void SdkWrapper::OnChangeRole(int32 ret_code)
{
	//DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)m_pMainDlg;
	//pMainDlg->ChangeRoleComplete(ret_code);
}


void SdkWrapper::OnEndpointsUpdateInfo(AVRoomMulti::EndpointEventId eventid, std::vector<std::string> updatelist)
{
#if 0
	if (eventid == AVRoomMulti::EVENT_ID_ENDPOINT_EXIT) {
		for (int i = 0; i < updatelist.size(); i++) {
			for (int j = 0; j < m_curRequestViewIdentifierList.size(); j++) {
				//如果当前所要请求的画面的人已经离开房间，则不再渲染他的画面，并清除画面。
				if (m_curRequestViewIdentifierList[j] == updatelist[i]) {
					m_curRequestViewIdentifierList.erase(m_curRequestViewIdentifierList.begin() + j);
					m_curRequestViewParamList.erase(m_curRequestViewParamList.begin() + j);
					DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)m_pMainDlg;
					if (pMainDlg) {
						if (pMainDlg->m_viewBigVideoRender.m_identifier == updatelist[i]) {
							pMainDlg->m_viewBigVideoRender.Clear();
						}

						if (pMainDlg->m_viewLocalVideoRender.m_identifier == updatelist[i]) {
							pMainDlg->m_viewLocalVideoRender.Clear();
						}

						if (pMainDlg->m_viewScreenShareRender.m_identifier == updatelist[i]) {
							pMainDlg->m_viewScreenShareRender.Clear();
						}

						if (pMainDlg->m_viewRemoteVideoSmallRender1.m_identifier == updatelist[i]) {
							pMainDlg->m_viewRemoteVideoSmallRender1.Clear();
						} else if (pMainDlg->m_viewRemoteVideoSmallRender2.m_identifier == updatelist[i]) {
							pMainDlg->m_viewRemoteVideoSmallRender2.Clear();
						} else if (pMainDlg->m_viewRemoteVideoSmallRender3.m_identifier == updatelist[i]) {
							pMainDlg->m_viewRemoteVideoSmallRender3.Clear();
						} else if (pMainDlg->m_viewRemoteVideoSmallRender4.m_identifier == updatelist[i]) {
							pMainDlg->m_viewRemoteVideoSmallRender4.Clear();
						}
					}

					Dialog1v1VideoRender *p1v1VideoRenderDlg = (Dialog1v1VideoRender*)m_p1v1VideoRenderDlg;
					if (p1v1VideoRenderDlg && p1v1VideoRenderDlg->m_view1v1VideoRender.m_identifier == updatelist[i]) {
						p1v1VideoRenderDlg->m_view1v1VideoRender.Clear();
					}
					break;
				}
			}
		}
	} else if (eventid == AVRoomMulti::EVENT_ID_ENDPOINT_NO_CAMERA_VIDEO || eventid == AVRoomMulti::EVENT_ID_ENDPOINT_NO_SCREEN_VIDEO) {
		VideoSrcType videoSrcType = eventid == AVRoomMulti::EVENT_ID_ENDPOINT_NO_CAMERA_VIDEO ? VIDEO_SRC_TYPE_CAMERA : VIDEO_SRC_TYPE_SCREEN;
		for (int i = 0; i < updatelist.size(); i++) {
			for (int j = 0; j < m_curRequestViewIdentifierList.size(); j++) {
				//如果当前所要请求的画面的人已经离开房间，则不再渲染他的画面，并清除画面。
				if (m_curRequestViewIdentifierList[j] == updatelist[i] && m_curRequestViewParamList[j].video_src_type == videoSrcType) {
					m_curRequestViewIdentifierList.erase(m_curRequestViewIdentifierList.begin() + j);
					m_curRequestViewParamList.erase(m_curRequestViewParamList.begin() + j);
					DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)m_pMainDlg;
					if (pMainDlg) {
						if (pMainDlg->m_viewBigVideoRender.m_identifier == updatelist[i] && pMainDlg->m_viewBigVideoRender.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewBigVideoRender.Clear();
						}

						if (pMainDlg->m_viewLocalVideoRender.m_identifier == updatelist[i] && pMainDlg->m_viewLocalVideoRender.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewLocalVideoRender.Clear();
						}

						if (pMainDlg->m_viewScreenShareRender.m_identifier == updatelist[i] && pMainDlg->m_viewScreenShareRender.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewScreenShareRender.Clear();
						}

						if (pMainDlg->m_viewRemoteVideoSmallRender1.m_identifier == updatelist[i] && pMainDlg->m_viewRemoteVideoSmallRender1.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewRemoteVideoSmallRender1.Clear();
						} else if (pMainDlg->m_viewRemoteVideoSmallRender2.m_identifier == updatelist[i] && pMainDlg->m_viewRemoteVideoSmallRender2.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewRemoteVideoSmallRender2.Clear();
						} else if (pMainDlg->m_viewRemoteVideoSmallRender3.m_identifier == updatelist[i] && pMainDlg->m_viewRemoteVideoSmallRender3.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewRemoteVideoSmallRender3.Clear();
						} else if (pMainDlg->m_viewRemoteVideoSmallRender4.m_identifier == updatelist[i] && pMainDlg->m_viewRemoteVideoSmallRender4.m_videoSrcType == videoSrcType) {
							pMainDlg->m_viewRemoteVideoSmallRender4.Clear();
						}
					}

					Dialog1v1VideoRender *p1v1VideoRenderDlg = (Dialog1v1VideoRender*)m_p1v1VideoRenderDlg;
					if (p1v1VideoRenderDlg && p1v1VideoRenderDlg->m_view1v1VideoRender.m_identifier == updatelist[i] && p1v1VideoRenderDlg->m_view1v1VideoRender.m_videoSrcType == videoSrcType) {
						p1v1VideoRenderDlg->m_view1v1VideoRender.Clear();
					}
					break;
				}
			}
		}
	}
#endif
	OnEndpointsUpdateInfo();
}

void SdkWrapper::OnEndpointsUpdateInfo()
{
	if (!m_pContext || !m_pRoom || !m_pAudCtrl || !m_pVidCtrl || !m_pAudMgr || !m_pVidMgr)return;
	//::PostMessage(GetMainHWnd(), WM_ON_ENDPOINTS_UPDATE_INFO, AV_OK, 0);
}
void SdkWrapper::OnPrivilegeDiffNotify(int32 privilege)
{
	//TODO, to process room event
}

void SdkWrapper::OnCameraSettingNotify(int width, int height, int fps)
{

}

//处理进房间时自动接收的摄像头视频的事件通知。
void SdkWrapper::OnSemiAutoRecvCameraVideo(std::vector<std::string> identifierList)
{
	if (identifierList.size() > 0 && m_pVidMgr != NULL) {
		//开启远端设备，才能获取视频流进行渲染。
		AVRemoteVideoDevice *pVideoDevice = (AVRemoteVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_REMOTE_VIDEO);
		if (pVideoDevice) {
			//视频渲染前，先设定渲染画面大小和颜色格式。这些参数设置只影响到渲染，不影响到编解码。
			pVideoDevice->SetPreviewParam(m_remoteVideoParam.device_id, m_remoteVideoParam.width, m_remoteVideoParam.height, m_remoteVideoParam.color_format);
			pVideoDevice->SetPreviewCallback(&SdkWrapper::OnRemoteVideoCallback, this);
		}

		m_pVidMgr->SelectOutputDevice(DEVICE_REMOTE_VIDEO, true);

		std::vector<View> viewList;
		for (size_t i = 0; i < identifierList.size(); i++) {
			View view;
			view.video_src_type = VIDEO_SRC_TYPE_CAMERA;
			view.size_type = VIEW_SIZE_TYPE_BIG;//TODO这边暂时hardcode大小为大画面。实际上画面大小类型，可以通过实际收到的视频大小来获取。
			viewList.push_back(view);
		}

		//这些自动接收的视频所对应的画面信息，其实相当于属于已经请求的，所以要备份下来。
		UpdateRequestViewIdentifierList(identifierList);
		UpdateRequestViewParamList(viewList);
	}
}

void SdkWrapper::OnStartContextCompleteCallback(int result, void *pCustomData)
{
	((SdkWrapper*)pCustomData)->OnStartContextComplete(result);
}

void SdkWrapper::OnRequestViewListCompleteCallback(std::vector<std::string> identifierList, std::vector<View> viewList, int32 result, void *pCustomData)
{
	if (result == AV_OK) {
		SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
		pSdkWrapper->UpdateRequestViewIdentifierList(identifierList);
		pSdkWrapper->UpdateRequestViewParamList(viewList);
	}
}

void SdkWrapper::OnCancelAllViewCompleteCallback(int result, void *pCustomData)
{
}

void SdkWrapper::OnAudioDeviceOperationCallback(AVDeviceMgr *pAudMgr, AVDevice::DeviceOperation oper, const std::string &deviceId, int retCode, void *pCustomData)
{
	if (!pAudMgr)return;
	AVDevice *pDev = pAudMgr->GetDeviceById(deviceId);
	if (!pDev)return;

	if (pDev->GetType() == DEVICE_MIC) {
		if (retCode == AV_OK) {
			SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
			pSdkWrapper->SetDefaultMicState(oper == AVDevice::DEVICE_OPERATION_OPEN);
		}
	} else if (pDev->GetType() == DEVICE_PLAYER) {
		if (retCode == AV_OK) {
			SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
			pSdkWrapper->SetDefaultPlayerState(oper == AVDevice::DEVICE_OPERATION_OPEN);
		}
	} else if (pDev->GetType() == DEVICE_ACCOMPANY) {
		//TODO
	} else {
		//TODO
	}
}

void SdkWrapper::OnVideoDeviceOperationCallback(AVDeviceMgr *pVidMgr, AVDevice::DeviceOperation oper, const std::string &deviceId, int retCode, void *pCustomData)
{
	if (!pVidMgr)return;
	AVDevice *pDev = pVidMgr->GetDeviceById(deviceId);
	if (!pDev)return;

	if (pDev->GetType() == DEVICE_CAMERA) {
		if (retCode == AV_OK) {
			SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
			pSdkWrapper->SetDefaultCameraState(oper == AVDevice::DEVICE_OPERATION_OPEN);
		}
	}
	//else if(pDev->GetType() == DEVICE_REMOTE_VIDEO)
	//{
	//	//TODO
	//}	
	else if (pDev->GetType() == DEVICE_LOCAL_SCREEN_VIDEO) {
		if (retCode == AV_OK) {
			SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
			pSdkWrapper->SetDefaultScreenShareSendState(oper == AVDevice::DEVICE_OPERATION_OPEN);
		}
	}
	//else if(pDev->GetType() == DEVICE_REMOTE_SCREEN_VIDEO)
	//{
	//	//TODO
	//}
	//else
	//{
	//	//TODO
	//}
}

void SdkWrapper::OnVideoDeviceChangeCallback(AVDeviceMgr *pVidMgr, void *pCustomData)
{
	if (!pVidMgr)return;
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	int retCode = AV_OK;
	retCode = pSdkWrapper->GetCameraList(pSdkWrapper->m_cameraList);
	if (pSdkWrapper->m_selectedCameraId == "") {
		pSdkWrapper->m_selectedCameraId = retCode == AV_OK ? pSdkWrapper->m_cameraList[0].first : "";
	}
}

void SdkWrapper::OnPreTreatmentFun(VideoFrame *pFrameData, void *pCustomData)
{
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	if (!pSdkWrapper->GetDefaultCameraState())return;//自己没有打开摄像头，过滤掉
	//DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)pSdkWrapper->m_pMainDlg;
	//pMainDlg->OnVideoPreTreatment(pFrameData);
}

void SdkWrapper::OnChangeAuthorityCallback(int32 ret_code, void *pCustomData)
{
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	pSdkWrapper->OnChangeAuthority(ret_code);
}

void SdkWrapper::OnChangeRoleCallback(int32 ret_code, void *pCustomData)
{
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	pSdkWrapper->OnChangeRole(ret_code);
}

//外部捕获

/*
设置外部捕获的视频设备能力。
注意：
. 外部捕获跟内部摄像头是互斥的。
. 如果设置了外部捕获设备的能力后，就不能再使用内部摄像头了；也就是，如果要使用内部摄像头，就不能设置外部捕获设备的能力。
. 如果要使用外部捕获设备，不设置该能力也可以正常使用；当然，设置了，会给SDK提供更为准确的能力信息。
. 如果要设置，必须在进入房间前就设置好。
*/
int SdkWrapper::SetExternalCamAbility(CameraInfo* pCamInfo)
{
	if (!m_pContext) return AV_ERR_FAILED;

	if (!m_pVidCtrl)
		m_pVidCtrl = dynamic_cast<AVVideoCtrl*>(m_pContext->GetVideoCtrl());

	if (!m_pVidCtrl) return AV_ERR_FAILED;

	return m_pVidCtrl->SetExternalCapAbility(pCamInfo);
}

int SdkWrapper::OpenExternalCapture()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr)return AV_ERR_FAILED;
	AVExternalCapture *pExternalCapture = (AVExternalCapture*)m_pVidMgr->GetDeviceById(DEVICE_EXTERNAL_CAPTURE);
	if (pExternalCapture == NULL)return AV_ERR_DEVICE_NOT_EXIST;
	return m_pVidMgr->SelectInputDevice(DEVICE_EXTERNAL_CAPTURE, true);
}

int SdkWrapper::CloseExternalCapture()
{
	if (!m_pContext || !m_pAudCtrl || !m_pVidCtrl || !m_pVidMgr)return AV_ERR_FAILED;
	return m_pVidMgr->SelectInputDevice(DEVICE_EXTERNAL_CAPTURE, false);
}

int SdkWrapper::FillExternalCaptureFrame(VideoFrame* pFrame)
{
	if (m_pVidMgr) {
		AVExternalCapture* pExternalCapture = (AVExternalCapture*)(m_pVidMgr->GetDeviceById(DEVICE_EXTERNAL_CAPTURE));
		return pExternalCapture->OnCaptureFrame(*pFrame);
	}
	return AV_ERR_FAILED;
}

void SdkWrapper::SetVideoParam(AVSupportVideoPreview::PreviewParam param)
{
	m_remoteVideoParam.device_id = "";
	m_remoteVideoParam.width = param.width;
	m_remoteVideoParam.height = param.height;
	m_remoteVideoParam.color_format = param.color_format;
	m_remoteVideoParam.src_type = VIDEO_SRC_TYPE_CAMERA;

	if (m_pVidMgr) {
		AVRemoteVideoDevice *pVideoDevice = (AVRemoteVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_REMOTE_VIDEO);
		if (pVideoDevice) {
			//视频渲染前，先设定渲染画面大小和颜色格式。这些参数设置只影响到渲染，不影响到编解码。
			pVideoDevice->SetPreviewParam(m_remoteVideoParam.device_id, m_remoteVideoParam.width, m_remoteVideoParam.height, m_remoteVideoParam.color_format);
		}
	}
}

void SdkWrapper::ClearVideoParam()
{
	if (m_pVidMgr) {
		AVRemoteVideoDevice *pVideoDevice = (AVRemoteVideoDevice*)m_pVidMgr->GetDeviceById(DEVICE_REMOTE_VIDEO);
		if (pVideoDevice) {
			//清空预览参数，拿原始数据。
			pVideoDevice->ClearPreviewParam();
		}
	}
}

void SdkWrapper::EnableHostMode()
{

}
void SdkWrapper::OnAudioDeviveDetectNotify(AVDeviceMgr* device_mgr, DetectedDeviceInfo& info, bool*pbselect, void* custom_data)
{
	if (custom_data) {
		SdkWrapper*p = (SdkWrapper*)custom_data;
		p->OnDeviveDetecNotify(device_mgr, info, pbselect, custom_data);
	}

}

void SdkWrapper::OnDeviveDetecNotify(AVDeviceMgr* device_mgr, DetectedDeviceInfo& info, bool*pbSelect, void* custom_data)
{
	//是否启用新的设备
	if (info.isNewDevice) {
		//		int ret = ::MessageBoxW(GetMainHWnd(), L"是否要启用新插入的设备",L"警告", MB_OKCANCEL);
		// 		if (IDOK != ret && pbSelect ){
		// 			*pbSelect = false;
		// 		}
		// 		else
		{
			//*pbSelect = false;
			if (info.flow == Detect_Mic)
				m_selectedMicId = info.strGuid;
			else
				m_selectedPlayerId = info.strGuid;
		}
	} else {
		int deviceCount = device_mgr->GetInputDeviceCount();
		if (info.flow == Detect_Speaker)
			deviceCount = device_mgr->GetOutputDeviceCount();

		if (info.isUsedDevice && deviceCount > 0) {
			// 			int ret = ::MessageBoxW(GetMainHWnd(), L"正在使用的设备被拔出，是否要启用默认设备?",L"警告", MB_OKCANCEL);
			// 			if (IDOK != ret && pbSelect ){
			// 				*pbSelect = false;
			// 			}
			// 			else
			{
				//*pbSelect = false;
				if (info.flow == Detect_Mic)
					m_selectedMicId = device_mgr->GetInputDevice(0)->GetId();
				else
					m_selectedPlayerId = device_mgr->GetOutputDevice(0)->GetId();
			}
		} else if (info.isUsedDevice && deviceCount == 0) {
			//int ret = ::MessageBoxW(GetMainHWnd(), L"音频设备被弹出，请插入其它设备", L"警告", MB_OK);
		}
	}

	//简单判断下是否已经进入房间了
	if (m_pRoom) {
		DetectedDeviceInfo *pInfo = new DetectedDeviceInfo();
		*pInfo = info;
	}

}

int SdkWrapper::StartDeviceTest()
{
	if (m_pContext == NULL)return AV_ERR_FAILED;
	return m_pContext->StartDeviceTest(SdkWrapper::OnStartDeviceTestCompleteCallback, this);
}

int SdkWrapper::StopDeviceTest()
{
	if (m_pContext == NULL)return AV_ERR_FAILED;
	m_deviceTestSelectedMicId = "";
	m_deviceTestSelectedPlayerId = "";
	m_deviceTestSelectedCameraId = "";

	m_pDeviceTest = NULL;
	return m_pContext->StopDeviceTest(SdkWrapper::OnStopDeviceTestCompleteCallback, this);
}


void SdkWrapper::DeviceTestSetSelectedMicId(std::string deviceId)
{
	m_deviceTestSelectedMicId = deviceId;
}

int SdkWrapper::DeviceTestOpenMic()
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	return m_pDeviceTest->EnableDevice(m_deviceTestSelectedMicId, true);
}

int SdkWrapper::DeviceTestCloseMic()
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	return m_pDeviceTest->EnableDevice(m_deviceTestSelectedMicId, false);
}

int SdkWrapper::DeviceTestGetMicList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &deviceList)
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	int deviceCount = m_pDeviceTest->GetDeviceCountByType(DEVICE_MIC);
	std::vector<std::string> idList = m_pDeviceTest->GetDeviceIdListByType(DEVICE_MIC);
	std::vector<std::string> nameList = m_pDeviceTest->GetDeviceNameListByType(DEVICE_MIC);

	for (int i = 0; i < deviceCount; i++) {
		std::pair<std::string/*id*/, std::string/*name*/> device;
		device.first = idList[i];
		device.second = nameList[i];
		deviceList.push_back(device);
	}

	return AV_OK;
}

void SdkWrapper::DeviceTestSetMicVolume(uint32 value)
{
	if (!m_pDeviceTest) return;
	AVMicDevice *pDevice = (AVMicDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedMicId);
	if (pDevice)pDevice->SetVolume(value);
}

uint32 SdkWrapper::DeviceTestGetMicVolume()
{
	if (!m_pDeviceTest) return MIN_AUDIO_DEVICE_VOLUME;
	AVMicDevice *pDevice = (AVMicDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedMicId);
	if (pDevice)return pDevice->GetVolume();
	else return MIN_AUDIO_DEVICE_VOLUME;
}

uint32 SdkWrapper::DeviceTestGetMicDynamicVolume()
{
	if (!m_pDeviceTest) return MIN_AUDIO_DEVICE_VOLUME;
	AVMicDevice *pDevice = (AVMicDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedMicId);
	if (pDevice)return pDevice->GetDynamicVolume();
	else return MIN_AUDIO_DEVICE_VOLUME;
}


void SdkWrapper::DeviceTestSetSelectedPlayerId(std::string deviceId)
{
	m_deviceTestSelectedPlayerId = deviceId;
}

int SdkWrapper::DeviceTestOpenPlayer()
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	return m_pDeviceTest->EnableDevice(m_deviceTestSelectedPlayerId, true);
}

int SdkWrapper::DeviceTestClosePlayer()
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	return m_pDeviceTest->EnableDevice(m_deviceTestSelectedPlayerId, false);
}

int SdkWrapper::DeviceTestGetPlayerList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &deviceList)
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	int deviceCount = m_pDeviceTest->GetDeviceCountByType(DEVICE_PLAYER);
	std::vector<std::string> idList = m_pDeviceTest->GetDeviceIdListByType(DEVICE_PLAYER);
	std::vector<std::string> nameList = m_pDeviceTest->GetDeviceNameListByType(DEVICE_PLAYER);

	for (int i = 0; i < deviceCount; i++) {
		std::pair<std::string/*id*/, std::string/*name*/> device;
		device.first = idList[i];
		device.second = nameList[i];
		deviceList.push_back(device);
	}

	return AV_OK;
}

void SdkWrapper::DeviceTestSetPlayerVolume(uint32 value)
{
	if (!m_pDeviceTest) return;
	AVPlayerDevice *pDevice = (AVPlayerDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedPlayerId);
	if (pDevice)pDevice->SetVolume(value);
}

uint32 SdkWrapper::DeviceTestGetPlayerVolume()
{
	if (!m_pDeviceTest) return MIN_AUDIO_DEVICE_VOLUME;
	AVPlayerDevice *pDevice = (AVPlayerDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedPlayerId);
	if (pDevice)return pDevice->GetVolume();
	else return MIN_AUDIO_DEVICE_VOLUME;
}

uint32 SdkWrapper::DeviceTestGetPlayerDynamicVolume()
{
	if (!m_pDeviceTest) return MIN_AUDIO_DEVICE_VOLUME;
	AVPlayerDevice *pDevice = (AVPlayerDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedPlayerId);
	if (pDevice)return pDevice->GetDynamicVolume();
	else return MIN_AUDIO_DEVICE_VOLUME;
}


void SdkWrapper::DeviceTestSetSelectedCameraId(std::string deviceId)
{
	m_deviceTestSelectedCameraId = deviceId;
}

int SdkWrapper::DeviceTestOpenCamera()
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	AVCameraDevice *pDevice = (AVCameraDevice*)m_pDeviceTest->GetDeviceById(m_deviceTestSelectedCameraId);
	if (pDevice) {
		pDevice->SetPreviewCallback(&SdkWrapper::OnDeviceTestLocalVideo, this);

		AVSupportVideoPreview::PreviewParam param;
		param.device_id = "";/*不指定id，代表每路视频都共用这组参数*/
		param.width = VIDEO_RENDER_BIG_VIEW_WIDTH;
		param.height = VIDEO_RENDER_BIG_VIEW_HEIGHT;
		param.color_format = COLOR_FORMAT_RGB24;//SDK1.3版本只支持COLOR_FORMAT_RGB24和COLOR_FORMAT_I420
		param.src_type = VIDEO_SRC_TYPE_CAMERA;//SDK1.3版本只支持VIDEO_SRC_TYPE_CAMERA
		pDevice->SetPreviewParam(param.device_id, param.width, param.height, param.color_format);
	}
	return m_pDeviceTest->EnableDevice(m_deviceTestSelectedCameraId, true);
}

int SdkWrapper::DeviceTestCloseCamera()
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	return m_pDeviceTest->EnableDevice(m_deviceTestSelectedCameraId, false);
}

int SdkWrapper::DeviceTestGetCameraList(std::vector<std::pair<std::string/*id*/, std::string/*name*/> > &deviceList)
{
	if (!m_pDeviceTest) return AV_ERR_FAILED;
	int deviceCount = m_pDeviceTest->GetDeviceCountByType(DEVICE_CAMERA);
	std::vector<std::string> idList = m_pDeviceTest->GetDeviceIdListByType(DEVICE_CAMERA);
	std::vector<std::string> nameList = m_pDeviceTest->GetDeviceNameListByType(DEVICE_CAMERA);

	for (int i = 0; i < deviceCount; i++) {
		std::pair<std::string/*id*/, std::string/*name*/> device;
		device.first = idList[i];
		device.second = nameList[i];
		deviceList.push_back(device);
	}

	return AV_OK;
}

void SdkWrapper::OnStartDeviceTestCompleteCallbackInternal(int result)
{
	m_pDeviceTest = dynamic_cast<AVDeviceTest*>(m_pContext->GetDeviceTest());
	if (!m_pDeviceTest) return;

	m_pDeviceTest->SetDeviceOperationCallback(OnDeviceTestDeviceOperationCallback, this);
	m_pDeviceTest->SetCameraDeviceDetectCallback(OnDeviceTestDeviceDetectCallback, this);
	m_deviceTestSelectedMicId = "";
	m_deviceTestSelectedPlayerId = "";
	m_deviceTestSelectedCameraId = "";
}

void SdkWrapper::OnStopDeviceTestCompleteCallbackInternal()
{
	//nothing to do.
}

void SdkWrapper::OnStartDeviceTestCompleteCallback(int result, void *pCustomData)
{
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	pSdkWrapper->OnStartDeviceTestCompleteCallbackInternal(result);
}

void SdkWrapper::OnStopDeviceTestCompleteCallback(void *pCustomData)
{
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;
	pSdkWrapper->OnStopDeviceTestCompleteCallbackInternal();
}

void SdkWrapper::OnDeviceTestLocalVideo(VideoFrame *pFrameData, void *pCustomData)
{
	SdkWrapper *pSdkWrapper = (SdkWrapper*)pCustomData;

	//DialogQAVSDKDemo *pMainDlg = (DialogQAVSDKDemo*)pSdkWrapper->m_pMainDlg;
	//pMainDlg->m_viewLocalVideoRender.DoRender(pFrameData);
}

void SdkWrapper::OnDeviceTestDeviceOperationCallback(AVDeviceTest *pDeviceTest, AVDevice::DeviceOperation oper,
	const std::string &deviceId, int retCode, void *pCustomData)
{
	if (!pDeviceTest) return;
	AVDevice *pDev = pDeviceTest->GetDeviceById(deviceId);
	if (!pDev)return;

	if (pDev->GetType() == DEVICE_MIC) {
	} else if (pDev->GetType() == DEVICE_PLAYER) {
	}
	if (pDev->GetType() == DEVICE_CAMERA) {
	} else {
		//TODO
	}
}

void SdkWrapper::OnDeviceTestDeviceDetectCallback(AVDeviceTest *pDeviceTest, void *pCustomData)
{
	if (!pDeviceTest) return;

	//::PostMessage(GetMainHWnd(), WM_ON_DEVICE_TEST_CAMERA_DETECT_OPERATION, NULL, NULL);
}

int SdkWrapper::StartInputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type, std::string path)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	if (path.size() == 0)
		return AV_ERR_FAILED;

	if (src_type == AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOSEND)
		m_strMixToSendPath = path;
	else if (src_type == AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOPLAY)
		m_strMixToPlayPath = path;

	EnableAudioData(src_type, true);
	return m_pAudCtrl->RegistAudioDataCallback(src_type, AudioDataCallback, this);
}

int SdkWrapper::StopInputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	EnableAudioData(src_type, false);
	return m_pAudCtrl->UnregistAudioDataCallback(src_type);
}

int SdkWrapper::StartOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	EnableAudioData(src_type, true);
	return m_pAudCtrl->RegistAudioDataCallback(src_type, AudioDataCallback, this);
}

int SdkWrapper::StopOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	EnableAudioData(src_type, false);
	return m_pAudCtrl->UnregistAudioDataCallback(src_type);
}

int SdkWrapper::StartInOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	EnableAudioData(src_type, true);
	return m_pAudCtrl->RegistAudioDataCallback(src_type, AudioDataCallback, this);
}

int SdkWrapper::StopInOutputAudioDataProcess(AVAudioCtrl::AudioDataSourceType src_type)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	EnableAudioData(src_type, false);
	return m_pAudCtrl->UnregistAudioDataCallback(src_type);
}

int SdkWrapper::EnableAudioData(AVAudioCtrl::AudioDataSourceType src_type, bool enable)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	if (!enable) {
		if (src_type == AVAudioCtrl::AUDIO_DATA_SOURCE_NETSTREM) {
			{
				SimpleAutoLock autoLock(&(m_AudioDataLock[src_type]));

				std::map<std::string/*id*/, FILE*>::iterator it = m_mapAudioDataNetStreams.begin();
				while (it != m_mapAudioDataNetStreams.end()) {
					if (it->second) {
						fclose(it->second);
						it->second = NULL;
					}

					it++;
				}
			}

			{
				SimpleAutoLock autoLock(&m_AudioDataMapLock);
				m_mapAudioDataNetStreams.clear();
			}
		} else if (src_type == AVAudioCtrl::AUDIO_DATA_SOURCE_VOICEDISPOSE) {
			//Do Nothing
		} else {
			std::map<AVAudioCtrl::AudioDataSourceType, FILE*>::iterator it = m_mapAudioData.find(src_type);
			if (it != m_mapAudioData.end()) {
				SimpleAutoLock autoLock(&(m_AudioDataLock[src_type]));

				if (m_mapAudioData[src_type]) {
					fclose(m_mapAudioData[src_type]);

					SimpleAutoLock autoLock(&m_AudioDataMapLock);
					m_mapAudioData.erase(it);
				}
			}
		}
	}

	m_mapAudioDataEnable[src_type] = enable;
	return AV_OK;
}

bool SdkWrapper::IsEnableAudioData(AVAudioCtrl::AudioDataSourceType src_type)
{
	if (!m_pAudCtrl)
		return false;

	return false;
}

int SdkWrapper::SetAudioDataFormat(AVAudioCtrl::AudioDataSourceType src_type, AudioFrameDesc audio_desc)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	return m_pAudCtrl->SetAudioDataFormat(src_type, audio_desc);
}

int SdkWrapper::GetAudioDataFormat(AVAudioCtrl::AudioDataSourceType src_type, AudioFrameDesc& audio_desc)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	return m_pAudCtrl->GetAudioDataFormat(src_type, audio_desc);
}

int SdkWrapper::SetAudioDataVolume(AVAudioCtrl::AudioDataSourceType src_type, float volume)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	return m_pAudCtrl->SetAudioDataVolume(src_type, volume);
}

int SdkWrapper::GetAudioDataVolume(AVAudioCtrl::AudioDataSourceType src_type, float* volume)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	return m_pAudCtrl->GetAudioDataVolume(src_type, volume);
}

int SdkWrapper::SetAudioDataDBVolume(AVAudioCtrl::AudioDataSourceType src_type, int volume)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	return m_pAudCtrl->SetAudioDataDBVolume(src_type, volume);
}

int SdkWrapper::GetAudioDataDBVolume(AVAudioCtrl::AudioDataSourceType src_type, int* volume)
{
	if (!m_pAudCtrl)
		return AV_ERR_FAILED;

	return m_pAudCtrl->GetAudioDataDBVolume(src_type, volume);
}

int SdkWrapper::AudioDataCallback(AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type)
{

	return 0;
	if (AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOSEND == src_type || (AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOPLAY == src_type)) {
		if (m_mapAudioDataEnable[src_type]) {
			if (!audio_frame)
				return AV_ERR_FAILED;

			std::string path;
			if (AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOSEND == src_type)
				path = m_strMixToSendPath;
			else if (AVAudioCtrl::AUDIO_DATA_SOURCE_MIXTOPLAY == src_type)
				path = m_strMixToPlayPath;

			int sampleRate = audio_frame->desc.sample_rate;
			int channelNum = audio_frame->desc.channel_num;

			{
				SimpleAutoLock autoLock(&m_AudioDataMapLock);

				std::map<AVAudioCtrl::AudioDataSourceType, FILE*>::iterator it = m_mapAudioData.find(src_type);
				if (it == m_mapAudioData.end())
					m_mapAudioData.insert(make_pair(src_type, (FILE*)NULL));
			}

			SimpleAutoLock autoLock(&(m_AudioDataLock[src_type]));

			if (!m_mapAudioData[src_type])
				m_mapAudioData[src_type] = fopen(path.c_str(), "rb");

			if (m_mapAudioData[src_type]) {
				audio_frame->desc.sample_rate = sampleRate;
				audio_frame->desc.channel_num = channelNum;
				audio_frame->data_size = sampleRate * channelNum * 2 / 50;
				audio_frame->desc.bits = 16;

				size_t size = fread(audio_frame->data, sizeof(uint8), audio_frame->data_size, m_mapAudioData[src_type]);
				if (size < audio_frame->data_size || feof(m_mapAudioData[src_type])) {
					fseek(m_mapAudioData[src_type], 0, SEEK_SET);
					fread(audio_frame->data, sizeof(uint8), audio_frame->data_size, m_mapAudioData[src_type]);
				}
				return AV_OK;
			} else {
				return AV_ERR_FAILED;
			}
		}
	} else if (AVAudioCtrl::AUDIO_DATA_SOURCE_NETSTREM == src_type) {
		if (m_mapAudioDataEnable[src_type]) {
			if (!audio_frame)
				return AV_ERR_FAILED;

			if (audio_frame->identifier.size() == 0)
				return AV_ERR_FAILED;

			{
				SimpleAutoLock autoLock(&m_AudioDataMapLock);

				std::map<std::string/*id*/, FILE*>::iterator it = m_mapAudioDataNetStreams.find(audio_frame->identifier);
				if (it == m_mapAudioDataNetStreams.end())
					m_mapAudioData.insert(make_pair(src_type, (FILE*)NULL));
			}

			if (m_AudioDataDesc[src_type].channel_num != audio_frame->desc.channel_num ||
				m_AudioDataDesc[src_type].sample_rate != audio_frame->desc.sample_rate) {
				m_AudioDataDesc[src_type].channel_num = audio_frame->desc.channel_num;
				m_AudioDataDesc[src_type].sample_rate = audio_frame->desc.sample_rate;

				SimpleAutoLock autoLock(&m_AudioDataMapLock);

				std::map<std::string/*id*/, FILE*>::iterator it = m_mapAudioDataNetStreams.begin();
				while (it != m_mapAudioDataNetStreams.end()) {
					if (it->second) {
						fclose(it->second);
						it->second = NULL;
					}

					it++;
				}
			}

			SimpleAutoLock autoLock(&(m_AudioDataLock[src_type]));

			if (!m_mapAudioDataNetStreams[audio_frame->identifier]) {
				SYSTEMTIME time = { 0 }; ::GetLocalTime(&time); FILETIME ftime = { 0 }; ::SystemTimeToFileTime(&time, &ftime);

				char strExt[256] = { 0 };
				sprintf(strExt, "%s_%s_%d_%d_%u", m_mapAudioDataName[src_type].c_str(), audio_frame->identifier.c_str(),
					audio_frame->desc.sample_rate, audio_frame->desc.channel_num, ftime.dwLowDateTime);

				std::string remoteViewFileName = std::string(strExt) + ".pcm";

				char szBmpPath[MAX_PATH] = { 0 }; GetModuleFileNameA(NULL, szBmpPath, MAX_PATH);
				//PathAppendA(szBmpPath, "..\\");
				//PathAppendA(szBmpPath, remoteViewFileName.c_str());

				m_mapAudioDataNetStreams[audio_frame->identifier] = fopen(szBmpPath, "wb+");
			}

			if (m_mapAudioDataNetStreams[audio_frame->identifier]) {
				fwrite(audio_frame->data, sizeof(uint8), audio_frame->data_size, m_mapAudioDataNetStreams[audio_frame->identifier]);
				fflush(m_mapAudioDataNetStreams[audio_frame->identifier]);
				return AV_OK;
			} else {
				return AV_ERR_FAILED;
			}
		}
	} else if (AVAudioCtrl::AUDIO_DATA_SOURCE_VOICEDISPOSE == src_type) {
		if (m_mapAudioDataEnable[src_type]) {
			if (!audio_frame)
				return AV_ERR_FAILED;

			uint32 size = audio_frame->data_size;
			short* pcm = (short*)audio_frame->data;

			while (size > 0) {
				(*pcm) = (*pcm) >> 2;

				pcm++;
				size -= 2;
			}
		}
	} else {
		if (m_mapAudioDataEnable[src_type]) {
			if (!audio_frame)
				return AV_ERR_FAILED;

			{
				SimpleAutoLock autoLock(&m_AudioDataMapLock);

				std::map<AVAudioCtrl::AudioDataSourceType, FILE*>::iterator it = m_mapAudioData.find(src_type);
				if (it == m_mapAudioData.end())
					m_mapAudioData.insert(make_pair(src_type, (FILE*)NULL));
			}

			if (m_AudioDataDesc[src_type].channel_num != audio_frame->desc.channel_num ||
				m_AudioDataDesc[src_type].sample_rate != audio_frame->desc.sample_rate) {
				m_AudioDataDesc[src_type].channel_num = audio_frame->desc.channel_num;
				m_AudioDataDesc[src_type].sample_rate = audio_frame->desc.sample_rate;

				if (m_mapAudioData[src_type]) {
					{
						SimpleAutoLock autoLock(&(m_AudioDataLock[src_type]));
						fclose(m_mapAudioData[src_type]);
					}

					{
						SimpleAutoLock autoLock(&m_AudioDataMapLock);
						m_mapAudioData[src_type] = NULL;
					}
				}
			}

			SimpleAutoLock autoLock(&(m_AudioDataLock[src_type]));

			if (!m_mapAudioData[src_type]) {
				SYSTEMTIME time = { 0 }; ::GetLocalTime(&time); FILETIME ftime = { 0 }; ::SystemTimeToFileTime(&time, &ftime);

				char strExt[256] = { 0 };
				sprintf(strExt, "%s_%d_%d_%u", m_mapAudioDataName[src_type].c_str(),
					audio_frame->desc.sample_rate, audio_frame->desc.channel_num, ftime.dwLowDateTime);

				std::string remoteViewFileName = std::string(strExt) + ".pcm";

				char szBmpPath[MAX_PATH] = { 0 }; GetModuleFileNameA(NULL, szBmpPath, MAX_PATH);
				//PathAppendA(szBmpPath, "..\\");
				//PathAppendA(szBmpPath, remoteViewFileName.c_str());

				m_mapAudioData[src_type] = fopen(szBmpPath, "wb+");
			}

			if (m_mapAudioData[src_type]) {
				fwrite(audio_frame->data, sizeof(uint8), audio_frame->data_size, m_mapAudioData[src_type]);
				fflush(m_mapAudioData[src_type]);
				return AV_OK;
			} else {
				return AV_ERR_FAILED;
			}
		}
	}
}

int SdkWrapper::AudioDataCallback(AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type, void* custom_data)
{
	return ((SdkWrapper*)custom_data)->AudioDataCallback(audio_frame, src_type);
}

void SdkWrapper::Set1v1VideoRenderDlg(void *p1v1VideoRenderDlg)
{
	//m_p1v1VideoRenderDlg = p1v1VideoRenderDlg;
}

UINT SdkWrapper::GetRoomId()
{
	return m_pRoom->GetRoomId();
}

