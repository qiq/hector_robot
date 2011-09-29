/**
 */
#include <config.h>

#include <stdlib.h>
#include <string.h>
#include "robot_common.h"
#include "MarkerResource.h"
#include "PrintTextResource.h"
#include "TextResource.h"

using namespace std;

PrintTextResource::PrintTextResource(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	horizontal = false;
	filename = NULL;

	props = new ObjectProperties<PrintTextResource>(this);
	props->Add("items", &PrintTextResource::GetItems);
	props->Add("horizontal", &PrintTextResource::GetHorizontal, &PrintTextResource::SetHorizontal);
	props->Add("filename", &PrintTextResource::GetFilename, &PrintTextResource::SetFilename, true);

	ofs = NULL;
}

PrintTextResource::~PrintTextResource() {
	if (ofs) {
		ofs->close();
		delete ofs;
	}
	free(filename);

	delete props;
}

char *PrintTextResource::GetItems(const char *name) {
	return int2str(items);
}

char *PrintTextResource::GetHorizontal(const char *name) {
	return bool2str(horizontal);
}

void PrintTextResource::SetHorizontal(const char *name, const char *value) {
	horizontal = str2bool(value);
}

char *PrintTextResource::GetFilename(const char *name) {
	return strdup(filename);
}

void PrintTextResource::SetFilename(const char *name, const char *value) {
	free(filename);
	filename = strdup(value);
}

bool PrintTextResource::Init(vector<pair<string, string> > *params) {
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

Resource *PrintTextResource::ProcessOutputSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int nFlags = tr->GetFlagsCount();
	int nForms = tr->GetFormCount();
	int nLemmas = tr->GetLemmaCount();
	int nPosTags = tr->GetPosTagCount();
	int nHeads = tr->GetHeadCount();
	int nDepRels = tr->GetDepRelCount();
	*ofs << "<doc id=\"" << tr->GetTextId() << "\" lang=\"" << tr->GetLanguage() << "\">\n";
	if (!horizontal) {
		for (int i = 0; i < nFlags; i++) {
			int flags = i < nFlags ? tr->GetFlags(i) : TextResource::TOKEN_NONE;
			if (flags & TextResource::TOKEN_PARAGRAPH_START)
				*ofs << "<p>\n";
			if (flags & TextResource::TOKEN_SENTENCE_START)
				*ofs << "<s>\n";
			*ofs << (flags & (TextResource::TOKEN_ABBR | TextResource::TOKEN_PUNCT | TextResource::TOKEN_TITLECASE | TextResource::TOKEN_UPPERCASE| TextResource::TOKEN_NUMERIC));
			if (i < nForms)
				*ofs << "\t" << tr->GetForm(i);
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
				*ofs << "<g/>\n";
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
	return new PrintTextResource(objects, id, threadIndex);
}
