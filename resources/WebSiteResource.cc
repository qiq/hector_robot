
#include "WebSiteResource.h"
#include "WebSiteResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebSiteResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebSiteResource"));
MemoryPool<WebSitePath, true> WebSiteResource::pool(1024);

WebSiteResource::WebSiteResource() {
	paths = NULL;
}

WebSiteResource::WebSiteResource(const WebSiteResource &wsr) : ProtobufResource(wsr), lock(wsr.lock), r(wsr.r), paths(NULL) {
	ProtobufToJarray();
	r.clear_paths();
}

WebSiteResource::~WebSiteResource() {
	JudySLFreeArray(&paths, NULL);
}

Resource *WebSiteResource::Clone() {
	return new WebSiteResource(*this);
}

void WebSiteResource::Clear() {
	Resource::Clear();
	r.Clear();
	JudySLFreeArray(&paths, NULL);
	paths = NULL;
	addr.SetEmpty();
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
		snprintf(buf, sizeof(buf), ", ip expire: %ld", this->GetIpAddrExpire());
		s += buf;
	}
	if (this->GetIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", robots expire: %ld", this->GetRobotsExpire());
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

vector<ResourceAttrInfo*> *WebSiteResource::GetAttrInfoList() {
	vector<ResourceAttrInfo*> *result = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<WebSiteResource> *ai;

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitInt("id", &WebSiteResource::GetId, &WebSiteResource::SetId);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitInt("status", &WebSiteResource::GetStatus, &WebSiteResource::SetStatus);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitInt("urlScheme", &WebSiteResource::GetUrlScheme, &WebSiteResource::SetUrlScheme);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitString("urlHost", &WebSiteResource::GetUrlHost, &WebSiteResource::SetUrlHost);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitInt("urlPort", &WebSiteResource::GetUrlPort, &WebSiteResource::SetUrlPort);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitIpAddr("ipAddr", &WebSiteResource::GetIpAddr, &WebSiteResource::SetIpAddr);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitLong("ipAddrExpire", &WebSiteResource::GetIpAddrExpire, &WebSiteResource::SetIpAddrExpire);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitArrayString("allowUrls", &WebSiteResource::GetAllowUrl, &WebSiteResource::SetAllowUrl, &WebSiteResource::ClearAllowUrls, &WebSiteResource::CountAllowUrls);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitArrayString("disallowUrls", &WebSiteResource::GetDisallowUrl, &WebSiteResource::SetDisallowUrl, &WebSiteResource::ClearDisallowUrls, &WebSiteResource::CountDisallowUrls);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitLong("robotsExpire", &WebSiteResource::GetRobotsExpire, &WebSiteResource::SetRobotsExpire);
	result->push_back(ai);

	ai = new ResourceAttrInfoT<WebSiteResource>(typeId);
	ai->InitInt("robotsRedirectCount", &WebSiteResource::GetRobotsRedirectCount, &WebSiteResource::SetRobotsRedirectCount);
	result->push_back(ai);

	return result;
}
