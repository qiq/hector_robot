/**
PageResourceGenerator.la, input, native
Generate PageResource with random content. Mainly for testing purposes.

Dependencies: none

Parameters:
items		r/o	Total items processed
maxItems	init	Number of items to load
idPrefix	r/w	Prefix to be used in PageResource URL

Status:
0 (PageResource default)
*/

#ifndef _PAGE_RESOURCE_GENERATOR_H_
#define _PAGE_RESOURCE_GENERATOR_H_

#include <config.h>

#include <string>
#include <vector>
#include "Module.h"
#include "ObjectProperties.h"

class PageResourceGenerator : public Module {
public:
	PageResourceGenerator(ObjectRegistry *objects, const char *id, int threadIndex);
	~PageResourceGenerator();
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

	ObjectProperties<PageResourceGenerator> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int typeId;		// type of resource to generate (PageResource)
};

inline Module::Type PageResourceGenerator::GetType() {
	return INPUT;
}

inline char *PageResourceGenerator::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool PageResourceGenerator::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *PageResourceGenerator::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
