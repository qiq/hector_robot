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

// Rule: (to cele: Conditions, Actions)
// Condition: variable operator value

// TODO do webresources pridat zpracovani hlavicky (hash_map) a jeji zapis zpet (asi pri serializaci)

class Filter : public Module {
public:
	Filter(ObjectRegistry *objects, const char *id, int threadIndex);
	~Filter();
	bool Init(vector<pair<string, string> > *params);
	Module::Type getType();
	Resource *Process(Resource *resource);

private:
	int typeId;		// to create TestResource

	int items;		// guarded by ObjectLock

	ObjectValues<Filter> *values;

	char *getItems(const char *name);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	vector<string> *listNamesSync();

public:
	// one left-hand condition (cond1 && cond2 => action1, action2)
	class Condition {
	public:
		enum OperatorType {
			INT_EQ, INT_NE, INT_LT, INT_LE, INT_GT, INT_GE,
			STRING_EQ, STRING_NE, STRING_LT, STRING_GT,
			STRING_REGEX, STRING_NREGEX, STRING_REGEX_SUBST,
			IP4_EQ, IP4_NE,
			IP6_EQ, IP6_NE,
		};

		Condition() {};
		~Condition() {};
		bool Init(string *data);
		bool isTrue(WebResource *wr);
		string getId() { return ""; }; // for logging

	private:
		// variable getters
		const char *(WebResource::*fgs)();
		int (WebResource::*fgi)();
		ip4_addr_t (WebResource::*fga4)();
		ip6_addr_t (WebResource::*fga6)();
		const char *(WebResource::*fgsn)(const char*);
		// variable setters
		void (WebResource::*fss)(const char*);
		void (WebResource::*fsi)(int);
		void (WebResource::*fssn)(const char*, const char*);
		// operator
		OperatorType op;
		bool length;
		std::string name;	// used for header values
		// value
		std::string sValue;
		int iValue;
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
		string getId() { return ""; }; // for logging

	private:
		// action: ACCEPT | DROP | CONTINUE | STATUS = xxx
		ActionType type;
		// variable setters
		void (WebResource::*fss)(const char*);
		void (WebResource::*fsi)(int);
		void (WebResource::*fsa4)(ip4_addr_t);
		void (WebResource::*fsa6)(ip6_addr_t);
		void (WebResource::*fssn)(const char*, const char*);
		// values
		std::string name;	// used for header values
		std::string sValue;
		int iValue;
		ip4_addr_t a4Value;
		ip6_addr_t a6Value;

		static log4cxx::LoggerPtr logger;
	};

	// the whole rule: condtions and action
	class Rule {
	public:
		Rule() {};
		~Rule() {};
		bool Init(string *data);
		bool Apply(WebResource *wr);
	private:
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
