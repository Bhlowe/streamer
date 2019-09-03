#pragma once
// HttpServer agnostic request handlers

#include "json/json.h"
#include <string>
#include "PeerConnectionManager.h"
#include "API.h"


class API2 : public API
{
	public:

	API2(PeerConnectionManager * webRtcServer, const Json::Value &in);
	
	Json::Value config;
	virtual const Json::Value test(const Json::Value &in) { return in; }		// echo test
	virtual const Json::Value addSource(const Json::Value &in);
	virtual const Json::Value removeSource(const Json::Value &in);
	virtual void installMethods();
};

