/**
ExtractUrl.la, multi, native
Extract URLs from PageResource using flex.

Dependencies: none

Parameters:
items		r/o	Total items processed
newUrlStatus	r/w	Status to be set for new-url PageResources
imageLinks	r/w	Also extract image links

Status:
original WR: untouched
new WR: status is set according to newUrlStatus parameter. Default is 0.
*/

#ifndef _MODULES_EXTRACT_URL_H_
#define _MODULES_EXTRACT_URL_H_

#include <config.h>

#include <queue>
#include <string>
#include <tr1/unordered_set>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "ExtractUrlLexer.h"
#include "PageResource.h"
#include "SitePathMD5.h"

class GURL;

class ExtractUrl : public Module {
public:
	ExtractUrl(ObjectRegistry *objects, const char *id, int threadIndex);
	~ExtractUrl();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	bool ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources, int *processingResources);

private:
	int items;			// items processed
	int newUrlStatus;		// status to be set for new-url PageResources
	bool imageLinks;		// also extract image links (e.g. <img src=""/>)
	std::string allowedSchemes;	// allowed schemes, separated by space, by default "http"

	char *GetItems(const char *name);
	char *GetNewUrlStatus(const char *name);
	void SetNewUrlStatus(const char *name, const char *value);
	char *GetImageLinks(const char *name);
	void SetImageLinks(const char *name, const char *value);
	char *GetAllowedSchemes(const char *name);
	void SetAllowedSchemes(const char *name, const char *value);

	ObjectProperties<ExtractUrl> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	void AppendUrl(const char *url, GURL *base, std::queue<Resource*> *outputResources);

	std::tr1::unordered_set<std::string> allowedSchemesSet;
	std::tr1::unordered_set<SitePathMD5, SitePathMD5_hash, SitePathMD5_equal> seen;
	// for flex
	void *scanner;
	scanner_state state;

	int urlResourceTypeId;
};

inline Module::Type ExtractUrl::GetType() {
	return MULTI;
}

inline char *ExtractUrl::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool ExtractUrl::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *ExtractUrl::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
