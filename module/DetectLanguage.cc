/**
 */
#include <config.h>

#include <string.h>
#include "unicode/normlzr.h"
#include "unicode/unistr.h"
#include "unicode/ustring.h"
#include "robot_common.h"
#include "DetectLanguage.h"
#include "TextResource.h"

using namespace std;

DetectLanguage::DetectLanguage(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;

	props = new ObjectProperties<DetectLanguage>(this);
	props->Add("items", &DetectLanguage::GetItems);

	lt = NULL;
}

DetectLanguage::~DetectLanguage() {
	delete lt;
	delete props;
}

char *DetectLanguage::GetItems(const char *name) {
	return int2str(items);
}

bool DetectLanguage::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	lt = new LanguageTools();

	return true;
}

Resource *DetectLanguage::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	string text;
	int nWords = tr->GetFormCount();
	if (nWords > 0) {
		// create text containing all words
		text.reserve(nWords * 10);
		bool space = false;
		for (int i = 0; i < nWords; i++) {
			if (space)
				text.append(" ");
			text.append(tr->GetForm(i));
			space = tr->GetFlags(i) & TextResource::TOKEN_NO_SPACE;
		}
	} else {
		text = tr->GetText();
	}

	// detect language
	string language = lt->Detect(text);
	if (!language.empty())
		tr->SetLanguage(language);

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new DetectLanguage(objects, id, threadIndex);
}
