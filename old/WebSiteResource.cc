
#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "WebSiteResource.h"
#include "WebSiteResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr WebSiteResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebSiteResource"));
MemoryPool<WebSitePath, true> WebSiteResource::pool(1024);
WebSiteResourceInfo WebSiteResource::resourceInfo;

WebSiteResourceInfo::WebSiteResourceInfo() {
	SetTypeId(11);
	SetTypeString("WebSiteResource");
	SetTypeStringTerse("WSR");
	SetObjectName("WebSiteResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<WebSiteResource> *ai;

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitInt32("id", &WebSiteResource::GetId, &WebSiteResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitInt32("status", &WebSiteResource::GetStatus, &WebSiteResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitInt32("urlScheme", &WebSiteResource::GetUrlScheme, &WebSiteResource::SetUrlScheme);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitString("urlHost", &WebSiteResource::GetUrlHost, &WebSiteResource::SetUrlHost);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitInt32("urlPort", &WebSiteResource::GetUrlPort, &WebSiteResource::SetUrlPort);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitIpAddr("ipAddr", &WebSiteResource::GetIpAddr, &WebSiteResource::SetIpAddr);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitUInt32("ipAddrExpire", &WebSiteResource::GetIpAddrExpire, &WebSiteResource::SetIpAddrExpire);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitArrayString("allowUrls", &WebSiteResource::GetAllowUrl, &WebSiteResource::SetAllowUrl, &WebSiteResource::ClearAllowUrls, &WebSiteResource::CountAllowUrls);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitArrayString("disallowUrls", &WebSiteResource::GetDisallowUrl, &WebSiteResource::SetDisallowUrl, &WebSiteResource::ClearDisallowUrls, &WebSiteResource::CountDisallowUrls);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitUInt32("robotsExpire", &WebSiteResource::GetRobotsExpire, &WebSiteResource::SetRobotsExpire);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(GetTypeId());
	ai->InitInt32("robotsRedirectCount", &WebSiteResource::GetRobotsRedirectCount, &WebSiteResource::SetRobotsRedirectCount);
	l->push_back(ai);

	SetAttrInfoList(l);
}

void WebSiteResource::LoadIpAddr() {
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

void WebSiteResource::SaveIpAddr() {
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

bool WebSiteResource::ProtobufToJarray() {
	for (int i = 0; i < r.paths_size(); i++) {
		const ::hector::resources::WebSitePath &p = r.paths(i);
		WebSitePath *wsp = pool.Alloc();
		wsp->SetPathStatus((WebSitePath::PathStatus)p.path_status());
		wsp->SetLastPathStatusUpdate(p.last_path_status_update());
		wsp->SetErrorCount(p.error_count());
		wsp->SetCksum(p.cksum());
		wsp->SetLastModified(p.last_modified());
		wsp->SetModificationHistory(p.modification_history());
		// JSLI(PValue, paths, paths->path())
		const char *path = p.path().c_str();
		PWord_t PValue = (PWord_t)JudySLIns(&paths, (uint8_t*)path, NULL);
		if (PValue == PJERR) {
			LOG4CXX_ERROR(logger, "Malloc failed");
			return false;
		}
		*PValue = (Word_t)wsp;
	}
	return true;
}

// fill protocol-buffers space using JArray data
void WebSiteResource::JarrayToProtobuf() {
	uint8_t path[MAX_PATH_SIZE];
	path[0] = '\0';
	PWord_t PValue;
	// JSLF(PValue, paths, path);		// get first string
	PValue = (PWord_t)JudySLFirst(paths, path, NULL);	// get first string
	while (PValue) {
		WebSitePath *wsp = (WebSitePath*)*PValue;
		// add to protocol-buffers
		::hector::resources::WebSitePath *p = r.add_paths();
		p->set_path((char*)path);
		p->set_path_status(wsp->GetPathStatus());
		p->set_last_path_status_update(wsp->GetLastPathStatusUpdate());
		p->set_error_count(wsp->GetErrorCount());
		p->set_cksum(wsp->GetCksum());
		p->set_last_modified(wsp->GetLastModified());
		p->set_modification_history(wsp->GetModificationHistory());
		// JSLN(PValue, paths, path);	// get next string
		PValue = (PWord_t)JudySLNext(paths, path, NULL);	// get next string
	}
}

WebSitePath *WebSiteResource::GetPathInfo(const char *path, bool create) {
	PWord_t PValue;
	PValue = (PWord_t)JudySLGet(paths, (uint8_t*)path, NULL);
	if (!PValue) {
		if (!create)
			return NULL;
		WebSitePath *wsp = pool.Alloc();
		PValue = (PWord_t)JudySLIns(&paths, (uint8_t*)path, NULL);
		if (PValue == PJERR) {
			LOG4CXX_ERROR(logger, "Malloc failed");
			return NULL;
		}
		*PValue = (Word_t)wsp;
	}
	return (WebSitePath*)*PValue;
}

vector<string> *WebSiteResource::GetPathList() {
	vector<string> *result = new vector<string>();

	uint8_t path[MAX_PATH_SIZE];
	path[0] = '\0';
	PWord_t PValue;
	// JSLF(PValue, paths, path);		// get first string
	PValue = (PWord_t)JudySLFirst(paths, path, NULL);	// get first string
	while (PValue) {
		result->push_back((char*)path);
		// JSLN(PValue, paths, path);	// get next string
		PValue = (PWord_t)JudySLNext(paths, path, NULL);	// get next string
	}
	return result;
}

void WebSiteResource::ClearPathsRefreshing() {
	uint8_t path[MAX_PATH_SIZE];
	path[0] = '\0';
	PWord_t PValue;
	// JSLF(PValue, paths, path);		// get first string
	PValue = (PWord_t)JudySLFirst(paths, path, NULL);	// get first string
	while (PValue) {
		WebSitePath *wsp = (WebSitePath*)*PValue;
		wsp->SetRefreshing(false);
		// JSLN(PValue, paths, path);	// get next string
		PValue = (PWord_t)JudySLNext(paths, path, NULL);	// get next string
	}
}

string WebSiteResource::ToString(Object::LogLevel logLevel) {
	string s;

	char buf[1024];
	snprintf(buf, sizeof(buf), "[WSR %d %d] (%s %s:%d), ip: ", this->GetId(), this->GetStatus(), Scheme_Name((Scheme)this->GetUrlScheme()).c_str(), this->GetUrlHost().c_str(), this->GetUrlPort());
	s = buf;
	s += addr.ToString();
	if (this->GetIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", ip expire: %u", this->GetIpAddrExpire());
		s += buf;
	}
	if (this->GetIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", robots expire: %u", this->GetRobotsExpire());
		s += buf;
	}
	vector<string> *v = this->GetAllowUrls();
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
	v = this->GetDisallowUrls();
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
	v = this->GetPathList();
	if (v->size() > 0) {
		s += "\nPaths:";
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			WebSitePath *wsp = GetPathInfo(iter->c_str(), false);
			if (wsp) {
				snprintf(buf, sizeof(buf), "\n %s: %d %d %d %d %x", iter->c_str(), wsp->GetPathStatus(), wsp->GetLastPathStatusUpdate(), wsp->GetCksum(), wsp->GetLastModified(), wsp->GetModificationHistory());
				s += buf;
			}
		}
	}
	delete v;

	return s;
}

#else

extern "C" Resource* create() {
	return (Resource*)new WebSiteResource();
}

#endif
