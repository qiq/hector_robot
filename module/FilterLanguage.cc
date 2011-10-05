/**
 */
#include <config.h>

#include <string.h>
#include "robot_common.h"
#include "FilterLanguage.h"
#include "TextResource.h"

using namespace std;

FilterLanguage::FilterLanguage(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	allowedLanguages = NULL;
	disallowedLanguages = NULL;

	props = new ObjectProperties<FilterLanguage>(this);
	props->Add("items", &FilterLanguage::GetItems);
	props->Add("allowedLanguages", &FilterLanguage::GetAllowedLanguages, &FilterLanguage::SetAllowedLanguages);
	props->Add("disallowedLanguages", &FilterLanguage::GetDisallowedLanguages, &FilterLanguage::SetDisallowedLanguages);
}

FilterLanguage::~FilterLanguage() {
	free(allowedLanguages);
	free(disallowedLanguages);
	delete props;
}

char *FilterLanguage::GetItems(const char *name) {
	return int2str(items);
}

char *FilterLanguage::GetAllowedLanguages(const char *name) {
	return strdup(allowedLanguages);
}

void FilterLanguage::SetAllowedLanguages(const char *name, const char *value) {
	free(allowedLanguages);
	allowedLanguages = strdup(value);
	string s(value);
	vector<string> v;
	splitOnWs(v, s);
	allowedLanguagesSet.clear();
	for (vector<string>::iterator iter = v.begin(); iter != v.end(); ++iter)
		allowedLanguagesSet.insert(*iter);
}

char *FilterLanguage::GetDisallowedLanguages(const char *name) {
	return strdup(allowedLanguages);
}

void FilterLanguage::SetDisallowedLanguages(const char *name, const char *value) {
	free(disallowedLanguages);
	disallowedLanguages = strdup(value);
	string s(value);
	vector<string> v;
	splitOnWs(v, s);
	disallowedLanguagesSet.clear();
	for (vector<string>::iterator iter = v.begin(); iter != v.end(); ++iter)
		disallowedLanguagesSet.insert(*iter);
}

bool FilterLanguage::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	if (allowedLanguagesSet.size() == 0 && disallowedLanguagesSet.size() == 0) {
		LOG_ERROR(this, "allowedLanguages or disallowedLanguages must be defined");
		return false;
	}

	return true;
}

Resource *FilterLanguage::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int nWords = tr->GetFormCount();

	string langs = tr->GetLanguage();
	if (langs.empty()) {
		LOG_DEBUG_R(this, tr, "Language not specified");
		return resource;
	}

	// get language codes
	vector<string> v;
	splitOnWs(v, langs);
	if (v.size() == 1) {
		// whole document
		if ((allowedLanguagesSet.size() > 0 && allowedLanguagesSet.find(v[0]) == allowedLanguagesSet.end()) || (disallowedLanguagesSet.size() > 0 && disallowedLanguagesSet.find(v[0]) != disallowedLanguagesSet.end())) {
			LOG_DEBUG_R(this, tr, "Not allowed language (" << v[0] << "), document deleted: " << tr->GetTextId());
			tr->SetFlag(Resource::DELETED);
		}
	} else {
		// filter paragraphs (some paragraps may be deleted,
		// others survive)
		vector<string> languages;
		int totalDeleted = 0;
		vector<bool> deleted(nWords);
		int idx = 0;
		int vi = 0;
		int size = 0;
		for (int i = 0; i < nWords; i++) {
			if (tr->GetFlags(i) & TextResource::TOKEN_PARAGRAPH_START) {
				if (size > 0) {
					if ((allowedLanguagesSet.size() > 0 && allowedLanguagesSet.find(v[vi]) == allowedLanguagesSet.end()) || (disallowedLanguagesSet.size() > 0 && disallowedLanguagesSet.find(v[vi]) != disallowedLanguagesSet.end())) {
						for (int j = 0; j < size; j++)
							deleted[idx++] = true;
						totalDeleted += size;
					} else {
						for (int j = 0; j < size; j++)
							deleted[idx++] = false;
						languages.push_back(v[vi]);
					}
					size = 0;
					vi++;
				}
			}
			size++;
		}
		if (size > 0) {
			if ((allowedLanguagesSet.size() > 0 && allowedLanguagesSet.find(v[vi]) == allowedLanguagesSet.end()) || (disallowedLanguagesSet.size() > 0 && disallowedLanguagesSet.find(v[vi]) != disallowedLanguagesSet.end())) {
				for (int i = 0; i < size; i++)
					deleted[idx++] = true;
				totalDeleted += size;
			} else {
				for (int i = 0; i < size; i++)
					deleted[idx++] = false;
				languages.push_back(v[vi]);
			}
		}

		// delete duplicated contents
		if (totalDeleted < nWords) {
			tr->DeleteWords(deleted);
			tr->SetLanguage(join(' ', languages));
		} else {
			// all paragraphs were marked duplicate
			LOG_DEBUG_R(this, tr, "Not allowed language (" << v[0] << "), document deleted: " << tr->GetTextId());
			tr->SetFlag(Resource::DELETED);
		}
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new FilterLanguage(objects, id, threadIndex);
}
