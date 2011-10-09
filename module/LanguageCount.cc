/**
 */
#include <config.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <string.h>
#include "robot_common.h"
#include "LanguageCount.h"
#include "TextResource.h"

using namespace std;

LanguageCount::LanguageCount(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;

	props = new ObjectProperties<LanguageCount>(this);
	props->Add("items", &LanguageCount::GetItems);
	props->Add("languageCount", &LanguageCount::GetLanguageCount);
}

LanguageCount::~LanguageCount() {
	char *lc = GetLanguageCount("languageCount");
	LOG_DEBUG(this, lc);
	free(lc);
	delete props;
}

char *LanguageCount::GetItems(const char *name) {
	return int2str(items);
}

char *LanguageCount::GetLanguageCount(const char *name) {
	string result;
	for (tr1::unordered_map<string, uint64_t>::iterator iter = count.begin(); iter != count.end(); ++iter) {
		char s[100];
		snprintf(s, sizeof(s), "%s%s:%"PRIu64, result.length() > 0 ? " " : "", iter->first.c_str(), iter->second);
		result.append(s);
	}
	return strdup(result.c_str());
}

bool LanguageCount::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	return true;
}

Resource *LanguageCount::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	string langs = tr->GetLanguage();
	vector<string> v;
	splitOnWs(v, langs);
	for (vector<string>::iterator iter = v.begin(); iter != v.end(); ++iter) {
		tr1::unordered_map<string, uint64_t>::iterator iter2 = count.find(*iter);
		if (iter2 != count.end()) {
			iter2->second++;
		} else {
			count[*iter] = 1;
		}
	}

	items++;

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new LanguageCount(objects, id, threadIndex);
}
