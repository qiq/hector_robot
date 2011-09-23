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
	paragraphLevel = false;

	props = new ObjectProperties<DetectLanguage>(this);
	props->Add("items", &DetectLanguage::GetItems);
	props->Add("paragraphLevel", &DetectLanguage::GetParagraphLevel, &DetectLanguage::SetParagraphLevel);

	lt = NULL;
}

DetectLanguage::~DetectLanguage() {
	delete lt;
	delete props;
}

char *DetectLanguage::GetItems(const char *name) {
	return int2str(items);
}

char *DetectLanguage::GetParagraphLevel(const char *name) {
	return bool2str(paragraphLevel);
}

void DetectLanguage::SetParagraphLevel(const char *name, const char *value) {
	paragraphLevel = str2bool(value);
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
	string languages;

	string text;
	int nWords = tr->GetFormCount();
	if (nWords > 0) {
		// create text containing all words
		text.reserve(nWords * 10);
		bool space = false;
		for (int i = 0; i < nWords; i++) {
			int flags = tr->GetFlags(i);
			if (paragraphLevel && i > 0 && (flags & TextResource::TOKEN_PARAGRAPH_START)) {
				string l = lt->Detect(text);
				if (!languages.empty())
					languages += " ";
				languages += l.empty() ? "?" : l;
				text.clear();
				space = false;
			}
			if (space)
				text.append(" ");
			text.append(tr->GetForm(i));
			space = !(flags & TextResource::TOKEN_NO_SPACE);
		}
	} else {
		text = tr->GetText();
	}

	// detect language
	string l = lt->Detect(text);
	if (!languages.empty())
		languages += " ";
	languages += l.empty() ? "?" : l;

	tr->SetLanguage(languages);

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new DetectLanguage(objects, id, threadIndex);
}
