/**
 * Extract URLs from WebResource using flex.
 */

#ifndef _MODULES_URL_EXTRACTOR_H_
#define _MODULES_URL_EXTRACTOR_H_

#include <config.h>

#include <queue>
#include <string>
#include <tr1/unordered_map>
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
	int ProcessMulti(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources);
	int ProcessingResources();

private:
	int items;		// ObjectLock, items processed
	int newUrlStatus;	// ObjectLock, status to be set for new-url WebResources
	ObjectValues<UrlExtractor> *values;
	char *getItems(const char *name);
	char *getNewUrlStatus(const char *name);
	void setNewUrlStatus(const char *name, const char *value);

	// for flex
	void *scanner;
	scanner_state state;

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
};

inline Module::Type UrlExtractor::getType() {
	return MULTI;
}

inline char *UrlExtractor::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool UrlExtractor::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool UrlExtractor::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *UrlExtractor::listNamesSync() {
	return values->listNamesSync();
}

#endif
