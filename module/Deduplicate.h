/**
Deduplicate.la, simple, native
Deduplicate deletes documents or paragraphs that contain duplicate content, in
other words only non-duplicate content is passed to the output. It uses bloom
filter to store previously-seen n-grams and requires ~1.25 bytes per n-gram.

Dependencies: none

Parameters:
items			r/o	Total items processed
totalSize		init	(long) Total number of n-grams we are processing
deduplicateParagraphs	r/w	(bool) Work on paragraph level (otherwise full
				docs are compared)
threshold		r/w	(float) fraction of the n-grams that must be
				duplicated, to be considered duplicate, default is 0.3)
ngram			r/w	number of consecutive words to consider,
				default is 8
*/

#ifndef _MODULES_DEDUPLICATE_H_
#define _MODULES_DEDUPLICATE_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "NgramBloomFilter.h"
#include "ObjectProperties.h"

class Deduplicate : public Module {
public:
	Deduplicate(ObjectRegistry *objects, const char *id, int threadIndex);
	~Deduplicate();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	bool deduplicateParagraphs;
	int64_t totalSize;
	double threshold;
	int ngram;

	char *GetItems(const char *name);
	char *GetTotalSize(const char *name);
        void SetTotalSize(const char *name, const char *value);
	char *GetDeduplicateParagraphs(const char *name);
        void SetDeduplicateParagraphs(const char *name, const char *value);
	char *GetThreshold(const char *name);
        void SetThreshold(const char *name, const char *value);
	char *GetNgram(const char *name);
        void SetNgram(const char *name, const char *value);

	ObjectProperties<Deduplicate> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	NgramBloomFilter *bloom;
};

inline Module::Type Deduplicate::GetType() {
	return SIMPLE;
}

inline char *Deduplicate::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool Deduplicate::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *Deduplicate::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
