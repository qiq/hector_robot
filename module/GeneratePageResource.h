/**
GeneratePageResource.la, input, native
Generate PageResource with random content. Mainly for testing purposes.

Dependencies: none

Parameters:
items		r/o	Total items processed
maxItems	init	Number of items to load
idPrefix	r/w	Prefix to be used in PageResource URL

Status:
0 (PageResource default)
*/

#ifndef _GENERATE_PAGE_RESOURCE_H_
#define _GENERATE_PAGE_RESOURCE_H_

#include <config.h>

#include <string>
#include <vector>
#include "Module.h"
#include "ObjectProperties.h"

class GeneratePageResource : public Module {
public:
	GeneratePageResource(ObjectRegistry *objects, const char *id, int threadIndex);
	~GeneratePageResource();
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

	ObjectProperties<GeneratePageResource> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int typeId;		// type of resource to generate (PageResource)
};

inline Module::Type GeneratePageResource::GetType() {
	return INPUT;
}

inline char *GeneratePageResource::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool GeneratePageResource::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *GeneratePageResource::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
