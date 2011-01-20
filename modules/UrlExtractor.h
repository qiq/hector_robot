/**
UrlExtractor.la, multi, native
Extract URLs from WebResource using flex.

Dependencies: none

Parameters:
items		r/o	Total items processed
newUrlStatus	r/w	Status to be set for new-url WebResources
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
#include "ObjectValues.h"
#include "UrlExtractorLexer.h"
#include "WebResource.h"

class UrlExtractor : public Module {
public:
	UrlExtractor(ObjectRegistry *objects, const char *id, int threadIndex);
	~UrlExtractor();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	int ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources);

private:
	int items;		// ObjectLock, items processed
	int newUrlStatus;	// ObjectLock, status to be set for new-url WebResources
	bool imageLinks;	// ObjectLock, also extract image links (e.g. <img src=""/>)
	std::string allowedSchemes;	// ObjectLock, allowed schemes, separated by space, by default "http"
	std::tr1::unordered_set<std::string> allowedSchemesSet;

	char *getItems(const char *name);
	char *getNewUrlStatus(const char *name);
	void setNewUrlStatus(const char *name, const char *value);
	char *getImageLinks(const char *name);
	void setImageLinks(const char *name, const char *value);
	char *getAllowedSchemes(const char *name);
	void setAllowedSchemes(const char *name, const char *value);

	ObjectValues<UrlExtractor> *values;
	char *GetValueSync(const char *name);
	bool SetValueSync(const char *name, const char *value);
	std::vector<std::string> *ListNamesSync();

	std::tr1::unordered_set<std::string> urls;
	// for flex
	void *scanner;
	scanner_state state;
};

inline Module::Type UrlExtractor::getType() {
	return MULTI;
}

inline char *UrlExtractor::GetValueSync(const char *name) {
	return values->GetValue(name);
}

inline bool UrlExtractor::SetValueSync(const char *name, const char *value) {
	return values->SetValue(name, value);
}

inline std::vector<std::string> *UrlExtractor::ListNamesSync() {
	return values->ListNames();
}

#endif
