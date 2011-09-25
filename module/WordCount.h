/**
WordCount.la, simple, native
WordCount counts number of words in a resource. Currently, only TextResource is
supported, but there may be more resource types supported in the future.

Dependencies: none

Parameters:
items			r/o	Total items processed
*/

#ifndef _MODULES_WORD_COUNT_H_
#define _MODULES_WORD_COUNT_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class WordCount : public Module {
public:
	WordCount(ObjectRegistry *objects, const char *id, int threadIndex);
	~WordCount();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;

	char *GetItems(const char *name);
	char *GetWordCount(const char *name);

	ObjectProperties<WordCount> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	uint64_t nParagraph;
	uint64_t nSentence;
	uint64_t nWord;
	uint64_t nForm;
	uint64_t nLemma;
	uint64_t nPosTag;
	uint64_t nHead;
	uint64_t nDepRel;
};

inline Module::Type WordCount::GetType() {
	return SIMPLE;
}

inline char *WordCount::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool WordCount::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *WordCount::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
