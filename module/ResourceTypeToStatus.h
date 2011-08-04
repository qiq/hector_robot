/**
ResourceTypeToStatus.la, simple, native
Set status to the TypeId of a Resource.

Dependencies: none

Parameters:
items		r/o	Total items processed
*/

#ifndef _MODULES_RESOURCE_TYPE_TO_STATUS_H_
#define _MODULES_RESOURCE_TYPE_TO_STATUS_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class ResourceTypeToStatus : public Module {
public:
	ResourceTypeToStatus(ObjectRegistry *objects, const char *id, int threadIndex);
	~ResourceTypeToStatus();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed

	char *GetItems(const char *name);

	ObjectProperties<ResourceTypeToStatus> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();
};

inline Module::Type ResourceTypeToStatus::GetType() {
	return SIMPLE;
}

inline char *ResourceTypeToStatus::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool ResourceTypeToStatus::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *ResourceTypeToStatus::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
