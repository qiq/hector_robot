#include <config.h>

#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "TextResource.h"
#include "TextResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr TextResource::logger(log4cxx::Logger::getLogger("resources.TextResource"));
TextResourceInfo TextResource::resourceInfo;

TextResourceInfo::TextResourceInfo() {
	SetTypeId(15);
	SetTypeString("TextResource");
	SetTypeStringTerse("TR");
	SetObjectName("TextResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<TextResource> *ai;

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitInt32("id", &TextResource::GetId, &TextResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitInt32("status", &TextResource::GetStatus, &TextResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitString("text", &TextResource::GetText, &TextResource::SetText);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitString("textId", &TextResource::GetTextId, &TextResource::SetTextId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitArrayInt32("flags", &TextResource::GetFlags, &TextResource::SetFlags, &TextResource::ClearFlags, &TextResource::GetFlagsCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitArrayString("form", &TextResource::GetForm, &TextResource::SetForm, &TextResource::ClearForm, &TextResource::GetFormCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitArrayString("lemma", &TextResource::GetLemma, &TextResource::SetLemma, &TextResource::ClearLemma, &TextResource::GetLemmaCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitArrayString("posTag", &TextResource::GetPosTag, &TextResource::SetPosTag, &TextResource::ClearPosTag, &TextResource::GetPosTagCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitArrayInt32("head", &TextResource::GetHead, &TextResource::SetHead, &TextResource::ClearHead, &TextResource::GetHeadCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<TextResource>(GetTypeId());
	ai->InitArrayString("depRel", &TextResource::GetDepRel, &TextResource::SetDepRel, &TextResource::ClearDepRel, &TextResource::GetDepRelCount);
	l->push_back(ai);

	SetAttrInfoList(l);
}

void TextResource::DeleteWords(const vector<pair<int, int> > &indexLength) {
	vector<bool> delIndex(r.flags_size(), false);
	for (vector<pair<int, int> >::const_iterator iter = indexLength.begin(); iter != indexLength.end(); ++iter) {
		for (int i = iter->first; i < iter->first+iter->second; i++)
			delIndex[i] = true;
	}
	DeleteWords(delIndex);
}

void TextResource::DeleteWords(const vector<bool> &del) {
	google::protobuf::RepeatedField<google::protobuf::int32> *flags = r.mutable_flags();
	google::protobuf::RepeatedPtrField<string> *forms = r.mutable_form();
	google::protobuf::RepeatedPtrField<string> *lemmas = r.mutable_lemma();
	google::protobuf::RepeatedPtrField<string> *posTags = r.mutable_postag();
	google::protobuf::RepeatedField<google::protobuf::int32> *heads = r.mutable_head();
	google::protobuf::RepeatedPtrField<string> *depRels = r.mutable_deprel();
	int i = 0;
	int j = 0;
	while (j < (int)del.size()) {
		if (del[j]) {
			// just advance source pointer
			j++;
		} else {
			if (i != j) {
				// copy (swap elements)
				flags->SwapElements(i, j);
				if (j < forms->size())
					forms->SwapElements(i, j);
				if (j < lemmas->size())
					lemmas->SwapElements(i, j);
				if (j < posTags->size())
					posTags->SwapElements(i, j);
				if (j < heads->size())
					heads->SwapElements(i, j);
				if (j < depRels->size())
					depRels->SwapElements(i, j);
			}
			j++;
			i++;
		}
	}
	while (i < flags->size())
		flags->RemoveLast();
	while (i < forms->size())
		forms->RemoveLast();
	while (i < lemmas->size())
		lemmas->RemoveLast();
	while (i < posTags->size())
		posTags->RemoveLast();
	while (i < heads->size())
		heads->RemoveLast();
	while (i < depRels->size())
		depRels->RemoveLast();
}

string TextResource::ToString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[%s %d %d] ", resourceInfo.GetTypeStringTerse(), GetId(), GetStatus());
	s += buf;
	if (r.has_text_id() && r.text_id().size() > 0) {
		s += "TextId: ";
		s += r.text_id();
		s += "\n";
	}
	if (r.has_language()) {
		s += "Lang: ";
		s += r.language().size() > 0 ? r.language() : "unknown";
		s += "\n";
	}
	if (r.has_text() && r.text().size() > 0) {
		s += "Text: ";
		s += r.text();
		s += "\n";
	}
	for (int i = 0; i < r.flags_size(); i++) {
		snprintf(buf, sizeof(buf), "%s%d", i > 0 ? " " : "", (int)r.flags(i));
		s += buf;
		if (r.form_size() > i) {
			s += "/";
			s += r.form(i);
		}
		if (r.lemma_size() > i) {
			s += "/";
			s += r.lemma(i);
		}
		if (r.postag_size() > i) {
			s += "/";
			s += r.postag(i);
		}
		if (r.head_size() > i) {
			s += "/";
			snprintf(buf, sizeof(buf), "%d", r.head(i));
			s += buf;
		}
		if (r.deprel_size() > i) {
			s += "/";
			s += r.deprel(i);
		}
	}

	return s;
}

#else

extern "C" Resource* hector_resource_create() {
	return (Resource*)new TextResource();
}

#endif
