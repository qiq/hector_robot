/**
Normalize.la, simple, native
Normalize text or tokens in a TextResource (convert UTF-8 to NFC) and possibly
other transformations.

Dependencies: icu

Parameters:
items			r/o	Total items processed
*/

#ifndef _MODULES_NORMALIZE_H_
#define _MODULES_NORMALIZE_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class Normalize : public Module {
public:
	Normalize(ObjectRegistry *objects, const char *id, int threadIndex);
	~Normalize();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed

	char *GetItems(const char *name);

	ObjectProperties<Normalize> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();
};

inline Module::Type Normalize::GetType() {
	return SIMPLE;
}

inline char *Normalize::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool Normalize::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *Normalize::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
