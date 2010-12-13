/**
Scheduler.la, simple, native
Schedule next refresh of a resource. It uses time quantization of 1000 seconds
(00:16:40). Files are named after the absolute time of processing.

Dependencies: protobuf

Parameters:
items		r/o	Total items processed
outputDir	r/w	Where to save files to.

Status:
untouched
*/

#ifndef _MODULES_SCHEDULER_H_
#define _MODULES_SCHEDULER_H_

#include <config.h>

#include <queue>
#include <string>
#include <tr1/unordered_map>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "common.h"
#include "Module.h"
#include "ObjectValues.h"
#include "WebResource.h"

class Scheduler : public Module {
public:
	Scheduler(ObjectRegistry *objects, const char *id, int threadIndex);
	~Scheduler();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	Resource *ProcessSimple(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	char *outputDir;	// ObjectLock, where to save resource files

	char *getItems(const char *name);
	char *getOutputDir(const char *name);
	void setOutputDir(const char *name, const char *value);

	ObjectValues<Scheduler> *values;
	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();

	struct OpenFile {
		int fd;
		google::protobuf::io::FileOutputStream* stream;
	};

	uint32_t currentTime;
	std::tr1::unordered_map<int, OpenFile*> openFiles;
};

inline Module::Type Scheduler::getType() {
	return SIMPLE;
}

inline char *Scheduler::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool Scheduler::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool Scheduler::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *Scheduler::listNamesSync() {
	return values->listNamesSync();
}

#endif
