/**
 * UpdateUrls module.
 */
#include <config.h>

#include <assert.h>
#include "MarkerResource.h"
#include "PageResource.h"
#include "UrlResource.h"
#include "UpdateUrls.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

UpdateUrls::UpdateUrls(ObjectRegistry *objects, const char *id, int threadIndex) : Module(objects, id, threadIndex) {
	items = 0;
	timeTick = DEFAULT_TIME_TICK;

	props = new ObjectProperties<UpdateUrls>(this);
	props->Add("items", &UpdateUrls::GetItems);
	props->Add("timeTick", &UpdateUrls::GetTimeTick, &UpdateUrls::SetTimeTick);

	markerRead = false;
	urlResourceTypeId = -1;
}

UpdateUrls::~UpdateUrls() {
	delete props;
}

char *UpdateUrls::GetItems(const char *name) {
	return int2str(items);
}

char *UpdateUrls::GetTimeTick(const char *name) {
	return int2str(timeTick);
}

void UpdateUrls::SetTimeTick(const char *name, const char *value) {
	timeTick = str2int(value);
}

bool UpdateUrls::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	urlResourceTypeId = Resource::GetRegistry()->NameToId("UrlResource");
	assert(urlResourceTypeId > 0);

	return true;
}

bool UpdateUrls::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources, int *processingResources) {
	if (expectingResources)
		*expectingResources = 1000;
	if (processingResources)
		*processingResources = 0;

	uint32_t currentTime = time(NULL);
	int resourcesProcessed = 0;
	while (inputResources->size() > 0) {
		Resource *r = inputResources->front();
		inputResources->pop();
		if (UrlResource::IsInstance(r)) {
			UrlResource *ur = static_cast<UrlResource*>(r);
			SitePathMD5 md5(ur->GetSiteMD5(), ur->GetPathMD5());
			if (seen.find(md5) == seen.end()) {
				// updated UrlResource, copy it to the output queue
				if (!markerRead)
					seen.insert(md5);
			} else {
				// updated and already processed: delete
				r->SetFlag(Resource::DELETED);
			}
		} else if (PageResource::IsInstance(r)) {
			PageResource *pr = static_cast<PageResource*>(r);
			SitePathMD5 md5(pr->GetSiteMD5(), pr->GetPathMD5());
			if (seen.find(md5) == seen.end()) {
				UrlResource *ur = static_cast<UrlResource*>(Resource::GetRegistry()->AcquireResource(urlResourceTypeId));
				ur->SetUrl(pr->GetUrl());
				ur->SetSiteMD5(pr->GetSiteMD5());
				ur->SetPathMD5(pr->GetPathMD5());
				outputResources->push(ur);
				if (!markerRead)
					seen.insert(md5);
			}
		} else if (MarkerResource::IsInstance(r)) {
			markerRead = true;
		}
		outputResources->push(r);

		// check timeout every 10000 SiteResources found
		if (++resourcesProcessed % 10000 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
			break;
	}

	return false;
}

bool UpdateUrls::SaveCheckpointSync(const char *path) {
	// TODO
	return true;
}

bool UpdateUrls::RestoreCheckpointSync(const char *path) {
	// TODO
	return true;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new UpdateUrls(objects, id, threadIndex);
}
