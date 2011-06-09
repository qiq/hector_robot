#include <config.h>

#include "robot_common.h"
#include "googleurl/src/gurl.h"
#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "PageResource.h"
#include "PageResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr PageResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.PageResource"));
PageResourceInfo PageResource::resourceInfo;

PageResourceInfo::PageResourceInfo() {
	SetTypeId(10);
	SetTypeString("PageResource");
	SetTypeStringTerse("PR");
	SetObjectName("PageResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<PageResource> *ai;

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitInt32("id", &PageResource::GetId, &PageResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitInt32("status", &PageResource::GetStatus, &PageResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitString("url", &PageResource::GetUrl, &PageResource::SetUrl);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitIpAddr("ipAddr", &PageResource::GetIpAddr, &PageResource::SetIpAddr);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitHashString("header", &PageResource::GetHeaderValue, &PageResource::SetHeaderValue, &PageResource::ClearHeader, &PageResource::ClearHeaderField, &PageResource::GetHeaderCount, &PageResource::GetHeaderNames, &PageResource::GetHeaderValues);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitInt32("redirectCount", &PageResource::GetRedirectCount, &PageResource::SetRedirectCount);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitString("content", &PageResource::GetContent, &PageResource::SetContent);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitInt32("urlScheme", &PageResource::GetUrlScheme, &PageResource::SetUrlScheme);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitString("urlUsername", &PageResource::GetUrlUsername, &PageResource::SetUrlUsername);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitString("urlPassword", &PageResource::GetUrlPassword, &PageResource::SetUrlPassword);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitString("urlHost", &PageResource::GetUrlHost, &PageResource::SetUrlHost);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitInt32("urlPort", &PageResource::GetUrlPort, &PageResource::SetUrlPort);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<PageResource>(GetTypeId());
	ai->InitString("urlPath", &PageResource::GetUrlPath, &PageResource::SetUrlPath);
	l->push_back(ai);

	SetAttrInfoList(l);
}

void PageResource::SetHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values) {
	r.clear_header_names();
	r.clear_header_values();
	assert(names.size() == values.size());
	for (int i = 0; i < (int)names.size(); i++) {
		r.add_header_names(names[i]);
		r.add_header_values(values[i]);
	}
}

vector<string> *PageResource::GetHeaderNames() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.header_names_size(); i++) {
		result->push_back(r.header_names(i));
	}
	return result;
}

vector<string> *PageResource::GetHeaderValues() {
	vector<string> *result = new vector<string>();
	for (int i = 0; i < r.header_values_size(); i++) {
		result->push_back(r.header_values(i));
	}
	return result;
}

void PageResource::LoadHeaders() {
	headers.clear();
	for (int i = 0; i < r.header_names_size(); i++)
		headers[r.header_names(i)] = r.header_values(i);
	header_map_ready = 1;
}

void PageResource::SaveHeaders() {
	r.clear_header_names();
	r.clear_header_values();
	for (tr1::unordered_map<string, string>::iterator iter = headers.begin(); iter != headers.end(); ++iter) {
		r.add_header_names(iter->first);
		r.add_header_values(iter->second);
	}
	header_map_dirty = 0;
}

void PageResource::LoadIpAddr() {
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

void PageResource::SaveIpAddr() {
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

void PageResource::SetHeaderValue(const std::string &name, const std::string &value) {
	if (!header_map_ready)
		LoadHeaders();
	headers[name] = value;
	header_map_dirty = 1;
}

const std::string PageResource::GetHeaderValue(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	tr1::unordered_map<string, string>::iterator iter = headers.find(name);
	if (iter == headers.end())
		return empty_string;
	return iter->second;
}

int PageResource::GetHeaderCount() {
	if (header_map_ready)
		return headers.size();
	else
		return r.header_names_size();
}

void PageResource::ClearHeaderField(const std::string &name) {
	if (!header_map_ready)
		LoadHeaders();
	headers.erase(name);
	header_map_dirty = 1;
}

void PageResource::ClearHeader() {
	r.clear_header_names();
	r.clear_header_values();
	headers.clear();
	header_map_ready = 1;
	header_map_dirty = 1;
}

string PageResource::ToString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[PR %d %d] url: %s", this->GetId(), this->GetStatus(), this->GetUrl().c_str());
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

extern "C" Resource* hector_resource_create() {
	return (Resource*)new PageResource();
}

#endif
