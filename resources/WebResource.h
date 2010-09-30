/**
 * Class representing queue of resources (mainly HTML pages) while processing.
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
	bool Deserialize(std::string *s);
	int getSerializedSize();
	bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
	bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
	// used by queues in case there is limit on queue size
	int getSize();
	// return string representation of the resource (e.g. for debugging purposes)
	char *toString(Object::LogLevel = Object::INFO);

	void setUrl(const char *url);
	const char *getUrl();
	void setTime(long time);
	long getTime();
	void setMimeType(const char *mimeType);
	const char *getMimeType();
	void setContent(const char *content);
	const char *getContent();
	void setHeaderFields(std::vector<std::string> *names, std::vector<std::string> *values);
	std::vector<std::string> *getHeaderNames();
	void setHeaderValue(const char *name, const char *value);
	const char *getHeaderValue(const char *name);
	void setExtractedUrls(std::vector<std::string> *extracted_urls);
	std::vector<std::string> *getExtractedUrls();
	void setIp4Addr(ip4_addr_t addr);
	ip4_addr_t getIp4Addr();
	void setIp6Addr(ip6_addr_t addr);
	ip6_addr_t getIp6Addr();
	void setIpAddrExpire(long time);
	long getIpAddrExpire();

        void setUrlScheme(const char *urlScheme);
        const char *getUrlScheme();
        void setUrlUsername(const char *urlUsername);
        const char *getUrlUsername();
        void setUrlPassword(const char *urlPassword);
        const char *getUrlPassword();
        void setUrlHost(const char *urlHost);
        const char *getUrlHost();
        void setUrlPort(int port);
        int getUrlPort();
        void setUrlPath(const char *urlPath);
        const char *getUrlPath();
        void setUrlQuery(const char *urlQuery);
        const char *getUrlQuery();
        void setUrlRef(const char *urlRef);
        const char *getUrlRef();

	static const int typeId = 10;

	typedef struct {
		Resource::FieldType type;
		union {
			const char *(WebResource::*s)();
			int (WebResource::*i)();
			long (WebResource::*l)();
			ip4_addr_t (WebResource::*a4)();
			ip6_addr_t (WebResource::*a6)();
			const char *(WebResource::*s2)(const char*);
		} get;
		union {
			void (WebResource::*s)(const char *);
			void (WebResource::*i)(int);
			void (WebResource::*l)(long);
			void (WebResource::*a4)(ip4_addr_t);
			void (WebResource::*a6)(ip6_addr_t);
			void (WebResource::*s2)(const char*, const char*);
		} set;
	} FieldInfo;

	// get info about an item
	static FieldInfo getFieldInfo(const char *name);

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

inline bool WebResource::Deserialize(std::string *s) {
	header_map_ready = false;
	header_map_dirty = false;
	return MessageDeserialize(&r, s);
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


inline void WebResource::setUrl(const char *url) {
	r.set_url(url);
}

inline const char *WebResource::getUrl() {
	return r.url().c_str();
}

inline void WebResource::setTime(long time) {
	r.set_time(time);
}

inline long WebResource::getTime() {
	return (long)r.time();
}

inline void WebResource::setMimeType(const char *mimeType) {
	r.set_mime_type(mimeType);
}

inline const char *WebResource::getMimeType() {
	return r.mime_type().c_str();
}

inline void WebResource::setContent(const char *content) {
	r.set_content(content);
}

inline const char *WebResource::getContent() {
	return r.content().c_str();
}

inline void WebResource::setIp4Addr(ip4_addr_t addr) {
	r.set_ip4_addr(addr.addr);
}

inline ip4_addr_t WebResource::getIp4Addr() {
	ip4_addr_t a;
	a.addr = r.ip4_addr();
	return a;
}

inline void WebResource::setIp6Addr(ip6_addr_t addr) {
	uint64_t a = 0, b = 0;
	for (int i = 0; i < 8; i++) {
		a = a*8 + addr.addr[i];
		b = b*8 + addr.addr[i+8];
	}
	r.set_ip6_addr_1(a);
	r.set_ip6_addr_2(b);
}

inline ip6_addr_t WebResource::getIp6Addr() {
	ip6_addr_t addr;
	uint64_t a = r.ip6_addr_1();
	uint64_t b = r.ip6_addr_2();
	for (int i = 7; i >= 0; i--) {
		addr.addr[i] = a & 0x00000000000000FF;
		a >>= 8;
		addr.addr[i+8] = b & 0x00000000000000FF;
		a >>= 8;
	}
	return addr;
}

inline void WebResource::setIpAddrExpire(long time) {
	r.set_ip_addr_expire(time);
}

inline long WebResource::getIpAddrExpire() {
	return (long)r.ip_addr_expire();
}

inline void WebResource::setUrlScheme(const char *urlScheme) {
	r.set_url_scheme(urlScheme);
}

inline const char *WebResource::getUrlScheme() {
	return r.url_scheme().c_str();
}

inline void WebResource::setUrlUsername(const char *urlUsername) {
	r.set_url_username(urlUsername);
}

inline const char *WebResource::getUrlUsername() {
	return r.url_username().c_str();
}

inline void WebResource::setUrlPassword(const char *urlPassword) {
	r.set_url_password(urlPassword);
}

inline const char *WebResource::getUrlPassword() {
	return r.url_password().c_str();
}

inline void WebResource::setUrlHost(const char *urlHost) {
	r.set_url_host(urlHost);
}

inline const char *WebResource::getUrlHost() {
	return r.url_host().c_str();
}

inline void WebResource::setUrlPort(int urlPort) {
	r.set_url_port(urlPort);
}

inline int WebResource::getUrlPort() {
	return r.url_port();
}

inline void WebResource::setUrlPath(const char *urlPath) {
	r.set_url_path(urlPath);
}

inline const char *WebResource::getUrlPath() {
	return r.url_path().c_str();
}

inline void WebResource::setUrlQuery(const char *urlQuery) {
	r.set_url_query(urlQuery);
}

inline const char *WebResource::getUrlQuery() {
	return r.url_query().c_str();
}

inline void WebResource::setUrlRef(const char *urlRef) {
	r.set_url_ref(urlRef);
}

inline const char *WebResource::getUrlRef() {
	return r.url_ref().c_str();
}

#endif
