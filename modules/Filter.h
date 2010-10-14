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

class Filter : public Module {
public:
	Filter(ObjectRegistry *objects, const char *id, int threadIndex);
	~Filter();
	bool Init(vector<pair<string, string> > *params);
	Module::Type getType();
	Resource *ProcessSimple(Resource *resource);

	class Condition;
	class Action;
	class Rule;

private:
	int typeId;		// not reachable outside module

	int items;		// ObjectLock
	char *ruleList;		// initOnly
	char *ruleFile;		// initOnly

	vector<Filter::Rule*> rules;

	ObjectValues<Filter> *values;

	char *getItems(const char *name);
	char *getRuleList(const char *name);
	void setRuleList(const char *name, const char *value);
	char *getRuleFile(const char *name);
	void setRuleFile(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
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
		bool Init(string *data, int lineNo);
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
			CLEAR,
		};

		Action() {};
		~Action() {};
		bool Init(string *data, int lineNo);
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
		bool Init(string *line, int lineNo);
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

inline bool Filter::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline vector<string> *Filter::listNamesSync() {
	return values->listNamesSync();
}

#endif
