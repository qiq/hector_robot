/**
 *
 */
#include <config.h>

#include <assert.h>
#include <stdio.h>
#include "common.h"
#include "Resources.h"
#include "Filter.h"

Filter::Filter(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	ruleFile = NULL;
	ruleList = NULL;

	values = new ObjectValues<Filter>(this);
	values->addGetter("items", &Filter::getItems);
	values->addGetter("ruleList", &Filter::getRuleList);
	values->addSetter("ruleList", &Filter::setRuleList, true);
	values->addGetter("ruleFile", &Filter::getRuleFile);
	values->addSetter("ruleFile", &Filter::setRuleFile, true);
}

Filter::~Filter() {
	free(ruleFile);
	free(ruleList);

	delete values;

	for (vector<Rule*>::iterator iter = rules.begin(); iter != rules.end(); ++iter)
		delete (*iter);
}

char *Filter::getItems(const char *name) {
	return int2str(items);
}

char *Filter::getRuleList(const char *name) {
	return ruleList ? strdup(ruleList) : NULL;
}

void Filter::setRuleList(const char *name, const char *value) {
	free(ruleList);
	ruleList = strdup(value);
}

char *Filter::getRuleFile(const char *name) {
	return ruleFile ? strdup(ruleFile) : NULL;
}

void Filter::setRuleFile(const char *name, const char *value) {
	free(ruleFile);
	ruleFile = strdup(value);
}

bool Filter::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;
	typeId = Resources::Name2Id("WebResource");
	if (typeId < 0) {
		LOG_ERROR("Cannot load WebResource library");
		return false;
	}

	string data;
	if (ruleFile) {
		FILE *f = fopen(ruleFile, "r");
		if (!f) {
			LOG_ERROR("Cannot open file: " << ruleFile << ": " << strerror(errno));
			return false;
		}
		fseek(f, 0, SEEK_END);
		long len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *buffer = (char*)malloc(len);
		if (fread(buffer, len, 1, f) < 0) {
			LOG_ERROR("Cannot read file: " << ruleFile << ": " << strerror(errno));
			return false;
		}
		fclose(f);
		data.append(buffer, len);
		free(buffer);
		if (data.length() > 0)
			data += '\n';
	}
	if (ruleList)
		data += ruleList;

	// create rules
	int lineNo = 0;
	while (data.length() > 0) {
		lineNo++;
		// one line: possibly concatenate lines, if there is
		// a backslash at the end of the line
		size_t pos = 0;
		do {
			pos = data.find_first_of('\n', pos);
			if (pos == string::npos)
				break;
		} while (pos > 0 && data.at(pos-1) == '\\');
		string line;
		if (pos != string::npos) {
			line = data.substr(0, pos);
			data.erase(0, pos+1);
		} else {
			line = data;
			data.clear();
		}

		// skip empty lines
		skipWs(&line);
		if (line.length() == 0)
			continue;

		// skip lines starting with hash
		if (line.at(0) == '#')
			continue;

		Rule *r = new Rule();
		if (!r->Init(&line, lineNo)) {
			delete r;
			return false;
		}
		rules.push_back(r);
	}

	return true;
}

Resource *Filter::ProcessSimple(Resource *resource) {
	WebResource *wr = dynamic_cast<WebResource*>(resource);
	if (wr) {
		ObjectLockWrite();
		items++;
		ObjectUnlock();
		// process rule-by rule and deal with the resource accordingly
		int i = 1;
		for (vector<Rule*>::iterator iter = rules.begin(); iter != rules.end(); ++iter) {
			switch ((*iter)->Apply(wr)) {
			case Filter::Action::ACCEPT:
				LOG_DEBUG("[" << wr->getId() << "]: ACCEPT #" << i);
				return resource;
			case Filter::Action::DROP:
				LOG_DEBUG("[" << wr->getId() << "]: DROP #" << i);
				delete wr;
				return NULL;
			case Filter::Action::CONTINUE:
			default:
				// do nothing
				break;
			}
			i++;
		}
	}
	return resource;
}

log4cxx::LoggerPtr Filter::Condition::logger(log4cxx::Logger::getLogger("module.Filter.Condition"));

bool Filter::Condition::Init(string *data, int lineNo) {
	// first part, what: label | length(label) | label[name]
	length = false;
	string label;
	if (!parseLabel(data, &label)) {
		LOG4CXX_ERROR(logger, "Invalid label: " << *data << " (line " << lineNo << ")");
		return false;
	}
	if (label == "length") {
		if (data->at(0) == '(') {
			data->erase(0, 1);
			label.clear();
			if (parseLabel(data, &label)) {
				if (data->at(0) == ')') {
					data->erase(0, 1);
					length = true;
				}
			}
		}
		if (!length) {
			LOG4CXX_ERROR(logger, "Invalid length: " << *data << " (line " << lineNo << ")");
			return false;
		}
	}
	if (label.empty()) {
		LOG4CXX_ERROR(logger, "No label: " << data);
		return false;
	}
	info = WebResource::getFieldInfo(label.c_str());
	if (info.type == Resource::UNKNOWN) {
		LOG4CXX_ERROR(logger, "Invalid label encountered: " << label << " (line " << lineNo << ")");
		return false;
	}
	if (info.type == Resource::STRING2) {
		// header: consume [, label2, ]
		if (data->length() == 0 || data->at(0) != '[') {
			LOG4CXX_ERROR(logger, "Invalid format: [ expected" << *data << " (line " << lineNo << ")");
			return false;
		}
		if (!parseLabel(data, &label)) {
			LOG4CXX_ERROR(logger, "Invalid label: " << *data << " (line " << lineNo << ")");
			return false;
		}
		this->name = label;
		if (data->length() == 0 || data->at(0) != '[') {
			LOG4CXX_ERROR(logger, "Invalid format: ] expected" << *data << " (line " << lineNo << ")");
			return false;
		}
	}

	// second part, operator < | > | == | != | =~ | !~
	skipWs(data);
        size_t offset = data->find_first_not_of("<>=!~");
	string op;
	if (offset != string::npos) {
		if (offset == 0) {
			LOG4CXX_ERROR(logger, "No operator (line " << lineNo << ")");
			return false;
		}
		op = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		op = *data;
		data->clear();
	}

	bool error = false;
	if (op == "==") {
		switch (this->info.type) {
		case Resource::STRING:
			this->op = STRING_EQ;
			break;
		case Resource::INT:
			this->op = INT_EQ;
			break;
		case Resource::LONG:
			this->op = LONG_EQ;
			break;
		default:
			error = true;
		}
	} else if (op == "!=" || op == "<>") {
		switch (this->info.type) {
		case Resource::STRING:
			this->op = STRING_NE;
			break;
		case Resource::INT:
			this->op = INT_NE;
			break;
		case Resource::LONG:
			this->op = LONG_NE;
			break;
		default:
			error = true;
		}
	} else if (op == "<") {
		switch (this->info.type) {
		case Resource::STRING:
			this->op = STRING_LT;
			break;
		case Resource::INT:
			this->op = INT_LT;
			break;
		case Resource::LONG:
			this->op = LONG_LT;
			break;
		default:
			error = true;
		}
	} else if (op == ">") {
		switch(this->info.type) {
		case Resource::STRING:
			this->op = STRING_GT;
			break;
		case Resource::INT:
			this->op = INT_GT;
			break;
		case Resource::LONG:
			this->op = LONG_GT;
			break;
		default:
			error = true;
		}
	} else if (op == "<=") {
		switch (this->info.type) {
		case Resource::INT:
			this->op = INT_LE;
			break;
		case Resource::LONG:
			this->op = LONG_LE;
			break;
		default:
			error = true;
		}
	} else if (op == ">=") {
		switch (this->info.type) {
		case Resource::INT:
			this->op = INT_GE;
			break;
		case Resource::LONG:
			this->op = LONG_GE;
			break;
		default:
			error = true;
		}
	} else if (op == "=~") {
		switch (this->info.type) {
		case Resource::STRING:
			this->op = STRING_REGEX;
			break;
		case Resource::IP4:
			this->op = IP4_EQ;
			break;
		case Resource::IP6:
			this->op = IP6_EQ;
			break;
		default:
			error = true;
		}
	} else if (op == "!~") {
		switch (this->info.type) {
		case Resource::STRING:
			this->op = STRING_NREGEX;
			break;
		case Resource::IP4:
			this->op = IP4_NE;
			break;
		case Resource::IP6:
			this->op = IP6_NE;
			break;
		default:
			error = true;
		}
	}
	if (error) {
		LOG4CXX_ERROR(logger, "Invalid operator: " << op << " (line " << lineNo << ")");
		return false;
	}

	// third part, value: "string" | integer | /regex/ | s/regex/replacement/flags | ip4_addr | ip6_addr
	switch (this->op) {
	case INT_EQ:
	case INT_NE:
	case INT_LT:
	case INT_LE:
	case INT_GT:
	case INT_GE:
		if (!parseInt(data, &this->iValue)) {
			LOG4CXX_ERROR(logger, "Invalid value: " << *data << " (line " << lineNo << ")");
			return false;
		}
		break;
	case LONG_EQ:
	case LONG_NE:
	case LONG_LT:
	case LONG_LE:
	case LONG_GT:
	case LONG_GE:
		if (!parseLong(data, &this->lValue)) {
			LOG4CXX_ERROR(logger, "Invalid value: " << *data << " (line " << lineNo << ")");
			return false;
		}
		break;
	case STRING_EQ:
	case STRING_NE:
	case STRING_LT:
	case STRING_GT:
		if (!parseString(data, &this->sValue, '"')) {
			LOG4CXX_ERROR(logger, "Invalid value: " << *data << " (line " << lineNo << ")");
			return false;
		}
		break;
	case STRING_REGEX:
	case STRING_NREGEX:
		{
			skipWs(data);
			bool subst = false;
			this->global = false;
			if (data->length() > 0 && data->at(0) == 's') {
				data->erase(0, 1);
				subst = true;
			}
			string match;
			if (!parseString(data, &match, '/')) {
				LOG4CXX_ERROR(logger, "Invalid regex: " << *data << " (line " << lineNo << ")");
				return false;
			}
			if (subst) {
				if (!parseString(data, &this->replacement, '/')) {
					LOG4CXX_ERROR(logger, "Invalid regex replacement: " << *data << " (line " << lineNo << ")");
					return false;
				}
				this->op = STRING_REGEX_SUBST;
			}
			// process g/i
			bool caseless = false;
			while (data->length() > 0) {
				char c = data->at(0);
				if (c == 'g') {
					this->global = true;
					data->erase(0, 1);
				} else if (c == 'i') {
					caseless = true;
					data->erase(0, 1);
				} else {
					break;
				}
			}
			this->regex = new pcrecpp::RE(match, caseless ? pcrecpp::RE_Options().set_caseless(true).set_utf8(true) : pcrecpp::RE_Options().set_utf8(true));
			break;
		}
	case IP4_EQ:
	case IP4_NE:
		if (!parseIp4(data, &this->a4Value)) {
			LOG4CXX_ERROR(logger, "Invalid IP address: " << *data << " (line " << lineNo << ")");
			return false;
		}
		if (data->length() > 0 && data->at(0) == '/') {
			data->erase(0, 1);
			if (!parseInt(data, &this->prefix) || this->prefix < 0 || this->prefix > 32) {
				LOG4CXX_ERROR(logger, "Invalid prefix: " << *data << " (line " << lineNo << ")");
				return false;
			}
		} else {
			this->prefix = 32;
		}
		break;
	case IP6_EQ:
	case IP6_NE:
		if (!parseIp6(data, &this->a6Value)) {
			LOG4CXX_ERROR(logger, "Invalid IPv6 address: " << *data << " (line " << lineNo << ")");
			return false;
		}
		if (data->length() > 0 && data->at(0) == '/') {
			data->erase(0, 1);
			if (!parseInt(data, &this->prefix) || this->prefix < 0 || this->prefix > 128) {
				LOG4CXX_ERROR(logger, "Invalid prefix: " << *data << " (line " << lineNo << ")");
				return false;
			}
		} else {
			this->prefix = 128;
		}
		break;
	}
	return true;
}

bool Filter::Condition::isTrue(WebResource *wr) {
	const char *sValue;
	int iValue;
	ip4_addr_t a4Value;
	ip6_addr_t a6Value;
	switch (this->info.type) {
	case Resource::STRING:
		sValue = (wr->*info.get.s)();
		if (!sValue)
			return false;
		break;
	case Resource::INT:
		iValue = (wr->*info.get.i)();
		break;
	case Resource::LONG:
		iValue = (wr->*info.get.l)();
		break;
	case Resource::IP4:
		a4Value = (wr->*info.get.a4)();
		if (prefix)
			a4Value.addr &= ((uint32_t)0xffffffff) << (32-prefix);
		break;
	case Resource::IP6:
		{
			a6Value = (wr->*info.get.a6)();
			if (!prefix)
				prefix = 128;
			int a = prefix / 8;
			int b = prefix % 8;
			if (a < 16)
				a6Value.addr[a] &= 0xFF & (0xFF << (8-b));
			for (int i = a+1; i < 16; i++)
				a6Value.addr[i] = 0x00;
		}
		break;
	case Resource::STRING2:
		sValue = (wr->*info.get.s2)(name.c_str());
		if (!sValue)
			return false;
	}

	if (length)
		iValue = strlen(sValue);

	switch (op) {
	case INT_EQ:
		return iValue == this->iValue;
	case INT_NE:
		return iValue != this->iValue;
	case INT_LT:
		return iValue < this->iValue;
	case INT_LE:
		return iValue > this->iValue;
	case INT_GT:
		return iValue <= this->iValue;
	case INT_GE:
		return iValue >= this->iValue;
	case STRING_EQ:
		return !strcmp(sValue, this->sValue.c_str());
	case STRING_NE:
		return strcmp(sValue, this->sValue.c_str());
	case STRING_LT:
		return strcmp(sValue, this->sValue.c_str()) < 0;
	case STRING_GT:
		return strcmp(sValue, this->sValue.c_str()) > 0;
	case STRING_REGEX:
		return this->regex->FullMatch(sValue);
	case STRING_NREGEX:
		return !this->regex->FullMatch(sValue);
	case STRING_REGEX_SUBST:
		{
			string s = string(sValue);
			bool result;
			if (this->global)
				result = this->regex->GlobalReplace(replacement, &s);
			else
				result = this->regex->Replace(replacement, &s);
			if (info.type == Resource::STRING)
				(wr->*info.set.s)(s.c_str());
			else if (info.type == Resource::STRING2)
				(wr->*info.set.s2)(name.c_str(), s.c_str());
			return result;
		}
	case IP4_EQ:
		return a4Value.addr == this->a4Value.addr;
	case IP4_NE:
		return a4Value.addr != this->a4Value.addr;
	case IP6_EQ:
		return memcmp(&a6Value, &this->a6Value, sizeof(ip6_addr_t)) == 0;
	case IP6_NE:
		return memcmp(&a6Value, &this->a6Value, sizeof(ip6_addr_t)) != 0;
	default:
		LOG4CXX_ERROR(logger, "Invalid condition operator: " << op);
	}
	return false;
}

log4cxx::LoggerPtr Filter::Action::logger(log4cxx::Logger::getLogger("module.Filter.Action"));

bool Filter::Action::Init(string *data, int lineNo) {
	// first part, what: label | label[name] | ACCEPT | DROP | CONTINUE
	string label;
	if (!parseLabel(data, &label)) {
		LOG4CXX_ERROR(logger, "Expected label: " << *data << " (line " << lineNo << ")");
		return false;
	}
	if (label.empty()) {
		LOG4CXX_ERROR(logger, "No label: " << *data << " (line " << lineNo << ")");
		return false;
	}
	if (label == "ACCEPT") {
		this->type = ACCEPT;
		return true;
	} else if (label == "DROP") {
		this->type = DROP;
		return true;
	} else if (label == "CONTINUE") {
		this->type = CONTINUE;
		return true;
	} else if (label == "clear") {
		this->type = CLEAR;
		if (data->length() == 0 || data->at(0) != '(') {
			LOG4CXX_ERROR(logger, "Invalid format: ( expected" << *data << " (line " << lineNo << ")");
			return false;
		}
		data->erase(0, 1);
		if (!parseString(data, &label, ')')) {
			LOG4CXX_ERROR(logger, "Invalid label encountered: " << *data << " (line " << lineNo << ")");
			return false;
		}
		info = WebResource::getFieldInfo(label.c_str());
		if (info.type == Resource::UNKNOWN) {
			LOG4CXX_ERROR(logger, "Invalid label encountered: " << label << " (line " << lineNo << ")");
			return false;
		}
		return true;
	} else if (label == "STATUS") {
		this->type = SETVAL;
		this->info.type = Resource::INT;
		this->info.get.i = &WebResource::getStatus;
		this->info.set.i = &WebResource::setStatus;
		// to be continued :)
	} else {
		this->type = SETVAL;
		info = WebResource::getFieldInfo(label.c_str());
		if (info.type == Resource::UNKNOWN) {
			LOG4CXX_ERROR(logger, "Invalid label encountered: " << label << " (line " << lineNo << ")");
			return false;
		}
		if (info.type == Resource::STRING2) {
			// header: consume [, label2, ]
			if (data->length() == 0 || data->at(0) != '[') {
				LOG4CXX_ERROR(logger, "Invalid format: [ expected" << *data << " (line " << lineNo << ")");
				return false;
			}
			if (!parseLabel(data, &label)) {
				LOG4CXX_ERROR(logger, "Unvalid label: " << *data << " (line " << lineNo << ")");
				return false;
			}
			this->name = label;
			if (data->length() == 0 || data->at(0) != '[') {
				LOG4CXX_ERROR(logger, "Invalid format: ] expected" << *data << " (line " << lineNo << ")");
				return false;
			}
		}
	}

	// second part, operator =
	skipWs(data);
	if (data->length() == 0 || data->at(0) != '=') {
		LOG4CXX_ERROR(logger, "= expected, got: " << *data << " (line " << lineNo << ")");
		return false;
	}
	data->erase(0, 1);

	// third part, value
	bool error = false;
	switch (this->info.type) {
	case Resource::STRING:
	case Resource::STRING2:
		if (!parseString(data, &this->sValue, '"'))
			error = true;
		break;
	case Resource::INT:
		if (!parseInt(data, &this->iValue))
			error = true;
		break;
	case Resource::LONG:
		if (!parseLong(data, &this->lValue))
			error = true;
		break;
	case Resource::IP4:
		if (!parseIp4(data, &this->a4Value))
			error = true;
		break;
	case Resource::IP6:
		if (!parseIp6(data, &this->a6Value))
			error = true;
		break;
	}
	if (error) {
		LOG4CXX_ERROR(logger, "Invalid value: " << *data << " (line " << lineNo << ")");
		return false;
	}
	return true;
}

Filter::Action::ActionType Filter::Action::Apply(WebResource *wr) {
	switch (type) {
	case ACCEPT:
		return ACCEPT;
	case DROP:
		return DROP;
	case CONTINUE:
		return CONTINUE;
	case CLEAR:
		(wr->*info.clear)();
		return CLEAR;
	case SETVAL:
		switch (info.type) {
		case Resource::STRING:
			(wr->*info.set.s)(sValue.c_str());
			break;
		case Resource::INT:
			(wr->*info.set.i)(iValue);
			break;
		case Resource::LONG:
			(wr->*info.set.l)(lValue);
		case Resource::IP4:
			(wr->*info.set.a4)(a4Value);
			break;
		case Resource::IP6:
			(wr->*info.set.a6)(a6Value);
			break;
		case Resource::STRING2:
			(wr->*info.set.s2)(name.c_str(), sValue.c_str());
			break;
		default:
			LOG4CXX_ERROR(logger, "No value setter to use");
		}
		return SETVAL;
	}
	LOG4CXX_ERROR(logger, "Invalid action type: " << type);
	return DROP;
}

log4cxx::LoggerPtr Filter::Rule::logger(log4cxx::Logger::getLogger("module.Filter.Rule"));

Filter::Rule::~Rule() {
	for (vector<Condition*>::iterator iter = conditions.begin(); iter != conditions.end(); ++iter)
		delete (*iter);
	for (vector<Action*>::iterator iter = actions.begin(); iter != actions.end(); ++iter)
		delete (*iter);
}

bool Filter::Rule::Init(string *line, int lineNo) {
	// create a rule from line: parse [condition|*] => [action]
	skipWs(line);
	if (line->length() > 0 && line->at(0) == '*') {
		line->erase(0, 1);
	} else {
		while (true) {
			Condition *c = new Condition();
			if (!c->Init(line, lineNo)) {
				delete c;
				return false;
			}
			conditions.push_back(c);
			skipWs(line);
			if (line->length() <= 1 || line->at(0) != '&' || line->at(1) != '&')
				break;
			line->erase(0, 2);
		}
	}
	// => 
	skipWs(line);
	if (line->length() <= 1 || line->at(0) != '=' || line->at(1) != '>') {
		LOG4CXX_ERROR(logger, "Expected => : " << *line << " (line " << lineNo << ")");
		return false;
	}
	line->erase(0, 2);

	// actions
	while (true) {
		Action *a = new Action();
		if (!a->Init(line, lineNo)) {
			delete a;
			return false;
		}
		actions.push_back(a);
		skipWs(line);
		if (line->length() == 0 || line->at(0) != ',')
			break;
		line->erase(0, 1);
	}
	if (line->length() > 0) {
		LOG4CXX_ERROR(logger, "Invalid trailer: " << *line << " (line " << lineNo << ")");
		return false;
	}
	return true;
}

Filter::Action::ActionType Filter::Rule::Apply(WebResource *wr) {
	for (vector<Condition*>::iterator iter = conditions.begin(); iter != conditions.end(); ++iter) {
		if (!(*iter)->isTrue(wr))
			return Filter::Action::CONTINUE;
	}
	Filter::Action::ActionType result = Filter::Action::CONTINUE;
	for (vector<Action*>::iterator iter = actions.begin(); iter != actions.end(); ++iter) {
		Filter::Action::ActionType r = (*iter)->Apply(wr);
		if (r == Filter::Action::ACCEPT || r == Filter::Action::DROP || r == Filter::Action::CONTINUE)
			result = r;
	}
	return result;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new Filter(objects, id, threadIndex);
}
