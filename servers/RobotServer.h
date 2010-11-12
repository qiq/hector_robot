/*
 *
 */

#ifndef _ROBOT_SERVER_
#define _ROBOT_SERVER_

#include "config.h"

#include <vector>
#include <tr1/unordered_map>
#include "BaseServer.h"

class RobotServer : public BaseServer {
public:
	RobotServer(ObjectRegistry *objects, std::vector<ProcessingEngine*> *engines);
	~RobotServer();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	bool HandleExtension(SimpleHTTPConn *conn);

protected:
	static log4cxx::LoggerPtr logger;
};

#endif
