#include <config.h>

#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "SiteResource.h"
#include "SiteResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr SiteResource::logger(log4cxx::Logger::getLogger("resources.SiteResource"));
SiteResourceInfo SiteResource::resourceInfo;

SiteResourceInfo::SiteResourceInfo() {
	SetTypeId(11);
	SetTypeString("SiteResource");
	SetTypeStringTerse("SR");
	SetObjectName("SiteResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<SiteResource> *ai;

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitInt32("id", &SiteResource::GetId, &SiteResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitInt32("status", &SiteResource::GetStatus, &SiteResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitInt32("urlScheme", &SiteResource::GetUrlScheme, &SiteResource::SetUrlScheme);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitString("urlHost", &SiteResource::GetUrlHost, &SiteResource::SetUrlHost);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitInt32("urlPort", &SiteResource::GetUrlPort, &SiteResource::SetUrlPort);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitIpAddr("ipAddr", &SiteResource::GetIpAddr, &SiteResource::SetIpAddr);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitUInt32("ipAddrExpire", &SiteResource::GetIpAddrExpire, &SiteResource::SetIpAddrExpire);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitArrayString("allowUrls", &SiteResource::GetAllowUrl, &SiteResource::SetAllowUrl, &SiteResource::ClearAllowUrls, &SiteResource::CountAllowUrls);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitArrayString("disallowUrls", &SiteResource::GetDisallowUrl, &SiteResource::SetDisallowUrl, &SiteResource::ClearDisallowUrls, &SiteResource::CountDisallowUrls);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitUInt32("robotsExpire", &SiteResource::GetRobotsExpire, &SiteResource::SetRobotsExpire);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<SiteResource>(GetTypeId());
	ai->InitInt32("robotsRedirectCount", &SiteResource::GetRobotsRedirectCount, &SiteResource::SetRobotsRedirectCount);
	l->push_back(ai);

	SetAttrInfoList(l);
}

void SiteResource::LoadIpAddr() {
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

void SiteResource::SaveIpAddr() {
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

/*bool SiteResource::Skip(ResourceInputStream &input) {
	uint32_t size;
	if (!input.ReadVarint32(&size))
                return false;
	input.Skip(size);
	return true;
}*/

string SiteResource::ToString(Object::LogLevel logLevel) {
	string s;

	char buf[1024];
	snprintf(buf, sizeof(buf), "[%s %d %d] (%s://%s:%d), md5: %llu, ip: ", resourceInfo.GetTypeStringTerse(), GetId(), GetStatus(), Scheme_Name((Scheme)GetUrlScheme()).c_str(), GetUrlHost().c_str(), GetUrlPort(), site_md5);
	s = buf;
	s += addr.ToString();
	if (GetIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", ip expire: %u", GetIpAddrExpire());
		s += buf;
	}
	if (GetIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", robots expire: %u", GetRobotsExpire());
		s += buf;
	}
	if (robots_redirect_count) {
		snprintf(buf, sizeof(buf), ", robots redirs: %u", robots_redirect_count);
		s += buf;
	}
	vector<string> *v = GetAllowUrls();
	if (v->size() > 0) {
		s += "\nAllow: ";
		bool first = true;
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			if (first)
				first = false;
			else
				s += ", ";
			s += *iter;
		}
	}
	delete v;
	v = GetDisallowUrls();
	if (v->size() > 0) {
		s += "\nDisallow: ";
		bool first = true;
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			if (first)
				first = false;
			else
				s += ", ";
			s += *iter;
		}
	}
	delete v;

	return s;
}

#else

extern "C" Resource* hector_resource_create() {
	return (Resource*)new SiteResource();
}

#endif
