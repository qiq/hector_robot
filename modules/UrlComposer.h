/**
 * Compose URL from components. Optionally clean components.
 * Requires WebResource to work on.
 */

#ifndef _MODULES_URL_COMPOSE_H_
#define _MODULES_URL_COMPOSE_H_

#include <config.h>

#include "Module.h"
#include "ObjectValues.h"

class UrlComposer : public Module {
public:
	UrlComposer(ObjectRegistry *objects, const char *id, int threadIndex);
	~UrlComposer();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	Resource *ProcessSimple(Resource *resource);

private:
	int typeId;		// not accessible outside module

	int items;		// ObjectLock
	bool clear;		// ObjectLock

	ObjectValues<UrlComposer> *values;

	char *getItems(const char *name);
	char *getClear(const char *name);
	void setClear(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
};

inline Module::Type UrlComposer::getType() {
	return SIMPLE;
}

inline char *UrlComposer::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool UrlComposer::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool UrlComposer::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *UrlComposer::listNamesSync() {
	return values->listNamesSync();
}

#endif
