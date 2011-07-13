/**
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include <unistr.h>
#include <unictype.h>
#include "robot_common.h"
#include "Tokenizer.h"
#include "TextResource.h"

using namespace std;

Tokenizer::Tokenizer(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxSentenceSize = 200;
	tokenizerLibrary = NULL;

	props = new ObjectProperties<Tokenizer>(this);
	props->Add("items", &Tokenizer::GetItems);
	props->Add("maxSentenceSize", &Tokenizer::GetMaxSentenceSize, &Tokenizer::SetMaxSentenceSize, true);
	props->Add("tokenizerLibrary", &Tokenizer::GetTokenizerLibrary, &Tokenizer::SetTokenizerLibrary, true);

	lookahead = 2;
}

Tokenizer::~Tokenizer() {
	free(tokenizerLibrary);
	
	delete props;
}

char *Tokenizer::GetItems(const char *name) {
	return int2str(items);
}

char *Tokenizer::GetMaxSentenceSize(const char *name) {
	return int2str(maxSentenceSize);
}

void Tokenizer::SetMaxSentenceSize(const char *name, const char *value) {
	maxSentenceSize = str2int(value);
}

char *Tokenizer::GetTokenizerLibrary(const char *name) {
	return strdup(tokenizerLibrary);
}

void Tokenizer::SetTokenizerLibrary(const char *name, const char *value) {
	free(tokenizerLibrary);
	tokenizerLibrary = strdup(value);
}

char *Tokenizer::GetMarkParagraphs(const char *name) {
	return bool2str(markParagraphs);
}

void Tokenizer::SetMarkParagraphs(const char *name, const char *value) {
	markParagraphs = str2bool(value);
}

bool Tokenizer::Init(vector<pair<string, string> > *params) {
	int result;

	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	// load library
	if (tokenizerLibrary) {
		// call fixup_init()
		// initialize lookahead
		// TODO
		LOG_ERROR(this, "xxx");
		return false;
	}

	return true;
}

Token *Tokenizer::AcquireToken() {
	Token *result;
	if (free.size() > 0) {
		result = free.back();
		free.pop_back();
	} else {
		result = new Token();
	}
	return result; 
}

void Tokenizer::ReleaseToken(Token *token) {
	free.push_back(token);
}

/*
void fixup(vector<Token*> &tokens, int index) {
	tokens[index]->SetFlag(hector::resource::TOKEN_SENTENCE_START);
}
*/

void Tokenizer::FlushSentence(int n) {
	int base = tr->GetFormCount();
	for (int i = 0; i < n; i++) {
		tr->SetForm(base+i, tokens[i]->GetText());
		tr->SetFlags(base+i, tokens[i]->GetFlags());
		free.push_back(tokens[i]);
	}
	for (int i = n; i < tokens.size(); i++)
		tokens[i-n] = tokens[i];
	tokens.resize(tokens.size()-n);
}

void Tokenizer::AppendToken(Token *token) {
	if (token)
		tokens.push_back(token);

	// is current token first in a sentence? (segmentation)
	if (current == 0) {
		tokens[current]->SetFlag(hector::resources::TOKEN_SENTENCE_START);
	} else if (tokens[current-1]->TestFlag(hector::resources::TOKEN_PUNCT)) {
		string text = tokens[current-1]->GetText();
		if (text == "." || text == "?" || text == "!")
			tokens[current]->SetFlag(hector::resources::TOKEN_SENTENCE_START);
	}

	// call fixup
	if (fixup && tokens.size() >= current+lookahead+1)
		(*fixup)(tokens, current);

	// test process output (possible sentence start)
	if (current > 0 && tokens[current]->TestFlag(hector::resources::TOKEN_SENTENCE_START)) {
		FlushSentence(current);
		current = 0;
	} else {
		current++;
	}
}

#define ISALNUM(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_L|UC_CATEGORY_MASK_N|UC_CATEGORY_MASK_S)
#define ISLOWERCASE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Ll)
#define ISUPPERCASE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Lu)
#define ISNUMERIC(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_N)
#define ISPUNCT(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_P)
#define ISSPACE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Z)
#define ISNEWLINE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Zl)

enum state_type {
	ALNUM = 1,
	PUNCT = 2,
	SPACE = 3,
};

Resource *Tokenizer::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	string text = tr->GetText();

	// main tokenizer routine
	const uint8_t *u8 = (uint8_t*)text.c_str();
	ucs4_t c;
	const uint8_t *start = NULL;
	bool titlecase = true;
	bool uppercase = true;
	bool numeric = false;
	bool newline = false;
	state_type state = SPACE;
	while (u8) {
		const uint8_t *prev = u8;
		u8 = u8_next(&c, u8);
		switch (state) {
		case ALNUM:
			if (ISALNUM(c)) {
				if (ISLOWERCASE(c)) {
					uppercase = false;
					if (prev == start)
						titlecase = false;
				} else if (ISUPPERCASE(c)) {
					if (prev != start)
						titlecase = false;
				} else if (ISNUMERIC(c)) {
					numeric = true;
				}
			} else if (ISPUNCT(c)) {
				// text token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag(hector::resources::TOKEN_NO_SPACE);
				if (titlecase)
					t->SetFlag(hector::resources::TOKEN_TITLECASE);
				if (uppercase)
					t->SetFlag(hector::resources::TOKEN_UPPERCASE);
				if (numeric)
					t->SetFlag(hector::resources::TOKEN_NUMERIC);
				if (markParagraphs && newline >= 2)
					t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PARAGRAPH_START|hector::resources::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = PUNCT;
				start = u8;
			} else if (ISSPACE(c)) {
				// text token, no_space = 0
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				if (titlecase)
					t->SetFlag(hector::resources::TOKEN_TITLECASE);
				if (uppercase)
					t->SetFlag(hector::resources::TOKEN_UPPERCASE);
				if (numeric)
					t->SetFlag(hector::resources::TOKEN_NUMERIC);
				if (markParagraphs && newline >= 2)
					t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PARAGRAPH_START|hector::resources::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = SPACE;
				start = NULL;
				newline = 0;
			}
			break;
		case PUNCT:
			if (ISALNUM(c)) {
				// punct token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PUNCT|hector::resources::TOKEN_NO_SPACE));
				if (markParagraphs && newline >= 2)
					t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PARAGRAPH_START|hector::resources::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = ALNUM;
				start = u8;
				titlecase = true;
				uppercase = true;
				numeric = false;
			} else if (ISPUNCT(c)) {
				// punct token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PUNCT|hector::resources::TOKEN_NO_SPACE));
				if (markParagraphs && newline >= 2)
					t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PARAGRAPH_START|hector::resources::TOKEN_SENTENCE_START));
				AppendToken(t);

				start = u8;
			} else if (ISSPACE(c)) {
				// punct token, no_space = 0
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag(hector::resources::TOKEN_PUNCT);
				if (markParagraphs && newline >= 2)
					t->SetFlag((hector::resources::Flags)(hector::resources::TOKEN_PARAGRAPH_START|hector::resources::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = SPACE;
				start = NULL;
				newline = 0;
			}
			break;
		case SPACE:
			if (ISALNUM(c)) {
				state = ALNUM;
				start = u8;
				titlecase = true;
				uppercase = true;
				numeric = false;
			} else if (ISPUNCT(c)) {
				state = PUNCT;
				start = u8;
			} else if (ISNEWLINE(c)) {
				newline++;
			}
			break;
		default:
			break;
		}
	}

	// TODO: more fixups and flush

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Tokenizer(objects, id, threadIndex);
}
