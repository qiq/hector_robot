/**
 *
 */
#include <config.h>

#include <assert.h>
#include "common.h"
#include "Resources.h"
#include "Filter.h"
#include "WebResource.h"

Filter::Filter(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	values = new ObjectValues<Filter>(this);

	values->addGetter("items", &Filter::getItems);
}

Filter::~Filter() {
	delete values;
}

char *Filter::getItems(const char *name) {
	return int2str(items);
}

bool Filter::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;
	typeId = Resources::Name2Id("WebResource");
	if (typeId < 0) {
		LOG_ERROR("Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *Filter::Process(Resource *resource) {
	WebResource *wr = dynamic_cast<WebResource*>(resource);
	//if (wr) {
	//}
	return resource;
}

log4cxx::LoggerPtr Filter::Condition::logger(log4cxx::Logger::getLogger("module.Filter.Condition"));

bool Filter::Condition::Init(string *data) {
	// first part, what: label | length(label) | label[name]
	string label = parseLabel(data);
	if (label == "length") {
		length = true;
		label = parseLabel(data);
	}
	if (label.empty()) {
		LOG_ERROR("Expected label: " << data);
		return false;
	}
	if (label == "url") {
		this->fgs = &WebResource::getUrl;
		this->fss = &WebResource::setUrl;
	} else if (label == "time") {
		//this->fgi = &WebResource::getTime;
		// FIXME
	} else if (label == "mimeType") {
		this->fgs = &WebResource::getMimeType;
		this->fss = &WebResource::setMimeType;
	} else if (label == "content") {
		this->fgs = &WebResource::getContent;
		this->fss = &WebResource::setContent;
	} else if (label == "header") {
		// FIXME
	} else if (label == "urlScheme") {
		this->fgs = &WebResource::getUrlScheme;
		this->fss = &WebResource::setUrlScheme;
	} else if (label == "urlUsername") {
		this->fgs = &WebResource::getUrlUsername;
		this->fss = &WebResource::setUrlUsername;
	} else if (label == "urlPassword") {
		this->fgs = &WebResource::getUrlPassword;
		this->fss = &WebResource::setUrlPassword;
	} else if (label == "urlHost") {
		this->fgs = &WebResource::getUrlHost;
		this->fss = &WebResource::setUrlHost;
	} else if (label == "urlPort") {
		this->fgi = &WebResource::getUrlPort;
		this->fsi = &WebResource::setUrlPort;
	} else if (label == "urlPath") {
		this->fgs = &WebResource::getUrlPath;
		this->fss = &WebResource::setUrlPath;
	} else if (label == "urlQuery") {
		this->fgs = &WebResource::getUrlQuery;
		this->fss = &WebResource::setUrlQuery;
	} else if (label == "urlRef") {
		this->fgs = &WebResource::getUrlRef;
		this->fss = &WebResource::setUrlRef;
	} else {
		LOG_ERROR("Invalid token, expected label: " << label);
		return false;
	}

	// second part, operator < | > | == | != | =~ | !~
	skipWs(data);
        size_t offset = data->find_first_not_of("<>=!~");
	string op;
	if (offset != string::npos) {
		if (offset == 0) {
			LOG_ERROR("No operator");
			return false;
		}
		op = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		op = *data;
		data->clear();
	}

	bool error;
	if (op == "==") {
		if (this->fgs)
			this->op = STRING_EQ;
		else if (this->fgi)
			this->op = INT_EQ;
		else if (this->fga4)
			this->op = IP4_EQ;
		else if (this->fga6)
			this->op = IP6_EQ;
		else
			error = true;
	} else if (op == "!=" || op == "<>") {
		if (this->fgs)
			this->op = STRING_NE;
		else if (this->fgi)
			this->op = INT_NE;
		else if (this->fga4)
			this->op = IP4_NE;
		else if (this->fga6)
			this->op = IP6_NE;
		else
			error = true;
	} else if (op == "<") {
		if (this->fgs)
			this->op = STRING_LT;
		else if (this->fgi)
			this->op = INT_LT;
		else
			error = true;
	} else if (op == ">") {
		if (this->fgs) 
			this->op = STRING_GT;
		else if (this->fgi)
			this->op = INT_GT;
		else
			error = true;
	} else if (op == "<=") {
		if (this->fgi)
			this->op = INT_LE;
		else
			error = true;
	} else if (op == ">=") {
		if (this->fgi)
			this->op = INT_GE;
		else
			error = true;
	} else if (op == "=~") {
		if (this->fgs)
			this->op = STRING_REGEX;
		else
			error = true;
	} else if (op == "!~") {
		if (this->fgs)
			this->op = STRING_NREGEX;
		else
			error = true;
	}
	if (error) {
		LOG_ERROR("Invalid operator: " << op);
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
		iValue = parseInt(data);
		break;
	case STRING_EQ:
	case STRING_NE:
	case STRING_LT:
	case STRING_GT:
		sValue = parseString(data, '"');
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
			string match = parseString(data, '/');
			if (subst) {
				this->replacement = parseString(data, '/');
				this->op = STRING_REGEX_SUBST;
				if (data->length() > 0 && data->at(0) == 'g') {
					this->global = true;
					data->erase(0, 1);
				}
			}
			this->regex = new pcrecpp::RE(match, pcrecpp::UTF8());
			break;
		}
	case IP4_EQ:
	case IP4_NE:
		a4Value = parseIp4(data);
		break;
	case IP6_EQ:
	case IP6_NE:
		a6Value = parseIp6(data);
		break;
	}
	return true;
}

bool Filter::Condition::isTrue(WebResource *wr) {
	const char *sValue;
	int iValue;
	ip4_addr_t a4Value;
	ip6_addr_t a6Value;
	if (fgs) {
		sValue = (wr->*fgs)();
		if (!sValue)
			return false;
	} else if (fgsn) {
		sValue = (wr->*fgsn)(name.c_str());
		if (!sValue)
			return false;
	} else if (fgi) {
		iValue = (wr->*fgi)();
	} else if (fga4) {
		a4Value = (wr->*fga4)();
		if (prefix)
			a4Value.addr &= ((uint32_t)0xffffffff) << (32-prefix);
	} else if (fga6) {
		a6Value = (wr->*fga6)();
		if (!prefix)
			prefix = 128;
		int a = prefix / 8;
		int b = prefix % 8;
		if (a < 16)
			a6Value.addr[a] &= 0xFF << (8-b);
		for (int i = a+1; i < 16; i++)
			a6Value.addr[i] = 0x00;
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
			if (fss)
				(wr->*fss)(s.c_str());
			else
				(wr->*fssn)(name.c_str(), s.c_str());
			return result;
		}
	case IP4_EQ:
		return memcmp(&a4Value, &this->a4Value, sizeof(ip4_addr_t));
	case IP4_NE:
		return !memcmp(&a4Value, &this->a4Value, sizeof(ip4_addr_t));
	case IP6_EQ:
		return memcmp(&a6Value, &this->a6Value, sizeof(ip6_addr_t));
	case IP6_NE:
		return !memcmp(&a6Value, &this->a6Value, sizeof(ip6_addr_t));
	default:
		LOG_ERROR("Invalid condition operator: " << op);
	}
	return false;
}

log4cxx::LoggerPtr Filter::Action::logger(log4cxx::Logger::getLogger("module.Filter.Action"));

bool Filter::Action::Init(string *data) {
	//TODO
}

Filter::Action::ActionType Filter::Action::Apply(WebResource *wr) {
	switch (type) {
	case ACCEPT:
		return ACCEPT;
	case DROP:
		return DROP;
	case CONTINUE:
		return CONTINUE;
	case SETVAL:
		if (fss)
			(wr->*fss)(sValue.c_str());
		else if (this->fssn)
			(wr->*fssn)(name.c_str(), sValue.c_str());
		else if (this->fsi)
			(wr->*fsi)(iValue);
		else if (this->fsa4)
			(wr->*fsa4)(a4Value);
		else if (this->fsa6)
			(wr->*fsa6)(a6Value);
		else
			LOG_ERROR("No value setter to use");
		return CONTINUE;
	}
	LOG_ERROR("Invalid action type: " << type);
	return DROP;
}

log4cxx::LoggerPtr Filter::Rule::logger(log4cxx::Logger::getLogger("module.Filter.Rule"));

bool Filter::Rule::Init(string *data) {
// parse && create one condition
	// find one line (possibly concatenate lines, if there is backslash at
	// the end of the line
	while (true) {
	}
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new Filter(objects, id, threadIndex);
}
