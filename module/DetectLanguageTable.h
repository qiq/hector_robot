/**
DetectLanguageTable.la, simple, native
DetectLanguage text of tokens in a TextResource using language tables prepared
by DetectLanguageTableTrain.pm. It does the same as DetectLanguageTable.pm, but
is hopefully faster.

Dependencies: icu

Parameters:
items			r/o	Total items processed
filenamePrefix		r/o	Files containing most common words, one
				word per line.
paragraphLevel		r/w	Do a language recognition on paragraphs
				(instead of whole documents)
*/

#ifndef _MODULES_DETECT_LANGUAGE_TABLE_H_
#define _MODULES_DETECT_LANGUAGE_TABLE_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class DetectLanguageTable : public Module {
public:
	DetectLanguageTable(ObjectRegistry *objects, const char *id, int threadIndex);
	~DetectLanguageTable();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	char *filenamePrefix;
	char *defaultLanguage;
	bool paragraphLevel;

	char *GetItems(const char *name);
	char *GetFilenamePrefix(const char *name);
	void SetFilenamePrefix(const char *name, const char *value);
	char *GetDefaultLanguage(const char *name);
	void SetDefaultLanguage(const char *name, const char *value);
	char *GetParagraphLevel(const char *name);
	void SetParagraphLevel(const char *name, const char *value);

	ObjectProperties<DetectLanguageTable> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::tr1::unordered_map<std::string, std::vector<int>*> words;
	std::tr1::unordered_map<std::string, int> lang2id;
	std::tr1::unordered_map<int, std::string> id2lang;
	int defaultLanguageId;
};

inline Module::Type DetectLanguageTable::GetType() {
	return SIMPLE;
}

inline char *DetectLanguageTable::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool DetectLanguageTable::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *DetectLanguageTable::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
