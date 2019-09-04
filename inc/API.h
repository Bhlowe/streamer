#pragma once
#include "json/json.h"
#include <string>
#include "PeerConnectionManager.h"

// A singleton class to dispatch all http responses for webrtc streaming.
// Is http server agnostic, so all requests get a JSON input and respond with JSON.
class API
{
	public:

	API(PeerConnectionManager * webRtcServer);
	PeerConnectionManager * webRtcServer;
		
	// our RequestMethod dispatch format for calls below.
	// input is json, output is json.
	typedef const Json::Value(API::*RequestMethod)(const Json::Value &in);

	virtual const Json::Value getMediaList(const Json::Value &in);
	virtual const Json::Value getVideoDeviceList(const Json::Value &in);
	virtual const Json::Value getAudioDeviceList(const Json::Value &in);
	virtual const Json::Value getIceServers(const Json::Value &in);
	virtual const Json::Value call(const Json::Value &in);
	virtual const Json::Value hangup(const Json::Value &in);
	virtual const Json::Value createOffer(const Json::Value &in);
	virtual const Json::Value setAnswer(const Json::Value &in);
	virtual const Json::Value getIceCandidate(const Json::Value &in);
	virtual const Json::Value addIceCandidate(const Json::Value &in);
	virtual const Json::Value getPeerConnectionList(const Json::Value &in);
	virtual const Json::Value getStreamList(const Json::Value &in);
	virtual const Json::Value version(const Json::Value &in);
	virtual const Json::Value log(const Json::Value &in);
	virtual const Json::Value help(const Json::Value &in);
	
	protected: 
    virtual const Json::Value error(int error_code, const std::string & error_msg);   // create JSONObject with "error_msg", "error_code"
	virtual const Json::Value error(const std::string & error_msg) { return error(404, error_msg); } 
	
	virtual const Json::Value success(); // return {} empty json
	virtual bool isAuthorized(std::string cmd, const Json::Value &in) { return true; };	// input is command, returns true if authorized
	
	public: 
	virtual const Json::Value dispatch(std::string cmd,const Json::Value &in);			// input is command, such as call, help, getMediaList.
	
	virtual void addMethod(std::string cmd, API::RequestMethod m);
	virtual void installMethods();
	
	virtual std::map<std::string,API::RequestMethod> getFunctionMap() { return functionMap; }
	private:
	std::map<std::string, API::RequestMethod> functionMap;		// list of known methods

};

