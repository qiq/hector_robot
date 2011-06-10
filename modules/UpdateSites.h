/**
UpdateSites.la, multi, native
Update site with changed sites. Expects changed-sites, then original sites on
the input. Output: site resources to be saved in the resource file.

In fact it only filters out updated (and saved) sites from the second bunch.

Dependencies: none

Parameters:
items			r/o	Total items processed
timeTick		r/w	Max time to spend in ProcessMulti()
*/

#ifndef _MODULES_UPDATE_SITES_H_
#define _MODULES_UPDATE_SITES_H_

#include <config.h>

#include <tr1/unordered_map>
#include "Module.h"
#include "ObjectProperties.h"

class UpdateSites : public Module {
public:
	UpdateSites(ObjectRegistry *objects, const char *id, int threadIndex);
	~UpdateSites();
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

	ObjectProperties<UpdateSites> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	bool markerRead;

	// site resources that we have already processed
	std::tr1::unordered_set<uint64_t> seen;
};

inline Module::Type UpdateSites::GetType() {
	return MULTI;
}

inline char *UpdateSites::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool UpdateSites::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *UpdateSites::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
