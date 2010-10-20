/*
 *
 */

#ifndef _ROBOT_SERVER_
#define _ROBOT_SERVER_

#include <pthread.h>
#include <vector>
#include <log4cxx/logger.h>
#include "ObjectRegistry.h"
#include "SimpleHTTPServer.h"
#include "SimpleHTTPConn.h"

class ProcessingEngine;

class RobotServer : public SimpleHTTPServer {
	ObjectRegistry *objects;
	std::vector<ProcessingEngine*> *engines;

	static log4cxx::LoggerPtr logger;
public:
	RobotServer(ObjectRegistry *objects, std::vector<ProcessingEngine*> *engines);
	~RobotServer();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	bool HandleRequest(SimpleHTTPConn *conn);
};

#endif
