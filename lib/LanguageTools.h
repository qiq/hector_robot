/**
  Language.h
*/

#ifndef _LIB_LANGUAGE_TOOLS_H_
#define _LIB_LANGUAGE_TOOLS_H_

#include <config.h>

class LanguageTools {
public:
	LanguageTools();
	~LanguageTools();

	std::string Detect(std::string &s);
private:
};

#endif
