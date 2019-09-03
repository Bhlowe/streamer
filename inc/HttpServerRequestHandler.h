/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerRequestHandler.h
** 
** -------------------------------------------------------------------------*/

#pragma once

#include <list>
#include <map>
#include <functional>

#include "json/json.h"
#include "CivetServer.h"
#include "API.h"


/* ---------------------------------------------------------------------------
**  http callback
** -------------------------------------------------------------------------*/
class HttpServerRequestHandler : public CivetServer
{
	public:
	HttpServerRequestHandler(API * api, const std::string prefix, const std::vector<std::string>& options); 
	const std::string prefix;		//  prefix for all webrtcserver calls. Set to "/api/" by default
	API * api;
};



