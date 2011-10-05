/**
 * Text resource used in various NLP processing modules. It contains original
 * text, tokenized words and sentences, lemmas, POS-tags, head and deprel from
 * the parser.
 */

#ifndef _TEXT_RESOURCE_H_
#define _TEXT_RESOURCE_H_

#include <config.h>

#include <string>
#include <utility>
#include <vector>
#include <log4cxx/logger.h>
#include "common.h"
#include "Resource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "TextResource.pb.h"

class ResourceAttrInfo;

class TextResourceInfo : public ResourceInfo {
public:
	TextResourceInfo();
};

class TextResource : public Resource {
public:
	enum Flags {
		TOKEN_NONE = 0,
		TOKEN_PARAGRAPH_START = 1,
		TOKEN_SENTENCE_START = 2,
		TOKEN_NO_SPACE = 4,
		TOKEN_ABBR = 8,
		TOKEN_PUNCT = 16,
		TOKEN_TITLECASE = 32,
		TOKEN_UPPERCASE = 64,
		TOKEN_NUMERIC = 128,
		TOKEN_UNRECOGNIZED = 256,
	};

	TextResource();
	TextResource(const TextResource &wr);
	~TextResource();
	// create copy of a resource
	Resource *Clone();
	void Clear();
	// save and restore resource
	bool Serialize(ResourceOutputStream &output);
	bool Deserialize(ResourceInputStream &input, bool headerOnly);
	bool Skip(ResourceInputStream &input);
	// used by queues in case there is limit on queue size
	int GetSize();
	// get info about this resource
	ResourceInfo *GetResourceInfo();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string ToString(Object::LogLevel = Object::INFO);
	// delete words (index + length pairs or just indices to be deleted)
	void DeleteWords(const std::vector<std::pair<int, int> > &indexLength);
	void DeleteWords(const std::vector<bool> &del);

	// TextResource-specific
	void SetText(const std::string &text);
	const std::string GetText();
	void ClearText();
	void SetTextId(const std::string &text);
	const std::string GetTextId();
	void ClearTextId();
	void SetLanguage(const std::string &text);
	const std::string GetLanguage();
	void ClearLanguage();
	void SetFlags(int index, int flags);
	int GetFlags(int index);
	void ClearFlags();
	int GetFlagsCount();
	void SetForm(int index, const std::string &text);
	const std::string GetForm(int index);
	void ClearForm();
	int GetFormCount();
	void SetLemma(int index, const std::string &text);
	const std::string GetLemma(int index);
	void ClearLemma();
	int GetLemmaCount();
	void SetPosTag(int index, const std::string &text);
	const std::string GetPosTag(int index);
	void ClearPosTag();
	int GetPosTagCount();
	void SetHead(int index, int flags);
	int GetHead(int index);
	void ClearHead();
	int GetHeadCount();
	void SetDepRel(int index, const std::string &text);
	const std::string GetDepRel(int index);
	void ClearDepRel();
	int GetDepRelCount();

	static bool IsInstance(Resource *resource);

protected:
	// saved properties
	hector::resources::TextResource r;

	static TextResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

inline TextResource::TextResource() {
}

inline TextResource::TextResource(const TextResource &wr) : Resource(wr), r(wr.r) {
}

inline TextResource::~TextResource() {
}

inline Resource *TextResource::Clone() {
	return new TextResource(*this);
}

inline void TextResource::Clear() {
	Resource::Clear();
	r.Clear();
}

inline bool TextResource::Serialize(ResourceOutputStream &output) {
	output.SerializeMessage(r);
	return true;
}

inline bool TextResource::Deserialize(ResourceInputStream &input, bool headerOnly) {
	if (headerOnly)
		return true;
	return input.ParseMessage(r);
}

inline bool TextResource::Skip(ResourceInputStream &input) {
	return input.ParseMessage(r, 0, true);
}

inline int TextResource::GetSize() {
	if (r.has_text() && r.text().size() > 0) {
		return r.text().size();
	} else {
		return r.flags().size();
	}
}

inline ResourceInfo *TextResource::GetResourceInfo() {
	return &TextResource::resourceInfo;
}

inline void TextResource::SetText(const std::string &text) {
	r.set_text(text);
}

inline const std::string TextResource::GetText() {
	return r.text();
}

inline void TextResource::ClearText() {
	r.clear_text();
}

inline void TextResource::SetTextId(const std::string &textId) {
	r.set_text_id(textId);
}

inline const std::string TextResource::GetTextId() {
	return r.text_id();
}

inline void TextResource::ClearTextId() {
	r.clear_text_id();
}

inline void TextResource::SetLanguage(const std::string &language) {
	r.set_language(language);
}

inline const std::string TextResource::GetLanguage() {
	return r.language();
}

inline void TextResource::ClearLanguage() {
	r.clear_language();
}

inline void TextResource::SetFlags(int index, int flags) {
	while (index >= r.flags_size())
		r.add_flags(TOKEN_NONE);
	r.set_flags(index, flags);
}

inline int TextResource::GetFlags(int index) {
	return (int)r.flags(index);
}

inline void TextResource::ClearFlags() {
	r.clear_flags();
}

inline int TextResource::GetFlagsCount() {
	return r.flags_size();
}

inline void TextResource::SetForm(int index, const std::string &form) {
	while (index >= r.form_size())
		r.add_form("");
	r.set_form(index, form);
}

inline const std::string TextResource::GetForm(int index) {
	return r.form(index);
}

inline void TextResource::ClearForm() {
	r.clear_form();
}

inline int TextResource::GetFormCount() {
	return r.form_size();
}

inline void TextResource::SetLemma(int index, const std::string &lemma) {
	while (index >= r.lemma_size())
		r.add_lemma("");
	r.set_lemma(index, lemma);
}

inline const std::string TextResource::GetLemma(int index) {
	return r.lemma(index);
}

inline void TextResource::ClearLemma() {
	r.clear_lemma();
}

inline int TextResource::GetLemmaCount() {
	return r.lemma_size();
}

inline void TextResource::SetPosTag(int index, const std::string &postag) {
	while (index >= r.postag_size())
		r.add_postag("");
	r.set_postag(index, postag);
}

inline const std::string TextResource::GetPosTag(int index) {
	return r.postag(index);
}

inline void TextResource::ClearPosTag() {
	r.clear_postag();
}

inline int TextResource::GetPosTagCount() {
	return r.postag_size();
}

inline void TextResource::SetHead(int index, int head) {
	while (index >= r.head_size())
		r.add_head(0);
	r.set_head(index, head);
}

inline int TextResource::GetHead(int index) {
	return (int)r.head(index);
}

inline void TextResource::ClearHead() {
	r.clear_head();
}

inline int TextResource::GetHeadCount() {
	return r.head_size();
}

inline void TextResource::SetDepRel(int index, const std::string &deprel) {
	while (index >= r.deprel_size())
		r.add_deprel("");
	r.set_deprel(index, deprel);
}

inline const std::string TextResource::GetDepRel(int index) {
	return r.deprel(index);
}

inline void TextResource::ClearDepRel() {
	r.clear_deprel();
}

inline int TextResource::GetDepRelCount() {
	return r.deprel_size();
}

inline bool TextResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
