/**
 */
#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "robot_common.h"
#include "MarkerResource.h"
#include "TextResourceRead.h"
#include "TextResource.h"

using namespace std;

TextResourceRead::TextResourceRead(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	horizontal = false;
	filename = NULL;

	props = new ObjectProperties<TextResourceRead>(this);
	props->Add("items", &TextResourceRead::GetItems);
	props->Add("horizontal", &TextResourceRead::GetHorizontal, &TextResourceRead::SetHorizontal);
	props->Add("filename", &TextResourceRead::GetFilename, &TextResourceRead::SetFilename, true);

	textResourceTypeId = -1;
	ifs = NULL;
}

TextResourceRead::~TextResourceRead() {
	if (ifs) {
		ifs->close();
		delete ifs;
	}
	free(filename);

	delete props;
}

char *TextResourceRead::GetItems(const char *name) {
	return int2str(items);
}

char *TextResourceRead::GetHorizontal(const char *name) {
	return bool2str(horizontal);
}

void TextResourceRead::SetHorizontal(const char *name, const char *value) {
	horizontal = str2bool(value);
}

char *TextResourceRead::GetFilename(const char *name) {
	return strdup(filename);
}

void TextResourceRead::SetFilename(const char *name, const char *value) {
	free(filename);
	filename = strdup(value);
}

bool TextResourceRead::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	// open output file
	if (!filename) {
		LOG_ERROR(this, "filename not defined");
		return false;
	}

	ifs = new ifstream(filename);
	if (!ifs->is_open()) {
		LOG_ERROR(this, "Cannot open file: " << filename);
		return false;
	}

	textResourceTypeId = Resource::GetRegistry()->NameToId("TextResource");
	if (textResourceTypeId <= 0) {
		LOG_ERROR(this, "Unknown resource type: TextResource");
		return false;
	}

	return true;
}

string GetDocId(string &s) {
	if (!s.compare(0, 5, "<doc ")) {
		size_t idb = s.find("id=\"", 5);
		if (idb != string::npos) {
			idb += 4;
			size_t ide = s.find("\"", idb);
			if (ide != string::npos)
				return s.substr(idb, ide-idb);
		}
	}
	return "unknown";
}

Resource *TextResourceRead::ProcessInputSync(bool sleep) {
	if (docId.length() == 0) {
		if (getline(*ifs, docId).eof())
			return NULL;
		docId = GetDocId(docId);
	}
	TextResource *tr = static_cast<TextResource*>(Resource::GetRegistry()->AcquireResource(textResourceTypeId));
	tr->SetTextId(docId);
	docId.clear();
	string s;
	int flags = TextResource::TOKEN_NONE;
	int index = 0;
	vector<string> v;
	vector<string> v2;
	while (!getline(*ifs, s).eof()) {
		if (!s.compare(0, 5, "<doc ")) {
			docId = GetDocId(s);
			break;
		}
		if (!horizontal) {
			skipWs(s);
			if (s.length() == 0) {
				flags |= TextResource::TOKEN_SENTENCE_START;
			} else if (s[0] == '<') {
				if (!s.compare(1, 2, "p>")) {
					flags |= TextResource::TOKEN_PARAGRAPH_START;
				} else if (!s.compare(1, 2, "s>")) {
					flags |= TextResource::TOKEN_SENTENCE_START;
				} else if (!s.compare(1, 3, "g/>")) {
					if (index > 0)
						tr->SetFlags(index-1, tr->GetFlags(index-1)|TextResource::TOKEN_NO_SPACE);
				}
			} else {
				chomp(s);
				split(v, '\t', s);
				if (v.size() > 0)
					flags |= atoi(v[0].c_str());
				if (v.size() > 1)
					tr->SetForm(index, v[1]);
				if (v.size() > 2)
					tr->SetLemma(index, v[2]);
				if (v.size() > 3)
					tr->SetPosTag(index, v[3]);
				if (v.size() > 4)
					tr->SetHead(index, atoi(v[4].c_str()));
				if (v.size() > 5)
					tr->SetDepRel(index, v[5]);
				tr->SetFlags(index, flags);
				flags = TextResource::TOKEN_NONE;
				index++;
			}
		} else {
			// split on tabs, then on white-spaces
			skipWs(s);
			if (s.length() == 0)
				continue;
			chomp(s);
			split(v, '\t', s);
			flags = TextResource::TOKEN_SENTENCE_START;
			for (vector<string>::iterator iter = v.begin(); iter != v.end(); ++iter) {
				split(v2, ' ', *iter);
				if (v2.size() > 0)
					tr->SetForm(index, v2[0]);
				if (v2.size() > 1)
					tr->SetLemma(index, v2[1]);
				if (v2.size() > 2)
					tr->SetPosTag(index, v2[2]);
				if (v2.size() > 3)
					tr->SetHead(index, atoi(v2[3].c_str()));
				if (v2.size() > 4)
					tr->SetDepRel(index, v2[4]);
				tr->SetFlags(index, flags);
				flags = TextResource::TOKEN_NONE;
				index++;
			}
		}
	}

	return tr;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new TextResourceRead(objects, id, threadIndex);
}
