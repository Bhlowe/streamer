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
#include "WebRTCStreamer.h"

int simple(const Json::Value & config);


/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int simple(int argc, char* argv[])
{
	const char *configFile = "streamer.json";	// read default json file
	Json::Value config;
	std::ifstream stream(configFile);
	if (stream.good())
		stream >> config;
	else 
		config = Json::objectValue;	// assign {}
	
	WebRTCStreamer streamer(config);
	bool ok = streamer.init();
	if (ok)
	{
		streamer.run();
	}
}
