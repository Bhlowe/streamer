/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerHandler.h
** 
** -------------------------------------------------------------------------*/
#pragma once

#include <functional>

#include "CivetServer.h"
#include "PeerConnectionManager.h"

typedef std::function<Json::Value(const struct mg_request_info *, const Json::Value &)> httpFunction;

/* ---------------------------------------------------------------------------
**  http callback
** -------------------------------------------------------------------------*/
class HttpServerRequestHandler : public CivetServer
{
	public:
		HttpServerRequestHandler(PeerConnectionManager* webRtcServer, const std::vector<std::string>& options); 
	
		httpFunction getFunction(const std::string& uri);
		
		virtual Json::Value handleRequest(const struct mg_request_info *, const Json::Value &request);
		
		
	protected:
		PeerConnectionManager* m_webRtcServer;
		std::map<std::string,httpFunction> m_func;
		virtual void installHandlers();
};
