/**
DetectLanguage.la, simple, native
DetectLanguage text or tokens in a TextResource (convert UTF-8 to NFC) and possibly
other transformations.

Dependencies: Google CLD

Parameters:
items			r/o	Total items processed
paragraphLevel		r/w	Do a language recognition on paragraphs
				(instead of whole documents)
*/

#ifndef _MODULES_DETECT_LANGUAGE_H_
#define _MODULES_DETECT_LANGUAGE_H_

#include <config.h>

#include <string>
#include "common.h"
#include "LanguageTools.h"
#include "Module.h"
#include "ObjectProperties.h"

class DetectLanguage : public Module {
public:
	DetectLanguage(ObjectRegistry *objects, const char *id, int threadIndex);
	~DetectLanguage();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	bool paragraphLevel;

	char *GetItems(const char *name);
	char *GetParagraphLevel(const char *name);
	void SetParagraphLevel(const char *name, const char *value);

	ObjectProperties<DetectLanguage> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	LanguageTools *lt;
};

inline Module::Type DetectLanguage::GetType() {
	return SIMPLE;
}

inline char *DetectLanguage::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool DetectLanguage::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *DetectLanguage::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
