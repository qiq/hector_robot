/**
 * Class representing one web site (DNS name): IP address, pages, etc.
 * It uses Google Protocol Buffers to de/serialize.
 * This resource may be shared across more than one module, so locking of
 * mutable variables is necessary.
 *
 * We store
 * - scheme, hostname, port (key)
 * - ip4 & ip6 address; 0 = no address known
 * - address expire time; when to do a new DNS resolution request
 * - list of allowed and disallowed URLs; from parsed robots.txt
 * - robots.txt expire time; when to get robots.txt again
 * - list of paths
 * 	- checksum (fast), 32bits
 *	- status: ???
 *	- last updated: when we last downloaded the page
 */

#ifndef _WEB_SITE_RESOURCE_H_
#define _WEB_SITE_RESOURCE_H_

#include <config.h>

#include <functional>
#include <string>
#include <tr1/unordered_map>
#include <vector>
#include <Judy.h>
#include <log4cxx/logger.h>
#include "common.h"
#include "MemoryPool.h"
#include "ProtobufResource.h"
#include "ResourceFieldInfo.h"
#include "RWLock.h"
#include "WebSiteResource.pb.h"
#include "WebSitePath.h"

#define DEFAULT_MODIFICATION_HISTORY 37
#define MIN_MODIFICATION_HISTORY 16
#define MAX_MODIFICATION_HISTORY 41

#define MAX_PATH_SIZE 2048

class WebSiteResource : public ProtobufResource {
public:
	WebSiteResource();
	WebSiteResource(const WebSiteResource &wsr);
	~WebSiteResource();
	// create copy of a resource
	Resource *Clone();
	// save and restore resource
	std::string *Serialize();
	int GetSerializedSize();
	bool SerializeWithCachedSize(google::protobuf::io::CodedOutputStream *output);
	bool Deserialize(const char *data, int size);
	bool Deserialize(google::protobuf::io::CodedInputStream *input);
	// get info about a resource field
	ResourceFieldInfo *getFieldInfo(const char *name);
	// type id of a resource (to be used by Resources::CreateResource(typeid))
	int getTypeId();
	// type string of a resource
	const char *getTypeStr();
	const char *getTypeStrShort();
	// module prefix (e.g. Hector for Hector::TestResource)
	const char *getModuleStr();
	// id should be unique across all in-memory resources
	int getId();
	void setId(int id);
	// status may be tested in Processor to select target queue
	int getStatus();
	void setStatus(int status);
	// resource may contain link to other resource, it is only kept only in the memory
	Resource *getAttachedResource();
	void setAttachedResource(Resource *attachedResource);
	void clearAttachedResource();
	// used by queues in case there is limit on queue size
	int getSize();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string toString(Object::LogLevel = Object::INFO);

	// WebSiteResource-specific
	// preferred way: locks WSR and sets everything at once
	void setUrl(int urlScheme, const std::string &urlHost, int urlPort);
	void getUrl(int &urlScheme, std::string &urlHost, int &urlPort);
	void setIpAddrExpire(IpAddr &addr, long time);
	void getIpAddrExpire(IpAddr &addr, long &time);
	void setRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, long time);
	void getRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, long &time);
	int PathReadyToFetch(const char *path, long lastScheduled);
	bool PathNewLinkReady(const char *path, long currentTime);
	bool PathUpdateError(const char *path, long currentTime, int maxCount);
	bool PathUpdateRedirect(const char *path, long currentTime, bool redirectPermanent);
	bool PathUpdateOK(const char *path, long currentTime, long size, long cksum);
	long PathNextRefresh(const char *path);

	// change on-item methods
	void setUrlScheme(int urlScheme);
	int getUrlScheme();
	void clearUrlScheme();
	void setUrlHost(const std::string &urlHost);
	const std::string &getUrlHost();
	void clearUrlHost();
	void setUrlPort(int urlPort);
	int getUrlPort();
	void clearUrlPort();
	void setIpAddr(IpAddr &addr);
	IpAddr getIpAddr();
	void clearIpAddr();
	void setIpAddrExpire(long time);
	long getIpAddrExpire();
	void clearIpAddrExpire();
	void setAllowUrls(const std::vector<std::string> &allow_urls);
	std::vector<std::string> *getAllowUrls();
	void clearAllowUrls();
	void setDisallowUrls(const std::vector<std::string> &disallow_urls);
	std::vector<std::string> *getDisallowUrls();
	void clearDisallowUrls();
	void setRobotsExpire(long time);
	long getRobotsExpire();
	void clearRobotsExpire();
	void setRobotsRedirectCount(int redirects);
	int getRobotsRedirectCount();
	void clearRobotsRedirectCount();

	static const int typeId = 11;

protected:
	// this is a shared resource: all methods need to take the lock
	RWLock lock;
	// saved properties
	hector::resources::WebSiteResource r;
	// memory-only
	// paths in Judy array
	Pvoid_t paths;

	IpAddr addr;
	void LoadIpAddr();
	void SaveIpAddr();

	static MemoryPool<WebSitePath, true> pool;

	// helper methods to convert from Judy array to protobuf representation and vice-versa
	bool ProtobufToJarray();
	void JarrayToProtobuf();

	// path info get/set
	WebSitePath *getPathInfo(const char *path, bool create);
	std::vector<std::string> *getPathList();

	static log4cxx::LoggerPtr logger;
};

struct WebSiteResource_hash: public std::unary_function<WebSiteResource*, size_t> {
        /** hash function for the WebSiteResource& type */
        size_t operator() (WebSiteResource *wsr) const {
                return std::tr1::hash<std::string>()(wsr->getUrlHost())+13*wsr->getUrlPort()+373*wsr->getUrlScheme();
        }
};

struct WebSiteResource_equal {
	bool operator()(WebSiteResource* wsr1, WebSiteResource* wsr2) const {
		return (wsr1->getUrlHost() == wsr2->getUrlHost() && wsr1->getUrlPort() == wsr2->getUrlPort() && wsr1->getUrlScheme() == wsr2->getUrlScheme());
	}
};

inline ResourceFieldInfo *WebSiteResource::getFieldInfo(const char *name) {
	return new ResourceFieldInfoT<WebSiteResource>(name);
}

inline int WebSiteResource::getTypeId() {
	return typeId;
}

inline const char *WebSiteResource::getTypeStr() {
	return "WebSiteResource";
}

inline const char *WebSiteResource::getTypeStrShort() {
	return "WSR";
}

inline const char *WebSiteResource::getModuleStr() {
	return "HectorRobot";
}

inline int WebSiteResource::getSize() {
	return 1; //FIXME
}

inline int WebSiteResource::getId() {
	lock.LockRead();
	int result = id;
	lock.Unlock();
	return result;
}

inline void WebSiteResource::setId(int id) {
	lock.LockWrite();
	r.set_id(id);
	lock.Unlock();
}

inline int WebSiteResource::getStatus() {
	lock.LockRead();
	int result = status;
	lock.Unlock();
	return result;
}

inline void WebSiteResource::setStatus(int status) {
	lock.LockWrite();
	r.set_status(status);
	lock.Unlock();
}

inline void WebSiteResource::setAttachedResource(Resource *attachedResource) {
	lock.LockWrite();
        this->attachedResource = attachedResource;
	lock.Unlock();
}

inline Resource *WebSiteResource::getAttachedResource() {
	lock.LockRead();
        Resource *result = attachedResource;
	lock.Unlock();
	return result;
}

inline void WebSiteResource::clearAttachedResource() {
	lock.LockWrite();
        attachedResource = NULL;
	lock.Unlock();
}

inline std::string *WebSiteResource::Serialize() {
	lock.LockRead();
	SaveIpAddr();
	// fill protocol-buffers space using JArray
	JarrayToProtobuf();
	r.set_id(getId());
	r.set_status(getStatus());

	std::string *result = new std::string();
	r.SerializeToString(result);

	r.clear_paths();
	lock.Unlock();
	return result;
}

inline int WebSiteResource::GetSerializedSize() {
	lock.LockRead();
	SaveIpAddr();
	// fill protocol-buffers space using JArray
	JarrayToProtobuf();
	r.set_id(getId());
	r.set_status(getStatus());

	int result = r.ByteSize();

	lock.Unlock();
	return result;
}

inline bool WebSiteResource::SerializeWithCachedSize(google::protobuf::io::CodedOutputStream *output) {
	lock.LockRead();
	// IpAddr is already saved, paths are filled

	r.SerializeWithCachedSizes(output);

	r.clear_paths();
	lock.Unlock();
	return true;
}

inline bool WebSiteResource::Deserialize(const char *data, int size) {
	bool result = r.ParseFromArray((void*)data, size);

	// we keep id
	setStatus(r.status());
	ProtobufToJarray();
	r.clear_paths();
	LoadIpAddr();
	return result;
}

inline bool WebSiteResource::Deserialize(google::protobuf::io::CodedInputStream *input) {
	bool result = r.ParseFromCodedStream(input);

	// we keep id
	setStatus(r.status());
	ProtobufToJarray();
	r.clear_paths();
	LoadIpAddr();
	return result;
}

inline void WebSiteResource::setUrl(int urlScheme, const std::string &urlHost, int urlPort) {
	lock.LockWrite();
	r.set_url_scheme((Scheme)urlScheme);
	r.set_url_host(urlHost);
	r.set_url_port(urlPort);
	lock.Unlock();
}

inline void WebSiteResource::getUrl(int &urlScheme, std::string &urlHost, int &urlPort) {
	lock.LockRead();
	urlScheme = (int)r.url_scheme();
	urlHost = r.url_host();
	urlPort = r.url_port();
	lock.Unlock();
}

inline void WebSiteResource::setIpAddrExpire(IpAddr &addr, long time) {
	lock.LockWrite();
	this->addr = addr;
	r.set_ip_addr_expire(time);
	lock.Unlock();
}

inline void WebSiteResource::getIpAddrExpire(IpAddr &addr, long &time) {
	lock.LockRead();
	addr = this->addr;
	time = (long)r.ip_addr_expire();
	lock.Unlock();
}

inline void WebSiteResource::setRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, long time) {
	lock.LockWrite();
	r.clear_allow_urls();
	for (std::vector<std::string>::const_iterator iter = allow_urls.begin(); iter != allow_urls.end(); ++iter) {
		r.add_allow_urls(*iter);
	}
	r.clear_disallow_urls();
	for (std::vector<std::string>::const_iterator iter = disallow_urls.begin(); iter != disallow_urls.end(); ++iter) {
		r.add_disallow_urls(*iter);
	}
	r.set_robots_expire(time);
	lock.Unlock();
}

inline void WebSiteResource::getRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, long &time) {
	lock.LockRead();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		allow_urls.push_back(r.allow_urls(i));
	}
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		disallow_urls.push_back(r.disallow_urls(i));
	}
	time = (long)r.robots_expire();
	lock.Unlock();
}

// test whether path is ready to be fetched
// return: 0: OK, 1: invalid status, 2: status updated recently, 3: currently refreshing (locked)
inline int WebSiteResource::PathReadyToFetch(const char *path, long lastScheduled) {
	lock.LockWrite();
	WebSitePath *wsp = getPathInfo(path, true);
	int result = 1;
	if (wsp) {
		if (wsp->getPathStatus() != WebSitePath::OK && wsp->getPathStatus() != WebSitePath::NEW_LINK && wsp->getPathStatus() != WebSitePath::NONE) {
			result = 1;
		} else if (wsp->getLastPathStatusUpdate() > (uint32_t)lastScheduled) {
			result = 2;
		} else if  (wsp->getRefreshing()) {
			result = 3;
		} else {
			wsp->setRefreshing(true);
			result = 0;
		}
	}
	lock.Unlock();
	return result;
}

// test whether the path is new and ready to be scheduled for a fetch
inline bool WebSiteResource::PathNewLinkReady(const char *path, long currentTime) {
	lock.LockWrite();
	WebSitePath *wsp = getPathInfo(path, true);
	bool result = false;
	if (wsp) {
		if (wsp->getPathStatus() == WebSitePath::NONE && wsp->getLastPathStatusUpdate() == 0) {
			wsp->setPathStatus(WebSitePath::NEW_LINK);
			wsp->setLastPathStatusUpdate(currentTime);
			result = true;
		}
	}
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::PathUpdateError(const char *path, long currentTime, int maxCount) {
	lock.LockWrite();
	WebSitePath *wsp = getPathInfo(path, true);
	bool result = false;
	if (wsp) {
		int c = wsp->getErrorCount()+1;
		if (c < maxCount) {
			wsp->setPathStatus(WebSitePath::ERROR);
			result = true;
		} else {
			wsp->setPathStatus(WebSitePath::DISABLED);
		}
		wsp->setLastPathStatusUpdate(currentTime);
		wsp->setErrorCount(c);
		wsp->setRefreshing(false);
	}
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::PathUpdateRedirect(const char *path, long currentTime, bool redirectPermanent) {
	lock.LockWrite();
	WebSitePath *wsp = getPathInfo(path, true);
	bool result = false;
	if (wsp) {
		if (!redirectPermanent) {
			wsp->setPathStatus(WebSitePath::OK);
			result = true;
		} else {
			wsp->setPathStatus(WebSitePath::REDIRECT);
		}
		wsp->setErrorCount(0);
		wsp->setLastPathStatusUpdate(currentTime);
		wsp->setRefreshing(false);
		result = true;
	}
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::PathUpdateOK(const char *path, long currentTime, long size, long cksum) {
	lock.LockWrite();
	WebSitePath *wsp = getPathInfo(path, true);
	bool result = false;
	if (wsp) {
		if (wsp->getSize() != (uint32_t)size || wsp->getCksum() != (uint32_t)cksum) {
			wsp->setSize(size);
			wsp->setCksum(cksum);
			uint32_t lastModified = wsp->getLastModified();
			if (lastModified != (uint32_t)currentTime) {
				wsp->setLastModified(currentTime);
				int l;
				if (lastModified > 0) {
					l = floor(log((double)currentTime-lastModified)/log(1.5));
					if (l < MIN_MODIFICATION_HISTORY)
						l = MIN_MODIFICATION_HISTORY;
					if (l > MAX_MODIFICATION_HISTORY)
						l = MAX_MODIFICATION_HISTORY;
				} else {
					l = DEFAULT_MODIFICATION_HISTORY;
				}
				uint32_t history = wsp->getModificationHistory();
				history = (history << 8) | (l & 0xFF);
				wsp->setModificationHistory(history);
			}
		}
		wsp->setPathStatus(WebSitePath::OK);
		wsp->setErrorCount(0);
		wsp->setLastPathStatusUpdate(currentTime);
		wsp->setRefreshing(false);
		result = true;
	}
	lock.Unlock();
	return result;
}

// result < 0: do not schedule
inline long WebSiteResource::PathNextRefresh(const char *path) {
	lock.LockRead();
	WebSitePath *wsp = getPathInfo(path, true);
	long result = -1;
	if (wsp) {
		WebSitePath::PathStatus status = wsp->getPathStatus();
		if (status == WebSitePath::OK || status == WebSitePath::ERROR) {
			int errors = wsp->getErrorCount();
			if (!errors) {
				uint32_t history = wsp->getModificationHistory();
				int a = history >> 24;
				if (!a)
					a = DEFAULT_MODIFICATION_HISTORY;
				int b = (history >> 16) & 0xFF;
				if (!b)
					b = DEFAULT_MODIFICATION_HISTORY;
				int c = (history >> 8) & 0xFF;
				if (!c)
					c = DEFAULT_MODIFICATION_HISTORY;
				int d = history & 0xFF;
				if (!d)
					d = DEFAULT_MODIFICATION_HISTORY;
				result = floor(exp((double)((a + b*2 + c*3 + d*4)/10)*log(1.5)));
			} else {
				// 1d, 3d, 11d, 1m, 4m
				result = 25+errors*3;
			}
		}
	}
	lock.Unlock();
	return result;
}

inline void WebSiteResource::setUrlScheme(int urlScheme) {
	lock.LockWrite();
	r.set_url_scheme((Scheme)urlScheme);
	lock.Unlock();
}

inline int WebSiteResource::getUrlScheme() {
	lock.LockRead();
	int result = (int)r.url_scheme();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::clearUrlScheme() {
	lock.LockWrite();
	r.clear_url_scheme();
	lock.Unlock();
}

inline void WebSiteResource::setUrlHost(const std::string &urlHost) {
	lock.LockWrite();
	r.set_url_host(urlHost);
	lock.Unlock();
}

inline const std::string &WebSiteResource::getUrlHost() {
	lock.LockRead();
	const std::string &urlHost = r.url_host();
	lock.Unlock();
	return urlHost;
}

inline void WebSiteResource::clearUrlHost() {
	lock.LockWrite();
	r.clear_url_host();
	lock.Unlock();
}

inline void WebSiteResource::setUrlPort(int urlPort) {
	lock.LockWrite();
	r.set_url_port(urlPort);
	lock.Unlock();
}

inline int WebSiteResource::getUrlPort() {
	lock.LockRead();
	int result = r.url_port();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::clearUrlPort() {
	lock.LockWrite();
	r.clear_url_port();
	lock.Unlock();
}

inline void WebSiteResource::setIpAddr(IpAddr &addr) {
	lock.LockWrite();
	this->addr = addr;
	lock.Unlock();
}

inline IpAddr WebSiteResource::getIpAddr() {
	lock.LockRead();
	IpAddr a = addr;
	lock.Unlock();
	return a;
}

inline void WebSiteResource::clearIpAddr() {
	lock.LockWrite();
	addr.setEmpty();
	lock.Unlock();
}

inline void WebSiteResource::setIpAddrExpire(long time) {
	lock.LockWrite();
	r.set_ip_addr_expire(time);
	lock.Unlock();
}

inline long WebSiteResource::getIpAddrExpire() {
	lock.LockRead();
	long expire = (long)r.ip_addr_expire();
	lock.Unlock();
	return expire;
}

inline void WebSiteResource::clearIpAddrExpire() {
	lock.LockWrite();
	r.clear_ip_addr_expire();
	lock.Unlock();
}

inline void WebSiteResource::setAllowUrls(const std::vector<std::string> &allow_urls) {
	lock.LockWrite();
	r.clear_allow_urls();
	for (std::vector<std::string>::const_iterator iter = allow_urls.begin(); iter != allow_urls.end(); ++iter) {
		r.add_allow_urls(*iter);
	}
	lock.Unlock();
}

inline std::vector<std::string> *WebSiteResource::getAllowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	lock.LockRead();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		result->push_back(r.allow_urls(i));
	}
	lock.Unlock();
	return result;
}

inline void WebSiteResource::clearAllowUrls() {
	lock.LockWrite();
	r.clear_allow_urls();
	lock.Unlock();
}

inline void WebSiteResource::setDisallowUrls(const std::vector<std::string> &disallow_urls) {
	lock.LockWrite();
	r.clear_disallow_urls();
	for (std::vector<std::string>::const_iterator iter = disallow_urls.begin(); iter != disallow_urls.end(); ++iter) {
		r.add_disallow_urls(*iter);
	}
	lock.Unlock();
}

inline std::vector<std::string> *WebSiteResource::getDisallowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	lock.LockRead();
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		result->push_back(r.disallow_urls(i));
	}
	lock.Unlock();
	return result;
}

inline void WebSiteResource::clearDisallowUrls() {
	lock.LockWrite();
	r.clear_disallow_urls();
	lock.Unlock();
}

inline void WebSiteResource::setRobotsExpire(long time) {
	lock.LockWrite();
	r.set_robots_expire(time);
	lock.Unlock();
}

inline long WebSiteResource::getRobotsExpire() {
	lock.LockRead();
	long expire = (long)r.robots_expire();
	lock.Unlock();
	return expire;
}

inline void WebSiteResource::clearRobotsExpire() {
	lock.LockWrite();
	r.clear_robots_expire();
	lock.Unlock();
}

inline void WebSiteResource::setRobotsRedirectCount(int redirects) {
	lock.LockWrite();
	r.set_robots_redirect_count(redirects);
	lock.Unlock();
}

inline int WebSiteResource::getRobotsRedirectCount() {
	lock.LockRead();
	int redirects = r.robots_redirect_count();
	lock.Unlock();
	return redirects;
}

inline void WebSiteResource::clearRobotsRedirectCount() {
	lock.LockWrite();
	r.clear_robots_redirect_count();
	lock.Unlock();
}

#endif
