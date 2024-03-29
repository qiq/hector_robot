/**
 * Filter Resource according to various field props.
 */

#ifndef _MODULES_FILTER_H_
#define _MODULES_FILTER_H_

#include <config.h>

#include <tr1/unordered_map>
#include <vector>
#include <pcrecpp.h>
#include "robot_common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "Resource.h"
#include "ResourceAttrInfoT.h"

class Filter : public Module {
public:
	class Condition;
	class Action;
	class Rule;

	Filter(ObjectRegistry *objects, const char *id, int threadIndex);
	~Filter();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	std::vector<Filter::Rule*> *InitResource(Resource *resource);
	Module::Type GetType();
	Resource *ProcessSimple(Resource *resource);

private:
	int items;		// ObjectLock
	char *ruleList;		// initOnly
	char *ruleFile;		// initOnly

	char *GetItems(const char *name);
	char *GetRuleList(const char *name);
	void SetRuleList(const char *name, const char *value);
	char *GetRuleFile(const char *name);
	void SetRuleFile(const char *name, const char *value);

	ObjectProperties<Filter> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	bool IsInitOnly(const char *name);
	std::vector<std::string> *ListPropertiesSync();

	std::tr1::unordered_map<int, std::vector<Filter::Rule*>*> rules;

public:
	// one left-hand condition (cond1 && cond2 => action1, action2)
	class Condition {
	public:
		enum OperandType {
			UNKNOWN,
			STRING,
			INT,
			LONG,
			IP,
		};
		enum Operation {
			SCALAR,
			ARRAY,
			HASH,
			ARRAY_ALL,
			HASH_ALL,
			ARRAY_ANY,
			HASH_ANY,
			COUNT,
		};

		enum OperatorType {
			INT_EQ, INT_NE, INT_LT, INT_LE, INT_GT, INT_GE,
			LONG_EQ, LONG_NE, LONG_LT, LONG_LE, LONG_GT, LONG_GE,
			STRING_EQ, STRING_NE, STRING_LT, STRING_GT,
			STRING_REGEX, STRING_NREGEX, STRING_REGEX_SUBST,
			IP4_EQ, IP4_NE,
			IP6_EQ, IP6_NE,
		};

		Condition();
		~Condition();
		bool Init(std::string *data, int lineNo, Resource *resource);
		bool isTrue(Resource *wr);

	private:
		bool length;		// length(string)
		std::string name;	// used for header props
		// variable
		ResourceFieldInfo *info;
		// operator
		OperatorType op;
		// value
		std::string sValue;
		int iValue;
		long lValue;
		IpAddr addrValue;
		int prefix;
		pcrecpp::RE *regex;
		std::string replacement;
		bool global;

		static log4cxx::LoggerPtr logger;
	};

	// one action part of the rule (cond1 && cond2 => action1, action2)
	class Action {
	public:
		enum ActionType {
			ACCEPT,		// default
			DROP,
			CONTINUE,
		};

		Action();
		~Action();
		bool Init(std::string *data, int lineNo, Resource *resource);
		ActionType Apply(Resource *wr);

	private:
		// action: ACCEPT | CONTINUE | DROP
		ActionType type;
		// variable setters
		ResourceFieldInfo *info;
		// props
		std::string name;	// used for header props
		std::string sValue;
		int iValue;
		long lValue;
		IpAddr addrValue;

		static log4cxx::LoggerPtr logger;
	};

	// the whole rule: condtions and action
	class Rule {
	public:
		Rule() {};
		~Rule();
		bool Init(std::string *line, int lineNo, Resource *resource);
		Filter::Action::ActionType Apply(Resource *resource);
	private:
		std::vector<Condition*> conditions;
		std::vector<Action*> actions;

		static log4cxx::LoggerPtr logger;
	};
};

inline Module::Type Filter::GetType() {
	return SIMPLE;
}

inline char *Filter::GetPropertySync(const char *name) {
	return props->GetPropertySync(name);
}

inline bool Filter::SetPropertySync(const char *name, const char *value) {
	return props->SetPropertySync(name, value);
}

inline bool Filter::isInitOnly(const char *name) {
	return props->isInitOnly(name);
}

inline std::vector<std::string> *Filter::listNamesSync() {
	return props->listNamesSync();
}

inline Filter::Condition::Condition() {
	info = NULL;
}

inline Filter::Condition::~Condition() {
	delete(info);
}

inline Filter::Action::Action() {
	info = NULL;
}

inline Filter::Action::~Action() {
	delete(info);
}

#endif
