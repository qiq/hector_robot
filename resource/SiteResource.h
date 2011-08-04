/**
 * Information about a web site.
 */

#ifndef _SITE_RESOURCE_H_
#define _SITE_RESOURCE_H_

#include <config.h>

#include <vector>
#include <string>
#include <tr1/unordered_map>
#include <log4cxx/logger.h>
#include "common.h"
#include "SharedResource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "SiteResource.pb.h"

class ResourceAttrInfo;

class SiteResourceInfo : public ResourceInfo {
public:
	SiteResourceInfo();
};

class SiteResource : public SharedResource {
public:
	SiteResource();
	SiteResource(const SiteResource &wr);
	~SiteResource();
	// create copy of a resource
	Resource *Clone();
	void Clear();
	// save and restore resource
	bool Serialize(ResourceOutputStream &output);
	bool Deserialize(ResourceInputStream &input, bool headerOnly);
	bool Skip(ResourceInputStream &input);
	// used by queues in case there is limit on queue size
	int GetSize();
	// get info about this resource
	ResourceInfo *GetResourceInfo();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string ToString(Object::LogLevel = Object::INFO);

	// SiteResource-specific
	void SetSiteMD5(uint64_t md5);
	uint64_t GetSiteMD5();
	void ClearSiteMD5();
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

	void SetRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, uint32_t time);
	void GetRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, uint32_t &time);

	static bool IsInstance(Resource *resource);

protected:
	// memory-only properties
	int robots_redirect_count;
	bool header_read;
	// non-PB info
	uint64_t site_md5;
	// saved properties
	hector::resources::SiteResource r;

	// helper: IP address (parsed)
	IpAddr addr;
	void LoadIpAddr();
	void SaveIpAddr();

	static SiteResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

inline SiteResource::SiteResource() {
}

inline SiteResource::SiteResource(const SiteResource &wsr) : SharedResource(wsr), robots_redirect_count(0), header_read(false), r(wsr.r) {
}

inline SiteResource::~SiteResource() {
}

inline Resource *SiteResource::Clone() {
	return new SiteResource(*this);
}

inline void SiteResource::Clear() {
	Resource::Clear();
	robots_redirect_count = 0;
	header_read = false;
	r.Clear();
	addr.SetEmpty();
}

inline bool SiteResource::Serialize(ResourceOutputStream &output) {
	LockRead();
	SaveIpAddr();
	output.SerializeMessage(r);
	Unlock();
	return true;
}

inline bool SiteResource::Deserialize(ResourceInputStream &input, bool headerOnly) {
	if (headerOnly) {
		header_read = true;
		return input.ReadLittleEndian64(&site_md5);
	}
	if (!header_read) {
		if (!input.ReadLittleEndian64(&site_md5))
			return false;
	}
	bool result = input.ParseMessage(r);
	robots_redirect_count = 0;
	LoadIpAddr();
	return result;
}

inline bool SiteResource::Skip(ResourceInputStream &input) {
	if (!header_read) {
		if (!input.ReadLittleEndian64(&site_md5))
			return false;
	}
	header_read = false;
	return input.ParseMessage(r, 0, true);
}

inline int SiteResource::GetSize() {
	return 36+r.url_host().length()+r.allow_urls_size()*10+r.disallow_urls_size()*10;
}

inline ResourceInfo *SiteResource::GetResourceInfo() {
	return &SiteResource::resourceInfo;
}

inline void SiteResource::SetSiteMD5(uint64_t md5) {
	LockWrite();
	site_md5 = md5;
	Unlock();
}

inline uint64_t SiteResource::GetSiteMD5() {
	LockRead();
	uint64_t result = site_md5;
	Unlock();
	return result;
}

inline void SiteResource::ClearSiteMD5() {
	LockWrite();
	site_md5 = 0;
	Unlock();
}

inline void SiteResource::SetUrlScheme(int urlScheme) {
	LockWrite();
	r.set_url_scheme((Scheme)urlScheme);
	Unlock();
}

inline int SiteResource::GetUrlScheme() {
	LockRead();
	int result = (int)r.url_scheme();
	Unlock();
	return result;
}

inline void SiteResource::ClearUrlScheme() {
	LockWrite();
	r.clear_url_scheme();
	Unlock();
}

inline void SiteResource::SetUrlHost(const std::string &urlHost) {
	LockWrite();
	r.set_url_host(urlHost);
	Unlock();
}

inline const std::string SiteResource::GetUrlHost() {
	LockRead();
	const std::string &urlHost = r.url_host();
	Unlock();
	return urlHost;
}

inline void SiteResource::ClearUrlHost() {
	LockWrite();
	r.clear_url_host();
	Unlock();
}

inline void SiteResource::SetUrlPort(int urlPort) {
	LockWrite();
	r.set_url_port(urlPort);
	Unlock();
}

inline int SiteResource::GetUrlPort() {
	LockRead();
	int result = r.url_port();
	Unlock();
	return result;
}

inline void SiteResource::ClearUrlPort() {
	LockWrite();
	r.clear_url_port();
	Unlock();
}

inline void SiteResource::SetIpAddr(IpAddr &addr) {
	LockWrite();
	this->addr = addr;
	Unlock();
}

inline IpAddr SiteResource::GetIpAddr() {
	LockRead();
	IpAddr &a = addr;
	Unlock();
	return a;
}

inline void SiteResource::ClearIpAddr() {
	LockWrite();
	addr.SetEmpty();
	Unlock();
}

inline void SiteResource::SetIpAddrExpire(uint32_t time) {
	LockWrite();
	r.set_ip_addr_expire(time);
	Unlock();
}

inline uint32_t SiteResource::GetIpAddrExpire() {
	LockRead();
	uint32_t expire = r.ip_addr_expire();
	Unlock();
	return expire;
}

inline void SiteResource::ClearIpAddrExpire() {
	LockWrite();
	r.clear_ip_addr_expire();
	Unlock();
}

inline void SiteResource::SetAllowUrls(const std::vector<std::string> &allow_urls) {
	LockWrite();
	r.clear_allow_urls();
	for (std::vector<std::string>::const_iterator iter = allow_urls.begin(); iter != allow_urls.end(); ++iter) {
		r.add_allow_urls(*iter);
	}
	Unlock();
}

inline void SiteResource::SetAllowUrl(int index, const std::string &url) {
	LockWrite();
	r.set_allow_urls(index, url);
	Unlock();
}

inline std::vector<std::string> *SiteResource::GetAllowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	LockRead();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		result->push_back(r.allow_urls(i));
	}
	Unlock();
	return result;
}

inline const std::string SiteResource::GetAllowUrl(int index) {
	LockRead();
	std::string &result = *r.mutable_allow_urls(index);
	Unlock();
	return result;
}

inline int SiteResource::CountAllowUrls() {
	LockRead();
	int result = r.allow_urls_size();
	Unlock();
	return result;
}

inline void SiteResource::ClearAllowUrls() {
	LockWrite();
	r.clear_allow_urls();
	Unlock();
}

inline void SiteResource::SetDisallowUrls(const std::vector<std::string> &disallow_urls) {
	LockWrite();
	r.clear_disallow_urls();
	for (std::vector<std::string>::const_iterator iter = disallow_urls.begin(); iter != disallow_urls.end(); ++iter) {
		r.add_disallow_urls(*iter);
	}
	Unlock();
}

inline void SiteResource::SetDisallowUrl(int index, const std::string &url) {
	LockWrite();
	r.set_disallow_urls(index, url);
	Unlock();
}

inline std::vector<std::string> *SiteResource::GetDisallowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	LockRead();
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		result->push_back(r.disallow_urls(i));
	}
	Unlock();
	return result;
}

inline const std::string SiteResource::GetDisallowUrl(int index) {
	LockRead();
	std::string &result = *r.mutable_disallow_urls(index);
	Unlock();
	return result;
}

inline int SiteResource::CountDisallowUrls() {
	LockRead();
	int result = r.disallow_urls_size();
	Unlock();
	return result;
}

inline void SiteResource::ClearDisallowUrls() {
	LockWrite();
	r.clear_disallow_urls();
	Unlock();
}

inline void SiteResource::SetRobotsExpire(uint32_t time) {
	LockWrite();
	r.set_robots_expire(time);
	Unlock();
}

inline uint32_t SiteResource::GetRobotsExpire() {
	LockRead();
	uint32_t expire = r.robots_expire();
	Unlock();
	return expire;
}

inline void SiteResource::ClearRobotsExpire() {
	LockWrite();
	r.clear_robots_expire();
	Unlock();
}

inline void SiteResource::SetRobotsRedirectCount(int redirects) {
	LockWrite();
	robots_redirect_count = redirects;
	Unlock();
}

inline int SiteResource::GetRobotsRedirectCount() {
	LockRead();
	int redirects = robots_redirect_count;
	Unlock();
	return redirects;
}

inline void SiteResource::ClearRobotsRedirectCount() {
	LockWrite();
	robots_redirect_count = 0;
	Unlock();
}

inline void SiteResource::SetRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, uint32_t time) {
	LockWrite();
	r.clear_allow_urls();
	for (std::vector<std::string>::const_iterator iter = allow_urls.begin(); iter != allow_urls.end(); ++iter) {
		r.add_allow_urls(*iter);
	}
	r.clear_disallow_urls();
	for (std::vector<std::string>::const_iterator iter = disallow_urls.begin(); iter != disallow_urls.end(); ++iter) {
		r.add_disallow_urls(*iter);
	}
	r.set_robots_expire(time);
	Unlock();
}

inline void SiteResource::GetRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, uint32_t &time) {
	LockRead();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		allow_urls.push_back(r.allow_urls(i));
	}
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		disallow_urls.push_back(r.disallow_urls(i));
	}
	time = r.robots_expire();
	Unlock();
}


inline bool SiteResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
