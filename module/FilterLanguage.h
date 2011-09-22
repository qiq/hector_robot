/**
FilterLanguage.la, simple, native
FilterLanguage deletes documents or paragraphs that does not contain allowed
language.

Dependencies: none

Parameters:
items			r/o	Total items processed
allowedLanguages	r/w	List (space separated) of language codes (ISO639-1) that are allowed.
disallowedLanguages	r/w	List (space separated) of language codes (ISO639-1) that are not allowed.
*/

#ifndef _MODULES_DEDUPLICATE_H_
#define _MODULES_DEDUPLICATE_H_

#include <config.h>

#include <string>
#include <tr1/unordered_set>
#include "common.h"
#include "Module.h"
#include "NgramBloomFilter.h"
#include "ObjectProperties.h"

class FilterLanguage : public Module {
public:
	FilterLanguage(ObjectRegistry *objects, const char *id, int threadIndex);
	~FilterLanguage();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	char *allowedLanguages;
	char *disallowedLanguages;

	char *GetItems(const char *name);
	char *GetAllowedLanguages(const char *name);
        void SetAllowedLanguages(const char *name, const char *value);
	char *GetDisallowedLanguages(const char *name);
        void SetDisallowedLanguages(const char *name, const char *value);

	ObjectProperties<FilterLanguage> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::tr1::unordered_set<std::string> allowedLanguagesSet;
	std::tr1::unordered_set<std::string> disallowedLanguagesSet;
};

inline Module::Type FilterLanguage::GetType() {
	return SIMPLE;
}

inline char *FilterLanguage::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool FilterLanguage::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *FilterLanguage::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
