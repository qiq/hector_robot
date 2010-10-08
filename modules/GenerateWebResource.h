/**
 * Generate WebResource with random content. Meant to be used as a test.
 */

#ifndef _GENERATE_WEB_RESOURCE_H_
#define _GENERATE_WEB_RESOURCE_H_

#include <config.h>

#include "Module.h"
#include "ObjectValues.h"

class GenerateWebResource : public Module {
public:
	GenerateWebResource(ObjectRegistry *objects, const char *id, int threadIndex);
	~GenerateWebResource();
	bool Init(vector<pair<string, string> > *params);
	Module::Type getType();
	Resource *ProcessInput(bool sleep);

private:
	int typeId;		// to create TestResource

	int items;		// guarded by ObjectLock
	int maxItems;		// guarded by ObjectLock
	char *idPrefix;		// guarded by ObjectLock

	ObjectValues<GenerateWebResource> *values;

	char *getItems(const char *name);
	char *getMaxItems(const char *name);
	void setMaxItems(const char *name, const char *value);
	char *getIdPrefix(const char *name);
	void setIdPrefix(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	vector<string> *listNamesSync();
};

inline Module::Type GenerateWebResource::getType() {
	return INPUT;
}

inline char *GenerateWebResource::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool GenerateWebResource::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline vector<string> *GenerateWebResource::listNamesSync() {
	return values->listNamesSync();
}

#endif
