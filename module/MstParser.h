/**
MstParser.la, simple, native
MstParser runs MSTParser java package and is able to add its result into
TextResources.

Dependencies: none

Parameters:
items			r/o	Total items processed.
model			init	Model file to load.
decodeType		init	decode-type (non-proj).
order			init	Model order (2).
*/

#ifndef _MODULES_MSTPARSER_H_
#define _MODULES_MSTPARSER_H_

#include <config.h>

#include <string>
#include <vector>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class MstParser : public Module {
public:
	MstParser(ObjectRegistry *objects, const char *id, int threadIndex);
	~MstParser();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;
	char *parserDir;
	char *model;
	char *decodeType;
	int order;

	char *GetItems(const char *name);
	char *GetParserDir(const char *name);
        void SetParserDir(const char *name, const char *value);
	char *GetModel(const char *name);
        void SetModel(const char *name, const char *value);
	char *GetDecodeType(const char *name);
        void SetDecodeType(const char *name, const char *value);
	char *GetOrder(const char *name);
        void SetOrder(const char *name, const char *value);

	ObjectProperties<MstParser> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	pid_t pid;
	int fdin;
	int fdout;
	int fderr;
	std::string stderrBuffer;

	bool ReadWrite(std::string &writeBuffer, std::string &readBuffer, bool waitForRead);
	std::string &GetShortTag(std::string &tag);
	bool ParseSentence(std::vector<std::string> &form, std::vector<std::string> &tag, std::vector<int> &head, std::vector<std::string> &depRel);
};

inline Module::Type MstParser::GetType() {
	return SIMPLE;
}

inline char *MstParser::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool MstParser::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *MstParser::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
