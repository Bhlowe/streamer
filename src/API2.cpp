#include "API2.h"
#include <iostream>


API2::API2(PeerConnectionManager * webRtcServer, const Json::Value & config)
	: API(webRtcServer), config(config)
{
	std::cout << "API2\n";
	installMethods();

}


void API2::installMethods()
{
	std::cout << "installMethods\n";

	API::installMethods();

	addMethod("test", (API::RequestMethod) &API2::test);
	addMethod("addSource", (API::RequestMethod) &API2::addSource);
	addMethod("removeSource", (API::RequestMethod) &API2::removeSource);
	std::cout << "installMethods\n";

}


const Json::Value API2::addSource(const Json::Value &in)
{
	if (!in.isMember("video")) 
		return error("video required");
	if (!in.isMember("id")) 
		return error("id required");

    std::string id = in["id"].asString();
	if (id.empty())
      return error("id required");

	Json::Value source;
	source["video"] = in["video"];
	if (in.isMember("audio")) 
		source["audio"] = in["audio"];
	
	Json::Value urls = config["urls"];
	
	std::cout << "addSource: "<< source<<"\n";
	
	config["urls"][id] = source;
	return source;
}


const Json::Value API2::removeSource(const Json::Value &in)
{
	std::string id = in["id"].asString();
	if (id.empty()) 
		return error("id required");
	
	if (config["urls"].isMember(id))
	{
		config["urls"].removeMember(id);
		std::cout<<"Remove "<<id<<"\n";
		if (config["urls"].isMember(id))
			return this->error("Unable to remove?");
		return success();
	} 
	
	std::cout << "removeSource "<<id<< " failed not found:"<<config["urls"]<<"\n";
	return this->error("src not found");
}
	


