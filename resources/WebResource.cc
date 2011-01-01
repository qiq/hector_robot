#include <config.h>

#include "googleurl/src/gurl.h"
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));

WebResource::WebResource() {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
}

WebResource::WebResource(const WebResource &wr) : ProtobufResource(wr), r(wr.r), headers(wr.headers) {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
}

WebResource::~WebResource() {
}

ProtobufResource *WebResource::Clone() {
	return new WebResource(*this);
}

void WebResource::setHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values) {
	r.clear_header_names();
	r.clear_header_values();
	assert(names.size() == values.size());
	for (int i = 0; i < (int)names.size(); i++) {
		r.add_header_names(names[i]);
		r.add_header_values(values[i]);
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
	header_map_ready = 1;
}

void WebResource::SaveHeaders() {
	r.clear_header_names();
	r.clear_header_values();
	for (tr1::unordered_map<string, string>::iterator iter = headers.begin(); iter != headers.end(); ++iter) {
		r.add_header_names(iter->first);
		r.add_header_values(iter->second);
	}
	header_map_dirty = 0;
}

void WebResource::LoadIpAddr() {
	uint32_t a = r.ip4_addr();
	if (a != 0) {
		addr.setIp4Addr(r.ip4_addr());
	} else {
		uint64_t a1 = r.ip6_addr_1();
		uint64_t a2 = r.ip6_addr_2();
		if (a1 != 0 || a2 != 0) {
			addr.setIp6Addr(a1, true);
			addr.setIp6Addr(a2, false);
		} else {
			addr.setEmpty();
		}
	}
}

void WebResource::SaveIpAddr() {
	if (addr.isIp4Addr()) {
		if (!addr.isEmpty()) {
			r.set_ip4_addr(addr.getIp4Addr());
		} else {
			r.clear_ip4_addr();
		}
		r.clear_ip6_addr_1();
		r.clear_ip6_addr_2();
	
	} else {
		if (!addr.isEmpty()) {
			r.set_ip6_addr_1(addr.getIp6Addr(true));
			r.set_ip6_addr_2(addr.getIp6Addr(false));
		} else {
			r.clear_ip6_addr_1();
			r.clear_ip6_addr_2();
		}
		r.clear_ip4_addr();
	}
}

void WebResource::LoadParsedUrl() {
	// parse URL
	GURL *gurl = new GURL(r.url());
	if (gurl->SchemeIs("http"))
		url.scheme = HTTP;
	else if (gurl->SchemeIs("https"))
		url.scheme = HTTPS;
	else
		url.scheme = NONE;
	url.username = gurl->username();
	url.password = gurl->password();
	url.host = gurl->host();
	url.port = gurl->EffectiveIntPort();
	string p = gurl->path();
	string q = gurl->query();
	if (!q.empty()) {
		p += "?";
		p += q;
	}
	url.path = p;
	delete gurl;

	parsed_url_ready = 1;
}

void WebResource::SaveParsedUrl() {
	// construct url
	string s;
	int defaultPort;
	switch (url.scheme) {
	case HTTP:
		s = "http";
		defaultPort = 80;
		break;
	case HTTPS:
		s = "https";
		defaultPort = 443;
		break;
	case NONE:
	default:
		break;
	}
	s += "://";
	if (url.username != "") {
		s += url.username;
		s += ":";
		s += url.password;
		s += "@";
	}
	s += url.host;
	if (url.port != defaultPort) {
		char buffer[21];
		snprintf(buffer, sizeof(buffer), ":%d", url.port);
		s += buffer;
	}
	s += url.path;
	r.set_url(s);

	parsed_url_dirty = 0;
}

void WebResource::setHeaderValue(const std::string &name, const std::string &value) {
	if (!header_map_ready)
		LoadHeaders();
	headers[name] = value;
	header_map_dirty = 1;
}

const std::string &WebResource::getHeaderValue(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	tr1::unordered_map<string, string>::iterator iter = headers.find(name);
	if (iter == headers.end())
		return empty_string;
	return iter->second;
}

int WebResource::getHeaderCount() {
	if (header_map_ready)
		return headers.size();
	else
		return r.header_names_size();
}

void WebResource::clearHeaderField(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	headers.erase(name);
	header_map_dirty = 1;
}

void WebResource::clearHeaderFields() {
	r.clear_header_names();
	r.clear_header_values();
	headers.clear();
	header_map_ready = 1;
	header_map_dirty = 1;
}

string WebResource::toString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[WR %d %d] url: %s", this->getId(), this->getStatus(), this->getUrl().c_str());
	s = buf;
	snprintf(buf, sizeof(buf), " (%s", Scheme_Name((Scheme)this->getUrlScheme()).c_str());
	s += buf;
	if (this->getUrlUsername().length() > 0) {
		snprintf(buf, sizeof(buf), " %s:%s", this->getUrlUsername().c_str(), this->getUrlPassword().c_str());
		s += buf;
	}
	snprintf(buf, sizeof(buf), " %s:%d %s)", this->getUrlHost().c_str(), this->getUrlPort(), this->getUrlPath().c_str());
	s += buf;
	snprintf(buf, sizeof(buf), ", size: %d", (int)this->getContent().length());
	s += buf;
	s += ", ip: ";
	s += addr.toString();
	snprintf(buf, sizeof(buf), ", scheduled: %ld", this->getScheduled());
	s += buf;
	if (header_map_dirty)
		SaveHeaders();
	vector<string> *v = this->getHeaderNames();
	if (v->size() > 0) {
		s += "\nheaders: ";
		bool first = true;
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			const std::string &value = this->getHeaderValue(iter->c_str());
			if (first)
				first = false;
			else
				s += ", ";
			s += *iter;
			s += ": ";
			s += value;
		}
	}
	delete v;
	if (this->getContent().length() > 0) {
		s += "\ncontent:\n";
		s += this->getContent();
	}
	return s;
}

template<class T>
ResourceFieldInfoT<T>::ResourceFieldInfoT(const std::string &name) {
	if (name == "id") {
		type = INT;
		get_u.i = &WebResource::getId;
		set_u.i = &WebResource::setId;
		clear_all = NULL;
	} else if (name == "status") {
		type = INT;
		get_u.i = &WebResource::getStatus;
		set_u.i = &WebResource::setStatus;
		clear_all = NULL;
	} else if (name == "url") {
		type = STRING;
		get_u.s = &WebResource::getUrl;
		set_u.s = &WebResource::setUrl;
		clear_all = &WebResource::clearUrl;
	} else if (name == "ipAddr") {
		type = IP;
		get_u.ip = &WebResource::getIpAddr;
		set_u.ip = &WebResource::setIpAddr;
		clear_all = &WebResource::clearIpAddr;
	} else if (name == "header") {
		type = HASH_STRING;
		get_u.hs = &WebResource::getHeaderValue;
		set_u.hs = &WebResource::setHeaderValue;
		count = &WebResource::getHeaderCount;
		get_all_keys = &WebResource::getHeaderNames;
		delete_hash_item = &WebResource::clearHeaderField;
		clear_all = &WebResource::clearHeaderFields;
	} else if (name == "redirectCount") {
		type = INT;
		get_u.i = &WebResource::getRedirectCount;
		set_u.i = &WebResource::setRedirectCount;
		clear_all = &WebResource::clearRedirectCount;
	} else if (name == "content") {
		type = STRING;
		get_u.s = &WebResource::getContent;
		set_u.s = &WebResource::setContent;
		clear_all = &WebResource::clearContent;
	} else if (name == "scheduled") {
		type = LONG;
		get_u.l = &WebResource::getScheduled;
		set_u.l = &WebResource::setScheduled;
		clear_all = &WebResource::clearScheduled;
	} else if (name == "urlScheme") {
		type = INT;
		get_u.i = &WebResource::getUrlScheme;
		set_u.i = &WebResource::setUrlScheme;
		clear_all = &WebResource::clearUrlScheme;
	} else if (name == "urlUsername") {
		type = STRING;
		get_u.s = &WebResource::getUrlUsername;
		set_u.s = &WebResource::setUrlUsername;
		clear_all = &WebResource::clearUrlUsername;
	} else if (name == "urlPassword") {
		type = STRING;
		get_u.s = &WebResource::getUrlPassword;
		set_u.s = &WebResource::setUrlPassword;
		clear_all = &WebResource::clearUrlPassword;
	} else if (name == "urlHost") {
		type = STRING;
		get_u.s = &WebResource::getUrlHost;
		set_u.s = &WebResource::setUrlHost;
		clear_all = &WebResource::clearUrlHost;
	} else if (name == "urlPort") {
		type = INT;
		get_u.i = &WebResource::getUrlPort;
		set_u.i = &WebResource::setUrlPort;
		clear_all = &WebResource::clearUrlPort;
	} else if (name == "urlPath") {
		type = STRING;
		get_u.s = &WebResource::getUrlPath;
		set_u.s = &WebResource::setUrlPath;
		clear_all = &WebResource::clearUrlPath;
	}
}
