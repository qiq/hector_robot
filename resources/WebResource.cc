
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));

WebResource::WebResource() {
	header_map_ready = false;
	header_map_dirty = false;
}

WebResource::WebResource(const WebResource &wr) : ProtobufResource(wr), r(wr.r), headers(wr.headers) {
	header_map_ready = false;
	header_map_dirty = false;
}

WebResource::~WebResource() {
}

ProtobufResource *WebResource::Clone() {
	return new WebResource(*this);
}

int WebResource::getSize() {
	return 1; //FIXME
}

void WebResource::setHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values) {
	r.clear_header_names();
	r.clear_header_values();
	assert(names.size() == values.size());
	for (int i = 0; i < names.size(); i++) {
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

void WebResource::setHeaderValue(const std::string &name, const std::string &value) {
	if (!header_map_ready)
		LoadHeaders();
	headers[name] = value;
	header_map_dirty = true;
}

const std::string &WebResource::getHeaderValue(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	tr1::unordered_map<string, string>::iterator iter = headers.find(name);
	if (iter == headers.end())
		return empty_string;
	return iter->second;
}

void WebResource::clearHeaderFields() {
	r.clear_header_names();
	r.clear_header_values();
	headers.clear();
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
	snprintf(buf, sizeof(buf), " %s:%d %s %s)", this->getUrlHost().c_str(), this->getUrlPort(), this->getUrlPath().c_str(), this->getUrlQuery().c_str());
	s += buf;
	snprintf(buf, sizeof(buf), ", time: %ld, mime: %s, size: %d", this->getTime(), this->getMimeType().c_str(), this->getContent().length());
	s += buf;
	s += ", ip: ";
	s += addr.toString();
	if (this->getIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", ip expire: %ld", this->getIpAddrExpire());
		s += buf;
	}
	if (header_map_dirty)
		SaveHeaders();
	vector<string> *v = this->getHeaderNames();
	if (v->size() > 0) {
		s += "\nheaders:";
		bool first;
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
		clear_u.c = NULL;
	} else if (name == "status") {
		type = INT;
		get_u.i = &WebResource::getStatus;
		set_u.i = &WebResource::setStatus;
		clear_u.c = NULL;
	} else if (name == "url") {
		type = STRING;
		get_u.s = &WebResource::getUrl;
		set_u.s = &WebResource::setUrl;
		clear_u.c = &WebResource::clearUrl;
	} else if (name == "time") {
		type = LONG;
		get_u.l = &WebResource::getTime;
		set_u.l = &WebResource::setTime;
		clear_u.c = &WebResource::clearTime;
	} else if (name == "mimeType") {
		type = STRING;
		get_u.s = &WebResource::getMimeType;
		set_u.s = &WebResource::setMimeType;
		clear_u.c = &WebResource::clearMimeType;
	} else if (name == "content") {
		type = STRING;
		get_u.s = &WebResource::getContent;
		set_u.s = &WebResource::setContent;
		clear_u.c = &WebResource::clearContent;
	} else if (name == "header") {
		type = STRING2;
		get_u.s2 = &WebResource::getHeaderValue;
		set_u.s2 = &WebResource::setHeaderValue;
		clear_u.c = &WebResource::clearHeaderFields;
	} else if (name == "ipAddr") {
		type = IP;
		get_u.ip = &WebResource::getIpAddr;
		set_u.ip = &WebResource::setIpAddr;
		clear_u.c = &WebResource::clearIpAddr;
	} else if (name == "ipAddrExpire") {
		type = LONG;
		get_u.l = &WebResource::getIpAddrExpire;
		set_u.l = &WebResource::setIpAddrExpire;
		clear_u.c = &WebResource::clearIpAddrExpire;
	} else if (name == "urlScheme") {
		type = INT;
		get_u.i = &WebResource::getUrlScheme;
		set_u.i = &WebResource::setUrlScheme;
		clear_u.c = &WebResource::clearUrlScheme;
	} else if (name == "urlUsername") {
		type = STRING;
		get_u.s = &WebResource::getUrlUsername;
		set_u.s = &WebResource::setUrlUsername;
		clear_u.c = &WebResource::clearUrlUsername;
	} else if (name == "urlPassword") {
		type = STRING;
		get_u.s = &WebResource::getUrlPassword;
		set_u.s = &WebResource::setUrlPassword;
		clear_u.c = &WebResource::clearUrlPassword;
	} else if (name == "urlHost") {
		type = STRING;
		get_u.s = &WebResource::getUrlHost;
		set_u.s = &WebResource::setUrlHost;
		clear_u.c = &WebResource::clearUrlHost;
	} else if (name == "urlPort") {
		type = INT;
		get_u.i = &WebResource::getUrlPort;
		set_u.i = &WebResource::setUrlPort;
		clear_u.c = &WebResource::clearUrlPort;
	} else if (name == "urlPath") {
		type = STRING;
		get_u.s = &WebResource::getUrlPath;
		set_u.s = &WebResource::setUrlPath;
		clear_u.c = &WebResource::clearUrlPath;
	} else if (name == "urlQuery") {
		type = STRING;
		get_u.s = &WebResource::getUrlQuery;
		set_u.s = &WebResource::setUrlQuery;
		clear_u.c = &WebResource::clearUrlQuery;
	} else {
		type = UNKNOWN;
	}
}
