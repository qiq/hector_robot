/**
TextResourceRead.la, input, native
Read TextResource in vertical or horizontal form.

Dependencies: none

Parameters:
items		r/o	Total items processed.
horizontal	r/w	Whether to print horizontal (or horizontal)
			tab-separated-fields format.
filename	init	File to read.
*/

#ifndef _MODULES_TEXT_RESOURCE_READ_H_
#define _MODULES_TEXT_RESOURCE_READ_H_

#include <config.h>

#include <fstream>
#include <string>
#include <vector>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "TextResource.h"

class TextResourceRead : public Module {
public:
	TextResourceRead(ObjectRegistry *objects, const char *id, int threadIndex);
	~TextResourceRead();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessInputSync(bool sleep);

private:
	int items;		// ObjectLock, items processed
	bool horizontal;
	char *filename;

	char *GetItems(const char *name);
	char *GetHorizontal(const char *name);
        void SetHorizontal(const char *name, const char *value);
	char *GetFilename(const char *name);
	void SetFilename(const char *name, const char *value);

	ObjectProperties<TextResourceRead> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	int textResourceTypeId;
	std::ifstream *ifs;
	std::string docId;
};

inline Module::Type TextResourceRead::GetType() {
	return Module::INPUT;
}

inline char *TextResourceRead::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool TextResourceRead::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *TextResourceRead::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
