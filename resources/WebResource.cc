#include <config.h>

#include "googleurl/src/gurl.h"
#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));
WebResourceInfo WebResource::resourceInfo;

WebResourceInfo::WebResourceInfo() {
	SetTypeId(10);
	SetTypeString("WebResource");
	SetTypeStringTerse("WR");
	SetObjectName("WebResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<WebResource> *ai;

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitInt32("id", &WebResource::GetId, &WebResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitInt32("status", &WebResource::GetStatus, &WebResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitString("url", &WebResource::GetUrl, &WebResource::SetUrl);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitIpAddr("ipAddr", &WebResource::GetIpAddr, &WebResource::SetIpAddr);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitHashString("header", &WebResource::GetHeaderValue, &WebResource::SetHeaderValue, &WebResource::ClearHeader, &WebResource::ClearHeaderField, &WebResource::GetHeaderCount, &WebResource::GetHeaderNames, &WebResource::GetHeaderValues);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitInt32("redirectCount", &WebResource::GetRedirectCount, &WebResource::SetRedirectCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitString("content", &WebResource::GetContent, &WebResource::SetContent);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitUInt32("scheduled", &WebResource::GetScheduled, &WebResource::SetScheduled);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitInt32("urlScheme", &WebResource::GetUrlScheme, &WebResource::SetUrlScheme);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitString("urlUsername", &WebResource::GetUrlUsername, &WebResource::SetUrlUsername);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitString("urlPassword", &WebResource::GetUrlPassword, &WebResource::SetUrlPassword);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitString("urlHost", &WebResource::GetUrlHost, &WebResource::SetUrlHost);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitInt32("urlPort", &WebResource::GetUrlPort, &WebResource::SetUrlPort);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebResource>(GetTypeId());
	ai->InitString("urlPath", &WebResource::GetUrlPath, &WebResource::SetUrlPath);
	l->push_back(ai);

	SetAttrInfoList(l);
}

WebResource::WebResource() {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
}

WebResource::WebResource(const WebResource &wr) : Resource(wr), r(wr.r), headers(wr.headers) {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
}

WebResource::~WebResource() {
}

Resource *WebResource::Clone() {
	return new WebResource(*this);
}

void WebResource::Clear() {
	Resource::Clear();
	r.Clear();
	header_map_ready = 0;
	header_map_dirty = 0;
	headers.clear();
	addr.SetEmpty();
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
}

void WebResource::SetHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values) {
	r.clear_header_names();
	r.clear_header_values();
	assert(names.size() == values.size());
	for (int i = 0; i < (int)names.size(); i++) {
		r.add_header_names(names[i]);
		r.add_header_values(values[i]);
	}
}

vector<string> *WebResource::GetHeaderNames() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.header_names_size(); i++) {
		result->push_back(r.header_names(i));
	}
	return result;
}

vector<string> *WebResource::GetHeaderValues() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.header_values_size(); i++) {
		result->push_back(r.header_values(i));
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
		addr.SetIp4Addr(r.ip4_addr());
	} else {
		uint64_t a1 = r.ip6_addr_1();
		uint64_t a2 = r.ip6_addr_2();
		if (a1 != 0 || a2 != 0) {
			addr.SetIp6Addr(a1, true);
			addr.SetIp6Addr(a2, false);
		} else {
			addr.SetEmpty();
		}
	}
}

void WebResource::SaveIpAddr() {
	if (addr.IsIp4Addr()) {
		if (!addr.IsEmpty()) {
			r.set_ip4_addr(addr.GetIp4Addr());
		} else {
			r.clear_ip4_addr();
		}
		r.clear_ip6_addr_1();
		r.clear_ip6_addr_2();
	
	} else {
		if (!addr.IsEmpty()) {
			r.set_ip6_addr_1(addr.GetIp6Addr(true));
			r.set_ip6_addr_2(addr.GetIp6Addr(false));
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

void WebResource::SetHeaderValue(const std::string &name, const std::string &value) {
	if (!header_map_ready)
		LoadHeaders();
	headers[name] = value;
	header_map_dirty = 1;
}

const std::string WebResource::GetHeaderValue(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	tr1::unordered_map<string, string>::iterator iter = headers.find(name);
	if (iter == headers.end())
		return empty_string;
	return iter->second;
}

int WebResource::GetHeaderCount() {
	if (header_map_ready)
		return headers.size();
	else
		return r.header_names_size();
}

void WebResource::ClearHeaderField(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	headers.erase(name);
	header_map_dirty = 1;
}

void WebResource::ClearHeader() {
	r.clear_header_names();
	r.clear_header_values();
	headers.clear();
	header_map_ready = 1;
	header_map_dirty = 1;
}

string WebResource::ToString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[WR %d %d] url: %s", this->GetId(), this->GetStatus(), this->GetUrl().c_str());
	s = buf;
	snprintf(buf, sizeof(buf), " (%s", Scheme_Name((Scheme)this->GetUrlScheme()).c_str());
	s += buf;
	if (this->GetUrlUsername().length() > 0) {
		snprintf(buf, sizeof(buf), " %s:%s", this->GetUrlUsername().c_str(), this->GetUrlPassword().c_str());
		s += buf;
	}
	snprintf(buf, sizeof(buf), " %s:%d %s)", this->GetUrlHost().c_str(), this->GetUrlPort(), this->GetUrlPath().c_str());
	s += buf;
	snprintf(buf, sizeof(buf), ", size: %d", (int)this->GetContent().length());
	s += buf;
	s += ", ip: ";
	s += addr.ToString();
	snprintf(buf, sizeof(buf), ", scheduled: %u", this->GetScheduled());
	s += buf;
	if (header_map_dirty)
		SaveHeaders();
	vector<string> *v = this->GetHeaderNames();
	if (v->size() > 0) {
		s += "\nheaders: ";
		bool first = true;
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			const std::string &value = this->GetHeaderValue(iter->c_str());
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
	if (this->GetContent().length() > 0) {
		s += "\ncontent:\n";
		s += this->GetContent();
	}
	return s;
}

#else

extern "C" Resource* create() {
	return (Resource*)new WebResource();
}

#endif
