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
#include "HttpServerRequestHandler.h"


/* ---------------------------------------------------------------------------
**  http callback
** -------------------------------------------------------------------------*/
class HttpServerRequestHandler2 : public HttpServerRequestHandler
{
	public:
		HttpServerRequestHandler2(PeerConnectionManager* webRtcServer, const std::vector<std::string>& options, const Json::Value &config); 
	
				
	protected:
	Json::Value m_config;
	Json::Value getSource(const std::string &id);
	const Json::Value error(const std::string &err);
	const Json::Value unauthorized() { return this->error("unauthorized"); }
	const Json::Value success();
	const bool isAdmin(const struct mg_request_info *req_info, const Json::Value & in);
	
	
	const std::string getParam(const struct mg_request_info *req_info, const Json::Value & in, const std::string & param);

	
	std::map<std::string, std::string> tokenMap;            // token, stream_name           token is unique random string. stream_name is valid stream name/id


	virtual bool hasToken(const std::string &token, const std::string &stream_name);


	virtual bool hasSource(const std::string &stream_name);
	
	//virtual const Json::Value addStream(const std::string &stream_name, const std::string &url);
	//virtual const Json::Value removeStream(const std::string &stream_name);
	virtual const Json::Value addToken(const std::string &token, const std::string &stream_name);
	virtual const Json::Value removeToken(const std::string &token);
	virtual const Json::Value listTokens();
	virtual const Json::Value getStreams();

	virtual Json::Value handleRequest(const struct mg_request_info *, const Json::Value &request);

	
};
