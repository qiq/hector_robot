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
