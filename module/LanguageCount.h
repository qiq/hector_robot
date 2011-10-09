/**
LanguageCount.la, simple, native
LanguageCount counts number of languages in TextResources.

Dependencies: none

Parameters:
items			r/o	Total items processed
*/

#ifndef _MODULES_WORD_COUNT_H_
#define _MODULES_WORD_COUNT_H_

#include <config.h>

#include <string>
#include <tr1/unordered_map>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class LanguageCount : public Module {
public:
	LanguageCount(ObjectRegistry *objects, const char *id, int threadIndex);
	~LanguageCount();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;

	char *GetItems(const char *name);
	char *GetLanguageCount(const char *name);

	ObjectProperties<LanguageCount> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::tr1::unordered_map<std::string, uint64_t> count;
};

inline Module::Type LanguageCount::GetType() {
	return SIMPLE;
}

inline char *LanguageCount::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool LanguageCount::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *LanguageCount::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
