#include "Manager.h"
#include "tim.h"
#include <io.h>
#include <assert.h>
#include <process.h>

struct VideoFrameHeader{
	int width;
	int height;
	int size;
};

Manager::Manager( HWND& hwnd ):m_hWndMain(hwnd), m_loginCb(hwnd), m_logoutCb(hwnd)
{

}


Manager::~Manager()
{
}

void Manager::Login()
{
	LOGD("");
	TIMUser user;
	user.set_account_type(m_startParam.account_type);
	user.set_app_id_at_3rd(m_startParam.app_id_at3rd);
	user.set_identifier(m_startParam.identifier);

	TIMManager::get().set_env(0);  
	//设置模式。如果要支持日志上报功能，需要将模式设置为"1"。
	TIMManager::get().set_mode(1);

	//先初始化通信SDK
	TIMManager::get().Init();
	TIMManager::get().Login(m_startParam.sdk_app_id, user, m_userSig, &m_loginCb);
}

// TODO: 发给外部进程，发给 recorder;
void Manager::OnRemoteVideoCallback( VideoFrame *pFrameData )
{
	m_channel.SendStream(pFrameData, 0);
}

// TODO: 发给外部进程，发给 recorder; 可能存在的画面尺寸转换。
void Manager::OnLocalVideoCallback( VideoFrame *pFrameData )
{
	// user ---> index 
	return;
	m_channel.SendStream(pFrameData, 0);
}

void Manager::Logout()
{
	int ret = StopContext();
	if (ret == AV_OK) {
		DestroyContext();
		TIMManager::get().Logout(&m_logoutCb);
	} else {
		OnLogoutResult(ret);
	}
}

void Manager::OnStartContextComplete( int result )
{
	if (result == AV_OK) {
		LOGD("OnStartContextComplete success");
	} else {
		LOGE("OnStartContextComplete failed");
	}
	SdkWrapper::OnStartContextComplete(result);
	OnLoginResult(result);
}

void Manager::EnterRoom()
{
	// TODO: check, 登陆状态，测试设备，群组Id必须在1-4294967295之间

	m_enterRoomParam.auth_bits = AUTH_BITS_DEFAULT;
	m_enterRoomParam.audio_category = AUDIO_CATEGORY_VOICECHAT;
	m_enterRoomParam.video_recv_mode = VIDEO_RECV_MODE_MANUAL; 
	int retCode = SdkWrapper::EnterRoom(m_enterRoomParam);
	if (retCode != AV_OK)
		OnEnterRoomResult(retCode);
}

void Manager::StartAVSDK()
{
	CreateContext();
	GetAVContext()->SetSpearEngineMode(SPEAR_EGINE_MODE_WEBCLOUD);
	int ret = StartContext(m_startParam);
	if (ret == AV_OK) {
		LOGD("StartContext success");
	} else {
		LOGE("StartContext failed");
		OnLoginResult(ret);
	}
}

void Manager::OnEnterRoomComplete( int result )
{
	SdkWrapper::OnEnterRoomComplete(result);
	OnEnterRoomResult(result);
}

void Manager::Signal()
{
	while(!m_cmds.empty()) {
		string* msg = m_cmds.front();
		m_cmds.pop();

		try
		{
			Json obj = Json::Parse(msg->c_str());
			OnProcessMsg(obj);
			// TODO: dispatch message.
		}
		catch (exception& e)
		{
			LOGFMTE("json parse error: %s", e.what());
		}

		delete msg;
	}
}

void Manager::OnLoginResult( int retCode )
{
	SendMsg(retCode, "Login");

}

void Manager::OnLogoutResult( int retCode )
{
	SendMsg(retCode, "Logout");
}

void Manager::ExitRoom()
{
	int ret = SdkWrapper::ExitRoom();
	if (ret != AV_OK)
		OnExitRoomResult(ret);
}

void Manager::OnEnterRoomResult( int retCode )
{
	SendMsg(retCode, "EnterRoom");
}

void Manager::OnExitRoomResult( int retCode )
{
	SendMsg(retCode, "ExitRoom");
}

void Manager::OnRoomDisconnect( int reason )
{

}

void Manager::OpenCamera()
{
	int ret = SdkWrapper::OpenCamera();
	OnOpenCameraResult(ret);
}

void Manager::CloseCamera()
{
	int ret = SdkWrapper::CloseCamera();
	OnCloseCameraResult(ret);
}

void Manager::OnOpenCameraResult( int retCode )
{
	SendMsg(retCode, "OpenCamera");
}

void Manager::OnCloseCameraResult( int retCode )
{
	SendMsg(retCode, "CloseCamera");
}

void Manager::OpenMic()
{
	int ret = SdkWrapper::OpenMic();
	OnOpenMicResult(ret);
}

void Manager::CloseMic()
{
	int ret = SdkWrapper::CloseMic();
	OnCloseMicResult(ret);
}

void Manager::OnOpenMicResult( int retCode )
{
	SendMsg(retCode, "OpenMic");
}

void Manager::OnCloseMicResult( int retCode )
{
	SendMsg(retCode, "CloseMic");
}

void Manager::OpenSpeaker()
{
	int ret = SdkWrapper::OpenPlayer();
	OnOpenSpeakerResult(ret);
}

void Manager::CloseSpeaker()
{
	int ret = SdkWrapper::ClosePlayer();
	OnCloseSpeakerResult(ret);
}

void Manager::OnOpenSpeakerResult( int retCode )
{
	SendMsg(retCode, "OpenSpeaker");
}

void Manager::OnCloseSpeakerResult( int retCode )
{
	SendMsg(retCode, "CloseSpeaker");
}

void Manager::OnEndpointsUpdateInfo( AVRoomMulti::EndpointEventId eventid, std::vector<std::string> updatelist )
{
	// 得到当前所有节点信息

	AVEndpoint** endpointList[1];
	int size = GetEndpointList(endpointList);
	if (size == 0) return;

	Json json = Json::Parse("{}");
	Json param = Json::Parse("[]");
	json.AddProperty("type", Json("EndpointsUpdateInfo"));


	for (int i = 0 ; i < size; i++)
	{
		std::string identifier = endpointList[0][i]->GetId();

		if(identifier.size() == 0)
		{
			LOGW("identifier.size() == 0");
			continue;
		}

		AVEndpoint *endpoint = GetEndpointById(identifier);
		if(endpoint)
		{
			if(endpoint->HasAudio() || endpoint->HasCameraVideo() || endpoint->HasScreenVideo())
			{

				Json person = Json::Parse("{}");
				person.AddProperty("id", Json(identifier));
				if (endpoint->HasAudio())
					person.AddProperty("audio", Json(true));
				if (endpoint->HasCameraVideo())
					person.AddProperty("video", Json(true));
				param.Push(person);
			}			
		}
		else
		{
			LOGW("identifier.size() == 0");
			//ShowRetCode("UserUpdate endpoint == NULL", AV_ERR_FAILED);
		}
	}

	json.AddProperty("param", param);
	//m_channel.SendMsg(json);

	//SendToFather(json.ToString());

}

void Manager::OnRequestRemoteViewResult( int ret )
{

}

void Manager::OnRequestViewListComplete( std::vector<std::string> identifierList, std::vector<View> viewList, int32 result )
{
	// 不使用
}

void Manager::OnCancelAllViewCompleteCallback( int ret )
{
	// 不使用
}

void Manager::OnRecvChannel( char* buf, int size )
{
	//LOGFMTD("recv msg: %s", buf);
	m_cmds.push(new string(buf));
	::PostMessage(m_hWndMain, WM_UI_CUSTOM, NULL, NULL);
}

void Manager::RequestRemoteView( const vector<string>& ids )
{
	LOGD("");
	vector<View> views;
	LOGFMTD("identifiers size: %d", ids.size());
	for(size_t i = 0; i < ids.size(); i++) {
		LOGFMTD("identifier: %s", ids[i].c_str());
		AVEndpoint *endpoint = GetEndpointById(ids[i]);
		View view;
		view.size_type = VIEW_SIZE_TYPE_BIG;//这边，DEMO默认请求大画面；实际业务要请求大画面还是小画面，可以根据使用场景决定。
		view.video_src_type = VIDEO_SRC_TYPE_CAMERA;
		views.push_back(view);
		LOGD("");
	}
	LOGD("");
	int retCode = -1;

	if (ids.empty()) {
		LOGI("request list is empty, CancelAllView");
		retCode = CancelAllView();
	} else {
		retCode = RequestViewList(ids, views);
	}

	if (retCode != AV_OK) {
		retCode = RequestViewList(ids, views);
		LOGFMTE("RequestViewList fail. code: %d", retCode);
		OnRequestRemoteViewResult(retCode);
	}
}


#define MSG_TYPE_Init			"Init"
#define MSG_TYPE_Login			"Login"
#define MSG_TYPE_Logout			"Logout"
#define MSG_TYPE_EnterRoom		"EnterRoom"
#define MSG_TYPE_ExitRoom		"ExitRoom"
#define MSG_TYPE_OpenCamera		"OpenCamera"
#define MSG_TYPE_CloseCamera	"CloseCamera"
#define MSG_TYPE_OpenMic		"OpenMic"
#define MSG_TYPE_CloseMic		"CloseMic"
#define MSG_TYPE_OpenPlayer		"OpenPlayer"
#define MSG_TYPE_ClosePlayer	"ClosePlayer"
#define MSG_TYPE_GetEndpointList  "GetEndpointList"
#define MSG_TYPE_RequestView	"RequestView"
#define MSG_TYPE_OpenMic		"OpenMic"
#define MSG_TYPE_CloseMic		"CloseMic"
#define MSG_TYPE_OpenSpeaker	"OpenSpeaker"
#define MSG_TYPE_CloseSpeaker	"CloseSpeaker"

void Manager::OnProcessMsg( Json& obj )
{
	Json messageType = obj["type"];
	string _type = messageType.AsString();

	LOGFMTD("message type: %s", _type.c_str());
	if (_type == MSG_TYPE_Init) {
		Json param = obj["param"];
		m_startParam.sdk_app_id = param["sdk_app_id"].AsInt();
		m_startParam.account_type = param["account_type"].AsString();
		m_startParam.app_id_at3rd = param["app_id_at3rd"].AsString();
		m_startParam.identifier = param["identifier"].AsString();
		m_userSig = param["user_sig"].AsString();
	} else if (_type == MSG_TYPE_Login) {
		Login();
	} else if (_type == MSG_TYPE_Logout) {
		Logout();
	} else if (_type == MSG_TYPE_EnterRoom) {
		Json param = obj["param"];
		m_enterRoomParam.relation_id = param.AsInt();
		EnterRoom();
	} else if (_type == MSG_TYPE_ExitRoom) {
		ExitRoom();
	} else if (_type == MSG_TYPE_OpenCamera) {
		OpenCamera();
	} else if (_type == MSG_TYPE_CloseCamera) {
		CloseCamera();
	} else if (_type == MSG_TYPE_RequestView) {
		Json param = obj["param"];
		if (!param.IsArray()) {
			LOGE("list is not a array");
			return;
		}

		if (param.Size() > 4) {
			LOGE("RequestRemoteView size");
			return;
		}

		vector<string> ids;
		for (int i = 0; i < param.Size(); i++)
		{
			string identifier = param[i].AsString();
			if (identifier.empty()){
				continue;
			}
			ids.push_back(param[i].AsString());
			//SimpleAutoLock lock(&m_identifier2pipeLock);
			//m_identifier2pipe[identifier] = i + LOCAL_STREM + 1;
		}

		RequestRemoteView(ids);
	} else if (_type == MSG_TYPE_OpenMic){
		OpenMic();
	} else if (_type == MSG_TYPE_CloseMic) {
		CloseMic();
	} else if (_type == MSG_TYPE_OpenSpeaker) {
		OpenSpeaker();
	} else if (_type == MSG_TYPE_CloseSpeaker) {
		CloseSpeaker();
	}
}

// TODO: 拿到各端音频数据进行混音
int Manager::AudioDataCallback( AudioFrame* audio_frame, AVAudioCtrl::AudioDataSourceType src_type )
{
	return AV_OK;
}

int Manager::Init()
{
	ILog4zManager::getRef().config("config.cfg");
	ILog4zManager::getRef().start();
	return m_channel.Init(this);
}

void Manager::OnExitRoomComplete()
{
	OnExitRoomResult(AV_OK);
	SdkWrapper::OnExitRoomComplete();
}

void Manager::SendMsg( int retCode, const char* type )
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json(type));
	if (retCode == AV_OK) {
		json.AddProperty("code", Json(retCode));
	} else {
		json.AddProperty("code", Json(retCode));
	}
	m_channel.SendMsg(json);
}

int Channel::RecvMessage( char* buf, int size )
{
#if 0
	Sleep(100);
	return 1;
#else
	return _read(3, buf, size);	
#endif
}

int Channel::SendMsg(const char* buf, int size )
{
	return _write(4, buf, size);
}

int Channel::SendMsg( Json& json )
{
	string str = json.ToString();
	return SendMsg(str.c_str(), str.length());
}

int Channel::SendStream( char* buf, int size, int index )
{
	assert(index>0 && index<6);
	return _write(index + 5, buf, size);
}

int Channel::SendStream( VideoFrame* pFrameData, int index )
{

#define MAX_WRITE_SIZE 1000 * 1000

	assert(index>0 && index<6);
	int fd = index + 5;
	struct VideoFrameHeader h;
	h.width = pFrameData->desc.width;
	h.height = pFrameData->desc.height;
	h.size = pFrameData->data_size;

#if 0
	if (!fp) {
		fp = fopen("./vs.yuv", "w");
		//fwrite(&h, sizeof(h), 1, fp);
		int r = fwrite(pFrameData->data, 1, pFrameData->data_size, fp);
		fflush(fp);
		if (r < -1) {
			exit(1);
		}
	}
#endif

	int ret = _write(fd, &h, sizeof(h));
	if (ret != sizeof(h)) {
		return ret;
	}

	uint8_t* pdata = pFrameData->data;
	int leftSize = h.size;
	do 
	{
		int writeSize = leftSize > MAX_WRITE_SIZE ? MAX_WRITE_SIZE : leftSize;
		ret = _write(fd, pdata, writeSize);
		if (ret <= 0) {
			break;
		}
		pdata += ret;
		leftSize -= ret;

	} while (leftSize > 0);
	return ret;
}

static unsigned int WINAPI _func(PVOID pParam) 
{
	Channel* ch = reinterpret_cast<Channel*>(pParam);
	ch->RecvMsg();
	return 0;
}


int Channel::Init( Manager* mgr )
{
	m_mgr = mgr;
	uintptr_t ret =  _beginthreadex(NULL, 0, _func, (void*)this, 0, &m_iThreadID);
	return ret == NULL ? -1 : 0;
}

void Channel::RecvMsg()
{
	while(true) {
		char buf[1024];
		int ret = RecvMessage(buf, sizeof(buf) - 1);
		if (ret > 0) {
			buf[ret] = 0;
			LOGFMTD("recv msg: %s", buf);
			m_mgr->OnRecvChannel(buf, ret);
		} else {
			LOGFMTE("RecvMessage return err. code: %d", ret);
		}
	}
}
