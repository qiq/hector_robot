/**
 * Class representing resource (mainly HTML pages) while processing.
 * It uses Google Protocol Buffers to de/serialize.
 */

#ifndef _WEB_RESOURCE_H_
#define _WEB_RESOURCE_H_

#include <config.h>

#include <vector>
#include <string>
#include <tr1/unordered_map>
#include <log4cxx/logger.h>
#include "common.h"
#include "ProtobufResource.h"
#include "WebResource.pb.h"

class WebResource : public ProtobufResource {
public:
	WebResource();
	~WebResource();
	// create copy of a resource
	ProtobufResource *Clone();
	// get info about a resource field
	ResourceFieldInfo *getFieldInfo(const char *name);
	// type id of a resource (to be used by Resources::CreateResource(typeid))
	int getTypeId();
	// type string of a resource
	const char *getTypeStr();
	// module prefix (e.g. Hector for Hector::TestResource)
	const char *getModuleStr();
	// id should be unique across all resources
	int getId();
	void setId(int id);
	// status may be tested in Processor to select target queue
	int getStatus();
	void setStatus(int status);

	// save and restore resource
	std::string *Serialize();
	bool Deserialize(const char *data, int size);
	int getSerializedSize();
	bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
	bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
	// used by queues in case there is limit on queue size
	int getSize();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string *toString(Object::LogLevel = Object::INFO);

	// WebResource-specific
	void setUrl(const std::string &url);
	const std::string &getUrl();
	void clearUrl();
	void setTime(long time);
	long getTime();
	void clearTime();
	void setMimeType(const std::string &mimeType);
	const std::string &getMimeType();
	void clearMimeType();
	void setContent(const std::string &content);
	const std::string &getContent();
	std::string *getContentMutable();
	void clearContent();
	void setHeaderFields(std::vector<std::string> *names, std::vector<std::string> *values);
	std::vector<std::string> *getHeaderNames();
	void setHeaderValue(const std::string &name, const std::string &value);
	const std::string &getHeaderValue(const std::string &name);
	void clearHeaderFields();
	void setExtractedUrls(std::vector<std::string> *extracted_urls);
	std::vector<std::string> *getExtractedUrls();
	void clearExtractedUrls();
	void setIp4Addr(ip4_addr_t addr);
	ip4_addr_t getIp4Addr();
	void clearIp4Addr();
	void setIp6Addr(ip6_addr_t addr);
	ip6_addr_t getIp6Addr();
	void clearIp6Addr();
	void setIpAddrExpire(long time);
	long getIpAddrExpire();
	void clearIpAddrExpire();

	// Url parts
        void setUrlScheme(const std::string &urlScheme);
        const std::string &getUrlScheme();
	void clearUrlScheme();
        void setUrlUsername(const std::string &urlUsername);
        const std::string &getUrlUsername();
	void clearUrlUsername();
        void setUrlPassword(const std::string &urlPassword);
        const std::string &getUrlPassword();
	void clearUrlPassword();
        void setUrlHost(const std::string &urlHost);
        const std::string &getUrlHost();
	void clearUrlHost();
        void setUrlPort(int port);
        int getUrlPort();
	void clearUrlPort();
        void setUrlPath(const std::string &urlPath);
        const std::string &getUrlPath();
	void clearUrlPath();
        void setUrlQuery(const std::string &urlQuery);
	const std::string &getUrlQuery();
	void clearUrlQuery();

	static const int typeId = 10;

protected:
	// saved properties
	hector::resources::WebResource r;
	// memory-only
	bool header_map_ready;
	bool header_map_dirty;
	std::tr1::unordered_map<std::string, std::string> headers;

	void LoadHeaders();
	void SaveHeaders();

	static log4cxx::LoggerPtr logger;
};

class WebResourceFieldInfo : public ResourceFieldInfo {
public:
	WebResourceFieldInfo(const std::string &name);
	~WebResourceFieldInfo();

	const std::string &getString(Resource*);
	int getInt(Resource*);
	long getLong(Resource*);
	ip4_addr_t getIp4Addr(Resource*);
	ip6_addr_t getIp6Addr(Resource*);
	const std::string &getString2(Resource*, const std::string&);

	void setString(Resource*, const std::string&);
	void setInt(Resource*, int);
	void setLong(Resource*, long);
	void setIp4Addr(Resource*, ip4_addr_t);
	void setIp6Addr(Resource*, ip6_addr_t);
	void setString2(Resource*, const std::string&, const std::string&);

	void clear(Resource*);
	void clearString2(Resource*, const std::string&);

private:
	union {
		const std::string &(WebResource::*s)();
		int (WebResource::*i)();
		long (WebResource::*l)();
		ip4_addr_t (WebResource::*a4)();
		ip6_addr_t (WebResource::*a6)();
		const std::string &(WebResource::*s2)(const std::string&);
	} get_u;
	union {
		void (WebResource::*s)(const std::string&);
		void (WebResource::*i)(int);
		void (WebResource::*l)(long);
		void (WebResource::*a4)(ip4_addr_t);
		void (WebResource::*a6)(ip6_addr_t);
		void (WebResource::*s2)(const std::string&, const std::string&);
	} set_u;
	union {
		void (WebResource::*c)();
		void (WebResource::*s2)(const std::string&);
	} clear_u;
};

inline ResourceFieldInfo *WebResource::getFieldInfo(const char *name) {
	return new WebResourceFieldInfo(name);
}

inline int WebResource::getTypeId() {
	return typeId;
}

inline const char *WebResource::getTypeStr() {
	return "WebResource";
}

inline const char *WebResource::getModuleStr() {
	return "HectorRobot";
}

inline int WebResource::getId() {
	return r.id();
}

inline void WebResource::setId(int id) {
	r.set_id(id);
}

inline int WebResource::getStatus() {
	return r.status();
}

inline void WebResource::setStatus(int status) {
	r.set_status(status);
}

inline std::string *WebResource::Serialize() {
	if (header_map_dirty)
		SaveHeaders();
	return MessageSerialize(&r);
}

inline bool WebResource::Deserialize(const char *data, int size) {
	header_map_ready = false;
	header_map_dirty = false;
	return MessageDeserialize(&r, data, size);
}

inline int WebResource::getSerializedSize() {
	if (header_map_dirty)
		SaveHeaders();
	return MessageGetSerializedSize(&r);
}

inline bool WebResource::Serialize(google::protobuf::io::ZeroCopyOutputStream *output) {
	if (header_map_dirty)
		SaveHeaders();
	return MessageSerialize(&r, output);
}

inline bool WebResource::Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size) {
	header_map_ready = false;
	header_map_dirty = false;
	return MessageDeserialize(&r, input, size);
}


inline void WebResource::setUrl(const std::string &url) {
	r.set_url(url);
}

inline const std::string &WebResource::getUrl() {
	return r.url();
}

inline void WebResource::clearUrl() {
	r.clear_url();
}

inline void WebResource::setTime(long time) {
	r.set_time(time);
}

inline long WebResource::getTime() {
	return (long)r.time();
}

inline void WebResource::clearTime() {
	r.clear_time();
}

inline void WebResource::setMimeType(const std::string &mimeType) {
	r.set_mime_type(mimeType);
}

inline const std::string &WebResource::getMimeType() {
	return r.mime_type();
}

inline void WebResource::clearMimeType() {
	r.clear_mime_type();
}

inline void WebResource::setContent(const std::string &content) {
	r.set_content(content);
}

inline const std::string &WebResource::getContent() {
	return r.content();
}

inline std::string *WebResource::getContentMutable() {
	return r.mutable_content();
}

inline void WebResource::clearContent() {
	r.clear_content();
}

inline void WebResource::setIp4Addr(ip4_addr_t addr) {
	r.set_ip4_addr(addr.addr);
}

inline ip4_addr_t WebResource::getIp4Addr() {
	ip4_addr_t a;
	a.addr = r.ip4_addr();
	return a;
}

inline void WebResource::clearIp4Addr() {
	r.clear_ip4_addr();
}

inline void WebResource::setIp6Addr(ip6_addr_t addr) {
	uint64_t a = 0, b = 0;
	for (int i = 0; i < 8; i++) {
		a = (a << 8) + addr.addr[15-i];
		b = (b << 8) + addr.addr[7-i];
	}
	r.set_ip6_addr_1(a);
	r.set_ip6_addr_2(b);
}

inline ip6_addr_t WebResource::getIp6Addr() {
	ip6_addr_t addr;
	uint64_t a = r.ip6_addr_1();
	uint64_t b = r.ip6_addr_2();
	for (int i = 0; i < 8; i++) {
		addr.addr[8+i] = a & 0x00000000000000FF;
		a >>= 8;
		addr.addr[i] = b & 0x00000000000000FF;
		b >>= 8;
	}
	return addr;
}

inline void WebResource::clearIp6Addr() {
	r.clear_ip6_addr_1();
	r.clear_ip6_addr_2();
}

inline void WebResource::setIpAddrExpire(long time) {
	r.set_ip_addr_expire(time);
}

inline long WebResource::getIpAddrExpire() {
	return (long)r.ip_addr_expire();
}

inline void WebResource::clearIpAddrExpire() {
	r.clear_ip_addr_expire();
}

inline void WebResource::setUrlScheme(const std::string &urlScheme) {
	r.set_url_scheme(urlScheme);
}

inline const std::string &WebResource::getUrlScheme() {
	return r.url_scheme();
}

inline void WebResource::clearUrlScheme() {
	r.clear_url_scheme();
}

inline void WebResource::setUrlUsername(const std::string &urlUsername) {
	r.set_url_username(urlUsername);
}

inline const std::string &WebResource::getUrlUsername() {
	return r.url_username();
}

inline void WebResource::clearUrlUsername() {
	r.clear_url_username();
}

inline void WebResource::setUrlPassword(const std::string &urlPassword) {
	r.set_url_password(urlPassword);
}

inline const std::string &WebResource::getUrlPassword() {
	return r.url_password();
}

inline void WebResource::clearUrlPassword() {
	r.clear_url_password();
}

inline void WebResource::setUrlHost(const std::string &urlHost) {
	r.set_url_host(urlHost);
}

inline const std::string &WebResource::getUrlHost() {
	return r.url_host();
}

inline void WebResource::clearUrlHost() {
	r.clear_url_host();
}

inline void WebResource::setUrlPort(int urlPort) {
	r.set_url_port(urlPort);
}

inline int WebResource::getUrlPort() {
	return r.url_port();
}

inline void WebResource::clearUrlPort() {
	r.clear_url_port();
}

inline void WebResource::setUrlPath(const std::string &urlPath) {
	r.set_url_path(urlPath);
}

inline const std::string &WebResource::getUrlPath() {
	return r.url_path();
}

inline void WebResource::clearUrlPath() {
	r.clear_url_path();
}

inline void WebResource::setUrlQuery(const std::string &urlQuery) {
	r.set_url_query(urlQuery);
}

inline const std::string &WebResource::getUrlQuery() {
	return r.url_query();
}

inline void WebResource::clearUrlQuery() {
	r.clear_url_query();
}

#endif
