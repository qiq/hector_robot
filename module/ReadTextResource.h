/**
ReadTextResource.la, input, native
Read TextResource in vertical or horizontal form.

Dependencies: none

Parameters:
items		r/o	Total items processed.
horizontal	r/w	Whether to print horizontal (or horizontal)
			tab-separated-fields format.
filename	init	File to read.
*/

#ifndef _MODULES_READ_TEXT_RESOURCE_H_
#define _MODULES_READ_TEXT_RESOURCE_H_

#include <config.h>

#include <fstream>
#include <string>
#include <vector>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "TextResource.h"

class ReadTextResource : public Module {
public:
	ReadTextResource(ObjectRegistry *objects, const char *id, int threadIndex);
	~ReadTextResource();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessInputSync(bool sleep);

private:
	int items;		// ObjectLock, items processed
	bool horizontal;
	char *filename;

	char *GetItems(const char *name);
	char *GetHorizontal(const char *name);
        void SetHorizontal(const char *name, const char *value);
	char *GetFilename(const char *name);
	void SetFilename(const char *name, const char *value);

	ObjectProperties<ReadTextResource> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int textResourceTypeId;
	std::ifstream *ifs;
	std::string docId;
};

inline Module::Type ReadTextResource::GetType() {
	return Module::INPUT;
}

inline char *ReadTextResource::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool ReadTextResource::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *ReadTextResource::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
