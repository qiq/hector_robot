/**
 */
#include <config.h>

#include <string.h>
#include "robot_common.h"
#include "FilterUnrecognized.h"
#include "TextResource.h"

using namespace std;

FilterUnrecognized::FilterUnrecognized(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxUnrecognizedRatio = 0.1;

	props = new ObjectProperties<FilterUnrecognized>(this);
	props->Add("items", &FilterUnrecognized::GetItems);
	props->Add("maxUnrecognizedRatio", &FilterUnrecognized::GetMaxUnrecognizedRatio, &FilterUnrecognized::SetMaxUnrecognizedRatio);
	props->Add("reversed", &FilterUnrecognized::GetReversed, &FilterUnrecognized::SetReversed);
}

FilterUnrecognized::~FilterUnrecognized() {
	delete props;
}

char *FilterUnrecognized::GetItems(const char *name) {
	return int2str(items);
}

char *FilterUnrecognized::GetMaxUnrecognizedRatio(const char *name) {
	return double2str(maxUnrecognizedRatio);
}

void FilterUnrecognized::SetMaxUnrecognizedRatio(const char *name, const char *value) {
	maxUnrecognizedRatio = str2double(value);
}

char *FilterUnrecognized::GetReversed(const char *name) {
	return bool2str(reversed);
}

void FilterUnrecognized::SetReversed(const char *name, const char *value) {
	reversed = str2bool(value);
}

bool FilterUnrecognized::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	return true;
}

Resource *FilterUnrecognized::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int totalDeleted = 0;
	int unrecognized = 0;
	int words = 0;
	int nWords = tr->GetFormCount();
	vector<bool> deleted(nWords);
	int idx = 0;
	for (int i = 0; i < nWords; i++) {
		int flags = tr->GetFlags(i);
		if (i > 0 && flags & TextResource::TOKEN_PARAGRAPH_START) {
			bool del = unrecognized >= 2 && (double)unrecognized/words > maxUnrecognizedRatio;
			if (reversed)
				del = del ? false : true;
			if (del) {
				for (int i = 0; i < words; i++)
					deleted[idx++] = true;
				totalDeleted += words;
			} else {
				for (int i = 0; i < words; i++)
					deleted[idx++] = false;
			}
			unrecognized = 0;
			words = 0;
		}

		if (flags & TextResource::TOKEN_UNRECOGNIZED)
			unrecognized++;
		words++;
	}
	if (words > 0) {
		bool del = unrecognized >= 2 && (double)unrecognized/words > maxUnrecognizedRatio;
		if (reversed)
			del = del ? false : true;
		if (del) {
			for (int i = 0; i < words; i++)
				deleted[idx++] = true;
			totalDeleted += words;
		} else {
			for (int i = 0; i < words; i++)
				deleted[idx++] = false;
		}
	}

	if (totalDeleted < nWords) {
		tr->DeleteWords(deleted);
	} else {
		// all paragraphs were deleted
		LOG_DEBUG_R(this, tr, "All paragraphs contain unrecognized words, document deleted: " << tr->GetTextId());
		tr->SetFlag(Resource::DELETED);
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new FilterUnrecognized(objects, id, threadIndex);
}
