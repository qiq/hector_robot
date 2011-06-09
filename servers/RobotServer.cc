/**
 *
 */

#include "RobotServer.h"
#include "Object.h"
#include "ProcessingEngine.h"

log4cxx::LoggerPtr RobotServer::logger(log4cxx::Logger::getLogger("servers.robot.RobotServer"));

RobotServer::RobotServer(ObjectRegistry *objects, vector<ProcessingEngine*> *engines): BaseServer(objects, engines) {
}

bool RobotServer::Init(std::vector<std::pair<std::string, std::string> > *params) {
	return BaseServer::Init(params);
}

bool RobotServer::HandleExtension(SimpleHTTPConn *conn) {
	return false;
}

// factory functions

extern "C" SimpleHTTPServer* hector_server_create(ObjectRegistry *objects, std::vector<ProcessingEngine*> *engines) {
	return new RobotServer(objects, engines);
}
