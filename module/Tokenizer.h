/**
Tokenizer.la, simple, native
Tag TextResources sentences with Featurama tagger (first, we apply morphology
and then we run tagger).

Dependencies: libunistring

Parameters:
items			r/o	Total items processed
maxSentenceSize		init	Maximum sentence size, default is 200.
tokenizerLibrary	init	Language specific library.
markParagraphs		r/w	Empty line means end-of-paragraph.
*/

#ifndef _MODULES_TOKENIZER_H_
#define _MODULES_TOKENIZER_H_

#include <config.h>

#include <string>
#include <vector>
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

class Tokenizer : public Module {
public:
	Tokenizer(ObjectRegistry *objects, const char *id, int threadIndex);
	~Tokenizer();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	int maxSentenceSize;
	char *tokenizerLibrary;
	bool markParagraphs;	// empty line means new paragraph

	char *GetItems(const char *name);
	char *GetMaxSentenceSize(const char *name);
        void SetMaxSentenceSize(const char *name, const char *value);
	char *GetTokenizerLibrary(const char *name);
        void SetTokenizerLibrary(const char *name, const char *value);
	char *GetMarkParagraphs(const char *name);
        void SetMarkParagraphs(const char *name, const char *value);

	ObjectProperties<Tokenizer> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	Token *AcquireToken();
	void ReleaseToken(Token *token);
	void FlushSentence(int n);
	void AppendToken(Token *token);

	TextResource *tr;	// text resource we are currently working on
	int (*fixup)(std::vector<Token*> &tokens, int index);
	void (*finish)();

	int current;		// current token we consider
	int lookahead;		// how many tokens we look further while
				// considering the current token
	std::vector<Token*> tokens;
	std::vector<Token*> freeTokens;
};

inline Module::Type Tokenizer::GetType() {
	return SIMPLE;
}

inline char *Tokenizer::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool Tokenizer::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *Tokenizer::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
