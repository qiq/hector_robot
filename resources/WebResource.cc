
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));

WebResource::WebResource() {
}

WebResource::~WebResource() {
}

ProtobufResource *WebResource::Clone() {
	return new WebResource(*this);
}

int WebResource::getSize() {
	return 1; //FIXME
}

void WebResource::setHeaderFields(vector<string> *header_fields) {
	r.clear_header_fields();
	for (vector<string>::iterator iter = header_fields->begin(); iter != header_fields->end(); ++iter) {
		r.add_header_fields(*iter);
	}
}

vector<string> *WebResource::getHeaderFields() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.header_fields_size(); i++) {
		result->push_back(r.header_fields(i));
	}
	return result;
}

vector<string> *WebResource::getHeaderFieldNames() {
}

bool WebResource::setHeaderField(const char *name, const char *value) {
}

const char *WebResource::getHeaderField(const char *name) {
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

char *WebResource::toString() {
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
	snprintf(buf, sizeof(buf), ", time: %u, mime: %s, size: %d\n", this->getTime(), this->getMimeType(), strlen(this->getContent()));
	s += buf;
	vector<string> *v = this->getHeaderFields();
	if (v) {
		s += "headers:\n";
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			s += *iter;
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

