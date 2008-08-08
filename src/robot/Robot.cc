/**
 * Main robot class
 */

#include "log4cxx/logger.h"
#include "log4cxx/propertyconfigurator.h"
#include "Config.h"
#include "ProcessingChain.h"
#include "RobotHTTPServer.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

vector<ProcessingChain*> processingChains;

log4cxx::LoggerPtr logger = Logger::getLogger("robot.Robot");

bool parseConfig(const char *fileName) {
	// read & parse config file
	Config *config = new Config();
	config->parseFile(fileName);

	// find which programs we should run
	int run = config->getSize("robot", "run");
	for (int i = 0; i < run; i++) {
		const char *processName = config->getValue("robot", "run", i);
		const char *pType = config->getType(processName);
		if (!strcmp(pType, "ProcessingChain")) {
			ProcessingChain *pc = new ProcessingChain();
			if (!pc->Init(config, processName))
				return false;
			processingChains.push_back(pc);
		} else {
			LOG4CXX_ERROR(logger, "Unknown process type: " << pType);
		}
	}
	return true;
}

int main(int argc, char *argv[]) {
	// set up logging
	PropertyConfigurator::configure("../config/robot.log.props");
	// process Config file
	bool parsed = parseConfig("../config/config.xml"); // FIXME: configurable config file path :-)
	printf("parsed: %d\n", parsed ? 1 : 0);

	// create HTTP server
	RobotHTTPServer *server = new RobotHTTPServer(NULL, 1234);
	server->RestrictAccess("127.0.0.1");
	server->Start(2);

//	fprintf(stderr, "x: %s\n", config->getValue("robot", "server-port"));
//	fprintf(stderr, "x: %s\n", config->getValue("robot", "server-port", 1));

	printf("Server running\n");
	char s[10];
	fgets(s, sizeof(s), stdin);
	exit(0);
}
