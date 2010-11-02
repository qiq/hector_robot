/*
 *
 */

#ifndef _ROBOT_SERVER_
#define _ROBOT_SERVER_

#include "config.h"

#include <vector>
#include <tr1/unordered_map>
#include "ObjectRegistry.h"
#include "SimpleHTTPServer.h"
#include "SimpleHTTPConn.h"

class ProcessingEngine;

class RobotServer : public SimpleHTTPServer {
	ObjectRegistry *objects;
	std::vector<ProcessingEngine*> *engines;
	std::tr1::unordered_map<string, ProcessingEngine*> name2engine;
	int resourceId;

	static log4cxx::LoggerPtr logger;
public:
	RobotServer(ObjectRegistry *objects, std::vector<ProcessingEngine*> *engines);
	~RobotServer();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	bool HandleRequest(SimpleHTTPConn *conn);
};

#endif
