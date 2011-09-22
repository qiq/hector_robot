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
		if (allowedLanguagesSet.find(v[0]) == allowedLanguagesSet.end() && disallowedLanguagesSet.find(v[0]) != disallowedLanguagesSet.end()) {
			LOG_DEBUG_R(this, tr, "Not allowed language (" << v[0] << "), document deleted: " << tr->GetTextId());
			tr->SetFlag(Resource::DELETED);
		}
	} else {
		// filter paragraphs (some paragraps may be deleted,
		// others survive)

		int start = 0;
		int totalDeleted = 0;
		vector<unsigned> deleted;	// offset + length of paragragraphs to be deleted
		int vi = 0;
		int size = 0;
		for (int i = 0; i < nWords; i++) {
			if (tr->GetFlags(i) & TextResource::TOKEN_PARAGRAPH_START) {
				if (size > 0) {
					if (allowedLanguagesSet.find(v[vi]) == allowedLanguagesSet.end() && disallowedLanguagesSet.find(v[vi]) != disallowedLanguagesSet.end()) {
						deleted.push_back(start);
						deleted.push_back(size);
						totalDeleted += size;
					}
					size = 0;
				}
				start = i;
				vi++;
			}
			size++;
		}
		if (size > 0) {
			if (allowedLanguagesSet.find(v[vi]) == allowedLanguagesSet.end() && disallowedLanguagesSet.find(v[vi]) != disallowedLanguagesSet.end()) {
				deleted.push_back(start);
				deleted.push_back(size);
				totalDeleted += size;
			}
		}

		// delete duplicated contents
		if (totalDeleted == nWords) {
			// all paragraphs were marked duplicate
			LOG_DEBUG_R(this, tr, "Not allowed language (" << v[0] << "), document deleted: " << tr->GetTextId());
			tr->SetFlag(Resource::DELETED);
		} else if (totalDeleted > 0) {
			// copy contents of TextResource with deleted paragraphs
			vector<int> flags;
			vector<string> forms;
			vector<string> lemmas;
			vector<string> posTags;
			vector<int> heads;
			vector<string> depRels;
			int nLemmas = tr->GetLemmaCount();
			int nPosTags = tr->GetPosTagCount();
			int nHeads = tr->GetHeadCount();
			int nDepRels = tr->GetDepRelCount();

			int deletedStart = deleted[0];
			int deletedEnd = deletedStart+deleted[1]-1;
			int j = 2;
			for (int i = 0; i < nWords; i++) {
				if (i < deletedStart) {
					// copy contents
					flags.push_back(tr->GetFlags(i));
					forms.push_back(tr->GetForm(i));
					if (i < nLemmas)
						lemmas.push_back(tr->GetLemma(i));
					if (i < nPosTags)
						posTags.push_back(tr->GetPosTag(i));
					if (i < nHeads)
						heads.push_back(tr->GetHead(i));
					if (i < nDepRels)
						depRels.push_back(tr->GetDepRel(i));
				} else {
					// ignore contents
					if (i == deletedEnd) {
						// next delted paragraph
						if (j+1 < (int)deleted.size()) {
							deletedStart = deleted[j];
							deletedEnd = deletedStart+deleted[j+1]-1;
							j += 2;
						} else {
							deletedStart = nWords+1;
						}
					}
				}
			}

			// really change contents
			tr->ClearFlags();
			tr->ClearForm();
			tr->ClearLemma();
			tr->ClearPosTag();
			tr->ClearHead();
			tr->ClearDepRel();
			for (int i = 0; i < (int)forms.size(); i++) {
				tr->SetFlags(i, flags[i]);
				tr->SetForm(i, forms[i]);
				if (i < (int)lemmas.size())
					tr->SetLemma(i, lemmas[i]);
				if (i < (int)posTags.size())
					tr->SetPosTag(i, posTags[i]);
				if (i < (int)heads.size())
					tr->SetHead(i, heads[i]);
				if (i < (int)depRels.size())
					tr->SetDepRel(i, depRels[i]);
			}
		}
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new FilterLanguage(objects, id, threadIndex);
}
