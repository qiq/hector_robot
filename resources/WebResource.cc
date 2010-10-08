
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));

WebResource::WebResource() {
	this->header_map_ready = false;
	this->header_map_dirty = false;
}

WebResource::~WebResource() {
}

ProtobufResource *WebResource::Clone() {
	return new WebResource(*this);
}

int WebResource::getSize() {
	return 1; //FIXME
}

void WebResource::setHeaderFields(std::vector<std::string> *names, std::vector<std::string> *values) {
	r.clear_header_names();
	r.clear_header_values();
	assert(names->size() == values->size());
	for (int i = 0; i < names->size(); i++) {
		r.add_header_names((*names)[i]);
		r.add_header_values((*values)[i]);
	}
}

vector<string> *WebResource::getHeaderNames() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.header_names_size(); i++) {
		result->push_back(r.header_names(i));
	}
	return result;
}

void WebResource::LoadHeaders() {
	headers.clear();
	for (int i = 0; i < r.header_names_size(); i++)
		headers[r.header_names(i)] = r.header_values(i);
	header_map_ready = true;
}

void WebResource::SaveHeaders() {
	r.clear_header_names();
	r.clear_header_values();
	for (tr1::unordered_map<string, string>::iterator iter = headers.begin(); iter != headers.end(); ++iter) {
		r.add_header_names(iter->first);
		r.add_header_values(iter->second);
	}
	header_map_dirty = false;
}

void WebResource::setHeaderValue(const char *name, const char *value) {
	if (!header_map_ready)
		LoadHeaders();
	headers[name] = value;
	header_map_dirty = true;
}

const char *WebResource::getHeaderValue(const char *name) {
	if (!header_map_ready)
		LoadHeaders();
	tr1::unordered_map<string, string>::iterator iter = headers.find(name);
	if (iter == headers.end())
		return NULL;
	return iter->second.c_str();
}

void WebResource::clearHeaderFields() {
	r.clear_header_names();
	r.clear_header_values();
	headers.clear();
}

void WebResource::setExtractedUrls(vector<string> *extracted_urls) {
	r.clear_extracted_urls();
	for (vector<string>::iterator iter = extracted_urls->begin(); iter != extracted_urls->end(); ++iter) {
		r.add_extracted_urls(*iter);
	}
}

vector<string> *WebResource::getExtractedUrls() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.extracted_urls_size(); i++) {
		result->push_back(r.extracted_urls(i));
	}
	return result;
}

void WebResource::clearExtractedUrls() {
	r.clear_extracted_urls();
}

char *WebResource::toString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "WebResource [%d, %d]: url: %s", this->getId(), this->getStatus(), this->getUrl());
	s = buf;
	if (strlen(this->getUrlScheme()) > 0) {
		snprintf(buf, sizeof(buf), " (%s", this->getUrlScheme());
		s += buf;
		if (strlen(this->getUrlUsername()) > 0) {
			snprintf(buf, sizeof(buf), " %s:%s", this->getUrlUsername(), this->getUrlPassword());
			s += buf;
		}
		snprintf(buf, sizeof(buf), " %s:%d %s %s)", this->getUrlHost(), this->getUrlPort(), this->getUrlPath(), this->getUrlQuery());
		s += buf;
	}
	snprintf(buf, sizeof(buf), ", time: %ld, mime: %s, size: %d", this->getTime(), this->getMimeType(), strlen(this->getContent()));
	s += buf;
	char *a = ip4Addr2Str(this->getIp4Addr());
	snprintf(buf, sizeof(buf), ", ip4: %s", a);
	free(a);
	s += buf;
	a = ip6Addr2Str(this->getIp6Addr());
	snprintf(buf, sizeof(buf), ", ip6: %s", a);
	free(a);
	s += buf;
	if (this->getIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", ip expire: %ld", this->getIpAddrExpire());
		s += buf;
	}
	s += "\n";
	vector<string> *v = this->getHeaderNames();
	if (v) {
		s += "headers:\n";
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			const char *value = this->getHeaderValue(iter->c_str());
			s += *iter;
			s += ": ";
			s += value;
			s += "\n";
		}
		delete v;
	}
	v = this->getExtractedUrls();
	if (v) {
		s += "urls:\n";
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			s += *iter;
			s += "\n";
		}
		delete v;
	}
	if (strlen(this->getContent()) > 0) {
		s += "content:\n";
		s += this->getContent();
		s += "\n";
	}
	return strdup(s.c_str());
}

WebResource::FieldInfo WebResource::getFieldInfo(const char *name) {
	WebResource::FieldInfo result;
	if (!strcmp(name, "id")) {
		result.type = INT;
		result.get.i = &WebResource::getId;
		result.set.i = &WebResource::setId;
		result.clear = NULL;
	} else if (!strcmp(name, "status")) {
		result.type = INT;
		result.get.i = &WebResource::getStatus;
		result.set.i = &WebResource::setStatus;
		result.clear = NULL;
	} else if (!strcmp(name, "url")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrl;
		result.set.s = &WebResource::setUrl;
		result.clear = &WebResource::clearUrl;
	} else if (!strcmp(name, "time")) {
		result.type = LONG;
		result.get.l = &WebResource::getTime;
		result.set.l = &WebResource::setTime;
		result.clear = &WebResource::clearTime;
	} else if (!strcmp(name, "mimeType")) {
		result.type = STRING;
		result.get.s = &WebResource::getMimeType;
		result.set.s = &WebResource::setMimeType;
		result.clear = &WebResource::clearMimeType;
	} else if (!strcmp(name, "content")) {
		result.type = STRING;
		result.get.s = &WebResource::getContent;
		result.set.s = &WebResource::setContent;
		result.clear = &WebResource::clearContent;
	} else if (!strcmp(name, "header")) {
		result.type = STRING2;
		result.get.s2 = &WebResource::getHeaderValue;
		result.set.s2 = &WebResource::setHeaderValue;
		result.clear = &WebResource::clearHeaderFields;
	} else if (!strcmp(name, "ip4Addr")) {
		result.type = IP4;
		result.get.a4 = &WebResource::getIp4Addr;
		result.set.a4 = &WebResource::setIp4Addr;
		result.clear = &WebResource::clearIp4Addr;
	} else if (!strcmp(name, "ip6Addr")) {
		result.type = IP6;
		result.get.a6 = &WebResource::getIp6Addr;
		result.set.a6 = &WebResource::setIp6Addr;
		result.clear = &WebResource::clearIp6Addr;
	} else if (!strcmp(name, "ipAddrExpire")) {
		result.type = LONG;
		result.get.l = &WebResource::getIpAddrExpire;
		result.set.l = &WebResource::setIpAddrExpire;
		result.clear = &WebResource::clearIpAddrExpire;
	} else if (!strcmp(name, "urlScheme")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlScheme;
		result.set.s = &WebResource::setUrlScheme;
		result.clear = &WebResource::clearUrlScheme;
	} else if (!strcmp(name, "urlUsername")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlUsername;
		result.set.s = &WebResource::setUrlUsername;
		result.clear = &WebResource::clearUrlUsername;
	} else if (!strcmp(name, "urlPassword")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlPassword;
		result.set.s = &WebResource::setUrlPassword;
		result.clear = &WebResource::clearUrlPassword;
	} else if (!strcmp(name, "urlHost")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlHost;
		result.set.s = &WebResource::setUrlHost;
		result.clear = &WebResource::clearUrlHost;
	} else if (!strcmp(name, "urlPort")) {
		result.type = INT;
		result.get.i = &WebResource::getUrlPort;
		result.set.i = &WebResource::setUrlPort;
		result.clear = &WebResource::clearUrlPort;
	} else if (!strcmp(name, "urlPath")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlPath;
		result.set.s = &WebResource::setUrlPath;
		result.clear = &WebResource::clearUrlPath;
	} else if (!strcmp(name, "urlQuery")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlQuery;
		result.set.s = &WebResource::setUrlQuery;
		result.clear = &WebResource::clearUrlQuery;
	} else {
		result.type = UNKNOWN;
	}
	return result;
}
