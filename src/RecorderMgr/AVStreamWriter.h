#pragma once

#include "../Util.h"
#include "header.h"
#include <string>
using std::string;

class AVEncoder;
class AVStreamWriter: public SimpleThread
{
public:
	AVStreamWriter(void);

	void PushPacket(Packet* pkt) { m_pkts.pushWithSignal(pkt); }
	void SetOuturl(const string& url) { m_url = url; }
	void SetAVEncoder(AVEncoder* encoder) { m_encoder = encoder; }
	bool SendPacket(Packet* pkt);

	bool OpenStream();
	bool CloseStream();

private:
	virtual void run();
private:
	AVStream* m_videoStream;
	AVStream* m_audioStream;
	AVFormatContext* m_fmtCtx;
	AVEncoder* m_encoder;
	string m_url;
	SimpleQueue<Packet*> m_pkts;
};

