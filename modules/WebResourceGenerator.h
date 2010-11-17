/**
 * Generate WebResource with random content. Meant to be used as a test.
 */

#ifndef _WEB_RESOURCE_GENERATOR_H_
#define _WEB_RESOURCE_GENERATOR_H_

#include <config.h>

#include <string>
#include <vector>
#include "Module.h"
#include "ObjectValues.h"

class WebResourceGenerator : public Module {
public:
	WebResourceGenerator(ObjectRegistry *objects, const char *id, int threadIndex);
	~WebResourceGenerator();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	Resource *ProcessInput(bool sleep);

private:
	int typeId;		// not accessible outside module

	int items;		// ObjectLock
	int maxItems;		// initOnly
	char *idPrefix;		// ObjectLock

	ObjectValues<WebResourceGenerator> *values;

	char *getItems(const char *name);
	char *getMaxItems(const char *name);
	void setMaxItems(const char *name, const char *value);
	char *getIdPrefix(const char *name);
	void setIdPrefix(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
};

inline Module::Type WebResourceGenerator::getType() {
	return INPUT;
}

inline char *WebResourceGenerator::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool WebResourceGenerator::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool WebResourceGenerator::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *WebResourceGenerator::listNamesSync() {
	return values->listNamesSync();
}

#endif
