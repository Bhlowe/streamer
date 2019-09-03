#include "API.h"
#include <iostream>

API::API(PeerConnectionManager * webRtcServer)
	: webRtcServer(webRtcServer)
{
	installMethods();
}

void API::addMethod(std::string cmd, RequestMethod m)
{
	functionMap[cmd] = m;
	// std::cout <<"add "<<cmd<<" "<<func[cmd]<<" m="<<m<<"\n";
}


void API::installMethods()
{
	addMethod("call", &API::call);
	addMethod("getMediaList", &API::getMediaList);
	addMethod("getVideoDeviceList", &API::getVideoDeviceList);
	addMethod("getAudioDeviceList", &API::getAudioDeviceList);
	addMethod("getIceServers", &API::getIceServers);
	addMethod("hangup", &API::hangup);
	addMethod("createOffer", &API::createOffer);
	addMethod("setAnswer", &API::setAnswer);
	addMethod("getIceCandidate", &API::getIceCandidate);
	addMethod("addIceCandidate", &API::addIceCandidate);
	addMethod("getPeerConnectionList", &API::getPeerConnectionList);
	addMethod("getStreamList", &API::getStreamList);
	addMethod("version", &API::version);
	addMethod("log", &API::log);
	addMethod("help", &API::help);
}

const Json::Value API::getMediaList(const Json::Value &in){
	return webRtcServer->getMediaList();
 }

const Json::Value API::getVideoDeviceList(const Json::Value &in){
	return webRtcServer->getVideoDeviceList();
 }

const Json::Value API::getAudioDeviceList(const Json::Value &in){
	return webRtcServer->getAudioDeviceList();
 }

// NOTE: in must be modified by the server dispatch routine to include "remote_addr" on all requests.
const Json::Value API::getIceServers(const Json::Value &in){
	
	if (!in.isMember("remote_addr"))
		std::cout<< "dev warning: server should have inserted remote_addr into input parameters\n";
	
	std::string remote_addr = in["remote_addr"].asString();
	std::cout<<"webRtcServer->getIceServers:remote_addr"<<remote_addr<<"\n";
	
	return webRtcServer->getIceServers(remote_addr);
 }

const Json::Value API::call(const Json::Value &in){
	std::string peerid = in["peerid"].asString();
	std::string url = in["url"].asString();
	std::string audiourl = in["audiourl"].asString();
	std::string options = in["options"].asString();
	return webRtcServer->call(peerid, url, audiourl, options, in);
 }

const Json::Value API::hangup(const Json::Value &in){
	std::string peerid = in["peerid"].asString();
	return webRtcServer->hangUp(peerid);
 }

const Json::Value API::createOffer(const Json::Value &in){
	
	std::string peerid = in["peerid"].asString();
	std::string url = in["url"].asString();
	std::string audiourl = in["audiourl"].asString();
	std::string options = in["options"].asString();
	return webRtcServer->createOffer(peerid, url, audiourl, options);
 }

const Json::Value API::setAnswer(const Json::Value &in){
	std::string peerid = in["peerid"].asString();
	webRtcServer->setAnswer(peerid, in);
	Json::Value answer(1);
	return answer;
}

const Json::Value API::addIceCandidate(const Json::Value &in){
	std::string peerid = in["peerid"].asString();
	return webRtcServer->addIceCandidate(peerid, in);
}
 

const Json::Value API::getIceCandidate(const Json::Value &in){
	std::string peerid = in["peerid"].asString();
	return webRtcServer->getIceCandidateList(peerid);
 }


const Json::Value API::getPeerConnectionList(const Json::Value &in){
	return webRtcServer->getPeerConnectionList();
 }

const Json::Value API::getStreamList(const Json::Value &in){
	return webRtcServer->getStreamList();
 }

const Json::Value API::version(const Json::Value &in){
	Json::Value answer(VERSION);
	return answer;
}
	

const Json::Value API::log(const Json::Value &in){
	if (in.isMember("level"))
	{
		int level = atoi(in["level"].asString().c_str());		
		rtc::LogMessage::LogToDebug((rtc::LoggingSeverity) level);
	}
	Json::Value answer(rtc::LogMessage::GetLogToDebug());
	return answer;
 }

const Json::Value API::help(const Json::Value &in) {
	Json::Value answer;
	for (auto it : functionMap) {
		std::string p = "/api/"+it.first;	// add in the prefix
		answer.append(p);
	}
	return answer;
 }


// create JSONObject with "error_msg", "error_code"
const Json::Value API::error(int error_code, const std::string & error_msg)
{
	Json::Value value;
	value["error_code"] = error_code;
	value["error_msg"] = error_msg;
	std::cout<< "Returning error:"<<error_code<<" "<<error_msg<<"\n";
	return value;
}

// return {} empty json
const Json::Value API::success()
{
	Json::Value value;
	return value;
}	


const Json::Value API::dispatch(std::string cmd, const Json::Value & in)
{
	if (!isAuthorized(cmd,in)) return error(401, "Unauthorized"); 
	std::map<std::string, RequestMethod>::iterator it = functionMap.find(cmd);
	if (it != functionMap.end())
	{
		RequestMethod method = it->second;
		const Json::Value out = (this->*method)(in); // if pDog is a pointer
		
		if (!out.isNull()) 
			return out;
		std::cout << "\n ** got null from: "<<cmd<< " in[" <<in<<"]\n";
		
		return success();
	} 
	std::string err("no such method:");
	err+=cmd;
	return error(404, err);
}

