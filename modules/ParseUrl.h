/**
 * Parse URL into components (mainly hostname and others)
 * Requires WebResource to work on.
 */

#ifndef _MODULES_PARSE_URL_H_
#define _MODULES_PARSE_URL_H_

#include <config.h>

#include "Module.h"
#include "ObjectValues.h"

class ParseUrl : public Module {
public:
	ParseUrl(ObjectRegistry *objects, const char *id, int threadIndex);
	~ParseUrl();
	bool Init(vector<pair<string, string> > *params);
	Module::Type getType();
	Resource *ProcessSimple(Resource *resource);

private:
	int typeId;		// not accessible outside module

	int items;		// ObjectLock, items processed

	ObjectValues<ParseUrl> *values;

	char *getItems(const char *name);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	vector<string> *listNamesSync();
};

inline Module::Type ParseUrl::getType() {
	return SIMPLE;
}

inline char *ParseUrl::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool ParseUrl::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool ParseUrl::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline vector<string> *ParseUrl::listNamesSync() {
	return values->listNamesSync();
}

#endif
