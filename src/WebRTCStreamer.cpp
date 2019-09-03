/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerRequestHandler.cpp
** 
** -------------------------------------------------------------------------*/

#include <string.h>
#include <iostream>
    
#include "WebRTCStreamer.h"
#include "API2.h"



/* ---------------------------------------------------------------------------
**  WebRTCSreamer 
** -------------------------------------------------------------------------*/


	
bool WebRTCStreamer::init() {
	std::cout << "Streamer:"<<config<<std::endl;	
	configureLogging();
	thread = rtc::Thread::Current();
	rtc::InitializeSSL();

	webRtcServer = createPeerConnectionManager();
	
	if (!webRtcServer->InitializePeerConnection())
	{
		std::cout << "Cannot Initialize WebRTC server" << std::endl;
		return false;
	}
	

	// create http api dispatch handler
	std::unique_ptr<API> api(createAPI(webRtcServer));
	
	// create default civetweb http server
	std::vector<std::string> options = getServerOptions();
	HttpServerRequestHandler httpServer(api.get(), "/api/", options);
	// start STUN server if needed
	std::unique_ptr<cricket::StunServer> stunserver = createOptionalStunServer();

	run();	// blocks

	return true;
}


void WebRTCStreamer::run() {
	thread->Run();
}

void WebRTCStreamer::cleanup() 
{
	rtc::CleanupSSL();
}




// get civetweb server options from json.
std::vector<std::string> WebRTCStreamer::getServerOptions()
{
	std::vector<std::string> options;
	options.push_back("document_root");
	options.push_back(config.get("document_root", "./html").asString());
	options.push_back("access_control_allow_origin");
	options.push_back(config.get("access_control_allow_origin", "*").asString());
	options.push_back("listening_ports");
	std::string port = config.get("listening_ports", "8000").asString();
	options.push_back(port);
	std::cout << "HTTP Listen at " << port << std::endl;
	std::string keys[] = {"ssl_certificate", "num_threads", "global_auth_file", "authentication_domain"}; 
	for(const std::string &key : keys)
	{
		if (config.isMember(key))
		{
			options.push_back(key);
			options.push_back(config.get(key,"").asString());
		}
	}
	return options;
}


std::unique_ptr<cricket::StunServer> WebRTCStreamer::createOptionalStunServer()
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

API * WebRTCStreamer::createAPI(PeerConnectionManager * webRtcServer)
{
	return new API2(webRtcServer, config);
}


// returns kPlatformDefaultAudio, kDummyAudio, etc. 
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

webrtc::AudioDeviceModule::AudioLayer WebRTCStreamer::getAudioLayer()
{
	int v = config.get("audio_layer", webrtc::AudioDeviceModule::kPlatformDefaultAudio).asInt();
	return (webrtc::AudioDeviceModule::AudioLayer)v;// atoi(optarg) : webrtc::AudioDeviceModule::kDummyAudio; break;	
}


void WebRTCStreamer::configureLogging()
{
	rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)config.get("log_level", rtc::LERROR).asInt());
	rtc::LogMessage::LogTimestamps();
	rtc::LogMessage::LogThreads();
	std::cout << "Logger level:" <<  rtc::LogMessage::GetLogToDebug() << std::endl;
}


std::list<std::string> WebRTCStreamer::getIceServerList()
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

PeerConnectionManager * WebRTCStreamer::createPeerConnectionManager()
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
	std::list<std::string> iceServerList = getIceServerList();

	return new PeerConnectionManager(iceServerList, urlVideoList, urlAudioList, getAudioLayer(), publishFilter);
}


#if 0
int simple(const Json::Value & config)
{	
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

#endif
