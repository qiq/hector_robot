/**
 * UpdateIndex module.
 */
#include <config.h>

#include <assert.h>
#include "IndexResource.h"
#include "MarkerResource.h"
#include "PageResource.h"
#include "UrlResource.h"
#include "UpdateIndex.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

UpdateIndex::UpdateIndex(ObjectRegistry *objects, const char *id, int threadIndex) : Module(objects, id, threadIndex) {
	items = 0;
	timeTick = DEFAULT_TIME_TICK;

	props = new ObjectProperties<UpdateIndex>(this);
	props->Add("items", &UpdateIndex::GetItems);
	props->Add("timeTick", &UpdateIndex::GetTimeTick, &UpdateIndex::SetTimeTick);

	markerRead = 0;

	indexResourceTypeId = -1;

	pool = new MemoryPool<Index, false>(1024);
}

UpdateIndex::~UpdateIndex() {
	delete pool;
	delete props;
}

char *UpdateIndex::GetItems(const char *name) {
	return int2str(items);
}

char *UpdateIndex::GetTimeTick(const char *name) {
	return int2str(timeTick);
}

void UpdateIndex::SetTimeTick(const char *name, const char *value) {
	timeTick = str2int(value);
}

bool UpdateIndex::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	indexResourceTypeId = Resource::GetRegistry()->NameToId("IndexResource");
	assert(indexResourceTypeId > 0);

	return true;
}

bool UpdateIndex::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources, int *processingResources) {
	if (expectingResources)
		*expectingResources = 1000;
	if (processingResources)
		*processingResources = 0;

	uint32_t currentTime = time(NULL);
	int resourcesProcessed = 0;
	while (inputResources->size() > 0) {
		Resource *r = inputResources->front();
		inputResources->pop();
		if (markerRead == 0) {
			if (UrlResource::IsInstance(r)) {
				UrlResource *ur = static_cast<UrlResource*>(r);
				SitePathMD5 md5(ur->GetSiteMD5(), ur->GetPathMD5());
				if (seen.find(md5) == seen.end()) {
					Index *idx = pool->Alloc();
					idx->lastModified = 0;
					idx->errorStatus = 0;
					idx->newLink = 1;
					idx->written = 0;
					seen[md5] = idx;
				}
			} else if (PageResource::IsInstance(r)) {
				PageResource *pr = static_cast<PageResource*>(r);
				SitePathMD5 md5(pr->GetSiteMD5(), pr->GetPathMD5());
				if (seen.find(md5) == seen.end()) {
					Index *idx = pool->Alloc();
					idx->lastModified = 0;
					idx->errorStatus = 0;
					idx->newLink = 0;
					idx->written = 0;
					seen[md5] = idx;
				}
			} else if (MarkerResource::IsInstance(r)) {
				markerRead = 1;
			}
		} else if (markerRead == 1) {
			if (IndexResource::IsInstance(r)) {
				IndexResource *ir = static_cast<IndexResource*>(r);
				SitePathMD5 md5(ir->GetSiteMD5(), ir->GetPathMD5());
				tr1::unordered_map<SitePathMD5, Index*, SitePathMD5_hash, SitePathMD5_equal>::iterator iter = seen.find(md5);
				if (iter != seen.end()) {
					Index *idx = iter->second;
					if (!idx->written) {
						// merge old index info with the new one
						// TODO
						idx->written = 1;
					}
				}
			} else if (MarkerResource::IsInstance(r)) {
				markerRead = 2;
				seenIterator = seen.begin();
			}
		}
		outputResources->push(r);

		// check timeout every 10000 SiteResources found
		if (++resourcesProcessed % 10000 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
			break;
	}

	if (markerRead == 2) {
		while (seenIterator != seen.end()) {
			if (!seenIterator->second->written) {
				// create new indexResource
				IndexResource *ir = static_cast<IndexResource*>(Resource::GetRegistry()->AcquireResource(indexResourceTypeId));
				ir->SetSiteMD5((*seenIterator).first.GetSiteMD5());
				ir->SetPathMD5((*seenIterator).first.GetPathMD5());
				// TODO: set somehow
				outputResources->push(ir);
			}
			// check timeout every 10000 SiteResources found
			if (++resourcesProcessed % 10000 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
				break;
			seenIterator++;
		}
	}

	return markerRead == 2 && seenIterator != seen.end() ? true : false;
}

bool UpdateIndex::SaveCheckpointSync(const char *path) {
	// TODO
	return true;
}

bool UpdateIndex::RestoreCheckpointSync(const char *path) {
	// TODO
	return true;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new UpdateIndex(objects, id, threadIndex);
}
