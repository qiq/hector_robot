/**
TextResourcePrint.la, output, native
Print TextResource in horizontal or horizontal form.

Dependencies: none

Parameters:
items		r/o	Total items processed.
horizontal	r/w	Whether to print horizontal (or horizontal)
			tab-separated-fields format.
filename	init	File to print to.
*/

#ifndef _MODULES_TEXT_RESOURCE_PRINT_H_
#define _MODULES_TEXT_RESOURCE_PRINT_H_

#include <config.h>

#include <fstream>
#include <string>
#include <vector>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "TextResource.h"

class TextResourcePrint : public Module {
public:
	TextResourcePrint(ObjectRegistry *objects, const char *id, int threadIndex);
	~TextResourcePrint();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessOutputSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	bool horizontal;
	char *filename;

	char *GetItems(const char *name);
	char *GetHorizontal(const char *name);
        void SetHorizontal(const char *name, const char *value);
	char *GetFilename(const char *name);
	void SetFilename(const char *name, const char *value);

	ObjectProperties<TextResourcePrint> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::ofstream *ofs;
};

inline Module::Type TextResourcePrint::GetType() {
	return Module::OUTPUT;
}

inline char *TextResourcePrint::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool TextResourcePrint::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *TextResourcePrint::ListPropertiesSync() {
	return props->ListProperties();
}

#endif