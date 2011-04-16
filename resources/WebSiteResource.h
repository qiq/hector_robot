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
#include "Resource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "RWLock.h"
#include "WebSiteResource.pb.h"
#include "WebSitePath.h"

#define DEFAULT_MODIFICATION_HISTORY 37
#define MIN_MODIFICATION_HISTORY 16
#define MAX_MODIFICATION_HISTORY 41

#define MAX_PATH_SIZE 2048

class ResourceAttrInfo;

class WebSiteResourceInfo : public ResourceInfo {
public:
	WebSiteResourceInfo();
};

class WebSiteResource : public Resource {
public:
	WebSiteResource();
	WebSiteResource(const WebSiteResource &wsr);
	~WebSiteResource();
	// create copy of a resource
	Resource *Clone();
	void Clear();
	// save and restore resource
	bool Serialize(ResourceOutputStream &output);
	bool Deserialize(ResourceInputStream &input);
	int GetId();
	void SetId(int id);
	// status may be tested in Processor to select target queue
	int GetStatus();
	void SetStatus(int status);
	// resource may contain link to other resource, it is only kept only in the memory
	Resource *GetAttachedResource();
	void SetAttachedResource(Resource *attachedResource);
	void ClearAttachedResource();
	// used by queues in case there is limit on queue size
	int GetSize();
	// get info about this resource
	ResourceInfo *GetResourceInfo();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string ToString(Object::LogLevel = Object::INFO);

	// WebSiteResource-specific
	// preferred way: locks WSR and sets everything at once
	void SetUrl(int urlScheme, const std::string &urlHost, int urlPort);
	void GetUrl(int &urlScheme, std::string &urlHost, int &urlPort);
	void SetIpAddrExpire(IpAddr &addr, uint32_t time);
	void GetIpAddrExpire(IpAddr &addr, uint32_t &time);
	void SetRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, uint32_t time);
	void GetRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, uint32_t &time);
	int PathReadyToFetch(const char *path, uint32_t currentTime, uint32_t lastScheduled);
	bool PathNewLinkReady(const char *path, uint32_t currentTime);
	bool PathUpdateError(const char *path, uint32_t currentTime, int maxCount);
	bool PathUpdateRedirect(const char *path, uint32_t currentTime, bool redirectPermanent);
	bool PathUpdateOK(const char *path, uint32_t currentTime, uint32_t size, uint32_t cksum);
	int PathNextRefresh(const char *path);

	// change on-item methods
	void SetUrlScheme(int urlScheme);
	int GetUrlScheme();
	void ClearUrlScheme();
	void SetUrlHost(const std::string &urlHost);
	const std::string GetUrlHost();
	void ClearUrlHost();
	void SetUrlPort(int urlPort);
	int GetUrlPort();
	void ClearUrlPort();
	void SetIpAddr(IpAddr &addr);
	IpAddr GetIpAddr();
	void ClearIpAddr();
	void SetIpAddrExpire(uint32_t time);
	uint32_t GetIpAddrExpire();
	void ClearIpAddrExpire();
	void SetAllowUrls(const std::vector<std::string> &allow_urls);
	void SetAllowUrl(int index, const std::string &url);
	std::vector<std::string> *GetAllowUrls();
	const std::string GetAllowUrl(int index);
	int CountAllowUrls();
	void ClearAllowUrls();
	void SetDisallowUrls(const std::vector<std::string> &disallow_urls);
	void SetDisallowUrl(int index, const std::string &url);
	std::vector<std::string> *GetDisallowUrls();
	const std::string GetDisallowUrl(int index);
	int CountDisallowUrls();
	void ClearDisallowUrls();
	void SetRobotsExpire(uint32_t time);
	uint32_t GetRobotsExpire();
	void ClearRobotsExpire();
	void SetRobotsRedirectCount(int redirects);
	int GetRobotsRedirectCount();
	void ClearRobotsRedirectCount();

	// helper methods
	void ClearPathsRefreshing();

	static bool IsInstance(Resource *resource);

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
	WebSitePath *GetPathInfo(const char *path, bool create);
	std::vector<std::string> *GetPathList();

	static WebSiteResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

struct WebSiteResource_hash: public std::unary_function<WebSiteResource*, size_t> {
        /** hash function for the WebSiteResource& type */
        size_t operator() (WebSiteResource *wsr) const {
                return std::tr1::hash<std::string>()(wsr->GetUrlHost())+13*wsr->GetUrlPort()+373*wsr->GetUrlScheme();
        }
};

struct WebSiteResource_equal {
	bool operator()(WebSiteResource* wsr1, WebSiteResource* wsr2) const {
		return (wsr1->GetUrlHost() == wsr2->GetUrlHost() && wsr1->GetUrlPort() == wsr2->GetUrlPort() && wsr1->GetUrlScheme() == wsr2->GetUrlScheme());
	}
};

inline int WebSiteResource::GetSize() {
	return 1; //FIXME
}

inline ResourceInfo *WebSiteResource::GetResourceInfo() {
	return &WebSiteResource::resourceInfo;
}

inline int WebSiteResource::GetId() {
	lock.LockRead();
	int result = Resource::GetId();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::SetId(int id) {
	lock.LockWrite();
	Resource::SetId(id);
	lock.Unlock();
}

inline int WebSiteResource::GetStatus() {
	lock.LockRead();
	int result = Resource::GetStatus();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::SetStatus(int status) {
	lock.LockWrite();
	Resource::SetStatus(status);
	lock.Unlock();
}

inline void WebSiteResource::SetAttachedResource(Resource *attachedResource) {
	lock.LockWrite();
	Resource::SetAttachedResource(attachedResource);
	lock.Unlock();
}

inline Resource *WebSiteResource::GetAttachedResource() {
	lock.LockRead();
        Resource *result = Resource::GetAttachedResource();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::ClearAttachedResource() {
	lock.LockWrite();
	Resource::SetAttachedResource(NULL);
	lock.Unlock();
}

inline bool WebSiteResource::Serialize(ResourceOutputStream &output) {
	lock.LockRead();
	// Save IpAddr and fill paths1
	SaveIpAddr();
	// fill protocol-buffers space using JArray
	JarrayToProtobuf();

	output.WriteVarint32(r.ByteSize());
	r.SerializeWithCachedSizes(output.GetCodedOutputStream());

	r.clear_paths();
	lock.Unlock();
	return true;
}

inline bool WebSiteResource::Deserialize(ResourceInputStream &input) {
	uint32_t size;
	if (!input.ReadVarint32(&size))
                return false;
	google::protobuf::io::CodedInputStream::Limit l = input.PushLimit(size);
	bool result = r.ParseFromCodedStream(input.GetCodedInputStream());
	input.PopLimit(l);

	ProtobufToJarray();
	r.clear_paths();
	LoadIpAddr();
	return result;
}

inline void WebSiteResource::SetUrl(int urlScheme, const std::string &urlHost, int urlPort) {
	lock.LockWrite();
	r.set_url_scheme((Scheme)urlScheme);
	r.set_url_host(urlHost);
	r.set_url_port(urlPort);
	lock.Unlock();
}

inline void WebSiteResource::GetUrl(int &urlScheme, std::string &urlHost, int &urlPort) {
	lock.LockRead();
	urlScheme = (int)r.url_scheme();
	urlHost = r.url_host();
	urlPort = r.url_port();
	lock.Unlock();
}

inline void WebSiteResource::SetIpAddrExpire(IpAddr &addr, uint32_t time) {
	lock.LockWrite();
	this->addr = addr;
	r.set_ip_addr_expire(time);
	lock.Unlock();
}

inline void WebSiteResource::GetIpAddrExpire(IpAddr &addr, uint32_t &time) {
	lock.LockRead();
	addr = this->addr;
	time = r.ip_addr_expire();
	lock.Unlock();
}

inline void WebSiteResource::SetRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, uint32_t time) {
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

inline void WebSiteResource::GetRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, uint32_t &time) {
	lock.LockRead();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		allow_urls.push_back(r.allow_urls(i));
	}
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		disallow_urls.push_back(r.disallow_urls(i));
	}
	time = r.robots_expire();
	lock.Unlock();
}

// test whether path is ready to be fetched
// return: 0: OK, 1: invalid status, 2: status updated recently, 3: currently refreshing (locked)
inline int WebSiteResource::PathReadyToFetch(const char *path, uint32_t currentTime, uint32_t lastScheduled) {
	lock.LockWrite();
	WebSitePath *wsp = GetPathInfo(path, true);
	int result = 1;
	if (wsp) {
		WebSitePath::PathStatus status = wsp->GetPathStatus();
		if (status != WebSitePath::OK && status != WebSitePath::NEW_LINK && status != WebSitePath::NONE) {
			result = 1;
		// NEW_LINK saved, but we want to download WR now anyway
		} else if (status != WebSitePath::NEW_LINK && wsp->GetLastPathStatusUpdate() > (uint32_t)lastScheduled) {
			result = 2;
		} else if  (wsp->GetRefreshing()) {
			result = 3;
		} else {
			// previously we did not want to 
			if (wsp->GetPathStatus() == WebSitePath::NONE) {
				wsp->SetPathStatus(WebSitePath::NEW_LINK);
				wsp->SetLastPathStatusUpdate(currentTime);
			}
			wsp->SetRefreshing(true);
			result = 0;
		}
	}
	lock.Unlock();
	return result;
}

// test whether the path is new and ready to be scheduled for a fetch
inline bool WebSiteResource::PathNewLinkReady(const char *path, uint32_t currentTime) {
	lock.LockWrite();
	WebSitePath *wsp = GetPathInfo(path, true);
	bool result = false;
	if (wsp) {
		if (wsp->GetPathStatus() == WebSitePath::NONE && wsp->GetLastPathStatusUpdate() == 0) {
			wsp->SetPathStatus(WebSitePath::NEW_LINK);
			wsp->SetLastPathStatusUpdate(currentTime);
			result = true;
		}
	}
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::PathUpdateError(const char *path, uint32_t currentTime, int maxCount) {
	lock.LockWrite();
	WebSitePath *wsp = GetPathInfo(path, true);
	bool result = false;
	if (wsp) {
		int c = wsp->GetErrorCount()+1;
		if (c < maxCount) {
			wsp->SetPathStatus(WebSitePath::ERROR);
			result = true;
		} else {
			wsp->SetPathStatus(WebSitePath::DISABLED);
		}
		wsp->SetLastPathStatusUpdate(currentTime);
		wsp->SetErrorCount(c);
		wsp->SetRefreshing(false);
	}
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::PathUpdateRedirect(const char *path, uint32_t currentTime, bool redirectPermanent) {
	lock.LockWrite();
	WebSitePath *wsp = GetPathInfo(path, true);
	bool result = false;
	if (wsp) {
		if (!redirectPermanent) {
			wsp->SetPathStatus(WebSitePath::OK);
			result = true;
		} else {
			wsp->SetPathStatus(WebSitePath::REDIRECT);
		}
		wsp->SetErrorCount(0);
		wsp->SetLastPathStatusUpdate(currentTime);
		wsp->SetRefreshing(false);
		result = true;
	}
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::PathUpdateOK(const char *path, uint32_t currentTime, uint32_t size, uint32_t cksum) {
	lock.LockWrite();
	WebSitePath *wsp = GetPathInfo(path, true);
	bool result = false;
	if (wsp) {
		if (wsp->GetSize() != (uint32_t)size || wsp->GetCksum() != (uint32_t)cksum) {
			wsp->SetSize(size);
			wsp->SetCksum(cksum);
			uint32_t lastModified = wsp->GetLastModified();
			if (lastModified != (uint32_t)currentTime) {
				wsp->SetLastModified(currentTime);
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
				uint32_t history = wsp->GetModificationHistory();
				history = (history << 8) | (l & 0xFF);
				wsp->SetModificationHistory(history);
			}
		}
		wsp->SetPathStatus(WebSitePath::OK);
		wsp->SetErrorCount(0);
		wsp->SetLastPathStatusUpdate(currentTime);
		wsp->SetRefreshing(false);
		result = true;
	}
	lock.Unlock();
	return result;
}

// result < 0: do not schedule
inline int32_t WebSiteResource::PathNextRefresh(const char *path) {
	lock.LockRead();
	WebSitePath *wsp = GetPathInfo(path, true);
	int result = 0;
	if (wsp) {
		WebSitePath::PathStatus status = wsp->GetPathStatus();
		if (status == WebSitePath::OK || status == WebSitePath::ERROR) {
			int errors = wsp->GetErrorCount();
			if (!errors) {
				uint32_t history = wsp->GetModificationHistory();
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
				result = floor(exp((double)(25+errors*3)*log(1.5)));
			}
		}
	}
	lock.Unlock();
	return result;
}

inline void WebSiteResource::SetUrlScheme(int urlScheme) {
	lock.LockWrite();
	r.set_url_scheme((Scheme)urlScheme);
	lock.Unlock();
}

inline int WebSiteResource::GetUrlScheme() {
	lock.LockRead();
	int result = (int)r.url_scheme();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::ClearUrlScheme() {
	lock.LockWrite();
	r.clear_url_scheme();
	lock.Unlock();
}

inline void WebSiteResource::SetUrlHost(const std::string &urlHost) {
	lock.LockWrite();
	r.set_url_host(urlHost);
	lock.Unlock();
}

inline const std::string WebSiteResource::GetUrlHost() {
	lock.LockRead();
	const std::string &urlHost = r.url_host();
	lock.Unlock();
	return urlHost;
}

inline void WebSiteResource::ClearUrlHost() {
	lock.LockWrite();
	r.clear_url_host();
	lock.Unlock();
}

inline void WebSiteResource::SetUrlPort(int urlPort) {
	lock.LockWrite();
	r.set_url_port(urlPort);
	lock.Unlock();
}

inline int WebSiteResource::GetUrlPort() {
	lock.LockRead();
	int result = r.url_port();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::ClearUrlPort() {
	lock.LockWrite();
	r.clear_url_port();
	lock.Unlock();
}

inline void WebSiteResource::SetIpAddr(IpAddr &addr) {
	lock.LockWrite();
	this->addr = addr;
	lock.Unlock();
}

inline IpAddr WebSiteResource::GetIpAddr() {
	lock.LockRead();
	IpAddr &a = addr;
	lock.Unlock();
	return a;
}

inline void WebSiteResource::ClearIpAddr() {
	lock.LockWrite();
	addr.SetEmpty();
	lock.Unlock();
}

inline void WebSiteResource::SetIpAddrExpire(uint32_t time) {
	lock.LockWrite();
	r.set_ip_addr_expire(time);
	lock.Unlock();
}

inline uint32_t WebSiteResource::GetIpAddrExpire() {
	lock.LockRead();
	uint32_t expire = r.ip_addr_expire();
	lock.Unlock();
	return expire;
}

inline void WebSiteResource::ClearIpAddrExpire() {
	lock.LockWrite();
	r.clear_ip_addr_expire();
	lock.Unlock();
}

inline void WebSiteResource::SetAllowUrls(const std::vector<std::string> &allow_urls) {
	lock.LockWrite();
	r.clear_allow_urls();
	for (std::vector<std::string>::const_iterator iter = allow_urls.begin(); iter != allow_urls.end(); ++iter) {
		r.add_allow_urls(*iter);
	}
	lock.Unlock();
}

inline void WebSiteResource::SetAllowUrl(int index, const std::string &url) {
	lock.LockWrite();
	r.set_allow_urls(index, url);
	lock.Unlock();
}

inline std::vector<std::string> *WebSiteResource::GetAllowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	lock.LockRead();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		result->push_back(r.allow_urls(i));
	}
	lock.Unlock();
	return result;
}

inline const std::string WebSiteResource::GetAllowUrl(int index) {
	lock.LockRead();
	std::string &result = *r.mutable_allow_urls(index);
	lock.Unlock();
	return result;
}

inline int WebSiteResource::CountAllowUrls() {
	lock.LockRead();
	int result = r.allow_urls_size();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::ClearAllowUrls() {
	lock.LockWrite();
	r.clear_allow_urls();
	lock.Unlock();
}

inline void WebSiteResource::SetDisallowUrls(const std::vector<std::string> &disallow_urls) {
	lock.LockWrite();
	r.clear_disallow_urls();
	for (std::vector<std::string>::const_iterator iter = disallow_urls.begin(); iter != disallow_urls.end(); ++iter) {
		r.add_disallow_urls(*iter);
	}
	lock.Unlock();
}

inline void WebSiteResource::SetDisallowUrl(int index, const std::string &url) {
	lock.LockWrite();
	r.set_disallow_urls(index, url);
	lock.Unlock();
}

inline std::vector<std::string> *WebSiteResource::GetDisallowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	lock.LockRead();
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		result->push_back(r.disallow_urls(i));
	}
	lock.Unlock();
	return result;
}

inline const std::string WebSiteResource::GetDisallowUrl(int index) {
	lock.LockRead();
	std::string &result = *r.mutable_disallow_urls(index);
	lock.Unlock();
	return result;
}

inline int WebSiteResource::CountDisallowUrls() {
	lock.LockRead();
	int result = r.disallow_urls_size();
	lock.Unlock();
	return result;
}

inline void WebSiteResource::ClearDisallowUrls() {
	lock.LockWrite();
	r.clear_disallow_urls();
	lock.Unlock();
}

inline void WebSiteResource::SetRobotsExpire(uint32_t time) {
	lock.LockWrite();
	r.set_robots_expire(time);
	lock.Unlock();
}

inline uint32_t WebSiteResource::GetRobotsExpire() {
	lock.LockRead();
	uint32_t expire = r.robots_expire();
	lock.Unlock();
	return expire;
}

inline void WebSiteResource::ClearRobotsExpire() {
	lock.LockWrite();
	r.clear_robots_expire();
	lock.Unlock();
}

inline void WebSiteResource::SetRobotsRedirectCount(int redirects) {
	lock.LockWrite();
	r.set_robots_redirect_count(redirects);
	lock.Unlock();
}

inline int WebSiteResource::GetRobotsRedirectCount() {
	lock.LockRead();
	int redirects = r.robots_redirect_count();
	lock.Unlock();
	return redirects;
}

inline void WebSiteResource::ClearRobotsRedirectCount() {
	lock.LockWrite();
	r.clear_robots_redirect_count();
	lock.Unlock();
}

inline bool WebSiteResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
