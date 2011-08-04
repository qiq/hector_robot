/**
 * UpdateSites module.
 */
#include <config.h>

#include "MarkerResource.h"
#include "SiteResource.h"
#include "UpdateSites.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

UpdateSites::UpdateSites(ObjectRegistry *objects, const char *id, int threadIndex) : Module(objects, id, threadIndex) {
	items = 0;
	timeTick = DEFAULT_TIME_TICK;

	props = new ObjectProperties<UpdateSites>(this);
	props->Add("items", &UpdateSites::GetItems);
	props->Add("timeTick", &UpdateSites::GetTimeTick, &UpdateSites::SetTimeTick);

	markerRead = false;
}

UpdateSites::~UpdateSites() {
	delete props;
}

char *UpdateSites::GetItems(const char *name) {
	return int2str(items);
}

char *UpdateSites::GetTimeTick(const char *name) {
	return int2str(timeTick);
}

void UpdateSites::SetTimeTick(const char *name, const char *value) {
	timeTick = str2int(value);
}

bool UpdateSites::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	return true;
}

bool UpdateSites::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources, int *processingResources) {
	if (expectingResources)
		*expectingResources = 1000;
	if (processingResources)
		*processingResources = 0;

	uint32_t currentTime = time(NULL);
	int resourcesProcessed = 0;
	while (inputResources->size() > 0) {
		Resource *r = inputResources->front();
		inputResources->pop();
		if (SiteResource::IsInstance(r)) {
			SiteResource *sr = static_cast<SiteResource*>(r);
			if (seen.find(sr->GetSiteMD5()) == seen.end()) {
				// not seen before: just copy it to the output queue
				if (!markerRead)
					seen.insert(sr->GetSiteMD5());
			} else {
				// updated and already processed: delete
				r->SetFlag(Resource::DELETED);
			}
			outputResources->push(r);
			items++;
		} else if (MarkerResource::IsInstance(r)) {
			markerRead = true;
		} else {
			outputResources->push(r);
		}

		// check timeout every 10000 SiteResources found
		if (++resourcesProcessed % 10000 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
			break;
	}

	return false;
}

bool UpdateSites::SaveCheckpointSync(const char *path) {
	// TODO
	return true;
}

bool UpdateSites::RestoreCheckpointSync(const char *path) {
	// TODO
	return true;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new UpdateSites(objects, id, threadIndex);
}
