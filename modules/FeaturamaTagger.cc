/**
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include <czmorphology/interface.h>
#include <featurama/perc.h>
#include <unistr.h>
#include <unictype.h>
#include "robot_common.h"
#include "FeaturamaTagger.h"
#include "TextResource.h"

using namespace std;

FeaturamaTagger::FeaturamaTagger(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	morphologyPrefix = NULL;
	useGuesser = true;
	taggerPrefix = NULL;

	props = new ObjectProperties<FeaturamaTagger>(this);
	props->Add("items", &FeaturamaTagger::GetItems);
	props->Add("morphologyPrefix", &FeaturamaTagger::GetMorphologyPrefix, &FeaturamaTagger::SetMorphologyPrefix, true);
	props->Add("useGuesser", &FeaturamaTagger::GetUseGuesser, &FeaturamaTagger::SetUseGuesser, true);
	props->Add("taggerPrefix", &FeaturamaTagger::GetTaggerPrefix, &FeaturamaTagger::SetTaggerPrefix, true);

	perc = NULL;
}

FeaturamaTagger::~FeaturamaTagger() {
	free(morphologyPrefix);
	free(taggerPrefix);
	if (perc) {
		perc_test_finish(perc);
		perc_destroy(perc);
	}
	delete props;
}

char *FeaturamaTagger::GetItems(const char *name) {
	return int2str(items);
}

char *FeaturamaTagger::GetMorphologyPrefix(const char *name) {
	return strdup(morphologyPrefix);
}

void FeaturamaTagger::SetMorphologyPrefix(const char *name, const char *value) {
	free(morphologyPrefix);
	morphologyPrefix = strdup(value);
}

char *FeaturamaTagger::GetUseGuesser(const char *name) {
	return bool2str(useGuesser);
}

void FeaturamaTagger::SetUseGuesser(const char *name, const char *value) {
	useGuesser = str2bool(value);
}

char *FeaturamaTagger::GetTaggerPrefix(const char *name) {
	return strdup(taggerPrefix);
}

void FeaturamaTagger::SetTaggerPrefix(const char *name, const char *value) {
	free(taggerPrefix);
	taggerPrefix = strdup(value);
}

bool FeaturamaTagger::Init(vector<pair<string, string> > *params) {
	int result;

	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	// initialize morphology
	if (!morphologyPrefix) {
		LOG_ERROR(this, "morphlogyPrefix not specified");
		return false;
	}
	result = lemmatize_init(morphologyPrefix, useGuesser);
	if (result != 0) {
		LOG_ERROR(this, "Error initialize czmorphology: " << result);
		return false;
	}

	// initialize tagger
	if (!taggerPrefix) {
		LOG_ERROR(this, "taggerPrefix not specified");
		return false;
	}

	perc = perc_init();
	char featureFile[1024];
	char dictFile[1024];
	char alphaFile[1024];
	snprintf(featureFile, sizeof(featureFile), "%s.f", taggerPrefix);
	snprintf(dictFile, sizeof(dictFile), "%s.dict", taggerPrefix);
	snprintf(alphaFile, sizeof(alphaFile), "%s.alpha", taggerPrefix);
	char header[] = "Form	Prefix1	Prefix2	Prefix3	Prefix4	Suffix1	Suffix2	Suffix3	Suffix4	Num	Cap	Dash	FollowingVerbTag	FollowingVerbLemma	Tag";
	if (!perc_test_init(perc, featureFile, dictFile, alphaFile, header, 0, 0, 3))
		return false;

	return true;
}

string MorphologyToOffer(char *s) {
	string result;
	string lemma;
	char *l = s;
	int llen = 0;
	char *t = NULL;
	while (*s) {
		switch (*s) {
		case '\t':
			if (t) {
				// tag: t..s
				result.append(t, s-t);
				result.append(1, ' ');
				result.append(l, llen);
				result.append(1, '\t');
			}
			t = NULL;
			l = s+1;
			llen = 0;
			break;
		case ' ':
			if (l && llen == 0) {
				llen = s-l;
			} else if (t) {
				// tag: t..s
				result.append(t, s-t);
				result.append(1, ' ');
				result.append(l, llen);
				result.append(1, '\t');
			}
			t = s+1;
			break;
		default:
			break;
		}
		s++;
	}
	if (t) {
		// tag: t..s
		result.append(t, s-t);
		result.append(1, ' ');
		result.append(l, llen);
		result.append(1, '\t');
	}
	return result;
}

Resource *FeaturamaTagger::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int nwords = tr->GetFormCount();
	vector<string> features;
	vector<string> verbs;
	vector<string> offers;
	for (int idx = 0; idx < nwords; idx++) {
		int flags = tr->GetFlags(idx);

		// start of a sentence
		if (flags & TextResource::TOKEN_SENTENCE_START && idx > 0) {
			int sentenceSize = features.size();
			perc_begin_sentence(perc);
			while (verbs.size() < features.size())
				verbs.push_back("NULL\tNULL");
			for (int i = 0; i < sentenceSize; i++) {
				string line = features[i];
				line.append(1, '\t');
				line.append(verbs[i]);
				line.append("\tNULL\t");
				line.append(offers[i]);
fprintf(stderr, "line: %s\n", line.c_str());
				perc_append_word(perc, line.c_str());
			}
			perc_end_sentence(perc);
			for (int i = 0; i < sentenceSize; i++) {
				tr->SetPosTag(idx-sentenceSize+i, perc_get_proposed_tag(perc, i, 0));
				tr->SetLemma(idx-sentenceSize+i, perc_get_additional_proposed_tag(perc, i, 0));
			}
			features.clear();
			verbs.clear();
			offers.clear();
		}

		// morphology of a word
		int dot_follows = 0;
		int hyphen_follows = 0;
		if (idx < nwords-1 && tr->GetFlags(idx+1) & TextResource::TOKEN_PUNCT) {
			string form = tr->GetForm(idx+1);
			if (form.c_str()[0] == '.')
				dot_follows = 1;
			else if (form.c_str()[0] == '-')
				hyphen_follows = 1;
		}
//		int next = idx < nwords-1 ? tr->GetFlags(idx+1) : 0;
		string form = tr->GetForm(idx);
		char *morph = lemmatize_token(form.c_str(), flags & TextResource::TOKEN_PUNCT, flags & TextResource::TOKEN_ABBR, flags & TextResource::TOKEN_NUMERIC, dot_follows, hyphen_follows);
		if (!morph) {
			LOG_ERROR(this, "Cannot lemmatize word: " << form << "(" << idx << ")");
			return resource;
		}
		string offer = MorphologyToOffer(morph);

		// construct features for the tagger
		string line;
		line.append(form);

		const char *s = form.c_str();
		ucs4_t c;
		const uint8_t *u8 = (uint8_t*)s;
		for (int i = 0; i < 4; i++) {
			const uint8_t *save = u8;
			u8 = u8_next(&c, u8);
			if (!u8)
				u8 = save;
			line.append(1, '\t');
			line.append(s, u8-(uint8_t*)s);
		}

		u8 = (uint8_t*)s+strlen(s);
		for (int i = 0; i < 4; i++) {
			const uint8_t *save = u8;
			u8 = u8_prev(&c, u8, (uint8_t*)s);
			if (!u8)
				u8 = save;
			line.append(1, '\t');
			line.append((char*)u8);
		}

		u8 = (uint8_t*)s;
		bool num = false;
		bool cap = false;
		bool dash = false;
		while (u8) {
			u8 = u8_next(&c, u8);
			if (uc_is_general_category_withtable(c, UC_CATEGORY_MASK_N))
				num = true;
			if (uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Lu))
				cap = true;
			if (uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Pd))
				dash = true;
		}
		line.append(1, '\t');
		line.append(1, num ? '1' : '0');
		line.append(1, '\t');
		line.append(1, cap ? '1' : '0');
		line.append(1, '\t');
		line.append(1, dash ? '1' : '0');

		// fill previous verbs
		size_t off = 0;
		if (offer.at(0) != 'V') {
			off = offer.find("\tV");
			if (off != string::npos)
				off++;
		}
		if (off != string::npos) {
			string verb = offer.substr(off, offer.find('\t', off));
			size_t space = verb.find(' ');
			if (space != string::npos)
				verb[space] = '\t';
			while (verbs.size() < features.size())
				verbs.push_back(verb);
		}
		features.push_back(line);
		offers.push_back(offer);
	}

	int sentenceSize = features.size();
	if (sentenceSize > 0) {
		perc_begin_sentence(perc);
		while (verbs.size() < features.size())
			verbs.push_back("NULL\tNULL");
		for (int i = 0; i < sentenceSize; i++) {
			string line = features[i];
			line.append(1, '\t');
			line.append(verbs[i]);
			line.append("\tNULL\t");
			line.append(offers[i]);
fprintf(stderr, "line: %s\n", line.c_str());
			perc_append_word(perc, line.c_str());
		}
		perc_end_sentence(perc);
		for (int i = 0; i < sentenceSize; i++) {
			tr->SetPosTag(nwords-sentenceSize+i, perc_get_proposed_tag(perc, i, 0));
			tr->SetLemma(nwords-sentenceSize+i, perc_get_additional_proposed_tag(perc, i, 0));
		}
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new FeaturamaTagger(objects, id, threadIndex);
}
