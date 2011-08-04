/**
UpdateIndex.la, multi, native
Update index stream with changed/new urls. There are three stages (separated by
a marker resource):
1) expect page and url resource, record info from them
2) read index resources and merge it with the stored data
3) append index resources for new urls
UrlResources and PageResources are passed untouched.

Dependencies: none

Parameters:
items			r/o	Total items processed
timeTick		r/w	Max time to spend in ProcessMulti()
*/

#ifndef _MODULES_UPDATE_INDEX_H_
#define _MODULES_UPDATE_INDEX_H_

#include <config.h>

#include <tr1/unordered_set>
#include "MemoryPool.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "SitePathMD5.h"

class UpdateIndex : public Module {
	struct Index {
		uint32_t lastModified;
		uint32_t errorStatus;
		unsigned newLink:1;
		unsigned written:1;
	};

public:
	UpdateIndex(ObjectRegistry *objects, const char *id, int threadIndex);
	~UpdateIndex();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	bool ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources, int *processingResources);
	bool SaveCheckpointSync(const char *path);
	bool RestoreCheckpointSync(const char *path);

private:
	// properties
	int items;
	int timeTick;

	char *GetItems(const char *name);
	char *GetTimeTick(const char *name);
	void SetTimeTick(const char *name, const char *value);

	ObjectProperties<UpdateIndex> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int markerRead;

	int indexResourceTypeId;

	MemoryPool<Index, false> *pool;

	// resources that we have already processed
	std::tr1::unordered_map<SitePathMD5, Index*, SitePathMD5_hash, SitePathMD5_equal> seen;
	std::tr1::unordered_map<SitePathMD5, Index*, SitePathMD5_hash, SitePathMD5_equal>::iterator seenIterator;
};

inline Module::Type UpdateIndex::GetType() {
	return MULTI;
}

inline char *UpdateIndex::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool UpdateIndex::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *UpdateIndex::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
