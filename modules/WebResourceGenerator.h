/**
WebResourceGenerator.la, input, native
Generate WebResource with random content. Mainly for testing purposes.

Dependencies: none

Parameters:
items		r/o	Total items processed
maxItems	init	Number of items to load
idPrefix	r/w	Prefix to be used in WebResource URL

Status:
0 (WebResource default)
*/

#ifndef _WEB_RESOURCE_GENERATOR_H_
#define _WEB_RESOURCE_GENERATOR_H_

#include <config.h>

#include <string>
#include <vector>
#include "Module.h"
#include "ObjectValues.h"

class WebResourceGenerator : public Module {
public:
	WebResourceGenerator(ObjectRegistry *objects, const char *id, int threadIndex);
	~WebResourceGenerator();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessInputSync(bool sleep);

private:
	int items;		// ObjectLock
	int maxItems;		// initOnly
	char *idPrefix;		// ObjectLock

	char *GetItems(const char *name);
	char *GetMaxItems(const char *name);
	void SetMaxItems(const char *name, const char *value);
	char *GetIdPrefix(const char *name);
	void SetIdPrefix(const char *name, const char *value);

	ObjectValues<WebResourceGenerator> *values;
	char *GetValueSync(const char *name);
	bool SetValueSync(const char *name, const char *value);
	std::vector<std::string> *ListNamesSync();

	int typeId;		// type of resource to generate (WebResource)
};

inline Module::Type WebResourceGenerator::GetType() {
	return INPUT;
}

inline char *WebResourceGenerator::GetValueSync(const char *name) {
	return values->GetValue(name);
}

inline bool WebResourceGenerator::SetValueSync(const char *name, const char *value) {
	return values->SetValue(name, value);
}

inline std::vector<std::string> *WebResourceGenerator::ListNamesSync() {
	return values->ListNames();
}

#endif
