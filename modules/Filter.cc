/**
 *
 */
#include <config.h>

#include <assert.h>
#include <stdio.h>
#include "common.h"
#include "Filter.h"

using namespace std;

Filter::Filter(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	ruleFile = NULL;
	ruleList = NULL;

	props = new ObjectProperties<Filter>(this);
	props->addGetter("items", &Filter::GetItems);
	props->addGetter("ruleList", &Filter::GetRuleList);
	props->addSetter("ruleList", &Filter::SetRuleList, true);
	props->addGetter("ruleFile", &Filter::GetRuleFile);
	props->addSetter("ruleFile", &Filter::SetRuleFile, true);
}

Filter::~Filter() {
	free(ruleFile);
	free(ruleList);

	delete props;

	for (tr1::unordered_map<int, vector<Rule*>*>::iterator iter = rules.begin(); iter != rules.end(); ++iter) {
		for (vector<Rule*>::iterator iter2 = iter->second->begin(); iter2 != iter->second->end(); ++iter2) {
			delete (*iter2);
		}
	}
}

char *Filter::GetItems(const char *name) {
	return int2str(items);
}

char *Filter::GetRuleList(const char *name) {
	return ruleList ? strdup(ruleList) : NULL;
}

void Filter::SetRuleList(const char *name, const char *value) {
	free(ruleList);
	ruleList = strdup(value);
}

char *Filter::GetRuleFile(const char *name) {
	return ruleFile ? strdup(ruleFile) : NULL;
}

void Filter::SetRuleFile(const char *name, const char *value) {
	free(ruleFile);
	ruleFile = strdup(value);
}

bool Filter::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;
	return true;
}

vector<Filter::Rule*> *Filter::InitResource(Resource *resource) {
	string data;
	if (ruleFile) {
		FILE *f = fopen(ruleFile, "r");
		if (!f) {
			LOG_ERROR(this, "Cannot open file: " << ruleFile << ": " << strerror(errno));
			return NULL;
		}
		fseek(f, 0, SEEK_END);
		long len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *buffer = (char*)malloc(len);
		if (fread(buffer, len, 1, f) < 0) {
			LOG_ERROR(this, "Cannot read file: " << ruleFile << ": " << strerror(errno));
			return NULL;
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
	vector<Filter::Rule*> *rules = new vector<Filter::Rule*>();
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
		if (!r->Init(&line, lineNo, resource)) {
			delete r;
			for (vector<Filter::Rule*>::iterator iter = rules->begin(); iter != rules->end(); ++iter)
				delete *iter;
			return NULL;
		}
		rules->push_back(r);
	}
	return rules;
}

Resource *Filter::ProcessSimple(Resource *resource) {
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	std::tr1::unordered_map<int, vector<Filter::Rule*>*>::iterator iter = rules.find(resource->GetTypeId());
	vector<Filter::Rule*> *rs;
	if (iter != rules.end()) {
		rs = iter->second;
	} else {
		rs = InitResource(resource);
		rules[resource->GetTypeId()] = rs;
		if (!rs)
			LOG_ERROR(this, "Cannot initialize Resource filter: " << resource->GetTypeStr());
		iter = rules.find(resource->GetTypeId());
	}
	if (!rs) {
		LOG_ERROR(this, "Cannot process Resource: " << resource->GetId());
		return resource;
	}
	
	// process rule-by rule and deal with the resource accordingly
	int i = 1;
	for (vector<Rule*>::iterator iter = rs->begin(); iter != rs->end(); ++iter) {
		switch ((*iter)->Apply(resource)) {
		case Filter::Action::ACCEPT:
			LOG_DEBUG(this, "[" << resource->GetId() << "]: ACCEPT #" << i);
			return resource;
		case Filter::Action::DROP:
			LOG_DEBUG(this, "[" << resource->GetId() << "]: DROP #" << i);
				// FIXME
				engine->DeleteResource(resource);
				return NULL;
			case Filter::Action::CONTINUE:
			default:
				// do nothing
				break;
			}
			i++;
		}
		return resource;
	}

	log4cxx::LoggerPtr Filter::Condition::logger(log4cxx::Logger::getLogger("module.Filter.Condition"));

	OpType Filter::Condition::FieldInfoToOpType(ResourceFieldInfo::FieldType type);
		OpType result;
		switch (type) {
		case ResourceFieldInfo::STRING:
		case ResourceFieldInfo::ARRAY_STRING:
		case ResourceFieldInfo::HASH_STRING:
			result = STRING;
			break;
		case ResourceFieldInfo::INT:
		case ResourceFieldInfo::ARRAY_INT:
		case ResourceFieldInfo::HASH_INT:
			result = INT;
			break;
		case ResourceFieldInfo::LONG:
		case ResourceFieldInfo::ARRAY_LONG:
		case ResourceFieldInfo::HASH_LONG:
			result = LONG;
			break;
		case ResourceFieldInfo::IP:
		case ResourceFieldInfo::ARRAY_IP:
		case ResourceFieldInfo::HASH_IP:
			result = IP;
			break;
		default:
			result = UNKNOWN;
			break;
		}
		return result;
	}

	bool ParseArray(yyscan_t *scanner, scanner_state *state, Operand *o, const char *name) {
		// a[]
		o->info = resource->GetFieldInfo(name);
		int type = o->info->GetType();
		if (type == ResourceFieldInfo::UNKNOWN) {
			LOG4CXX_ERROR(logger, "Invalid array name: " << text << " (line " << state->line << ")");
			return false;
		}
		switch (scanner_scan(&text, scanner)) {
		case TOK_NUMBER:
			// a[3]
			o->op = ARRAY;
			op->opType = FieldInfoToOpType(type);
			if (op->opType == UNKNOWN)
				return false;
			o->iValue = atoi(text);
			break;
		case TOK_STRING:
			// a["abc"]
			text = UnescapeString(text);
		case TOK_LABEL:
			// a[abc]
			o->op = HASH;
			op->opType = FieldInfoToOpType(type);
			if (op->opType == UNKNOWN)
				return false;
			o->sValue = text;
			break;
		case TOK_ALL:
			// a[*]
			if (type == ResourceFieldInfo::ARRAY_STRING || type == ResourceFieldInfo::ARRAY_INT || type == ResourceFieldInfo::ARRAY_LONG || type == ResourceFieldInfo::ARRAY_IP) {
				o->op = ARRAY_ALL;
			} else if (type == ResourceFieldInfo::HASH_STRING || type == ResourceFieldInfo::HASH_INT || type == ResourceFieldInfo::HASH_LONG || type == ResourceFieldInfo::HASH_IP) {
				o->op = HASH_ALL;
			} else {
				LOG4CXX_ERROR(logger, "Invalid array type: " << text << " (line " << state->line << ")");
				return false;
			}
			o->opType = FieldInfoToOpType(type);
			break;
		case TOK_ANY:
			// a[?]
			if (type == ResourceFieldInfo::ARRAY_STRING || type == ResourceFieldInfo::ARRAY_INT || type == ResourceFieldInfo::ARRAY_LONG || type == ResourceFieldInfo::ARRAY_IP) {
				o->op = ARRAY_ANY;
			} else if (type == ResourceFieldInfo::HASH_STRING || type == ResourceFieldInfo::HASH_INT || type == ResourceFieldInfo::HASH_LONG || type == ResourceFieldInfo::HASH_IP) {
				o->op = HASH_ANY;
			} else {
				LOG4CXX_ERROR(logger, "Invalid array type: " << text << " (line " << state->line << ")");
				return false;
			}
			o->opType = FieldInfoToOpType(type);
			break;
		case TOK_EOF:
			return false;
		default:
			LOG4CXX_ERROR(logger, "Invalid input encountered: " << text << " (line " << state->line << ")");
			return false;
		}
		return true;
	}

	

	bool Filter::Condition::ParseOperand(yyscan_t *scanner, scanner_state *state, Operand *o, bool lvalue) {
		o->type = UNKNOWN;
		o->length = false;
		o->info = NULL;
		char *text;
		switch (scanner_scan(&text, scanner)) {
		case TOK_FUNCTION_CLEAR:
			if (lvalue) {
			}
			switch (scanner_scan(&text, scanner)) {
			case TOK_LABEL:
				o->info = Resource::GetFieldInfo(text);
				int type = o->info->GetType();
				if (type != ResourceFieldInfo::ARRAY_) {
				}
				break;
			case TOK_EOF:
				return false;
			default:
			}
			break;
		case TOK_FUNCTION_DELETE:
			switch (scanner_scan(&text, scanner)) {
			case TOK_ARRAY:
				o->info = Resource::GetFieldInfo(text);
				int type = o->info->GetType();
				if (type != ResourceFieldInfo::ARRAY_) {
				}
				break;
			case TOK_EOF:
				return false;
			default:
				// ERROR
			}
			break;
		case TOK_FUNCTION_COUNT:
			break;
		case TOK_FUNCTION_LENGTH:
			if (lvalue) {
				LOG4CXX_ERROR(logger, "length() cannot be used as a l-value: (line " << state->line << ")");
				return false;
			}
			o->length = true;
			switch (scanner_scan(&text, scanner)) {
			case TOK_LABEL:
				// length(var)
				o->op = SCALAR:
				o->opType = STRING;
				o->info = resource->GetFieldInfo(text);
				if (o->info->GetType() != ResourceFieldInfo::STRING) {
					LOG4CXX_ERROR(logger, "Invalid length() variable: " << text << " (line " << state->line << ")");
					return false;
				}
				break;
			case TOK_STRING:
				// length("abc")
				o->op = SCALAR;
				o->opType = INT;
				iValue = strlen(UnescapeString(text));
				length = false;
				break;
			case TOK_ARRAY:
				if (!ParseArray())
					return false;
				if (o->opType != STRING) {
					LOG4CXX_ERROR(logger, "length() requires string type (line " << state->line << ")");
					return false;
				}
			case TOK_EOF:
				return false;
			default:
				LOG4CXX_ERROR(logger, "Invalid input encountered: " << text << " (line " << state->line << ")");
				return false;
			}
			break;
		case TOK_FUNCTION_DELETE_KEY:
		case TOK_FUNCTION_DELETE_VALUE:
		case TOK_FUNCTION_KEEP_KEY:
		case TOK_FUNCTION_KEEP_VALUE:
			break;
		case TOK_LABEL:
			// var
			o->op = SCALAR:
			o->info = resource->GetFieldInfo(text);
			op->opType = FieldInfoToOpType(o->info->GetType());
			if (op->opType == UNKNOWN)
				return false;
			break;
		case TOK_NUMBER:
			// 3
			if (lvalue) {
				LOG4CXX_ERROR(logger, "Constant cannot be used as a l-value: " << text << " (line " << state->line << ")");
				return false;
			}
			iValue = atoi(text);
			o->op = SCALAR;
			o->opType = INT;
			break;
		case TOK_STRING:
			// "abc"
			if (lvalue) {
				LOG4CXX_ERROR(logger, "Constant cannot be used as a l-value: " << text << " (line " << state->line << ")");
				return false;
			}
			sValue = UnescapeString(text);
			o->op = SCALAR;
			o->opType = STRING;
			break;
		case TOK_REGEX:
			// "abc"
			if (lvalue) {
				LOG4CXX_ERROR(logger, "Regular expression cannot be used as a l-value: " << text << " (line " << state->line << ")");
				return false;
			}
			o->re_subst = false;
			o->re_global = false;
			bool caseless = false;
			string match = text;
			bool re_end = false;
			while (!re_end) {
				switch (scanner_scan(&text, scanner)) {
				case TOK_REGEX_NOFLAGS:
					re_end = true;
					break;
				case TOK_REGEX_G:
					o->re_global = true;
					break;
				case TOK_REGEX_I:
					caseless = true;
					break;
				case TOK_REGEX_SUBST:
					o->re_subst = true;
					o->re_subst_text = text;
					break;
				case TOK_EOF:
					return false;
				default:
					LOG4CXX_ERROR(logger, "Invalid token: " << text << " (line " << state->line << ")");
					return false;
				}
			}
			o->regex = new pcrecpp::RE(match, caseless ? pcrecpp::RE_Options().set_caseless(true).set_utf8(true) : pcrecpp::RE_Options().set_utf8(true));
			o->op = SCALAR;
			o->opType = STRING;
			break;
		case TOK_IP4:
			o->ip_prefix = 0;
			if (!o->ip.parseIp4(text)) {
				LOG4CXX_ERROR(logger, "Invalid IPv4 address: " << text << " (line " << state->line << ")");
				return false;
			}
			switch (scanner_scan(&text, scanner)) {
			case TOK_IP_PREFIX:
				o->ip_prefix = atoi(text);
				break;
			case TOK_IP_NOPREFIX:
				break;
			case TOK_EOF:
				return false;
			}
			break;
		case TOK_IP6:
			o->ip_prefix = 0;
			if (!o->ip.parseIp6(text)) {
				LOG4CXX_ERROR(logger, "Invalid IPv6 address: " << text << " (line " << state->line << ")");
				return false;
			}
			switch (scanner_scan(&text, scanner)) {
			case TOK_IP_PREFIX:
				o->ip_prefix = atoi(text);
				break;
			case TOK_IP_NOPREFIX:
				break;
			case TOK_EOF:
				return false;
			}
			break;
		default:
			LOG4CXX_ERROR(logger, "Invalid input encountered: " << text << " (line " << state->line << ")");
			return false;
		}
	}

	void EvaluateOperand(Operand *o) {
		
	}

	bool filter::condition::init(yyscan_t *scanner, scanner_state *state) {
		length = false;
// parse operand1, operator, operand2
// check that operator is compatible with operands
// eval: eval operand1, operand2 and apply operator


	bool Filter::Condition::Init(string *data, int lineNo, Resource *resource) {
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
		info = resource->GetFieldInfo(label.c_str());
		if (info->GetType() == ResourceFieldInfo::UNKNOWN) {
			LOG4CXX_ERROR(logger, "Invalid label encountered: " << label << " (line " << lineNo << ")");
			return false;
		}
		if (info->GetType() == ResourceFieldInfo::STRING2) {
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
			switch (info->GetType()) {
			case ResourceFieldInfo::STRING:
				this->op = STRING_EQ;
				break;
			case ResourceFieldInfo::INT:
				this->op = INT_EQ;
				break;
			case ResourceFieldInfo::LONG:
				this->op = LONG_EQ;
				break;
			default:
				error = true;
			}
		} else if (op == "!=" || op == "<>") {
			switch (info->GetType()) {
			case ResourceFieldInfo::STRING:
				this->op = STRING_NE;
				break;
			case ResourceFieldInfo::INT:
				this->op = INT_NE;
				break;
			case ResourceFieldInfo::LONG:
				this->op = LONG_NE;
				break;
			default:
				error = true;
			}
		} else if (op == "<") {
			switch (info->GetType()) {
			case ResourceFieldInfo::STRING:
				this->op = STRING_LT;
				break;
			case ResourceFieldInfo::INT:
				this->op = INT_LT;
				break;
			case ResourceFieldInfo::LONG:
				this->op = LONG_LT;
				break;
			default:
				error = true;
			}
		} else if (op == ">") {
			switch(info->GetType()) {
			case ResourceFieldInfo::STRING:
				this->op = STRING_GT;
				break;
			case ResourceFieldInfo::INT:
				this->op = INT_GT;
				break;
			case ResourceFieldInfo::LONG:
				this->op = LONG_GT;
				break;
			default:
				error = true;
			}
		} else if (op == "<=") {
			switch (info->GetType()) {
			case ResourceFieldInfo::INT:
				this->op = INT_LE;
				break;
			case ResourceFieldInfo::LONG:
				this->op = LONG_LE;
				break;
			default:
				error = true;
			}
		} else if (op == ">=") {
			switch (info->GetType()) {
			case ResourceFieldInfo::INT:
				this->op = INT_GE;
				break;
			case ResourceFieldInfo::LONG:
				this->op = LONG_GE;
				break;
			default:
				error = true;
			}
		} else if (op == "=~") {
			switch (info->GetType()) {
			case ResourceFieldInfo::STRING:
				this->op = STRING_REGEX;
				break;
			case ResourceFieldInfo::IP4:
				this->op = IP4_EQ;
				break;
			case ResourceFieldInfo::IP6:
				this->op = IP6_EQ;
				break;
			default:
				error = true;
			}
		} else if (op == "!~") {
			switch (info->GetType()) {
			case ResourceFieldInfo::STRING:
				this->op = STRING_NREGEX;
				break;
			case ResourceFieldInfo::IP4:
				this->op = IP4_NE;
				break;
			case ResourceFieldInfo::IP6:
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

	bool Filter::Condition::isTrue(Resource *resource) {
		const char *sValue;
		int iValue;
		ip4_addr_t a4Value;
		ip6_addr_t a6Value;
		switch (info->GetType()) {
		case ResourceFieldInfo::STRING:
			sValue = info->GetString(resource);
			if (!sValue)
				return false;
			break;
		case ResourceFieldInfo::INT:
			iValue = info->GetInt(resource);
			break;
		case ResourceFieldInfo::LONG:
			lValue = info->GetLong(resource);
			break;
		case ResourceFieldInfo::IP4:
			a4Value = info->GetIp4Addr(resource);
			if (prefix)
				a4Value.addr &= ((uint32_t)0xffffffff) << (32-prefix);
			break;
		case ResourceFieldInfo::IP6:
			{
				a6Value = info->GetIp6Addr(resource);
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
		case ResourceFieldInfo::STRING2:
			sValue = info->GetString2(resource, name.c_str());
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
				if (info->GetType() == ResourceFieldInfo::STRING)
					info->SetString(resource, s.c_str());
				else if (info->GetType() == ResourceFieldInfo::STRING2)
					info->SetString2(resource, name.c_str(), s.c_str());
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

	bool Filter::Action::Init(string *data, int lineNo, Resource *resource) {
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
			info = resource->GetFieldInfo(label.c_str());
			if (info->GetType() == ResourceFieldInfo::UNKNOWN) {
				LOG4CXX_ERROR(logger, "Invalid label encountered: " << label << " (line " << lineNo << ")");
				return false;
			}
			return true;
		} else if (label == "STATUS") {
			this->type = SETVAL;
			info = resource->GetFieldInfo("status");
			// to be continued :)
		} else {
			this->type = SETVAL;
			info = resource->GetFieldInfo(label.c_str());
			if (info->GetType() == ResourceFieldInfo::UNKNOWN) {
				LOG4CXX_ERROR(logger, "Invalid label encountered: " << label << " (line " << lineNo << ")");
				return false;
			}
			if (info->GetType() == ResourceFieldInfo::STRING2) {
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
		switch (info->GetType()) {
		case ResourceFieldInfo::STRING:
		case ResourceFieldInfo::STRING2:
			if (!parseString(data, &this->sValue, '"'))
				error = true;
			break;
		case ResourceFieldInfo::INT:
			if (!parseInt(data, &this->iValue))
				error = true;
			break;
		case ResourceFieldInfo::LONG:
			if (!parseLong(data, &this->lValue))
				error = true;
			break;
		case ResourceFieldInfo::IP4:
			if (!parseIp4(data, &this->a4Value))
				error = true;
			break;
		case ResourceFieldInfo::IP6:
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

	Filter::Action::ActionType Filter::Action::Apply(Resource *resource) {
		switch (type) {
		case ACCEPT:
			return ACCEPT;
		case DROP:
			return DROP;
		case CONTINUE:
			return CONTINUE;
		case CLEAR:
			info->clear(resource);
			return CLEAR;
		case SETVAL:
			switch (info->GetType()) {
			case ResourceFieldInfo::STRING:
				info->SetString(resource, sValue.c_str());
				break;
			case ResourceFieldInfo::INT:
				info->SetInt(resource, iValue);
				break;
			case ResourceFieldInfo::LONG:
				info->SetLong(resource, lValue);
				break;
			case ResourceFieldInfo::IP4:
				info->SetIp4Addr(resource, a4Value);
				break;
			case ResourceFieldInfo::IP6:
				info->SetIp6Addr(resource, a6Value);
				break;
			case ResourceFieldInfo::STRING2:
				info->SetString2(resource, name.c_str(), sValue.c_str());
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

	bool Filter::Rule::Init(string *line, int lineNo, Resource *resource) {
		// create a rule from line: parse [condition|*] => [action]
		skipWs(line);
		if (line->length() > 0 && line->at(0) == '*') {
			line->erase(0, 1);
		} else {
			while (true) {
				Condition *c = new Condition();
				if (!c->Init(line, lineNo, resource)) {
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
			if (!a->Init(line, lineNo, resource)) {
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

	Filter::Action::ActionType Filter::Rule::Apply(Resource *resource) {
		for (vector<Condition*>::iterator iter = conditions.begin(); iter != conditions.end(); ++iter) {
			if (!(*iter)->isTrue(resource))
				return Filter::Action::CONTINUE;
		}
		Filter::Action::ActionType result = Filter::Action::CONTINUE;
		for (vector<Action*>::iterator iter = actions.begin(); iter != actions.end(); ++iter) {
			Filter::Action::ActionType r = (*iter)->Apply(resource);
			if (r == Filter::Action::ACCEPT || r == Filter::Action::DROP || r == Filter::Action::CONTINUE)
				result = r;
		}
		return result;
	}

	// the class factories

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new Filter(objects, id, threadIndex);
}
