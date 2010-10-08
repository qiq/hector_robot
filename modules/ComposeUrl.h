/**
 * Compose URL from components. Optionally clean components.
 * Requires WebResource to work on.
 */

#ifndef _MODULES_COMPOSE_URL_H_
#define _MODULES_COMPOSE_URL_H_

#include <config.h>

#include "Module.h"
#include "ObjectValues.h"

class ComposeUrl : public Module {
public:
	ComposeUrl(ObjectRegistry *objects, const char *id, int threadIndex);
	~ComposeUrl();
	bool Init(vector<pair<string, string> > *params);
	Module::Type getType();
	Resource *Process(Resource *resource);

private:
	int typeId;		// to create TestResource

	int items;		// guarded by ObjectLock
	bool clear;		// guarded by ObjectLock

	ObjectValues<ComposeUrl> *values;

	char *getItems(const char *name);
	char *getClear(const char *name);
	void setClear(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	vector<string> *listNamesSync();
};

inline Module::Type ComposeUrl::getType() {
	return SIMPLE;
}

inline char *ComposeUrl::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool ComposeUrl::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline vector<string> *ComposeUrl::listNamesSync() {
	return values->listNamesSync();
}

#endif
