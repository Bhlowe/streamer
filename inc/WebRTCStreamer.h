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
	
	virtual void setStreams(std::map<std::string,std::string> vURLs, std::map<std::string,std::string> aURLs, std::map<std::string,std::string>positions)
	{
		urlVideoList = vURLs;
		urlAudioList = aURLs;
		positionList = positions;
	}


	protected:
	Json::Value config;	// config settings for streamer. From json file, extracted from command line arguments, etc.

	rtc::Thread* thread=NULL;
	PeerConnectionManager * webRtcServer = NULL;
	API *api = NULL;
	cricket::StunServer * stunserver=NULL;
	HttpServerRequestHandler * httpServer = NULL;

	std::map<std::string,std::string> urlVideoList;
	std::map<std::string,std::string> urlAudioList;
	std::map<std::string,std::string> positionList;	


	virtual std::vector<std::string> getServerOptions();
	virtual void initStunServer();
	virtual API * createAPI();
	virtual void createHttpServer();

	// returns kPlatformDefaultAudio, kDummyAudio, etc. 
	virtual webrtc::AudioDeviceModule::AudioLayer getAudioLayer();
	virtual void initLogging();
	virtual std::list<std::string> getIceServerList();
	virtual PeerConnectionManager * createPeerConnectionManager();
	
};
