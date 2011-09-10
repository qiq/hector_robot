/**
 */
#include <config.h>

#include <string.h>
#include "robot_common.h"
#include "Deduplicator.h"
#include "TextResource.h"

using namespace std;

Deduplicator::Deduplicator(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	totalSize = 0;
	deduplicateParagraphs = false;
	threshold = 0.3;
	ngram = 8;

	props = new ObjectProperties<Deduplicator>(this);
	props->Add("items", &Deduplicator::GetItems);
	props->Add("totalSize", &Deduplicator::GetTotalSize, &Deduplicator::SetTotalSize, true);
	props->Add("deduplicateParagraphs", &Deduplicator::GetDeduplicateParagraphs, &Deduplicator::SetDeduplicateParagraphs);
	props->Add("threshold", &Deduplicator::GetThreshold, &Deduplicator::SetThreshold);
	props->Add("ngram", &Deduplicator::GetNgram, &Deduplicator::SetNgram);

	bloom = NULL;
}

Deduplicator::~Deduplicator() {
	delete bloom;
	delete props;
}

char *Deduplicator::GetItems(const char *name) {
	return int2str(items);
}

char *Deduplicator::GetTotalSize(const char *name) {
	return long2str(totalSize);
}

void Deduplicator::SetTotalSize(const char *name, const char *value) {
	totalSize = str2long(value);
}

char *Deduplicator::GetDeduplicateParagraphs(const char *name) {
	return bool2str(deduplicateParagraphs);
}

void Deduplicator::SetDeduplicateParagraphs(const char *name, const char *value) {
	deduplicateParagraphs = str2bool(value);
}

char *Deduplicator::GetThreshold(const char *name) {
	return double2str(threshold);
}

void Deduplicator::SetThreshold(const char *name, const char *value) {
	threshold = str2double(value);
}

char *Deduplicator::GetNgram(const char *name) {
	return int2str(ngram);
}

void Deduplicator::SetNgram(const char *name, const char *value) {
	ngram = str2int(value);
}

bool Deduplicator::Init(vector<pair<string, string> > *params) {
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

Resource *Deduplicator::ProcessSimpleSync(Resource *resource) {
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

		int start = 0;
		int totalDeleted = 0;
		vector<unsigned> deleted;	// offset + length of paragragraphs to be deleted
		vector<string> words;
		for (int i = 0; i < nWords; i++) {
			if (tr->GetFlags(i) & TextResource::TOKEN_PARAGRAPH_START) {
				if (words.size() > 0) {
					double ratio;
					if (bloom->TestDuplicate(words, &ratio)) {
						unsigned size = words.size();
						deleted.push_back(start);
						deleted.push_back(size);
						totalDeleted += size;
					}
					LOG_TRACE_R(this, tr, "d-ratio >= " << ratio);
					words.clear();
				}
				start = i;
			}
			words.push_back(tr->GetForm(i));
		}
		if (words.size() > 0) {
			double ratio;
			if (bloom->TestDuplicate(words, &ratio)) {
				unsigned size = words.size();
				deleted.push_back(start);
				deleted.push_back(size);
				totalDeleted += size;
			}
			LOG_TRACE_R(this, tr, "d-ratio >= " << ratio);
		}

		// delete duplicated contents
		if (totalDeleted == nWords) {
			// all paragraphs were marked duplicate
			LOG_DEBUG_R(this, tr, "Duplicate document deleted: " << tr->GetTextId());
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
	return new Deduplicator(objects, id, threadIndex);
}
