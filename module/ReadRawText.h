/**
ReadRawText.la, input, native
Read one or more files and create TextResources out of them (set text field
with the contents of every file).

Dependencies: none

Parameters:
items		r/o	Total items processed
filename	init	File(s) to lead.
listInFile	init	File(s) contain list of files to load (false).
mark		r/w	After all resources are read, emit MarkResource.
*/

#ifndef _MODULES_READ_RAW_TEXT_H_
#define _MODULES_READ_RAW_TEXT_H_

#include <config.h>

#include <deque>
#include <string>
#include <vector>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "TextResource.h"

class ReadRawText : public Module {
public:
	ReadRawText(ObjectRegistry *objects, const char *id, int threadIndex);
	~ReadRawText();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessInputSync(bool sleep);

private:
	int items;		// ObjectLock, items processed
	char *filename;
	bool listInFile;
	int mark;

	char *GetItems(const char *name);
	char *GetFilename(const char *name);
        void SetFilename(const char *name, const char *value);
	char *GetListInFile(const char *name);
        void SetListInFile(const char *name, const char *value);
	char *GetMark(const char *name);
	void SetMark(const char *name, const char *value);

	ObjectProperties<ReadRawText> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int textResourceTypeId;
	int markerResourceTypeId;
	bool markEmmited;
	std::deque<std::string> files;
};

inline Module::Type ReadRawText::GetType() {
	return Module::INPUT;
}

inline char *ReadRawText::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool ReadRawText::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *ReadRawText::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
