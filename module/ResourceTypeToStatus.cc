/**
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include "robot_common.h"
#include "ResourceTypeToStatus.h"

using namespace std;

ResourceTypeToStatus::ResourceTypeToStatus(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;

	props = new ObjectProperties<ResourceTypeToStatus>(this);
	props->Add("items", &ResourceTypeToStatus::GetItems);
}

ResourceTypeToStatus::~ResourceTypeToStatus() {
	delete props;
}

char *ResourceTypeToStatus::GetItems(const char *name) {
	return int2str(items);
}

bool ResourceTypeToStatus::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	return true;
}

Resource *ResourceTypeToStatus::ProcessSimpleSync(Resource *resource) {
	resource->SetStatus(resource->GetTypeId());
	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new ResourceTypeToStatus(objects, id, threadIndex);
}
