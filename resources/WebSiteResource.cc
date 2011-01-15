
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

void WebSiteResource::LoadIpAddr() {
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

void WebSiteResource::SaveIpAddr() {
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

bool WebSiteResource::ProtobufToJarray() {
	for (int i = 0; i < r.paths_size(); i++) {
		const ::hector::resources::WebSitePath &p = r.paths(i);
		WebSitePath *wsp = pool.Alloc();
		wsp->setPathStatus((WebSitePath::PathStatus)p.path_status());
		wsp->setLastPathStatusUpdate(p.last_path_status_update());
		wsp->setErrorCount(p.error_count());
		wsp->setCksum(p.cksum());
		wsp->setLastModified(p.last_modified());
		wsp->setModificationHistory(p.modification_history());
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
		p->set_path_status(wsp->getPathStatus());
		p->set_last_path_status_update(wsp->getLastPathStatusUpdate());
		p->set_error_count(wsp->getErrorCount());
		p->set_cksum(wsp->getCksum());
		p->set_last_modified(wsp->getLastModified());
		p->set_modification_history(wsp->getModificationHistory());
		// JSLN(PValue, paths, path);	// get next string
		PValue = (PWord_t)JudySLNext(paths, path, NULL);	// get next string
	}
}

WebSitePath *WebSiteResource::getPathInfo(const char *path, bool create) {
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

vector<string> *WebSiteResource::getPathList() {
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
		wsp->setRefreshing(false);
		// JSLN(PValue, paths, path);	// get next string
		PValue = (PWord_t)JudySLNext(paths, path, NULL);	// get next string
	}
}

string WebSiteResource::toString(Object::LogLevel logLevel) {
	string s;

	char buf[1024];
	snprintf(buf, sizeof(buf), "[WSR %d %d] (%s %s:%d), ip: ", this->getId(), this->getStatus(), Scheme_Name((Scheme)this->getUrlScheme()).c_str(), this->getUrlHost().c_str(), this->getUrlPort());
	s = buf;
	s += addr.toString();
	if (this->getIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", ip expire: %ld", this->getIpAddrExpire());
		s += buf;
	}
	if (this->getIpAddrExpire()) {
		snprintf(buf, sizeof(buf), ", robots expire: %ld", this->getRobotsExpire());
		s += buf;
	}
	vector<string> *v = this->getAllowUrls();
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
	v = this->getDisallowUrls();
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
	v = this->getPathList();
	if (v->size() > 0) {
		s += "\nPaths:";
		for (vector<string>::iterator iter = v->begin(); iter != v->end(); ++iter) {
			WebSitePath *wsp = getPathInfo(iter->c_str(), false);
			if (wsp) {
				snprintf(buf, sizeof(buf), "\n %s: %d %d %d %d %x", iter->c_str(), wsp->getPathStatus(), wsp->getLastPathStatusUpdate(), wsp->getCksum(), wsp->getLastModified(), wsp->getModificationHistory());
				s += buf;
			}
		}
	}
	delete v;

	return s;
}

template<class T>
ResourceFieldInfoT<T>::ResourceFieldInfoT(const std::string &name) {
	if (name == "id") {
		type = INT;
		get_u.i = &WebSiteResource::getId;
		set_u.i = &WebSiteResource::setId;
		clear_all = NULL;
	} else if (name == "status") {
		type = INT;
		get_u.i = &WebSiteResource::getStatus;
		set_u.i = &WebSiteResource::setStatus;
		clear_all = NULL;
	} else if (name == "urlScheme") {
		type = INT;
		get_u.i = &WebSiteResource::getUrlScheme;
		set_u.i = &WebSiteResource::setUrlScheme;
		clear_all = &WebSiteResource::clearUrlScheme;
	} else if (name == "urlHost") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlHost;
		set_u.s = &WebSiteResource::setUrlHost;
		clear_all = &WebSiteResource::clearUrlHost;
	} else if (name == "urlPort") {
		type = INT;
		get_u.i = &WebSiteResource::getUrlPort;
		set_u.i = &WebSiteResource::setUrlPort;
		clear_all = &WebSiteResource::clearUrlPort;
	} else if (name == "ipAddr") {
		type = IP;
		get_u.ip = &WebSiteResource::getIpAddr;
		set_u.ip = &WebSiteResource::setIpAddr;
		clear_all = &WebSiteResource::clearIpAddr;
	} else if (name == "ipAddrExpire") {
		type = LONG;
		get_u.l = &WebSiteResource::getIpAddrExpire;
		set_u.l = &WebSiteResource::setIpAddrExpire;
		clear_all = &WebSiteResource::clearIpAddrExpire;
	} else if (name == "allowUrls") {
		type = ARRAY_STRING;
		get_u.as = &WebSiteResource::getAllowUrl;
		set_u.as = &WebSiteResource::setAllowUrl;
		get_all_values_u.s = &WebSiteResource::getAllowUrls;
		set_all_values_u.s = &WebSiteResource::setAllowUrls;
		count = &WebSiteResource::countAllowUrls;
		clear_all = &WebSiteResource::clearAllowUrls;
	} else if (name == "disallowUrls") {
		type = ARRAY_STRING;
		get_u.as = &WebSiteResource::getDisallowUrl;
		set_u.as = &WebSiteResource::setDisallowUrl;
		get_all_values_u.s = &WebSiteResource::getDisallowUrls;
		set_all_values_u.s = &WebSiteResource::setDisallowUrls;
		count = &WebSiteResource::countDisallowUrls;
		clear_all = &WebSiteResource::clearDisallowUrls;
	} else if (name == "robotsExpire") {
		type = LONG;
		get_u.l = &WebSiteResource::getRobotsExpire;
		set_u.l = &WebSiteResource::setRobotsExpire;
		clear_all = &WebSiteResource::clearRobotsExpire;
	} else if (name == "robotsRedirectCount") {
		type = INT;
		get_u.i = &WebSiteResource::getRobotsRedirectCount;
		set_u.i = &WebSiteResource::setRobotsRedirectCount;
		clear_all = &WebSiteResource::clearRobotsRedirectCount;
	}
}
