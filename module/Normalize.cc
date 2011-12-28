/**
 */
#include <config.h>

#include <string.h>
#include "unicode/normlzr.h"
#include "unicode/unistr.h"
#include "unicode/ustring.h"
#include "robot_common.h"
#include "Normalize.h"
#include "TextResource.h"

using namespace std;

Normalize::Normalize(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;

	props = new ObjectProperties<Normalize>(this);
	props->Add("items", &Normalize::GetItems);
}

Normalize::~Normalize() {
	delete props;
}

char *Normalize::GetItems(const char *name) {
	return int2str(items);
}

bool Normalize::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	return true;
}

Resource *Normalize::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int nWords = tr->GetFormCount();
	if (nWords > 0) {
		// transform every word (token)
		for (int i = 0; i < nWords; i++) {
			string word = tr->GetForm(i);
			icu::UnicodeString s16 = icu::UnicodeString::fromUTF8(word);
			icu::UnicodeString normalized;
			UErrorCode status = U_ZERO_ERROR;
			icu::Normalizer::normalize(s16, UNORM_NFKC, 0, normalized, status);
			if (!U_FAILURE(status)) {
				word.clear();
				tr->SetForm(i, normalized.toUTF8String(word));
			} else {
				LOG_DEBUG(this, "Invalid UTF-8 sequence: " << word);
				tr->SetForm(i, "INVALID");
			}
		}

	} else {
		// transform whole text
		string text = tr->GetText();
		icu::UnicodeString s16 = icu::UnicodeString::fromUTF8(text);
		icu::UnicodeString normalized;
		UErrorCode status = U_ZERO_ERROR;
		icu::Normalizer::normalize(s16, UNORM_NFKC, 0, normalized, status);
		if (!U_FAILURE(status)) {
			text.clear();
			tr->SetText(normalized.toUTF8String(text));
		} else {
			LOG_DEBUG(this, "Invalid UTF-8 sequence: " << text);
			tr->ClearText();
		}
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Normalize(objects, id, threadIndex);
}
