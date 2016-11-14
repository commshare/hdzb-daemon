#pragma once
#include "./SdkWrapper.h"
#include "tim.h"
#include "log4z.h"
#include "gci-json.h"
#include "CustomWinMsg.h"
#include "RecorderMgr/RecoderMgr.h"


#include <vector>
#include <queue>
#include <map>

using namespace imcore;
using namespace ggicci;
using namespace zsummer::log4z;
//using namespace tencent::av;

#define LINE LOGD("")

class Manager;
//class RecoderMgr;

// 数据通道
class Channel {
	UINT32 m_iThreadID;
	Manager* m_mgr;
public:
	int Init(Manager* mgr);

	// 信令通道
	void RecvMsg();
	int SendMsg(Json& json);

	// 数据通道 0 - localstream, else remote stream.
	int SendStream(char* buf, int size, int index);
	int SendStream(VideoFrame* pFrameData, int fd);

private:
	int RecvMessage(char* buf, int size);
	int SendMsg(const char* buf, int size);
	
};


class Manager :
	public SdkWrapper {
	Channel m_channel;
	HMODULE m_hMoude;
public:
	Manager(HWND& hwnd);
	~Manager();
	

	int Init();

	// 接收来自进程外的消息
	void OnRecvChannel(char* buf, int size);


	// 因互动直播 SDK 的限制，必须由窗口线程调用此函数。处理 listenMsg 接收到的消息。
	void Signal();
	void OnProcessMsg(Json& obj);
	


	// 账号登入、登出。 TIM 登陆 --> TIM 回调 --> avsdk 初始化 --> avsdk 回调
	void Login();
	void Logout();
	void StartAVSDK();
	void OnLoginResult(int retCode); // 返回登陆成功、失败

	void SendMsg( int retCode, const char* type);

	void OnLogoutResult(int retCode);
	void OnStartContextComplete(int result);  // 初始化 avsdk 的结果。

	// 进入房间、退出房间
	void EnterRoom();
	void ExitRoom();
	void OnEnterRoomResult(int retCode);
	void OnExitRoomResult(int retCode);

	virtual void OnEnterRoomComplete(int result);
	virtual void OnExitRoomComplete();

	void OnRoomDisconnect(int reason);

	// 设备操作
	void OpenCamera();
	void CloseCamera();
	void OnOpenCameraResult(int retCode);
	void OnCloseCameraResult(int retCode);

	void OpenMic();
	void CloseMic();
	void OnOpenMicResult(int retCode);
	void OnCloseMicResult(int retCode);

	void OpenSpeaker();
	void CloseSpeaker();
	void OnOpenSpeakerResult(int retCode);
	void OnCloseSpeakerResult(int retCode);


	// 房间人员信息更新
	void OnEndpointsUpdateInfo(AVRoomMulti::EndpointEventId eventid, std::vector<std::string> updatelist);

	// 远端画面
	void RequestRemoteView(const vector<string>& identifiers );
	void OnRequestRemoteViewResult(int ret);
	void OnRequestViewListComplete(std::vector<std::string> identifierList, std::vector<View> viewList, int32 result); // 结果
	void OnCancelAllViewCompleteCallback(int ret);

	// record
	bool StartRecord();
	bool StopRecord();
	
private:
	// 获取视频数据
	virtual void OnRemoteVideoCallback(VideoFrame *pFrameData);
	virtual void OnLocalVideoCallback(VideoFrame *pFrameData);
	virtual int AudioDataCallback(AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type);
	//
private:
	// 登陆相关信息
	AVContext::StartParam m_startParam;
	string m_userSig;

	// 房间号
	AVRoomMulti::EnterParam m_enterRoomParam;

	//存储接收到的信令
	std::queue<string*> m_cmds; 

	//
	HWND& m_hWndMain;

	SimpleLock m_identifier2pipeLock;
	std::map<string, int> m_identifier2pipe;
	RecoderMgr* m_recorderMgr;

	// 登陆状态
	//bool m_isLogin;
	//bool m_isEnterRoom;

	class LoginCallBack : public TIMCallBack 
	{		
	public:
		LoginCallBack(HWND& hwnd):m_hWndMain(hwnd){}

		virtual void OnSuccess()
		{
			::PostMessage(m_hWndMain, WM_ON_LOGIN, (WPARAM)AV_OK, 0);
		}

		virtual void OnError(int retCode, const std::string &desc)
		{
			LOGFMTE("login error. code: %d, desc: %s", retCode, desc.c_str());
			::PostMessage(m_hWndMain, WM_ON_LOGIN, (WPARAM)retCode, NULL);
		}

		const HWND& m_hWndMain;
	};

	class LogoutCallBack : public TIMCallBack 
	{
	public:
		LogoutCallBack(HWND& hwnd):m_hWndMain(hwnd){}

		virtual void OnSuccess(){
			LINE;
			::PostMessage(m_hWndMain, WM_ON_LOGOUT, (WPARAM)AV_OK, 0);
		}
		virtual void OnError(int retCode, const std::string &desc) 
		{
			LOGFMTE("login error. code: %d, desc: %s", retCode, desc.c_str());
			::PostMessage(m_hWndMain, WM_ON_LOGOUT, (WPARAM)retCode, retCode); 
		}

		const HWND& m_hWndMain;
	};

	LoginCallBack m_loginCb;
	LogoutCallBack m_logoutCb;
};

