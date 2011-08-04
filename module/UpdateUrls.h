/**
UpdateUrls.la, multi, native
Update urls stream with changed urls. Expects changed-urls and pages, then
mark, then original urls on the input. Output: url resources to be saved in the
resource file. UrlResources are possibly deleted (duplicates), PageResources
are passed untouched.

Dependencies: none

Parameters:
items			r/o	Total items processed
timeTick		r/w	Max time to spend in ProcessMulti()
*/

#ifndef _MODULES_UPDATE_URLS_H_
#define _MODULES_UPDATE_URLS_H_

#include <config.h>

#include <tr1/unordered_map>
#include "Module.h"
#include "ObjectProperties.h"
#include "SitePathMD5.h"

class UpdateUrls : public Module {
public:
	UpdateUrls(ObjectRegistry *objects, const char *id, int threadIndex);
	~UpdateUrls();
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

	ObjectProperties<UpdateUrls> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	bool markerRead;

	int urlResourceTypeId;

	// site resources that we have already processed
	std::tr1::unordered_set<SitePathMD5, SitePathMD5_hash, SitePathMD5_equal> seen;
};

inline Module::Type UpdateUrls::GetType() {
	return MULTI;
}

inline char *UpdateUrls::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool UpdateUrls::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *UpdateUrls::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
