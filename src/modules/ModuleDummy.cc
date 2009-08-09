/**
 * Dummy module.
 */
#include <config.h>

#include <string.h>
#include "ModuleDummy.h"
#include "Server.h"
#include "WebResource.h"

const char *ModuleDummy::getDummy() {
	return dummy;
}

void ModuleDummy::setDummy(const char *value) {
	free(dummy);
	dummy = strdup(value);
}

ModuleDummy::ModuleDummy(ObjectRegistry *objects, const char *id): Module(objects, id) {
	dummy = NULL;

	getters["dummy"] = &ModuleDummy::getDummy;
	setters["dummy"] = &ModuleDummy::setDummy;
}

ModuleDummy::~ModuleDummy() {
	free(dummy);
}

bool ModuleDummy::Init(Config *config) {
	return true;
}

void ModuleDummy::Process(Resource *resource) {
	WebResource *wr = dynamic_cast<WebResource*>(resource);
	if (wr) {
		LOG4CXX_INFO(logger, "Dummy: processing resource " << wr->getURL());
	}
	return;
}

void ModuleDummy::createCheckpoint() {
	// TODO
}

const char *ModuleDummy::getValue(const char *name) {
	const char *result = NULL;
	stdext::hash_map<string, const char*(ModuleDummy::*)(), string_hash>::iterator iter = getters.find(name);
	if (iter != getters.end()) {
		lock.lock();
		result = (this->*(iter->second))();
		lock.unlock();
	}
	return result;
}

bool ModuleDummy::setValue(const char *name, const char *value) {
	stdext::hash_map<string, void(ModuleDummy::*)(const char*), string_hash>::iterator iter = setters.find(name);
	if (iter != setters.end()) {
		lock.lock();
		(this->*(iter->second))(value);
		lock.unlock();
		return true;
	}
	return false;
}

vector<string> *ModuleDummy::listNames() {
	vector<string> *result = new vector<string>();
	for (stdext::hash_map<string, const char*(ModuleDummy::*)(), string_hash>::iterator iter = getters.begin(); iter != getters.end(); iter++) {
		result->push_back(iter->first);
	}
	return result;
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id) {
	return new ModuleDummy(objects, id);
}

extern "C" void destroy(Module* p) {
	delete p;
}
