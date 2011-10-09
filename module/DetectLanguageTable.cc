/**
 */
#include <config.h>

#include <glob.h>
#include <string.h>
#include <fstream>
#include "unicode/unistr.h"
#include "unicode/ustring.h"
#include "robot_common.h"
#include "DetectLanguageTable.h"
#include "TextResource.h"

using namespace std;

DetectLanguageTable::DetectLanguageTable(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	filenamePrefix = NULL;
	paragraphLevel = true;

	props = new ObjectProperties<DetectLanguageTable>(this);
	props->Add("items", &DetectLanguageTable::GetItems);
	props->Add("filenamePrefix", &DetectLanguageTable::GetFilenamePrefix, &DetectLanguageTable::SetFilenamePrefix);
	props->Add("paragraphLevel", &DetectLanguageTable::GetParagraphLevel, &DetectLanguageTable::SetParagraphLevel);
}

DetectLanguageTable::~DetectLanguageTable() {
	free(filenamePrefix);
	delete props;
}

char *DetectLanguageTable::GetItems(const char *name) {
	return int2str(items);
}

char *DetectLanguageTable::GetFilenamePrefix(const char *name) {
	return strdup(filenamePrefix);
}

void DetectLanguageTable::SetFilenamePrefix(const char *name, const char *value) {
	free(filenamePrefix);
	filenamePrefix = strdup(value);
}

char *DetectLanguageTable::GetParagraphLevel(const char *name) {
	return bool2str(paragraphLevel);
}

void DetectLanguageTable::SetParagraphLevel(const char *name, const char *value) {
	paragraphLevel = str2bool(value);
}

bool DetectLanguageTable::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	if (!filenamePrefix) {
		LOG_ERROR(this, "filenamePrefix not defined");
		return false;
	}

	glob_t gb;
	char s[1024];
	snprintf(s, sizeof(s), "%s.*", filenamePrefix);
	int result = glob(s, 0, NULL, &gb);
	if (result != 0) {
		LOG_ERROR(this, "Files " << filenamePrefix << ".* not found: " << result);
		return false;
	}

	LOG_DEBUG(this, "Loading " << gb.gl_pathc << " files.");
	for (int i = 0; i < (int)gb.gl_pathc; i++) {
		ifstream ifs(gb.gl_pathv[i]);
		if (!ifs.is_open()) {
			LOG_ERROR(this, "Cannot open file: " << gb.gl_pathv[i]);
			return false;
		}
		string lang(gb.gl_pathv[i]);
		lang.erase(0, lang.find_last_of('.')+1);
		string s;
		while (!getline(ifs, s).eof()) {
			skipWs(s);
			if (s.length() > 0)
				words[s] = lang;
		}
		ifs.close();
	}
	globfree(&gb);

	return true;
}

Resource *DetectLanguageTable::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);
	string languages;

	tr1::unordered_map<string, int> score;
	int count = 0;
	int nWords = tr->GetFormCount();
	for (int i = 0; i < nWords; i++) {
		int flags = tr->GetFlags(i);
		string word = tr->GetForm(i);
		if (paragraphLevel && i > 0 && (flags & TextResource::TOKEN_PARAGRAPH_START)) {
			int max = -1;
			string lang = "?";
			for (tr1::unordered_map<string, int>::iterator iter = score.begin(); iter != score.end(); ++iter) {
fprintf(stderr, "%s, %d\n", iter->first.c_str(), iter->second);
				if (iter->second > max) {
					max = iter->second;
					lang = iter->first;
				}
			}
fprintf(stderr, "MAX: %s, %d\n", lang.c_str(), max);
			if (!languages.empty())
				languages += " ";
			languages += lang;
			score.clear();
			count = 0;
		}
		// lower-case word
		icu::UnicodeString s16 = icu::UnicodeString::fromUTF8(word);
		s16.toLower();
		word.clear();
		word = s16.toUTF8String(word);
fprintf(stderr, "%s\n", word.c_str());
		tr1::unordered_map<string, string>::iterator iter = words.find(word);
		if (iter != words.end()) {
			tr1::unordered_map<string, int>::iterator iter2 = score.find(iter->second);
			if (iter2 == score.end())
				score[iter->second] = 1;
			else
				score[iter->second]++;
fprintf(stderr, "%s\n", iter->second.c_str());
		}
		count++;
	}

	if (count) {
		int max = -1;
		string lang = "?";
		for (tr1::unordered_map<string, int>::iterator iter = score.begin(); iter != score.end(); ++iter) {
			if (iter->second > max) {
				max = iter->second;
				lang = iter->first;
			}
		}
		if (!languages.empty())
			languages += " ";
		languages += lang;
	}

	if (languages.empty())
		languages = "?";

	tr->SetLanguage(languages);

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new DetectLanguageTable(objects, id, threadIndex);
}
