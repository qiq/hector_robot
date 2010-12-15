
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
		WebSitePath *wsp = (WebSitePath*)PValue;
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

/*
WebSitePath *WebSiteResource::createPathInfo(const char *path) {
	PWord_t PValue;
	PValue = (PWord_t)JudySLGet(paths, (uint8_t*)path, NULL);
	if (PValue)
		return (WebSitePath*)PValue;
	WebSitePath *wsp = pool.alloc();
	PValue = (PWord_t)JudySLIns(&paths, (uint8_t*)path, NULL);
	if (PValue == PJERR) {
		LOG4CXX_ERROR(logger, "Malloc failed");
		return false;
	}
	*PValue = (Word_t)wsp;
	return wsp;
}*/

WebSitePath *WebSiteResource::getPathInfo(const char *path, bool create) {
	PWord_t PValue;
	PValue = (PWord_t)JudySLGet(paths, (uint8_t*)path, NULL);
	if (!PValue) {
		if (!create)
			return NULL;
		WebSitePath *wsp = pool.Alloc();
		wsp->setPathStatus(WebSitePath::NEW_LINK);
		PValue = (PWord_t)JudySLIns(&paths, (uint8_t*)path, NULL);
		if (PValue == PJERR) {
			LOG4CXX_ERROR(logger, "Malloc failed");
			NULL;
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
		clear_u.c = NULL;
	} else if (name == "status") {
		type = INT;
		get_u.i = &WebSiteResource::getStatus;
		set_u.i = &WebSiteResource::setStatus;
		clear_u.c = NULL;
/*	} else if (name == "url") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrl;
		set_u.s = &WebSiteResource::setUrl;
		clear_u.c = &WebSiteResource::clearUrl;
	} else if (name == "time") {
		type = LONG;
		get_u.l = &WebSiteResource::getTime;
		set_u.l = &WebSiteResource::setTime;
		clear_u.c = &WebSiteResource::clearTime;
	} else if (name == "mimeType") {
		type = STRING;
		get_u.s = &WebSiteResource::getMimeType;
		set_u.s = &WebSiteResource::setMimeType;
		clear_u.c = &WebSiteResource::clearMimeType;
	} else if (name == "content") {
		type = STRING;
		get_u.s = &WebSiteResource::getContent;
		set_u.s = &WebSiteResource::setContent;
		clear_u.c = &WebSiteResource::clearContent;
	} else if (name == "header") {
		type = STRING2;
		get_u.s2 = &WebSiteResource::getHeaderValue;
		set_u.s2 = &WebSiteResource::setHeaderValue;
		clear_u.c = &WebSiteResource::clearHeaderFields;
	} else if (name == "ip4Addr") {
		type = IP4;
		get_u.a4 = &WebSiteResource::getIp4Addr;
		set_u.a4 = &WebSiteResource::setIp4Addr;
		clear_u.c = &WebSiteResource::clearIp4Addr;
	} else if (name == "ip6Addr") {
		type = IP6;
		get_u.a6 = &WebSiteResource::getIp6Addr;
		set_u.a6 = &WebSiteResource::setIp6Addr;
		clear_u.c = &WebSiteResource::clearIp6Addr;
	} else if (name == "ipAddrExpire") {
		type = LONG;
		get_u.l = &WebSiteResource::getIpAddrExpire;
		set_u.l = &WebSiteResource::setIpAddrExpire;
		clear_u.c = &WebSiteResource::clearIpAddrExpire;
	} else if (name == "urlScheme") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlScheme;
		set_u.s = &WebSiteResource::setUrlScheme;
		clear_u.c = &WebSiteResource::clearUrlScheme;
	} else if (name == "urlUsername") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlUsername;
		set_u.s = &WebSiteResource::setUrlUsername;
		clear_u.c = &WebSiteResource::clearUrlUsername;
	} else if (name == "urlPassword") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlPassword;
		set_u.s = &WebSiteResource::setUrlPassword;
		clear_u.c = &WebSiteResource::clearUrlPassword;
	} else if (name == "urlHost") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlHost;
		set_u.s = &WebSiteResource::setUrlHost;
		clear_u.c = &WebSiteResource::clearUrlHost;
	} else if (name == "urlPort") {
		type = INT;
		get_u.i = &WebSiteResource::getUrlPort;
		set_u.i = &WebSiteResource::setUrlPort;
		clear_u.c = &WebSiteResource::clearUrlPort;
	} else if (name == "urlPath") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlPath;
		set_u.s = &WebSiteResource::setUrlPath;
		clear_u.c = &WebSiteResource::clearUrlPath;
	} else if (name == "urlQuery") {
		type = STRING;
		get_u.s = &WebSiteResource::getUrlQuery;
		set_u.s = &WebSiteResource::setUrlQuery;
		clear_u.c = &WebSiteResource::clearUrlQuery;
	} else {*/
		type = UNKNOWN;
	}
}
