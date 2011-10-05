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

	nParagraph = 0;
	nSentence = 0;
	nWord = 0;
	nForm = 0;
	nLemma = 0;
	nPosTag = 0;
	nHead = 0;
	nDepRel = 0;
}

WordCount::~WordCount() {
	LOG_DEBUG(this, "doc\tpara\tsent\tform\tword\tlemma\tpos\thead\trel");
	char *wc = GetWordCount("wordCount");
	LOG_DEBUG(this, wc);
	free(wc);
	delete props;
}

char *WordCount::GetItems(const char *name) {
	return int2str(items);
}

char *WordCount::GetWordCount(const char *name) {
	char buf[1024];
	snprintf(buf, sizeof(buf), "%d\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64, items, nParagraph, nSentence, nWord, nForm, nLemma, nPosTag, nHead, nDepRel);
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
		int flags = tr->GetFlags(i);
		if (!(flags & TextResource::TOKEN_PUNCT))
			nWord++;
		if (flags & TextResource::TOKEN_PARAGRAPH_START)
			nParagraph++;
		if (flags & TextResource::TOKEN_SENTENCE_START)
			nSentence++;
	}
	nForm += words;
	nLemma += tr->GetLemmaCount();
	nPosTag += tr->GetPosTagCount();
	nHead += tr->GetHeadCount();
	nDepRel += tr->GetDepRelCount();

	items++;

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new WordCount(objects, id, threadIndex);
}
