/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
**
** -------------------------------------------------------------------------*/

#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "p2p/base/stun_server.h"

#include "PeerConnectionManager.h"
#include "HttpServerRequestHandler.h"
#include "API.h"

#if WIN32
#include "getopt.h"
#endif

class WebRTCStreamer
{
	public:
	WebRTCStreamer(): config(Json::objectValue) {}
	WebRTCStreamer(Json::Value config) : config(config) {}

	virtual ~WebRTCStreamer() { cleanup(); }
	virtual bool init();
	virtual void run();
	virtual void cleanup();
	
	protected:
	Json::Value config;
	rtc::Thread* thread=null;
	PeerConnectionManager * webRtcServer;

	virtual std::vector<std::string> getServerOptions();
	virtual std::unique_ptr<cricket::StunServer> createOptionalStunServer();
	virtual API * createAPI(PeerConnectionManager * webRtcServer);

	// returns kPlatformDefaultAudio, kDummyAudio, etc. 
	virtual webrtc::AudioDeviceModule::AudioLayer getAudioLayer();
	virtual void configureLogging();
	virtual std::list<std::string> getIceServerList();
	virtual PeerConnectionManager * createPeerConnectionManager();

	
};




