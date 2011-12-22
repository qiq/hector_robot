#include <config.h>

#include <vector>
#include <tr1/unordered_map>
#include "Tokenize.h"
#include "TextResource.h"

using namespace std;

const int lookahead = 5;

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
	"mot", "voz", "opr",
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

extern "C" int fixup(vector<Token*> &tokens, int index) {
	// mark abbreviations
	Token *current = tokens[index];
	if (abbr.find(current->GetText()) != abbr.end())
		tokens[index]->SetFlag(TextResource::TOKEN_ABBR);

	// advanced end-of-sentence detection: We first go right through
	// no_space tokens and find a upper-cased word. Then, we go left and
	// look for a word. If found (and not abbreviation), we mark current
	// token as start-of-sentence.
	int state = 0;
	int upperIndex = -1;
	if (index >= 2) {
		for (int i = index; i < (int)tokens.size(); i++) {
			Token *t = tokens[i];
			if (t->TestFlag(TextResource::TOKEN_PUNCT)) {
				if (!t->TestFlag(TextResource::TOKEN_NO_SPACE))
					break;
				string s = t->GetText();
				if (s == ")" || s == "]" || s == "}")
					break;
			} else {
				if (t->TestFlag((TextResource::Flags)(TextResource::TOKEN_TITLECASE|TextResource::TOKEN_UPPERCASE))) {
					upperIndex = i;
					state = 1;
				}
				break;
			}
		}
	}
	bool eos_marker = false;
	bool space_seen = false;
	if (state == 1) {
		// found e.g. ,,Test
		// go through puctuations to the left and find a word
		for (int i = index-1; i >= 0; i--) {
			Token *t = tokens[i];
			if (!t->TestFlag(TextResource::TOKEN_NO_SPACE))
				space_seen = true;
			if (t->TestFlag(TextResource::TOKEN_PUNCT)) {
				if (t->TestFlag(TextResource::TOKEN_SENTENCE_START))
					break;
				string s = t->GetText();
				if (s == "." || s == "!" || s == "?")
					eos_marker = true;
			} else {
				// napr. "Helena
				Token *dot = tokens[i+1];
				if (t->TestFlag(TextResource::TOKEN_ABBR) && abbr_eos.find(t->GetText()) == abbr_eos.end() && t->TestFlag(TextResource::TOKEN_NO_SPACE) && dot->TestFlag(TextResource::TOKEN_PUNCT) && dot->GetText() == ".") {
					// reset start-of-sentence flag (abbreviation + dot)
					tokens[index]->ResetFlag((TextResource::Flags)(TextResource::TOKEN_SENTENCE_START|TextResource::TOKEN_PARAGRAPH_START));
				} else if (eos_marker) {
					if (!tokens[index]->TestFlag(TextResource::TOKEN_NUMERIC) || space_seen)
						tokens[index]->SetFlag(TextResource::TOKEN_SENTENCE_START);
				}
				break;
			}
		}
	}

	// deal with numbers: [+-]123[ .,]123
	int last = -1;
	if (current->TestFlag(TextResource::TOKEN_NUMERIC)) {
		last = index;
	} else if (current->TestFlag(TextResource::TOKEN_NO_SPACE) && tokens[index+1]->TestFlag(TextResource::TOKEN_NUMERIC)
		&& (current->GetText() == "+" || current->GetText() == "-")) {
		last = index + 1;
	}

	if (last >= 0) {
		int prev;
		do {
 			prev = last;
			while (last+1 < (int)tokens.size() && tokens[last+1]->TestFlag(TextResource::TOKEN_NUMERIC))
				last++;
			if (last+2 < (int)tokens.size() && tokens[last]->TestFlag(TextResource::TOKEN_NO_SPACE) && tokens[last+1]->TestFlag(TextResource::TOKEN_NO_SPACE) && tokens[last+2]->TestFlag(TextResource::TOKEN_NUMERIC) && (tokens[last+1]->GetText() == "." || tokens[last+1]->GetText() == ",")) {
				last += 2;
			}
		} while (last != prev);

		if (last > index) {
			// concatenate tokens index..last
			TextResource::Flags flags = TextResource::TOKEN_NONE;
			string s;
			for (int i = index; i <= last; i++) {
				s.append(tokens[i]->GetText());
				// copy paragraph & sentence starts
				if (tokens[i]->TestFlag(TextResource::TOKEN_PARAGRAPH_START))
					flags = (TextResource::Flags)(flags|TextResource::TOKEN_PARAGRAPH_START);
				if (tokens[i]->TestFlag(TextResource::TOKEN_SENTENCE_START))
					flags = (TextResource::Flags)(flags|TextResource::TOKEN_SENTENCE_START);
			}
			tokens[index]->SetText(s);
			tokens[index]->SetFlags((TextResource::Flags)(flags|tokens[last]->GetFlags()));
			tokens.erase(tokens.begin()+index+1, tokens.begin()+last+1);
			// need to restat as we consumed some tokens
			return true;
		}
	}

	// no need to restart (either not deleted tokens, or lookahead is
	// sufficient (for numbers)
	return false;
}
