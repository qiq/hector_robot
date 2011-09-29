/**
 */
#include <config.h>

#include <string.h>
#include "robot_common.h"
#include "Deduplicate.h"
#include "TextResource.h"

using namespace std;

Deduplicate::Deduplicate(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	totalSize = 0;
	deduplicateParagraphs = false;
	threshold = 0.3;
	ngram = 8;

	props = new ObjectProperties<Deduplicate>(this);
	props->Add("items", &Deduplicate::GetItems);
	props->Add("totalSize", &Deduplicate::GetTotalSize, &Deduplicate::SetTotalSize, true);
	props->Add("deduplicateParagraphs", &Deduplicate::GetDeduplicateParagraphs, &Deduplicate::SetDeduplicateParagraphs);
	props->Add("threshold", &Deduplicate::GetThreshold, &Deduplicate::SetThreshold);
	props->Add("ngram", &Deduplicate::GetNgram, &Deduplicate::SetNgram);

	bloom = NULL;
}

Deduplicate::~Deduplicate() {
	delete bloom;
	delete props;
}

char *Deduplicate::GetItems(const char *name) {
	return int2str(items);
}

char *Deduplicate::GetTotalSize(const char *name) {
	return long2str(totalSize);
}

void Deduplicate::SetTotalSize(const char *name, const char *value) {
	totalSize = str2long(value);
}

char *Deduplicate::GetDeduplicateParagraphs(const char *name) {
	return bool2str(deduplicateParagraphs);
}

void Deduplicate::SetDeduplicateParagraphs(const char *name, const char *value) {
	deduplicateParagraphs = str2bool(value);
}

char *Deduplicate::GetThreshold(const char *name) {
	return double2str(threshold);
}

void Deduplicate::SetThreshold(const char *name, const char *value) {
	threshold = str2double(value);
}

char *Deduplicate::GetNgram(const char *name) {
	return int2str(ngram);
}

void Deduplicate::SetNgram(const char *name, const char *value) {
	ngram = str2int(value);
}

bool Deduplicate::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	if (totalSize == 0) {
		LOG_ERROR(this, "totalSize not specified, cannot deduplicate.");
		return false;
	}

	bloom = new NgramBloomFilter(ngram, threshold, totalSize, 0.01, true);

	return true;
}

Resource *Deduplicate::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int nWords = tr->GetFormCount();

	if (!deduplicateParagraphs) {
		// whole documents only
		vector<string> words;
		for (int i = 0; i < nWords; i++)
			words.push_back(tr->GetForm(i));
		double ratio;
		if (bloom->TestDuplicate(words, &ratio)) {
			LOG_DEBUG_R(this, tr, "Duplicate document deleted: " << tr->GetTextId() << " (" << ratio << ")");
			tr->SetFlag(Resource::DELETED);
		} else {
			LOG_TRACE_R(this, tr, "d-ratio >= " << ratio);
		}
		return tr;
	} else {
		// deduplicate paragraphs (some paragraps may be deleted,
		// others survive)

		int totalDeleted = 0;
		vector<bool> deleted(nWords);
		int idx = 0;
		vector<string> words;
		for (int i = 0; i < nWords; i++) {
			if (tr->GetFlags(i) & TextResource::TOKEN_PARAGRAPH_START) {
				if (words.size() > 0) {
					double ratio;
					if (bloom->TestDuplicate(words, &ratio)) {
						for (int j = 0; j < (int)words.size(); j++)
							deleted[idx++] = true;
						totalDeleted += words.size();
					} else {
						for (int j = 0; j < (int)words.size(); j++)
							deleted[idx++] = false;
					}
					LOG_TRACE_R(this, tr, "d-ratio >= " << ratio);
					words.clear();
				}
			}
			words.push_back(tr->GetForm(i));
		}
		if (words.size() > 0) {
			double ratio;
			if (bloom->TestDuplicate(words, &ratio)) {
				for (int i = 0; i < (int)words.size(); i++)
					deleted[idx++] = true;
				totalDeleted += words.size();
			} else {
				for (int i = 0; i < (int)words.size(); i++)
					deleted[idx++] = false;
			}
			LOG_TRACE_R(this, tr, "d-ratio >= " << ratio);
		}

		// delete duplicated contents
		if (totalDeleted < nWords) {
			tr->DeleteWords(deleted);
		} else {
			// all paragraphs were marked duplicate
			LOG_DEBUG_R(this, tr, "Duplicate document deleted: " << tr->GetTextId());
			tr->SetFlag(Resource::DELETED);
		}
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Deduplicate(objects, id, threadIndex);
}
