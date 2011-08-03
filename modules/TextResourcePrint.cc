/**
 */
#include <config.h>

#include <stdlib.h>
#include <string.h>
#include "robot_common.h"
#include "MarkerResource.h"
#include "TextResourcePrint.h"
#include "TextResource.h"

using namespace std;

TextResourcePrint::TextResourcePrint(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	horizontal = false;
	filename = NULL;

	props = new ObjectProperties<TextResourcePrint>(this);
	props->Add("items", &TextResourcePrint::GetItems);
	props->Add("horizontal", &TextResourcePrint::GetHorizontal, &TextResourcePrint::SetHorizontal);
	props->Add("filename", &TextResourcePrint::GetFilename, &TextResourcePrint::SetFilename, true);

	ofs = NULL;
}

TextResourcePrint::~TextResourcePrint() {
	if (ofs) {
		ofs->close();
		delete ofs;
	}
	free(filename);

	delete props;
}

char *TextResourcePrint::GetItems(const char *name) {
	return int2str(items);
}

char *TextResourcePrint::GetHorizontal(const char *name) {
	return bool2str(horizontal);
}

void TextResourcePrint::SetHorizontal(const char *name, const char *value) {
	horizontal = str2bool(value);
}

char *TextResourcePrint::GetFilename(const char *name) {
	return strdup(filename);
}

void TextResourcePrint::SetFilename(const char *name, const char *value) {
	free(filename);
	filename = strdup(value);
}

bool TextResourcePrint::Init(vector<pair<string, string> > *params) {
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

	ofs = new ofstream(filename);
	if (!ofs->is_open()) {
		LOG_ERROR(this, "Cannot open file: " << filename);
		return false;
	}

	return true;
}

Resource *TextResourcePrint::ProcessOutputSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	*ofs << "Id: " << tr->GetTextId() << "\n";
	int nForms = tr->GetFormCount();
	int nFlags = tr->GetFlagsCount();
	int nLemmas = tr->GetLemmaCount();
	int nPosTags = tr->GetPosTagCount();
	int nHeads = tr->GetHeadCount();
	int nDepRels = tr->GetDepRelCount();
	if (!horizontal) {
		for (int i = 0; i < nForms; i++) {
			int flags = i < nFlags ? tr->GetFlags(i) : 0;
			if (flags & TextResource::TOKEN_PARAGRAPH_START)
				*ofs << "<p>\n";
			if (i > 0 && flags & TextResource::TOKEN_SENTENCE_START)
				*ofs << "\n";
			*ofs << tr->GetForm(i);
			if (i < nLemmas)
				*ofs << "\t" << tr->GetLemma(i);
			if (i < nPosTags)
				*ofs << "\t" << tr->GetPosTag(i);
			if (i < nHeads)
				*ofs << "\t" << tr->GetHead(i);
			if (i < nDepRels)
				*ofs << "\t" << tr->GetDepRel(i);
			*ofs << "\n";
			if (flags & TextResource::TOKEN_NO_SPACE)
				*ofs << "<d>\n";
		}
	} else {
		for (int i = 0; i < nForms; i++) {
			int flags = i < nFlags ? tr->GetFlags(i) : 0;
			if (i > 0) {
				if (flags & TextResource::TOKEN_SENTENCE_START)
					*ofs << "\n";
				else
					*ofs << "\t";
			}
			*ofs << tr->GetForm(i);
			if (i < nLemmas)
				*ofs << " " << tr->GetLemma(i);
			if (i < nPosTags)
				*ofs << " " << tr->GetPosTag(i);
			if (i < nHeads)
				*ofs << " " << tr->GetHead(i);
			if (i < nDepRels)
				*ofs << " " << tr->GetDepRel(i);
		}
		if (nForms > 0)
			*ofs << "\n";
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new TextResourcePrint(objects, id, threadIndex);
}
