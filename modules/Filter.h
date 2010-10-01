/**
 * Filter WebResource according to various field values.
 * Requires WebResource to work on.
 */

#ifndef _MODULES_FILTER_H_
#define _MODULES_FILTER_H_

#include <config.h>

#include <pcrecpp.h>
#include "Module.h"
#include "ObjectValues.h"
#include "robot_common.h"
#include "WebResource.h"

/*
 length(content) > 100 && mimeType != "text/html" => mimeType = "aaa", DROP
 header[xxx] == 'abc' => ...
 content =~ s/abc/def/ => CONTINUE
*/

class Filter : public Module {
public:
	Filter(ObjectRegistry *objects, const char *id, int threadIndex);
	~Filter();
	bool Init(vector<pair<string, string> > *params);
	Module::Type getType();
	Resource *Process(Resource *resource);

	class Condition;
	class Action;
	class Rule;

private:
	int typeId;		// to create TestResource

	int items;		// guarded by ObjectLock
	char *ruleList;		// guarded by ObjectLock
	char *ruleFile;		// guarded by ObjectLock

	vector<Filter::Rule*> rules;

	ObjectValues<Filter> *values;

	char *getItems(const char *name);
	char *getRuleList(const char *name);
	void setRuleList(const char *name, const char *value);
	char *getRuleFile(const char *name);
	void setRuleFile(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	vector<string> *listNamesSync();

public:
	// one left-hand condition (cond1 && cond2 => action1, action2)
	class Condition {
	public:
		enum OperatorType {
			INT_EQ, INT_NE, INT_LT, INT_LE, INT_GT, INT_GE,
			LONG_EQ, LONG_NE, LONG_LT, LONG_LE, LONG_GT, LONG_GE,
			STRING_EQ, STRING_NE, STRING_LT, STRING_GT,
			STRING_REGEX, STRING_NREGEX, STRING_REGEX_SUBST,
			IP4_EQ, IP4_NE,
			IP6_EQ, IP6_NE,
		};

		Condition() {};
		~Condition() {};
		bool Init(string *data);
		bool isTrue(WebResource *wr);

	private:
		bool length;		// length(string)
		std::string name;	// used for header values
		// variable
		WebResource::FieldInfo info;
		// operator
		OperatorType op;
		// value
		std::string sValue;
		int iValue;
		long lValue;
		ip4_addr_t a4Value;
		ip6_addr_t a6Value;
		int prefix;
		pcrecpp::RE *regex;
		std::string replacement;
		bool global;

		static log4cxx::LoggerPtr logger;
	};

	// one right-hand condition (cond1 && cond2 => action1, action2)
	class Action {
	public:
		enum ActionType {
			ACCEPT,
			DROP,
			CONTINUE,
			SETVAL,
		};

		Action() {};
		~Action() {};
		bool Init(string *data);
		ActionType Apply(WebResource *wr);

	private:
		// action: ACCEPT | DROP | CONTINUE | STATUS = xxx
		ActionType type;
		// variable setters
		WebResource::FieldInfo info;
		// values
		std::string name;	// used for header values
		std::string sValue;
		int iValue;
		long lValue;
		ip4_addr_t a4Value;
		ip6_addr_t a6Value;

		static log4cxx::LoggerPtr logger;
	};

	// the whole rule: condtions and action
	class Rule {
	public:
		Rule() {};
		~Rule();
		bool Init(string *line);
		Filter::Action::ActionType Apply(WebResource *wr);
	private:
		vector<Condition*> conditions;
		vector<Action*> actions;

		static log4cxx::LoggerPtr logger;
	};
};

inline Module::Type Filter::getType() {
	return SIMPLE;
}

inline char *Filter::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool Filter::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline vector<string> *Filter::listNamesSync() {
	return values->listNamesSync();
}

#endif
