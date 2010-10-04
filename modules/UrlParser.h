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
	Resource *Process(Resource *resource);

private:
	int typeId;		// to create TestResource

	int items;		// guarded by ObjectLock

	ObjectValues<ParseUrl> *values;

	char *getItems(const char *name);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
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

inline vector<string> *ParseUrl::listNamesSync() {
	return values->listNamesSync();
}

#endif
