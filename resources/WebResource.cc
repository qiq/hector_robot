
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
		snprintf(buf, sizeof(buf), " %s:%d %s %s %s)", this->getUrlHost(), this->getUrlPort(), this->getUrlPath(), this->getUrlQuery(), this->getUrlRef());
		s += buf;
	}
	snprintf(buf, sizeof(buf), ", time: %ld, mime: %s, size: %d\n", this->getTime(), this->getMimeType(), strlen(this->getContent()));
	s += buf;
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
		result.type = INTEGER;
		result.get.i = &WebResource::getId;
		result.set.i = &WebResource::setId;
	} else if (!strcmp(name, "status")) {
		result.type = INTEGER;
		result.get.i = &WebResource::getStatus;
		result.set.i = &WebResource::setStatus;
	} else if (!strcmp(name, "url")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrl;
		result.set.s = &WebResource::setUrl;
	} else if (!strcmp(name, "time")) {
		result.type = LONG;
		result.get.l = &WebResource::getTime;
		result.set.l = &WebResource::setTime;
	} else if (!strcmp(name, "mimeType")) {
		result.type = STRING;
		result.get.s = &WebResource::getMimeType;
		result.set.s = &WebResource::setMimeType;
	} else if (!strcmp(name, "mimeType")) {
		result.type = STRING;
		result.get.s = &WebResource::getMimeType;
		result.set.s = &WebResource::setMimeType;
	} else if (!strcmp(name, "content")) {
		result.type = STRING;
		result.get.s = &WebResource::getContent;
		result.set.s = &WebResource::setContent;
	} else if (!strcmp(name, "content")) {
		result.type = STRING;
		result.get.s = &WebResource::getContent;
		result.set.s = &WebResource::setContent;
	} else if (!strcmp(name, "header")) {
		result.type = STRING;
		result.get.s2 = &WebResource::getHeaderValue;
		result.set.s2 = &WebResource::setHeaderValue;
	} else if (!strcmp(name, "urlScheme")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlScheme;
		result.set.s = &WebResource::setUrlScheme;
	} else if (!strcmp(name, "UrlUsername")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlUsername;
		result.set.s = &WebResource::setUrlUsername;
	} else if (!strcmp(name, "UrlPassword")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlPassword;
		result.set.s = &WebResource::setUrlPassword;
	} else if (!strcmp(name, "UrlHost")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlHost;
		result.set.s = &WebResource::setUrlHost;
	} else if (!strcmp(name, "UrlPort")) {
		result.type = INTEGER;
		result.get.i = &WebResource::getUrlPort;
		result.set.i = &WebResource::setUrlPort;
	} else if (!strcmp(name, "UrlPath")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlPath;
		result.set.s = &WebResource::setUrlPath;
	} else if (!strcmp(name, "UrlQuery")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlQuery;
		result.set.s = &WebResource::setUrlQuery;
	} else if (!strcmp(name, "UrlRef")) {
		result.type = STRING;
		result.get.s = &WebResource::getUrlRef;
		result.set.s = &WebResource::setUrlRef;
	} else {
		result.type = UNKNOWN;
	}
	return result;
}
