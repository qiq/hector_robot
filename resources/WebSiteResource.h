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

#define MAX_PATH_SIZE 2048

typedef struct WebSitePath_ {
	uint32_t status;		// status(3B) + error count(1B): OK,
					// error (maybe more error types),
					// disabled (too many errors occrued),
					// redirect permanent
	uint32_t lastStatusUpdate;	// when status was updated
	uint32_t cksum;			// checksum, to see whether page was changed or not
	uint32_t lastModified;		// when page was last changed
	uint32_t modifiedHistory;	// 4x1B period of modification (2^n minutes)
} WebSitePath;

class WebSiteResource : public ProtobufResource {
public:
	WebSiteResource();
	WebSiteResource(const WebSiteResource &wsr);
	~WebSiteResource();
	// create copy of a resource
	Resource *Clone();
	// save and restore resource
	std::string *Serialize();
	bool Deserialize(const char *data, int size);
	int getSerializedSize();
	bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
	bool SerializeWithCachedSizes(google::protobuf::io::ZeroCopyOutputStream *output);
	bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
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

	// path info get/set
	bool setPathInfo(const char *path, const WebSitePath *info);
	const WebSitePath *getPathInfo(const char *path);
	std::vector<std::string> *getPathList();

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

	static MemoryPool<WebSitePath> pool;

	// helper methods to convert from Judy array to protobuf representation and vice-versa
	bool ProtobufToJarray();
	void JarrayToProtobuf();

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
	std::string *result = MessageSerialize(&r);
	r.clear_paths();
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::Deserialize(const char *data, int size) {
	bool result = MessageDeserialize(&r, data, size);
	ProtobufToJarray();
	r.clear_paths();
	LoadIpAddr();
	return result;
}

inline int WebSiteResource::getSerializedSize() {
	lock.LockRead();
	SaveIpAddr();
	// fill protocol-buffers space using JArray
	JarrayToProtobuf();
	int result = MessageGetSerializedSize(&r);
	r.clear_paths();
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::Serialize(google::protobuf::io::ZeroCopyOutputStream *output) {
	lock.LockRead();
	SaveIpAddr();
	// fill protocol-buffers space using JArray
	JarrayToProtobuf();
	bool result = MessageSerialize(&r, output);
	r.clear_paths();
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::SerializeWithCachedSizes(google::protobuf::io::ZeroCopyOutputStream *output) {
	lock.LockRead();
	// IpAddr is already saved
	// fill protocol-buffers space using JArray
	JarrayToProtobuf();
	bool result = MessageSerializeWithCachedSizes(&r, output);
	r.clear_paths();
	lock.Unlock();
	return result;
}

inline bool WebSiteResource::Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size) {
	bool result = MessageDeserialize(&r, input, size);
	ProtobufToJarray();
	r.clear_paths();
	LoadIpAddr();
	return result;
}

inline void WebSiteResource::setUrlScheme(int urlScheme) {
	r.set_url_scheme((Scheme)urlScheme);
}

inline int WebSiteResource::getUrlScheme() {
	return (int)r.url_scheme();
}

inline void WebSiteResource::clearUrlScheme() {
	r.clear_url_scheme();
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
	r.set_url_port(urlPort);
}

inline int WebSiteResource::getUrlPort() {
	return r.url_port();
}

inline void WebSiteResource::clearUrlPort() {
	r.clear_url_port();
}

inline void WebSiteResource::setIpAddr(IpAddr &addr) {
	lock.LockWrite();
	this->addr = addr;
	lock.Unlock();
}

inline IpAddr WebSiteResource::getIpAddr() {
	IpAddr a;
	lock.LockRead();
	a = addr;
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

inline void WebSiteResource::setAllowUrls(const std::vector<std::string> &allow_urls) {
	r.clear_allow_urls();
	for (std::vector<std::string>::const_iterator iter = allow_urls.begin(); iter != allow_urls.end(); ++iter) {
		r.add_allow_urls(*iter);
	}
}

inline std::vector<std::string> *WebSiteResource::getAllowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	for (int i = 0; i < r.allow_urls_size(); i++) {
		result->push_back(r.allow_urls(i));
	}
	return result;
}

inline void WebSiteResource::clearAllowUrls() {
	r.clear_allow_urls();
}

inline void WebSiteResource::setDisallowUrls(const std::vector<std::string> &disallow_urls) {
	r.clear_disallow_urls();
	for (std::vector<std::string>::const_iterator iter = disallow_urls.begin(); iter != disallow_urls.end(); ++iter) {
		r.add_disallow_urls(*iter);
	}
}

inline std::vector<std::string> *WebSiteResource::getDisallowUrls() {
	std::vector<std::string> *result = new std::vector<std::string>();
	for (int i = 0; i < r.disallow_urls_size(); i++) {
		result->push_back(r.disallow_urls(i));
	}
	return result;
}

inline void WebSiteResource::clearDisallowUrls() {
	r.clear_disallow_urls();
}

inline bool WebSiteResource::setPathInfo(const char *path, const WebSitePath *info) {
	lock.LockWrite();
	PWord_t PValue;
	PValue = (PWord_t)JudySLGet(paths, (uint8_t*)path, NULL);
	if (PValue) {
		WebSitePath *wsp = (WebSitePath*)PValue;
		*wsp = *info;
	} else {
		WebSitePath *wsp = pool.alloc();
		*wsp = *info;
		PValue = (PWord_t)JudySLIns(&paths, (uint8_t*)path, NULL);
		if (PValue == PJERR) {
			LOG4CXX_ERROR(logger, "Malloc failed");
			lock.Unlock();
			return false;
		}
		*PValue = (Word_t)wsp;
	}
	lock.Unlock();
	return true;
}

inline const WebSitePath *WebSiteResource::getPathInfo(const char *path) {
	lock.LockRead();
	PWord_t PValue;
	PValue = (PWord_t)JudySLGet(paths, (uint8_t*)path, NULL);
	WebSitePath *result = PValue ? (WebSitePath*)PValue : NULL;
	lock.Unlock();
	return result;
}

inline std::vector<std::string> *WebSiteResource::getPathList() {
	std::vector<std::string> *result = new std::vector<std::string>();

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

#endif
