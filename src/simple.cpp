/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
**
** -------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>

#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "p2p/base/stun_server.h"

#include "PeerConnectionManager.h"

#include "HttpServerRequestHandler.h"
#include "API.h"
#include "API2.h"

#include "streamer.h"

int simple(const Json::Value & config);


/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int simple(int argc, char* argv[])
{
	const char *configFile = "streamer.json";
	Json::Value config;
	std::ifstream stream(configFile);
	if (stream.good())
		stream >> config;
	else 
		config = Json::objectValue;	// assign {}
	
	return simple(config);
}

// get civetweb server options from json.
static std::vector<std::string> getServerOptions(const Json::Value & jopt)
{
	std::vector<std::string> options;
	options.push_back("document_root");
	options.push_back(jopt.get("document_root", "./html").asString());
	options.push_back("access_control_allow_origin");
	options.push_back(jopt.get("access_control_allow_origin", "*").asString());
	options.push_back("listening_ports");
	std::string port = jopt.get("listening_ports", "8000").asString();
	options.push_back(port);
	std::cout << "HTTP Listen at " << port << std::endl;
	std::string keys[] = {"ssl_certificate", "num_threads", "global_auth_file", "authentication_domain"}; 
	for(const std::string &key : keys)
	{
		if (jopt.isMember(key))
		{
			options.push_back(key);
			options.push_back(jopt.get(key,"").asString());
		}
	}
	return options;
}

static std::unique_ptr<cricket::StunServer> createOptionalStunServer(const Json::Value & config, rtc::Thread* thread)
{
	std::unique_ptr<cricket::StunServer> stunserver;

	if (config.isMember("local_stun_url")||config.get("use_local_stun", false).asBool())
	{
		rtc::SocketAddress server_addr;
		server_addr.FromString(config.get("local_stun_url", "0.0.0.0:3478").asString());
		rtc::AsyncUDPSocket* server_socket = rtc::AsyncUDPSocket::Create(thread->socketserver(), server_addr);
		
		if (server_socket)
		{
			stunserver.reset(new cricket::StunServer(server_socket));
			std::cout << "STUN Listening at " << server_addr.ToString() << std::endl;
		}
	}
	return stunserver;
}

/* 
Returns AudioLayer enum. Can be overridden with integer passed as "audio_layer" json value.
Defined: webrtc/modules/audio_device/include/audio_device.h
Currently, enum is:
	  enum AudioLayer {
			kPlatformDefaultAudio = 0,
			kWindowsWaveAudio = 1,
			kWindowsCoreAudio = 2,
			kLinuxAlsaAudio = 3,
			kLinuxPulseAudio = 4,
			kDummyAudio = 5
			};
	So to set dummy audio, add: audio_layer:5 to json
*/
static API * createAPI(PeerConnectionManager * webRtcServer, Json::Value config)
{
	return new API2(webRtcServer, config);
}


static webrtc::AudioDeviceModule::AudioLayer getAudioLayer(Json::Value config)
{
	int v = config.get("audio_layer", webrtc::AudioDeviceModule::kPlatformDefaultAudio).asInt();
	return (webrtc::AudioDeviceModule::AudioLayer)v;// atoi(optarg) : webrtc::AudioDeviceModule::kDummyAudio; break;
}
static void configureLogging(Json::Value config)
{
	rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)config.get("log_level", rtc::LERROR).asInt());
	rtc::LogMessage::LogTimestamps();
	rtc::LogMessage::LogThreads();
	std::cout << "Logger level:" <<  rtc::LogMessage::GetLogToDebug() << std::endl;
}

static std::list<std::string> getIceServerList(Json::Value config)
{
	std::list<std::string> iceServerList;
	std::string stun_server("stun:");
	iceServerList.push_back(std::string("stun:")+config.get("stun_server", "stun.l.google.com:19302").asString());
	if (config.isMember("turn_url"))
	{
		iceServerList.push_back(std::string("turn:")+config.get("turn_url","").asString());
	}
	return iceServerList;
}

static PeerConnectionManager * createPeerConnectionManager(Json::Value config)
{
	std::map<std::string,std::string> urlVideoList;
	std::map<std::string,std::string> urlAudioList;

	if (config.isMember("urls")) {
		Json::Value urls = config["urls"];
		for( auto it = urls.begin() ; it != urls.end() ; it++ ) {
				std::string name = it.key().asString();
				Json::Value value = *it;
				if (value.isMember("video")) {
					urlVideoList[name]=value["video"].asString();
				}
				if (value.isMember("audio")) {
					urlAudioList[name]=value["audio"].asString();
				}
		}
	}
	
	std::string publishFilter = config.get("publish_filter", ".*").asString();
	std::list<std::string> iceServerList = getIceServerList(config);

	return new PeerConnectionManager(iceServerList, urlVideoList, urlAudioList, getAudioLayer(config), publishFilter);


}

int simple(const Json::Value & config)
{	
	std::cout << "Streamer:"<<config<<std::endl;	
	configureLogging(config);

	rtc::Thread* thread = rtc::Thread::Current();
	rtc::InitializeSSL();

	// std::string publishFilter = config.get("publish_filter", ".*").asString();
	// std::list<std::string> iceServerList = getIceServerList(config);
	
	PeerConnectionManager * webRtcServer = createPeerConnectionManager(config);

	// PeerConnectionManager webRtcServer(iceServerList, urlVideoList, urlAudioList, getAudioLayer(config), publishFilter);
	

	if (!webRtcServer->InitializePeerConnection())
	{
		std::cout << "Cannot Initialize WebRTC server" << std::endl;
	}
	else
	{
		try {

			std::unique_ptr<API> api(createAPI(webRtcServer, config));
			std::vector<std::string> options = getServerOptions(config);
			HttpServerRequestHandler httpServer(api.get(), "/api/", options);

			// start STUN server if needed
			std::unique_ptr<cricket::StunServer> stunserver = createOptionalStunServer(config, thread);
			
			// mainloop
			thread->Run();

		} catch (const CivetException & ex) {
			std::cout << "Cannot Initialize start HTTP server exception:" << ex.what() << std::endl;
			return -1;
		}
	}

	rtc::CleanupSSL();
	return 0;
}

