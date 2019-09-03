/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** HttpServerRequestHandler.cpp
** 
** -------------------------------------------------------------------------*/

#include <string.h>
#include <iostream>
    
#include "HttpServerRequestHandler.h"


static int log_message(const struct mg_connection *conn, const char *message) 
{
    fprintf(stderr, "%s\n", message);
    return 0;
}

static struct CivetCallbacks _callbacks;
static const struct CivetCallbacks * getCivetCallbacks() 
{
    memset(&_callbacks, 0, sizeof(_callbacks));
    _callbacks.log_message = &log_message;
    return &_callbacks;
}


/* ---------------------------------------------------------------------------
**  Civet HTTP callback 
** -------------------------------------------------------------------------*/
class RequestHandler : public CivetHandler
{
  public:
	RequestHandler() {
	}	  
	
    bool handle(CivetServer *server, struct mg_connection *conn)
    {
        bool ret = false;
        const struct mg_request_info *req_info = mg_get_request_info(conn);
        HttpServerRequestHandler * httpServer = (HttpServerRequestHandler*) server;
        
		// read input
		const Json::Value in = this->getInputMessage(req_info, conn);

		// invoke API implementation
		std::string cmd(req_info->local_uri);
		std::size_t pos = cmd.find(httpServer->prefix);      // position of "/api/" in str
		
		if (pos==0)	
			cmd = cmd.substr(httpServer->prefix.length());
		else 
			std::cout << "expected cmd to start with prefix"<<std::endl;
		
		const Json::Value out = httpServer->api->dispatch(cmd, in);

		// fill out
		if (out.isNull() == false)
		{
			std::string answer(Json::StyledWriter().write(out));
			std::cout << "answer:" << answer << std::endl<<" bytes="<<answer.size()<<std::endl;	
			mg_printf(conn,"HTTP/1.1 200 OK\r\n");
			mg_printf(conn,"Access-Control-Allow-Origin: *\r\n");
			mg_printf(conn,"Content-Type: application/json\r\n");
			mg_printf(conn,"Content-Length: %zd\r\n", answer.size());
			mg_printf(conn,"Connection: close\r\n");
			mg_printf(conn,"\r\n");
			mg_write( conn, answer.c_str(), answer.size() );	// don't use printf- might include % escape codes
			ret = true;
		}	else
		{
			std::cout << "unexpected null: cmd="<<cmd<<" in:" << in<< std::endl;
		}		
        
        return ret;
    }
    bool handleGet(CivetServer *server, struct mg_connection *conn)
    {
        return handle(server, conn);
    }
    bool handlePost(CivetServer *server, struct mg_connection *conn)
    {
        return handle(server, conn);
    }

  private:
    Json::Value getInputMessage(const struct mg_request_info *req_info, struct mg_connection *conn) {
        Json::Value  jmessage;

        // read input
        long long tlen = req_info->content_length;
        if (tlen > 0)
        {
            std::string body;
            long long nlen = 0;
            const long long bufSize = 1024;
            char buf[bufSize];
            while (nlen < tlen) {
                long long rlen = tlen - nlen;
                if (rlen > bufSize) {
                    rlen = bufSize;
                }
                rlen = mg_read(conn, buf, (size_t)rlen);
                if (rlen <= 0) {
                    break;
                }
                body.append(buf, rlen);
                nlen += rlen;
            }

            // parse in
            Json::Reader reader;
            if (!reader.parse(body, jmessage))
            {
                std::cout << "Received unknown message:" << body << std::endl;
            }
        } 
		
		// add any GET args, even if it is a post request. 
		if (req_info->query_string)
		{
			// add any/all parameters to JSON
			std::string postData = req_info->query_string;
			std::stringstream tokenStream(postData);
			std::string keyValueStr, key, value;
			while (std::getline(tokenStream, keyValueStr, '&')) {
				if (keyValueStr.length() > 0) {
					std::stringstream ts(keyValueStr);
					std::getline(ts, key, '=');
					std::getline(ts, value, '=');
					if (!key.empty() && !value.empty()) {
						jmessage[key] = value;	// May want to call: UriCodec::decode(value);	 
					}
				}
			}
		}
		// hack the remote address of the client into the "input" json object so it can be used by getIceCandidate
		if (req_info->remote_addr)
			jmessage["remote_addr"] = req_info->remote_addr;
        return jmessage;
    }	
};


/* ---------------------------------------------------------------------------
**  Constructor
** -------------------------------------------------------------------------*/
HttpServerRequestHandler::HttpServerRequestHandler(API * api, const std::string prefix, const std::vector<std::string>& options) 
    : CivetServer(options, getCivetCallbacks()), api(api), prefix(prefix)
{
	std::string apiPrefix = prefix + "*";		// get all /api/ calls. 
	this->addHandler(apiPrefix, new RequestHandler());
}	
