/**
 */
#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistr.h>
#include <unictype.h>
#include "LibraryLoader.h"
#include "robot_common.h"
#include "Tokenizer.h"
#include "TextResource.h"

using namespace std;

Tokenizer::Tokenizer(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxSentenceSize = 200;
	tokenizerLibrary = NULL;
	markParagraphs = false;

	props = new ObjectProperties<Tokenizer>(this);
	props->Add("items", &Tokenizer::GetItems);
	props->Add("maxSentenceSize", &Tokenizer::GetMaxSentenceSize, &Tokenizer::SetMaxSentenceSize, true);
	props->Add("tokenizerLibrary", &Tokenizer::GetTokenizerLibrary, &Tokenizer::SetTokenizerLibrary, true);
	props->Add("markParagraphs", &Tokenizer::GetMarkParagraphs, &Tokenizer::SetMarkParagraphs);

	fixup = NULL;
	finish = NULL;
	current = 0;
	lookahead = 2;
}

Tokenizer::~Tokenizer() {
	if (finish)
		(*finish)();

	free(tokenizerLibrary);

	for (vector<Token*>::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
		delete *iter;
	for (vector<Token*>::iterator iter = freeTokens.begin(); iter != freeTokens.end(); ++iter)
		delete *iter;
	
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
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	// load library
	if (tokenizerLibrary) {
		// call fixup_init()
		int (*init)() = (int (*)())LibraryLoader::LoadLibrary(tokenizerLibrary, "fixup_init", false);
		if (!init) {
			LOG_ERROR(this, "Invalid library: " << tokenizerLibrary << " (no fixup_init())");
			return false;
		}
		lookahead = (*init)();
		if (lookahead < 2)
			lookahead = 2;
		fixup = (void(*)(std::vector<Token*> &tokens, int index))LibraryLoader::LoadLibrary(tokenizerLibrary, "fixup", false);
		if (!fixup) {
			LOG_ERROR(this, "Invalid library: " << tokenizerLibrary << " (no fixup())");
			return false;
		}
		finish = (void (*)())LibraryLoader::LoadLibrary(tokenizerLibrary, "fixup_finish", false);
	}

	return true;
}

Token *Tokenizer::AcquireToken() {
	Token *result;
	if (freeTokens.size() > 0) {
		result = freeTokens.back();
		result->Clear();
		freeTokens.pop_back();
	} else {
		result = new Token();
	}
	return result; 
}

void Tokenizer::ReleaseToken(Token *token) {
	freeTokens.push_back(token);
}

void Tokenizer::FlushSentence(int n) {
	int base = tr->GetFormCount();
	for (int i = 0; i < n; i++) {
		tr->SetForm(base+i, tokens[i]->GetText());
		tr->SetFlags(base+i, tokens[i]->GetFlags());
		freeTokens.push_back(tokens[i]);
	}
	for (int i = n; i < (int)tokens.size(); i++)
		tokens[i-n] = tokens[i];
	tokens.resize(tokens.size()-n);
}

void Tokenizer::AppendToken(Token *token) {
	if (token) {
		if (tokens.size() == 0)
			token->SetFlag(TextResource::TOKEN_SENTENCE_START);
		tokens.push_back(token);
	}
	// not enough tokens
	if ((int)tokens.size() <= current+lookahead)
		return;

	// is current token first in a sentence? (segmentation)
	if (current > 0 && tokens[current]->TestFlag(TextResource::TOKEN_TITLECASE) && tokens[current-1]->TestFlag(TextResource::TOKEN_PUNCT)) {
		string prev = tokens[current-1]->GetText();
		if (prev == "." || prev == "?" || prev == "!")
			tokens[current]->SetFlag(TextResource::TOKEN_SENTENCE_START);
	}
	if (current >= maxSentenceSize)
		tokens[current]->SetFlag(TextResource::TOKEN_SENTENCE_START);

	// call fixup
	if (fixup && (int)tokens.size() >= current+lookahead+1)
		(*fixup)(tokens, current);

	// test process output (possible sentence start)
	if (current > 0 && tokens[current]->TestFlag(TextResource::TOKEN_SENTENCE_START)) {
		FlushSentence(current);
		current = 1;
	} else {
		current++;
	}
}

#define ISALNUM(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_L|UC_CATEGORY_MASK_N|UC_CATEGORY_MASK_S)
#define ISLOWERCASE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Ll)
#define ISUPPERCASE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Lu)
#define ISNUMERIC(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_N)
#define ISPUNCT(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_P)
#define ISSPACE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Z|UC_CATEGORY_MASK_C)
#define ISNEWLINE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Zl|UC_CATEGORY_MASK_C)

enum state_type {
	ALNUM = 1,
	PUNCT = 2,
	SPACE = 3,
};

Resource *Tokenizer::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	tr = static_cast<TextResource*>(resource);

	string text = tr->GetText();

	// main tokenizer routine
	const uint8_t *u8 = (uint8_t*)text.c_str();
	const uint8_t *next = u8;
	ucs4_t c;
	const uint8_t *start = NULL;
	bool titlecase = false;
	bool uppercase = false;
	bool numeric = false;
	int newline = 2;
	state_type state = SPACE;
	current = 0;
	while (next) {
		u8 = next;
		next = u8_next(&c, next);
		switch (state) {
		case ALNUM:
			if (ISALNUM(c)) {
				if (ISLOWERCASE(c)) {
					uppercase = false;
				} else if (ISNUMERIC(c)) {
					numeric = true;
				}
			} else if (ISPUNCT(c)) {
				// text token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag(TextResource::TOKEN_NO_SPACE);
				if (titlecase)
					t->SetFlag(TextResource::TOKEN_TITLECASE);
				if (uppercase)
					t->SetFlag(TextResource::TOKEN_UPPERCASE);
				if (numeric)
					t->SetFlag(TextResource::TOKEN_NUMERIC);
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = PUNCT;
				start = u8;
				newline = 0;
			} else if (ISSPACE(c)) {
				// text token, no_space = 0
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				if (titlecase)
					t->SetFlag(TextResource::TOKEN_TITLECASE);
				if (uppercase)
					t->SetFlag(TextResource::TOKEN_UPPERCASE);
				if (numeric)
					t->SetFlag(TextResource::TOKEN_NUMERIC);
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = SPACE;
				start = NULL;
				newline = ISNEWLINE(c) ? 1 : 0;
			}
			break;
		case PUNCT:
			if (ISALNUM(c)) {
				// punct token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PUNCT|TextResource::TOKEN_NO_SPACE));
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = ALNUM;
				start = u8;
				titlecase = false;
				uppercase = false;
				numeric = false;
				if (ISUPPERCASE(c)) {
					titlecase = true;
					uppercase = true;
				} else if (ISNUMERIC(c)) {
					numeric = true;
				}
				newline = 0;
			} else if (ISPUNCT(c)) {
				// punct token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PUNCT|TextResource::TOKEN_NO_SPACE));
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				start = u8;
				newline = 0;
			} else if (ISSPACE(c)) {
				// punct token, no_space = 0
				Token *t = AcquireToken();
				t->SetText(start, u8-start);
				t->SetFlag(TextResource::TOKEN_PUNCT);
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = SPACE;
				start = NULL;
				newline = ISNEWLINE(c) ? 1 : 0;
			}
			break;
		case SPACE:
			if (ISALNUM(c)) {
				state = ALNUM;
				start = u8;
				numeric = false;
				titlecase = false;
				uppercase = false;
				if (ISUPPERCASE(c)) {
					titlecase = true;
					uppercase = true;
				} else if (ISNUMERIC(c)) {
					numeric = true;
				}
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

	switch (state) {
		case ALNUM: {
			// text token, no_space = 0
			Token *t = AcquireToken();
			t->SetText(start, u8-start);
			if (titlecase)
				t->SetFlag(TextResource::TOKEN_TITLECASE);
			if (uppercase)
				t->SetFlag(TextResource::TOKEN_UPPERCASE);
			if (numeric)
				t->SetFlag(TextResource::TOKEN_NUMERIC);
			if (markParagraphs && newline >= 2)
				t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
			AppendToken(t);
			}
			break;
		case PUNCT: {
			// punct token, no_space = 1
			Token *t = AcquireToken();
			t->SetText(start, u8-start);
			t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PUNCT));
			if (markParagraphs && newline >= 2)
				t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
			AppendToken(t);
			}
			break;
		case SPACE:
		default:
			break;
	}

	// call fixup for the rest of tokens (append empty tokens)
	for (int i = 0; i < lookahead; i++) {
		Token *t = AcquireToken();
		t->SetText(NULL, 0);
		t->SetFlags(TextResource::TOKEN_NONE);
		AppendToken(t);
	}
	FlushSentence(current);

	// release all tokens
	while (tokens.size() > 0) {
		freeTokens.push_back(tokens.back());
		tokens.pop_back();
	}

	tr->ClearText();

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Tokenizer(objects, id, threadIndex);
}
