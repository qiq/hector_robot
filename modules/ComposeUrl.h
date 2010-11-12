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
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	Resource *ProcessSimple(Resource *resource);

private:
	int typeId;		// not accessible outside module

	int items;		// ObjectLock
	bool clear;		// ObjectLock

	ObjectValues<ComposeUrl> *values;

	char *getItems(const char *name);
	char *getClear(const char *name);
	void setClear(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
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

inline bool ComposeUrl::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *ComposeUrl::listNamesSync() {
	return values->listNamesSync();
}

#endif
