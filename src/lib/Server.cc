// 
#include <config.h>

#include "Server.h"

log4cxx::LoggerPtr Server::logger(log4cxx::Logger::getLogger("lib.Server"));

Server::Server() {
	serverHost = NULL;
	//processingCahins = NULL;
	simpleHTTPServer = NULL;
}

Server::~Server() {
	free(serverHost);
	delete simpleHTTPServer;
}

bool Server::Init(Config *config, const char *id) {
	char buffer[1024];
	char *s;
	vector<string> *v;

	char *baseDir = config->getFirstValue("/Config/@baseDir");
	if (!baseDir) {
		LOG4CXX_ERROR(logger, "Cannot find baseDir");
		return false;
	}

	// threads
	snprintf(buffer, sizeof(buffer), "/Config/Server[@id='%s']/threads", id);
	s = config->getFirstValue(buffer);
	if (!s || sscanf(s, "%d", &threads) != 1) {
		LOG4CXX_ERROR(logger, "Invalid number of threads, using 1 thread");
		threads = 1;
	}
	free(s);

	// serverHost
	snprintf(buffer, sizeof(buffer), "/Config/Server[@id='%s']/serverHost", id);
	serverHost = config->getFirstValue(buffer);
	if (!serverHost) {
		LOG4CXX_ERROR(logger, "Server/serverHost not found");
		return false;
	}

	// serverPort
	snprintf(buffer, sizeof(buffer), "/Config/Server[@id='%s']/serverPort", id);
	s = config->getFirstValue(buffer);
	if (!s || sscanf(s, "%d", &serverPort) != 1) {
		LOG4CXX_ERROR(logger, "Server/serverPort not found");
		return false;
	}
	free(s);

	// create processing chain(s)
	snprintf(buffer, sizeof(buffer), "/Config/Server[@id='%s']/processingChain/@ref", id);
	v = config->getValues(buffer);
	for (vector<string>::iterator iter = v->begin(); iter != v->end(); iter++) {
		// TODO: create processing chain
printf("%s\n", iter->c_str());
	}
	delete v;

	// library
	snprintf(buffer, sizeof(buffer), "/Config/Server[@id='%s']/lib/@name", id);
	s = config->getFirstValue(buffer);
	if (!s) {
		LOG4CXX_ERROR(logger, "Server/lib not found");
		return false;
	}
	
	// load library
	snprintf(buffer, sizeof(buffer), "%s/%s", baseDir, s);
	SimpleHTTPServer *(*create)(Server*) = (SimpleHTTPServer*(*)(Server*))loadLibrary(buffer, "create");
	if (!create) {
		LOG4CXX_ERROR(logger, "Invalid library: " << buffer);
		return false;
	}
	simpleHTTPServer = (*create)(this);
	free(s);

	free(baseDir);
	return true;
}

void Server::Start(bool wait) {
	// start server
	LOG4CXX_INFO(logger, "Starting server " << serverHost << ":" << serverPort << " (" << threads << ")");
	simpleHTTPServer->Start(serverHost, serverPort, threads, true);
	if (wait)
		LOG4CXX_INFO(logger, "Stopping server");
}

void Server::Stop() {
	LOG4CXX_INFO(logger, "Stopping server");
	simpleHTTPServer->Stop();
}

const char *Server::getValue(const char *name) {
	// TODO
	return NULL;
}

bool Server::setValue(const char *name, const char *value) {
	// TODO
	return false;
}
