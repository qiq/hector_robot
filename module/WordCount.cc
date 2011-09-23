/**
 */
#include <config.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <string.h>
#include "robot_common.h"
#include "WordCount.h"
#include "TextResource.h"

using namespace std;

WordCount::WordCount(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;

	props = new ObjectProperties<WordCount>(this);
	props->Add("items", &WordCount::GetItems);
	props->Add("wordCount", &WordCount::GetWordCount);

	nWord = 0;
	nForm = 0;
	nLemma = 0;
	nPosTag = 0;
	nHead = 0;
	nDepRel = 0;
}

WordCount::~WordCount() {
	LOG_DEBUG(this, nForm << ", " << nLemma << ", " << nPosTag << ", " << nHead << ", " << nDepRel);
	delete props;
}

char *WordCount::GetItems(const char *name) {
	return int2str(items);
}

char *WordCount::GetWordCount(const char *name) {
	char buf[1024];
	snprintf(buf, sizeof(buf), "%" PRIu64 " (%" PRIu64 "), %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64, nForm, nWord, nLemma, nPosTag, nHead, nDepRel);
	return strdup(buf);
}

bool WordCount::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	return true;
}

Resource *WordCount::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int words = tr->GetFormCount();
	for (int i = 0; i < words; i++) {
		if (!(tr->GetFlags(i) & TextResource::TOKEN_PUNCT))
			nWord++;
	}
	nForm += words;
	nLemma += tr->GetLemmaCount();
	nPosTag += tr->GetPosTagCount();
	nHead += tr->GetHeadCount();
	nDepRel += tr->GetDepRelCount();

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new WordCount(objects, id, threadIndex);
}
