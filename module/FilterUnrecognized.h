/**
FilterUnrecognized.la, simple, native
FilterUnrecognized deletes documents or paragraphs that does not contain allowed
language.

Dependencies: none

Parameters:
items			r/o	Total items processed
maxUnrecognizedRatio	r/w	More unrecognized words means to discard the paragraph.
*/

#ifndef _MODULES_FILTER_UNRECOGNIZED_H_
#define _MODULES_FILTER_UNRECOGNIZED_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class FilterUnrecognized : public Module {
public:
	FilterUnrecognized(ObjectRegistry *objects, const char *id, int threadIndex);
	~FilterUnrecognized();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	double maxUnrecognizedRatio;

	char *GetItems(const char *name);
	char *GetMaxUnrecognizedRatio(const char *name);
        void SetMaxUnrecognizedRatio(const char *name, const char *value);

	ObjectProperties<FilterUnrecognized> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();
};

inline Module::Type FilterUnrecognized::GetType() {
	return SIMPLE;
}

inline char *FilterUnrecognized::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool FilterUnrecognized::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *FilterUnrecognized::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
