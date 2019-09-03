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

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int simple(int argc, char* argv[])
{
	const char* turnurl       = "";
	const char* defaultlocalstunurl  = "0.0.0.0:3478";
	const char* localstunurl  = NULL;
	const char* stunurl       = "stun.l.google.com:19302";
	int logLevel              = rtc::LERROR;
	
	const char* webroot       = "./html";
	std::string sslCertificate;
	webrtc::AudioDeviceModule::AudioLayer audioLayer = webrtc::AudioDeviceModule::kPlatformDefaultAudio;
	std::string streamName;
	std::map<std::string,std::string> urlVideoList;
	std::map<std::string,std::string> urlAudioList;
	std::string nbthreads;
	std::string passwdFile;
	std::string authDomain = "mydomain.com";
	std::string publishFilter(".*");

	std::string httpAddress("0.0.0.0:");
	std::string httpPort = "8000";
	const char * port = getenv("PORT");
	if (port)
	{
		httpPort = port;
	}
	httpAddress.append(httpPort);


	const char *configFile = "streamer.json";
	Json::Value config;
	std::ifstream stream(configFile);
	if (stream.good())
		stream >> config;
	else 
		config = Json::objectValue;	// assign {}
	
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

	std::cout << "Streamer:"<<config<<std::endl;
	

	rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)logLevel);
	rtc::LogMessage::LogTimestamps();
	rtc::LogMessage::LogThreads();
	std::cout << "Logger level:" <<  rtc::LogMessage::GetLogToDebug() << std::endl;

	rtc::Thread* thread = rtc::Thread::Current();
	rtc::InitializeSSL();

	// webrtc server
	std::list<std::string> iceServerList;
	iceServerList.push_back(std::string("stun:")+stunurl);
	if (strlen(turnurl)) {
		iceServerList.push_back(std::string("turn:")+turnurl);
	}
	
	std::vector<std::string> options;

	PeerConnectionManager webRtcServer(iceServerList, urlVideoList, urlAudioList, audioLayer, publishFilter);
	//API api(&webRtcServer);
	API2 api(&webRtcServer, config);
	
	if (!webRtcServer.InitializePeerConnection())
	{
		std::cout << "Cannot Initialize WebRTC server" << std::endl;
	}
	else
	{
		// http server

		options.push_back("document_root");
		options.push_back(webroot);
		options.push_back("access_control_allow_origin");
		options.push_back("*");
		options.push_back("listening_ports");
		options.push_back(httpAddress);
		if (!sslCertificate.empty()) {
			options.push_back("ssl_certificate");
			options.push_back(sslCertificate);
		}
		if (!nbthreads.empty()) {
			options.push_back("num_threads");
			options.push_back(nbthreads);
		}
		if (!passwdFile.empty()) {
			options.push_back("global_auth_file");
			options.push_back(passwdFile);
			options.push_back("authentication_domain");
			options.push_back(authDomain);
		}






		try {
			std::cout << "HTTP Listen at " << httpAddress << std::endl;

			HttpServerRequestHandler httpServer(&api, "/api/", options);

			// start STUN server if needed
			std::unique_ptr<cricket::StunServer> stunserver;
			if (localstunurl != NULL)
			{
				rtc::SocketAddress server_addr;
				server_addr.FromString(localstunurl);
				rtc::AsyncUDPSocket* server_socket = rtc::AsyncUDPSocket::Create(thread->socketserver(), server_addr);
				if (server_socket)
				{
					stunserver.reset(new cricket::StunServer(server_socket));
					std::cout << "STUN Listening at " << server_addr.ToString() << std::endl;
				}
			}
			
			// mainloop
			thread->Run();

		} catch (const CivetException & ex) {
			std::cout << "Cannot Initialize start HTTP server exception:" << ex.what() << std::endl;
		}
	}

	rtc::CleanupSSL();
	return 0;
}

