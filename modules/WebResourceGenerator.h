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
#include "ObjectProperties.h"

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

	ObjectProperties<WebResourceGenerator> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int typeId;		// type of resource to generate (WebResource)
};

inline Module::Type WebResourceGenerator::GetType() {
	return INPUT;
}

inline char *WebResourceGenerator::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool WebResourceGenerator::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *WebResourceGenerator::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
