//
#include <config.h>

#include "common.h"
#include "Module.h"

log4cxx::LoggerPtr Module::logger(log4cxx::Logger::getLogger("lib.processing_engine.Module"));

bool Module::Init(vector<pair<string, string> > *args) {
	LOG_ERROR(logger, "Unimplemented module " << getId() << " (Init)");
	return false;
}

module_t Module::getType() {
	LOG_ERROR(logger, "Unimplemented module " << getId() << " (getType)");
	return MODULE_INVALID;
}

Resource *Module::Process(Resource *resource) {
	LOG_ERROR(logger, "Unimplemented resource processing " << getId() << " (single)");
	return NULL;
}

int Module::Process(Resource **in, Resource **out) {
	LOG_ERROR(logger, "Unimplemented resource processing " << getId() << " (multi)");
	return 0;
}
