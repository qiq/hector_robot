/**
RawTextRead.la, input, native
Read one or more files and create TextResources out of them (set text field
with the contents of every file).

Dependencies: none

Parameters:
items		r/o	Total items processed
inputFiles	init	List of files to load
inputListFile	init	File with list of files to load
mark		r/w	After all resources are read, emit MarkResource.
*/

#ifndef _MODULES_RAW_TEXT_READ_H_
#define _MODULES_RAW_TEXT_READ_H_

#include <config.h>

#include <deque>
#include <string>
#include <vector>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "TextResource.h"

class RawTextRead : public Module {
public:
	RawTextRead(ObjectRegistry *objects, const char *id, int threadIndex);
	~RawTextRead();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessInputSync(bool sleep);

private:
	int items;		// ObjectLock, items processed
	char *inputFiles;
	char *inputListFile;
	int mark;

	char *GetItems(const char *name);
	char *GetInputFiles(const char *name);
        void SetInputFiles(const char *name, const char *value);
	char *GetInputListFile(const char *name);
        void SetInputListFile(const char *name, const char *value);
	char *GetMark(const char *name);
	void SetMark(const char *name, const char *value);

	ObjectProperties<RawTextRead> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int textResourceTypeId;
	int markerResourceTypeId;
	bool markEmmited;
	std::deque<std::string> files;
};

inline Module::Type RawTextRead::GetType() {
	return Module::INPUT;
}

inline char *RawTextRead::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool RawTextRead::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *RawTextRead::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
