/**
 */
#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "robot_common.h"
#include "MarkerResource.h"
#include "RawTextRead.h"
#include "TextResource.h"

using namespace std;

RawTextRead::RawTextRead(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	inputFiles = NULL;
	inputListFile = NULL;
	mark = -1;

	props = new ObjectProperties<RawTextRead>(this);
	props->Add("items", &RawTextRead::GetItems);
	props->Add("inputFiles", &RawTextRead::GetInputFiles, &RawTextRead::SetInputFiles, true);
	props->Add("inputListFile", &RawTextRead::GetInputListFile, &RawTextRead::SetInputListFile, true);
	props->Add("mark", &RawTextRead::GetMark, &RawTextRead::SetMark);

	textResourceTypeId = -1;
	markerResourceTypeId = -1;
	markEmmited = false;
}

RawTextRead::~RawTextRead() {
	free(inputFiles);
	free(inputListFile);

	delete props;
}

char *RawTextRead::GetItems(const char *name) {
	return int2str(items);
}

char *RawTextRead::GetInputFiles(const char *name) {
	return strdup(inputFiles);
}

void RawTextRead::SetInputFiles(const char *name, const char *value) {
	free(inputFiles);
	inputFiles = strdup(value);
}

char *RawTextRead::GetInputListFile(const char *name) {
	return strdup(inputListFile);
}

void RawTextRead::SetInputListFile(const char *name, const char *value) {
	free(inputListFile);
	inputListFile = strdup(value);
}

char *RawTextRead::GetMark(const char *name) {
	return int2str(mark);
}

void RawTextRead::SetMark(const char *name, const char *value) {
	mark = str2int(value);
}

bool RawTextRead::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	// append input files
	if (inputFiles) {
		vector<string> v;
		string s(inputFiles);
		splitOnWs(v, s);
		files.insert(files.end(), v.begin(), v.end());
	}
	// read file and append all files
	if (inputListFile) {
		string s;
		ifstream ifs(inputListFile);
		if (!ifs.is_open()) {
			LOG_ERROR(this, "Cannot open file: " << inputListFile);
			return false;
		}
		while (!getline(ifs, s).eof()) {
			skipWs(s);
			if (s.length() > 0)
				files.push_back(s);
		}
		ifs.close();
	}

	if (mark >= 0) {
		markerResourceTypeId = Resource::GetRegistry()->NameToId("MarkerResource");
		if (markerResourceTypeId <= 0) {
			LOG_ERROR(this, "Unknown resource type: MarkerResource");
			return false;
		}
	}
	textResourceTypeId = Resource::GetRegistry()->NameToId("TextResource");
	if (textResourceTypeId <= 0) {
		LOG_ERROR(this, "Unknown resource type: TextResource");
		return false;
	}

	return true;
}

Resource *RawTextRead::ProcessInputSync(bool sleep) {
	// open one file at a time
	while (true) {
		if (files.size() == 0) {
			if (mark >= 0 && !markEmmited) {
				MarkerResource *mr = static_cast<MarkerResource*>(Resource::GetRegistry()->AcquireResource(markerResourceTypeId));
				mr->SetMark(mark);
				markEmmited = true;
				return mr;
			}
			return NULL;
		}

		string filename = files.front();
		files.pop_front();
		string data;
		char buffer[1024*64];
		FILE *f = fopen(filename.c_str(), "r");
		if (!f) {
			LOG_ERROR(this, "Error open file " << filename << ": " << strerror(errno));
			continue;
		}

		size_t size;
		while ((size = fread(buffer, 1, sizeof(buffer), f)) > 0)
			data.append(buffer, size);
		fclose(f);
		TextResource *tr = static_cast<TextResource*>(Resource::GetRegistry()->AcquireResource(textResourceTypeId));
		tr->SetTextId(filename);
		tr->SetText(data);
		return tr;
	}
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new RawTextRead(objects, id, threadIndex);
}
