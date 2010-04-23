/*
 *
 */

#ifndef _ROBOT_SERVER_
#define _ROBOT_SERVER_

#include <pthread.h>
#include <log4cxx/logger.h>
#include "ObjectRegistry.h"
#include "SimpleHTTPServer.h"
#include "SimpleHTTPConn.h"

class RobotServer : public SimpleHTTPServer {
	ObjectRegistry *objects;

	static log4cxx::LoggerPtr logger;
public:
	RobotServer(ObjectRegistry *objects);
	~RobotServer();
	bool HandleRequest(SimpleHTTPConn *conn);
};

#endif
