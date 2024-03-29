/**
Tokenize.la, simple, native
Tag TextResources sentences with Featurama tagger (first, we apply morphology
and then we run tagger).

Dependencies: libunistring

Parameters:
items			r/o	Total items processed
maxSentenceLength	init	Maximum sentence lengths in tokens, default is 200.
maxWordLength		r/w	Maximum length of a word (in chars), default if 100.
tokenizerLibrary	init	Language specific library.
markParagraphs		r/w	Empty line means end-of-paragraph.
*/

#ifndef _MODULES_TOKENIZE_H_
#define _MODULES_TOKENIZE_H_

#include <config.h>

#include <string>
#include <vector>
#include <tr1/unordered_set>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "TextResource.h"

class Token {
public:
	Token(): flags(TextResource::TOKEN_NONE) {};
	~Token() {};

	void Clear();
	std::string &GetText();
	void SetText(std::string &text);
	void SetText(const uint8_t *text, int len);
	bool TestFlag(TextResource::Flags flag);
	void SetFlag(TextResource::Flags flag);
	void ResetFlag(TextResource::Flags flag);
	TextResource::Flags GetFlags();
	void SetFlags(TextResource::Flags flags);
private:
	std::string text;
	TextResource::Flags flags;
};

void Token::Clear() {
	text.clear();
	flags = TextResource::TOKEN_NONE;
}

std::string &Token::GetText() {
	return text;
}

void Token::SetText(std::string &text) {
	this->text = text;
}

void Token::SetText(const uint8_t *text, int len) {
	this->text.assign((const char*)text, len);
}

bool Token::TestFlag(TextResource::Flags flag) {
	return flags & flag;
}

void Token::SetFlag(TextResource::Flags flag) {
	flags = (TextResource::Flags)(flags|flag);
}

void Token::ResetFlag(TextResource::Flags flag) {
	flags = (TextResource::Flags)(flags & (flag ^ 0xFFFFFFFF));
}

TextResource::Flags Token::GetFlags() {
	return flags;
}
void Token::SetFlags(TextResource::Flags flags) {
	this->flags = flags;
}

class Tokenize : public Module {
public:
	Tokenize(ObjectRegistry *objects, const char *id, int threadIndex);
	~Tokenize();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	int maxSentenceLength;
	int maxWordLength;
	char *tokenizerLibrary;
	bool markParagraphs;	// empty line means new paragraph

	char *GetItems(const char *name);
	char *GetMaxSentenceLength(const char *name);
        void SetMaxSentenceLength(const char *name, const char *value);
	char *GetMaxWordLength(const char *name);
        void SetMaxWordLength(const char *name, const char *value);
	char *GetTokenizeLibrary(const char *name);
        void SetTokenizeLibrary(const char *name, const char *value);
	char *GetMarkParagraphs(const char *name);
        void SetMarkParagraphs(const char *name, const char *value);

	ObjectProperties<Tokenize> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	Token *AcquireToken();
	void ReleaseToken(Token *token);
	void FlushSentence(int n);
	void AppendToken(Token *token);

	TextResource *tr;	// text resource we are currently working on
	bool (*fixup)(std::vector<Token*> &tokens, int index);
	void (*finish)();

	int current;		// current token we consider
	int lookahead;		// how many tokens we look further while
				// considering the current token
	std::vector<Token*> tokens;
	std::vector<Token*> freeTokens;
	std::tr1::unordered_set<std::string> eosToken;
};

inline Module::Type Tokenize::GetType() {
	return SIMPLE;
}

inline char *Tokenize::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool Tokenize::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *Tokenize::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
