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
#include "Tokenize.h"
#include "TextResource.h"

using namespace std;

Tokenize::Tokenize(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxWordLength = 100;
	maxSentenceLength = 200;
	tokenizerLibrary = NULL;
	markParagraphs = false;

	props = new ObjectProperties<Tokenize>(this);
	props->Add("items", &Tokenize::GetItems);
	props->Add("maxSentenceLength", &Tokenize::GetMaxSentenceLength, &Tokenize::SetMaxSentenceLength, true);
	props->Add("maxWordLength", &Tokenize::GetMaxWordLength, &Tokenize::SetMaxWordLength);
	props->Add("tokenizerLibrary", &Tokenize::GetTokenizeLibrary, &Tokenize::SetTokenizeLibrary, true);
	props->Add("markParagraphs", &Tokenize::GetMarkParagraphs, &Tokenize::SetMarkParagraphs);

	fixup = NULL;
	finish = NULL;
	current = 0;
	lookahead = 2;
}

Tokenize::~Tokenize() {
	if (finish)
		(*finish)();

	free(tokenizerLibrary);

	for (vector<Token*>::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
		delete *iter;
	for (vector<Token*>::iterator iter = freeTokens.begin(); iter != freeTokens.end(); ++iter)
		delete *iter;
	
	delete props;
}

char *Tokenize::GetItems(const char *name) {
	return int2str(items);
}

char *Tokenize::GetMaxSentenceLength(const char *name) {
	return int2str(maxSentenceLength);
}

void Tokenize::SetMaxSentenceLength(const char *name, const char *value) {
	maxSentenceLength = str2int(value);
}

char *Tokenize::GetMaxWordLength(const char *name) {
	return int2str(maxWordLength);
}

void Tokenize::SetMaxWordLength(const char *name, const char *value) {
	maxWordLength = str2int(value);
}

char *Tokenize::GetTokenizeLibrary(const char *name) {
	return strdup(tokenizerLibrary);
}

void Tokenize::SetTokenizeLibrary(const char *name, const char *value) {
	free(tokenizerLibrary);
	tokenizerLibrary = strdup(value);
}

char *Tokenize::GetMarkParagraphs(const char *name) {
	return bool2str(markParagraphs);
}

void Tokenize::SetMarkParagraphs(const char *name, const char *value) {
	markParagraphs = str2bool(value);
}

bool Tokenize::Init(vector<pair<string, string> > *params) {
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
		fixup = (bool(*)(std::vector<Token*> &tokens, int index))LibraryLoader::LoadLibrary(tokenizerLibrary, "fixup", false);
		if (!fixup) {
			LOG_ERROR(this, "Invalid library: " << tokenizerLibrary << " (no fixup())");
			return false;
		}
		finish = (void (*)())LibraryLoader::LoadLibrary(tokenizerLibrary, "fixup_finish", false);
	}

	return true;
}

Token *Tokenize::AcquireToken() {
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

void Tokenize::ReleaseToken(Token *token) {
	freeTokens.push_back(token);
}

void Tokenize::FlushSentence(int n) {
	int base = tr->GetFormCount();
	for (int i = 0; i < n; i++) {
		string form = tokens[i]->GetText();
		// we have to reduce token length again, because new tokens
		// were created concatenating the successing ones
		if ((int)form.length() > maxWordLength) {
			const uint8_t *s = (uint8_t*)form.c_str();
			const uint8_t *end = s;
			ucs4_t c;
			for (int j = 0; end && j < maxWordLength; j++)
				end = u8_next(&c, end);
			if (end)
				form.resize(end-(uint8_t*)s);
		}
		tr->SetForm(base+i, form);
		tr->SetFlags(base+i, tokens[i]->GetFlags());
		freeTokens.push_back(tokens[i]);
	}
	for (int i = n; i < (int)tokens.size(); i++)
		tokens[i-n] = tokens[i];
	tokens.resize(tokens.size()-n);
}

void Tokenize::AppendToken(Token *token) {
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
	if (current >= maxSentenceLength)
		tokens[current]->SetFlag(TextResource::TOKEN_SENTENCE_START);

	// call fixup
	if (fixup && (int)tokens.size() >= current+lookahead+1) {
		bool restart = (*fixup)(tokens, current);
		// we do not advance, as some tokens were consumed and fixup
		// indicated that it needs to check again
		if (restart && (int)tokens.size() < current+lookahead+1)
			return;
	}

	// test process output (possible sentence start)
	if (current > 0 && tokens[current]->TestFlag(TextResource::TOKEN_SENTENCE_START)) {
		FlushSentence(current);
		current = 1;
	} else {
		current++;
	}
}

#define ISALNUM(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_L|UC_CATEGORY_MASK_N)
#define ISLOWERCASE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Ll)
#define ISUPPERCASE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Lu)
#define ISNUMERIC(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_N)
#define ISPUNCT(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_P|UC_CATEGORY_MASK_S)
#define ISSPACE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Z|UC_CATEGORY_MASK_C)
#define ISNEWLINE(c) uc_is_general_category_withtable(c, UC_CATEGORY_MASK_Zl|UC_CATEGORY_MASK_C)

enum state_type {
	ALNUM = 1,
	PUNCT = 2,
	SPACE = 3,
};

Resource *Tokenize::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	tr = static_cast<TextResource*>(resource);

	string text = tr->GetText();

	// main tokenizer routine
	const uint8_t *u8 = (uint8_t*)text.c_str();
	const uint8_t *next = u8;
	ucs4_t c;
	const uint8_t *begin = NULL;
	const uint8_t *end = NULL;
	int chars = 0;
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
				// only accept maxWordLength per token
				if (++chars <= maxWordLength)
					end = next;
			} else if (ISPUNCT(c)) {
				// text token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(begin, end-begin);
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
				begin = u8;
				end = next;
				chars = 1;
				newline = 0;
			} else if (ISSPACE(c)) {
				// text token, no_space = 0
				Token *t = AcquireToken();
				t->SetText(begin, end-begin);
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
				begin = NULL;
				end = NULL;
				chars = 0;
				newline = ISNEWLINE(c) ? 1 : 0;
			}
			break;
		case PUNCT:
			if (ISALNUM(c)) {
				// punct token, no_space = 1
				Token *t = AcquireToken();
				t->SetText(begin, end-begin);
				t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PUNCT|TextResource::TOKEN_NO_SPACE));
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = ALNUM;
				begin = u8;
				end = next;
				chars = 1;
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
				t->SetText(begin, end-begin);
				t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PUNCT|TextResource::TOKEN_NO_SPACE));
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				begin = u8;
				end = next;
				chars = 1;
				newline = 0;
			} else if (ISSPACE(c)) {
				// punct token, no_space = 0
				Token *t = AcquireToken();
				t->SetText(begin, end-begin);
				t->SetFlag(TextResource::TOKEN_PUNCT);
				if (markParagraphs && newline >= 2)
					t->SetFlag((TextResource::Flags)(TextResource::TOKEN_PARAGRAPH_START|TextResource::TOKEN_SENTENCE_START));
				AppendToken(t);

				state = SPACE;
				begin = NULL;
				end = NULL;
				chars = 0;
				newline = ISNEWLINE(c) ? 1 : 0;
			}
			break;
		case SPACE:
			if (ISALNUM(c)) {
				state = ALNUM;
				begin = u8;
				end = next;
				chars = 1;
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
				begin = u8;
				end = next;
				chars = 1;
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
			t->SetText(begin, end-begin);
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
			t->SetText(begin, end-begin);
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
	return new Tokenize(objects, id, threadIndex);
}
