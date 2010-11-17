/**
 * Parse URL into components (mainly hostname and others)
 * Requires WebResource to work on.
 */

#ifndef _MODULES_URL_PARSE_H_
#define _MODULES_URL_PARSE_H_

#include <config.h>

#include "Module.h"
#include "ObjectValues.h"

class UrlParser : public Module {
public:
	UrlParser(ObjectRegistry *objects, const char *id, int threadIndex);
	~UrlParser();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	Resource *ProcessSimple(Resource *resource);

private:
	int typeId;		// not accessible outside module

	int items;		// ObjectLock, items processed

	ObjectValues<UrlParser> *values;

	char *getItems(const char *name);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
};

inline Module::Type UrlParser::getType() {
	return SIMPLE;
}

inline char *UrlParser::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool UrlParser::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool UrlParser::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *UrlParser::listNamesSync() {
	return values->listNamesSync();
}

#endif
