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
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	char *outputDir;	// ObjectLock, where to save resource files

	char *GetItems(const char *name);
	char *GetOutputDir(const char *name);
	void SetOutputDir(const char *name, const char *value);

	ObjectValues<Scheduler> *values;
	char *GetValueSync(const char *name);
	bool SetValueSync(const char *name, const char *value);
	std::vector<std::string> *ListNamesSync();

	struct OpenFile {
		int fd;
		google::protobuf::io::FileOutputStream* file;
		google::protobuf::io::CodedOutputStream* stream;
	};

	uint32_t currentTime;
	std::tr1::unordered_map<int, OpenFile*> openFiles;

	void CloseFiles();

	int webResourceTypeId;	// WebResource typeId
};

inline Module::Type Scheduler::GetType() {
	return SIMPLE;
}

inline char *Scheduler::GetValueSync(const char *name) {
	return values->GetValue(name);
}

inline bool Scheduler::SetValueSync(const char *name, const char *value) {
	return values->SetValue(name, value);
}

inline std::vector<std::string> *Scheduler::ListNamesSync() {
	return values->ListNames();
}

#endif
