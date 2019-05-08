/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerHandler.cpp
**
** -------------------------------------------------------------------------*/

#include <iostream>
#include <csignal>

#include "HttpServerRequestHandler2.h"

/* ---------------------------------------------------------------------------
**  Constructor
** -------------------------------------------------------------------------*/
HttpServerRequestHandler2::HttpServerRequestHandler2(PeerConnectionManager* webRtcServer, const std::vector<std::string>& options, const Json::Value &config)
	: HttpServerRequestHandler(webRtcServer, options),
	m_config(config)
{
	
	if (!m_config.isMember("urls"))
	{
		Json::Value empty;
		m_config["urls"] = empty;
	}
	
	
	// http api callbacks

	// register handlers
	m_func["/api/getConfig"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		if (!isAdmin(req_info, in)) 
			return this->unauthorized();
		return m_config;
	};
	
	m_func["/api/getSources"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		if (!isAdmin(req_info, in)) return this->unauthorized();
		return m_config["urls"];
	};
		
	m_func["/webrtc-api/version"]                  = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		Json::Value answer;
		answer["version"] = VERSION;
		std::cout<<"version:"<< answer<<"\n";
		
		return answer;
	};

	// TAKE OUT.. or make null.. for older webrtc-streamer access to list of streams
	m_func["/webrtc-api/getMediaList"]          = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		
		if (!isAdmin(req_info, in)) return this->unauthorized();
			return m_config["urls"];

	};

	
	m_func["/api/addSource"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
    if (!isAdmin(req_info, in)) return unauthorized();
    std::string id = this->getParam(req_info, in, "id");
    if (id.empty())
      return this->error("id required");

	Json::Value source(in);
	source.removeMember("auth");
	source.removeMember("id");	// its the index.
	
	if (!source.isMember("video")) return this->error("video required");
  	Json::Value urls = m_config["urls"];
	
	std::cout<<"addSource: "<< source<<"\n";
	
	// urls[id] = source;
	m_config["urls"][id] = source;
	
	
	std::cout<<urls<<"\n";
	
	
	/*
    std::string video = this->getParam(req_info, in, "video");  // rtsp://
    std::string options = this->getParam(req_info, in, "options");
	
    if (video.empty())
      return this->error("video required");
  
	Json::Value source;
	source["video"] = url;
	source["options"] = options;
	urls[id] = source;
	*/
	
	return source;
  };

  
  	m_func["/api/call"]                  = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		std::string peerid;
		std::string audiourl;
		std::string options="rtptransport=tcp&timeout=60";
		std::cout << "api/call "<<in<<" args="<<req_info->query_string<<"\n";

		if (!req_info->query_string) return this->error("no query string");
		
		CivetServer::getParam(req_info->query_string, "peerid", peerid);
		if (peerid.empty()) return this->error("peerid not found");

		std::string id = this->getParam(req_info, in, "id");
		if (id.empty())
			id = this->getParam(req_info, in, "url");		// webrtc-streamer compat

		
		Json::Value source = this->getSource(id);
		if (!source) return this->error("source not found");
		
		std::string videourl=source["video"].asString();
		
		if (videourl.empty()) 
		{
			std::cout << "call failed, video url empty:"<<in<<"\n";
			return this->error("video stream not found");
		}
		
		Json::Value out = m_webRtcServer->call(peerid, videourl, audiourl, options, in);				
		std::cout << "cust call "<<videourl<<" "<<out<<"\n";
		return out;
	};


  
  
  	m_func["/api/createOffer"]           = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		std::cout << "custom createOffer!"<<std::endl;
		std::string peerid;
		std::string url;
		std::string audiourl;
		std::string options;
		if (req_info->query_string) {
            CivetServer::getParam(req_info->query_string, "peerid", peerid);
            CivetServer::getParam(req_info->query_string, "url", url);
            CivetServer::getParam(req_info->query_string, "audiourl", audiourl);
            CivetServer::getParam(req_info->query_string, "options", options);
        }
		return m_webRtcServer->createOffer(peerid, url, audiourl, options);
	};

  
  m_func["/api/removeSource"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
    if (!isAdmin(req_info, in)) 
		return this->unauthorized();
	std::string id = this->getParam(req_info, in, "id");
	if (id.empty()) 
		return error("id required");
	
	if (m_config["urls"].isMember(id))
	{
		m_config["urls"].removeMember(id);
		std::cout<<"Remove "<<id<<"\n";
		if (m_config["urls"].isMember(id))
			return this->error("Unable to remove?");
		return success();
		
	} 
	
	std::cout << "removeSource "<<id<< " failed not found:"<<m_config["urls"]<<"\n";
	
	
	return this->error("src not found");
	};
	
	httpFunction test = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {  
	Json::Value out;
	out["foo"] = "bar";
	return out;
	
	};
	
	m_func["/api/test"] = test;
	
	
	
	  m_func["/api/getStreams"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
		if (!isAdmin(req_info, in)) return unauthorized();
		return this->getStreams();
	};

  
  // body:{"src":"Macedo_WEBRTC_704x480","auth":"odie","token":"Macedo_WEBRTC_704x480_S10_OUOY"}
  m_func["/api/addToken"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
    if (!isAdmin(req_info, in)) return unauthorized();
	std::string id = getParam(req_info, in, "id");
	std::string src=getParam(req_info, in, "src");
	RTC_LOG(LS_ERROR) << "addToken id:"<<id<< " src:" << src;
	if (id.empty()) error("id required");
	if (src.empty()) error("src required");
	if (!this->hasSource(src))
	{
		return this->error("src not found");
	}
	
	tokenMap.insert(std::pair<std::string, std::string >(id, src));
	return success();
  };

  m_func["/api/removeToken"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
    if (!isAdmin(req_info, in)) return unauthorized();
		std::string id = getParam(req_info, in, "id");
	if (id.empty()) this->error("id required");

      return this->removeToken(id);
  };

  m_func["/api/getTokens"] = [this](const struct mg_request_info *req_info, const Json::Value & in) -> Json::Value {
	
	if (!isAdmin(req_info, in)) return unauthorized();
	return this->listTokens();
  };


	
 	for (auto it : m_func) {
		std::string n(it.first);
		if (n.find("/api/")==0)
		{
			n.replace(0, 5, "/webrtc-api/");
			std::cout<<"replace "<<n<<"\n";
			m_func[n] = it.second;
		}

		}
	
	this->installHandlers();
	
	
	
	
	
}

/*
const std::string HttpServerRequestHandler2::getParam(const struct mg_request_info *req_info,  std::string name)
{
	std::string out;
	if (req_info->query_string)
	{
		CivetServer::getParam(req_info->query_string, name.c_str(), out);
	}
	
	return out;
}
*/


const std::string  
HttpServerRequestHandler2::getParam(const struct mg_request_info *req_info, const Json::Value & in, const std::string & param)
{
  std::string out;

  if (!rtc::GetStringFromJsonObject(in, param.c_str(), &out))
  {
    if (req_info && req_info->query_string && strlen(req_info->query_string))
    {
      CivetServer::getParam(req_info->query_string, param.c_str(), out);
    }
  }

  if (out.empty())
    RTC_LOG(LS_ERROR) << "getParam "<<param<<" not found";
  return out;
}


const Json::Value HttpServerRequestHandler2::addToken(const std::string &token, const std::string &src)
{
}


const Json::Value HttpServerRequestHandler2::removeToken(const std::string &token)
{
	std::map<std::string, std::string >::iterator  it = tokenMap.find(token);
	if (it != tokenMap.end())
	{
		RTC_LOG(LS_ERROR) << "removeToken "<<token;
		tokenMap.erase(it);
		return success();
	}
    return this->error("token not found");
}

const Json::Value HttpServerRequestHandler2::listTokens()
{
	Json::Value value;
	for (auto token : tokenMap)
	{
		Json::Value e;
		e["src"]=token.second;
		value[token.first] = e;
	}
	if (!value) value = Json::objectValue;
	return value;
}





Json::Value HttpServerRequestHandler2::getSource(const std::string &id)
{
	if (m_config["urls"].isMember(id))
		return Json::Value(m_config["urls"][id]);
	return Json::objectValue;	// assign {}

}

const Json::Value HttpServerRequestHandler2::error(const std::string &err)
{
	Json::Value value;
	value["error"] = err;
	value["success"] = false;
	std::cout << "HttpServerRequestHandler2::error:"<<err<<"\n";
	return value;
}

const bool HttpServerRequestHandler2::isAdmin(const struct mg_request_info *req_info, const Json::Value & in)
{
	std::string auth = this->getParam(req_info, in, "auth");
	if (m_config.isMember("auth"))
	{
		int c = auth.compare(m_config["auth"].asString());
		return c==0;
	}
	
	return true;	
}

const Json::Value HttpServerRequestHandler2::success()
{
        Json::Value value;
        value["success"] = true;
        return value;
}

bool HttpServerRequestHandler2::hasToken(const std::string &token, const std::string &src)
 {
         bool has = hasSource(src);
         assert(has);

         std::map<std::string,std::string>::iterator it = tokenMap.find(token);
         if (it != tokenMap.end())
         {
                        if (has && src.compare(it->second)==0)
                                return true;
         }
         return false;
 }

const Json::Value HttpServerRequestHandler2::getStreams()
 {
	 #if 1
	 	Json::Value out(Json::arrayValue);
		Json::Value list = m_webRtcServer->getPeerConnectionList();
		for (int x=0;x<list.size();x++)
		{
			std::string id = list[x].getMemberNames()[0];
			out.append(id);
		}
		return out;
	 #else
	Json::Value out;
	int c = 0;
	std::cout<<"getStreams start\n";
	
	Json::Value list = m_webRtcServer->getPeerConnectionList();
	std::cout<<list<<"\n";
	std::cout<<"list size = "<<list.size()<<"\n";
	
	for (int x=0;x<list.size();x++)
	{
		Json::Value v = list[x];
		int m = v.getMemberNames().size();
		std::string id = v.getMemberNames()[0];
		Json::Value val = v[id];
		std::cout<<"id = "<<id<<" val="<<val<<" m="<<m<<"\n";
		out[id] = val;
	}

	std::cout<<"getStreams end"<<out<<"\n";
	#endif
	
	return out;
 }
 
 
 Json::Value HttpServerRequestHandler2::handleRequest(const struct mg_request_info *req_info, const Json::Value & in)
{
	std::cout<<"handleRequest2 "<<req_info->request_uri<<"\n";
	httpFunction fct = this->getFunction(req_info->request_uri);
	if (fct==NULL)
	{

		std::string n(req_info->request_uri);
		if (n.find("/webrtc-api/")==0)
		{
			n.replace(0, 12, "/api/");
			std::cout<<"replace "<<n<<"\n";
			
			fct = this->getFunction(n);
		}

	}
	
	if (fct != NULL)
	{
		Json::Value out = fct(req_info, in);
		std::cout << "handleRequest " << req_info->request_uri << out <<std::endl;
		if (!out) 
			out = Json::objectValue;	// assign {}
		return out;
	} else {
		std::cout << "getFunction failed!" << req_info->request_uri <<std::endl;

	Json::Value out;
	return out;
		
	}
}


 bool HttpServerRequestHandler2::hasSource(const std::string &src)
{
	return m_config["urls"].isMember(src);
}


