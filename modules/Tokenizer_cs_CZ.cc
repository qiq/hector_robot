#include <config.h>

#include <vector>
#include <tr1/unordered_map>
#include "Tokenizer.h"
#include "TextResource.h"

using namespace std;

const int lookahead = 2;

// Abbreviations usually NOT seen at the end of the sentence
// these abbrs should not be added: nám (seděl naproti nám), slov (škoda slov)
const char *abbr_list[] = {
	"prof", "doc", "RNDr", "MUDr", "PhDr", "JUDr", "MVDr", "Dr", "CSc", "DrSc",
	"PhD", "Ph", "D", "Ing", "Mgr", "Bc", "p", "mjr", "plk", "s", "RSDr",
	"max", "min", "pozn", "red",
	"tel", "adr", "např", "napr", "eg", "g", "p", "st", "str", "um", "tj",
	"j", "zn", "tzv", "sv", "c", "č", "min", "hod", "ev",
	"okr", "ul", "rep", "m", "ř", "r", "stř", "str", "sev", "již", "jiz", "záp",
	"zap", "vých", "vych",
	"angl", "fr", "čes", "ces", "něm", "nem", "it", "pol", "maď",
	"mad", "rus",
	"zl", "kr",
	"viz", "vs", "disamb",	// incorrect ones
	"atd", "apod", "aj",
	NULL
};

// Abbreviations usually seen at the end of the sentence
const char *abbr_list_eos[] = {
	"atd", "apod", "aj",
	NULL
};

tr1::unordered_set<string> abbr;
tr1::unordered_set<string> abbr_eos;

extern "C" int fixup_init() {
	// process abbr_list
	const char **s = abbr_list;
	while (*s) {
		abbr.insert(*s);
		s++;
	}
	// process abbr_list_eos
	s = abbr_list_eos;
	while (*s) {
		abbr_eos.insert(*s);
		s++;
	}

	return lookahead;
}

extern "C" void fixup_finish() {
}

extern "C" void fixup(vector<Token*> &tokens, int index) {
	// mark abbreviations
	Token *current = tokens[index];
	if (abbr.find(current->GetText()) != abbr.end())
		tokens[index]->SetFlag(TextResource::TOKEN_ABBR);

	// not end-of-sentence: napr. Novak
	if (index >= 2 && current->TestFlag(TextResource::TOKEN_SENTENCE_START)) {
		Token *dot = tokens[index-1];
		if (dot->TestFlag(TextResource::TOKEN_PUNCT) && dot->GetText() == ".") {
			Token *prev = tokens[index-2];
			if (prev->TestFlag(TextResource::TOKEN_ABBR) && abbr_eos.find(prev->GetText()) == abbr_eos.end()) {
				current->ResetFlag((TextResource::Flags)(TextResource::TOKEN_SENTENCE_START|TextResource::TOKEN_PARAGRAPH_START));
			}
		}
	}
}
