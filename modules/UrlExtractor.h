/**
UrlExtractor.la, multi, native
Extract URLs from PageResource using flex.

Dependencies: none

Parameters:
items		r/o	Total items processed
newUrlStatus	r/w	Status to be set for new-url PageResources
imageLinks	r/w	Also extract image links

Status:
original WR: untouched
new WR: status is set according to newUrlStatus parameter. Default is 2.
*/

#ifndef _MODULES_URL_EXTRACTOR_H_
#define _MODULES_URL_EXTRACTOR_H_

#include <config.h>

#include <queue>
#include <string>
#include <tr1/unordered_set>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "UrlExtractorLexer.h"
#include "PageResource.h"

class UrlExtractor : public Module {
public:
	UrlExtractor(ObjectRegistry *objects, const char *id, int threadIndex);
	~UrlExtractor();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	int ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources);

private:
	int items;		// ObjectLock, items processed
	int newUrlStatus;	// ObjectLock, status to be set for new-url PageResources
	bool imageLinks;	// ObjectLock, also extract image links (e.g. <img src=""/>)
	std::string allowedSchemes;	// ObjectLock, allowed schemes, separated by space, by default "http"
	std::tr1::unordered_set<std::string> allowedSchemesSet;

	char *GetItems(const char *name);
	char *GetNewUrlStatus(const char *name);
	void SetNewUrlStatus(const char *name, const char *value);
	char *GetImageLinks(const char *name);
	void SetImageLinks(const char *name, const char *value);
	char *GetAllowedSchemes(const char *name);
	void SetAllowedSchemes(const char *name, const char *value);

	ObjectProperties<UrlExtractor> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::tr1::unordered_set<std::string> urls;
	// for flex
	void *scanner;
	scanner_state state;

	int pageResourceTypeId;	// PageResource typeId
};

inline Module::Type UrlExtractor::GetType() {
	return MULTI;
}

inline char *UrlExtractor::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool UrlExtractor::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *UrlExtractor::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
