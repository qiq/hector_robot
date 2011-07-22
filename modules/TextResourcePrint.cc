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
	vertical = true;
	outputFile = NULL;

	props = new ObjectProperties<TextResourcePrint>(this);
	props->Add("items", &TextResourcePrint::GetItems);
	props->Add("vertical", &TextResourcePrint::GetVertical, &TextResourcePrint::SetVertical);
	props->Add("outputFile", &TextResourcePrint::GetOutputFile, &TextResourcePrint::SetOutputFile, true);

	ofs = NULL;
}

TextResourcePrint::~TextResourcePrint() {
	if (ofs) {
		ofs->close();
		delete ofs;
	}
	free(outputFile);

	delete props;
}

char *TextResourcePrint::GetItems(const char *name) {
	return int2str(items);
}

char *TextResourcePrint::GetVertical(const char *name) {
	return bool2str(vertical);
}

void TextResourcePrint::SetVertical(const char *name, const char *value) {
	vertical = str2bool(value);
}

char *TextResourcePrint::GetOutputFile(const char *name) {
	return strdup(outputFile);
}

void TextResourcePrint::SetOutputFile(const char *name, const char *value) {
	free(outputFile);
	outputFile = strdup(value);
}

bool TextResourcePrint::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	// open output file
	if (!outputFile) {
		LOG_ERROR(this, "outputFile not defined");
		return false;
	}

	ofs = new ofstream(outputFile);
	if (!ofs->is_open()) {
		LOG_ERROR(this, "Cannot open file: " << outputFile);
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
	if (vertical) {
		for (int i = 0; i < nForms; i++) {
			int flags = i < nFlags ? tr->GetFlags(i) : 0;
			if (flags & TextResource::TOKEN_PARAGRAPH_START)
				*ofs << "\n<p>\n";
			if (i > 0 && flags & TextResource::TOKEN_SENTENCE_START)
				*ofs << "\n\n";
			ofs << tr->GetForm(i);
			if (i < nLemmas)
				*ofs << "\t" << tr->GetForm(i);
			if (i < nPosTags)
				*ofs << "\t" << tr->GetPosTag(i);
			if (i < nHeads)
				*ofs << "\t" << tr->GetHead(i);
			if (i < nDepRels)
				*ofs << "\t" << tr->GetDepRel(i);
			if (flags & TextResource::TOKEN_NO_SPACE)
				*ofs << "\n<d>\n";
		}
	} else {
		for (int i = 0; i < nForms; i++) {
			int flags = i < nFlags ? tr->GetFlags() : 0;
			if (i > 0) {
				if (flags & TextResource::TOKEN_SENTENCE_START)
					*ofs << "\n";
				else
					*ofs << "\t";
			}
			*ofs << tr->GetForm(i);
			if (i < nLemmas)
				*ofs << " " << tr->GetForm(i);
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
