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
#include "IpAddr.h"
#include "ProtobufResource.h"
#include "ResourceFieldInfo.h"
#include "WebResource.pb.h"

class WebResource : public ProtobufResource {
public:
	typedef struct ParsedUrl_ {
		Scheme		scheme;
		std::string	username;
		std::string	password;
		std::string	host;
		int		port;
		std::string	path;
	} ParsedUrl;

	WebResource();
	WebResource(const WebResource &wr);
	~WebResource();
	// create copy of a resource
	ProtobufResource *Clone();
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
	// used by queues in case there is limit on queue size
	int getSize();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string toString(Object::LogLevel = Object::INFO);

	// WebResource-specific
	void setUrl(const std::string &url);
	const std::string &getUrl();
	void clearUrl();
	void ComposeUrl();		// construct URL from Url parts
	void setIpAddr(IpAddr &addr);
	IpAddr &getIpAddr();
	void clearIpAddr();
	void setHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values);
	std::vector<std::string> *getHeaderNames();
	void setHeaderValue(const std::string &name, const std::string &value);
	const std::string &getHeaderValue(const std::string &name);
	int getHeaderCount();
	void clearHeaderField(const std::string &name);
	void clearHeaderFields();
	void setRedirectCount(int count);
	int getRedirectCount();
	void clearRedirectCount();
	void setContent(const std::string &content);
	const std::string &getContent();
	std::string *getContentMutable();
	void clearContent();
	void setScheduled(long time);
	long getScheduled();
	void clearScheduled();

	// Url parts
        void setUrlScheme(int urlScheme);
        int getUrlScheme();
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

	static const int typeId = 10;

protected:
	// saved properties
	hector::resources::WebResource r;

	std::tr1::unordered_map<std::string, std::string> headers;
	int header_map_ready:1;
	int header_map_dirty:1;
	void LoadHeaders();
	void SaveHeaders();

	IpAddr addr;
	void LoadIpAddr();
	void SaveIpAddr();

	ParsedUrl url;
	int parsed_url_ready:1;
	int parsed_url_dirty:1;	
	void LoadParsedUrl();
	void SaveParsedUrl();

	static log4cxx::LoggerPtr logger;
};

inline ResourceFieldInfo *WebResource::getFieldInfo(const char *name) {
	return new ResourceFieldInfoT<WebResource>(name);
}

inline int WebResource::getTypeId() {
	return typeId;
}

inline const char *WebResource::getTypeStr() {
	return "WebResource";
}

inline const char *WebResource::getTypeStrShort() {
	return "WR";
}

inline const char *WebResource::getModuleStr() {
	return "HectorRobot";
}

inline int WebResource::getSize() {
	return r.content().length();
}

inline std::string *WebResource::Serialize() {
	if (header_map_dirty)
		SaveHeaders();
	if (parsed_url_dirty)
		SaveParsedUrl();
	SaveIpAddr();
	r.set_id(getId());
	r.set_status(getStatus());

	std::string *result = new std::string();
	r.SerializeToString(result);
	return result;
}

inline int WebResource::GetSerializedSize() {
	if (header_map_dirty)
		SaveHeaders();
	if (parsed_url_dirty)
		SaveParsedUrl();
	SaveIpAddr();
	r.set_id(getId());
	r.set_status(getStatus());

	return r.ByteSize();
}

inline bool WebResource::SerializeWithCachedSize(google::protobuf::io::CodedOutputStream *output) {
	// Headers, ParsedUrl, IpAddr, r.id and r.status were set in GetSerializedSize() already
	r.SerializeWithCachedSizes(output);
	return true;
}

inline bool WebResource::Deserialize(const char *data, int size) {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;

	bool result = r.ParseFromArray((void*)data, size);

	// we keep id
	setStatus(r.status());
	LoadIpAddr();
	return result;
}

inline bool WebResource::Deserialize(google::protobuf::io::CodedInputStream *input) {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;

	bool result = r.ParseFromCodedStream(input);

	// we keep id
	setStatus(r.status());
	LoadIpAddr();
	return result;
}

inline void WebResource::setUrl(const std::string &url) {
	parsed_url_ready = 0;
	// ignore anchor part
	size_t offset = url.find('#');
	if (offset != std::string::npos)
		r.set_url(url.substr(0, offset));
	else
		r.set_url(url);
}

inline const std::string &WebResource::getUrl() {
	if (parsed_url_dirty)
		SaveParsedUrl();
	return r.url();
}

inline void WebResource::clearUrl() {
	parsed_url_ready = 0;
	r.clear_url();
}

// construct URL from Url parts
inline void WebResource::ComposeUrl() {
	if (parsed_url_dirty)
		SaveParsedUrl();
}

inline void WebResource::setIpAddr(IpAddr &addr) {
	this->addr = addr;
}

inline IpAddr &WebResource::getIpAddr() {
	return addr;
}

inline void WebResource::clearIpAddr() {
	addr.setEmpty();
}

inline void WebResource::setRedirectCount(int count) {
	r.set_redirect_count(count);
}

inline int WebResource::getRedirectCount() {
	return r.redirect_count();
}

inline void WebResource::clearRedirectCount() {
	r.clear_redirect_count();
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

inline void WebResource::setScheduled(long time) {
	r.set_scheduled(time);
}

inline long WebResource::getScheduled() {
	return (long)r.scheduled();
}

inline void WebResource::clearScheduled() {
	r.clear_scheduled();
}

inline void WebResource::setUrlScheme(int urlScheme) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.scheme = (Scheme)urlScheme;
}

inline int WebResource::getUrlScheme() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return (int)url.scheme;
}

inline void WebResource::clearUrlScheme() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.scheme = NONE;
}

inline void WebResource::setUrlUsername(const std::string &urlUsername) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.username = urlUsername;
}

inline const std::string &WebResource::getUrlUsername() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.username;
}

inline void WebResource::clearUrlUsername() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.username.clear();
}

inline void WebResource::setUrlPassword(const std::string &urlPassword) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.password = urlPassword;
}

inline const std::string &WebResource::getUrlPassword() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.password;
}

inline void WebResource::clearUrlPassword() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.password.clear();
}

inline void WebResource::setUrlHost(const std::string &urlHost) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.host = urlHost;
}

inline const std::string &WebResource::getUrlHost() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.host;
}

inline void WebResource::clearUrlHost() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.host.clear();
}

inline void WebResource::setUrlPort(int urlPort) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.port = urlPort;
}

inline int WebResource::getUrlPort() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.port;
}

inline void WebResource::clearUrlPort() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.port = 0;
}

inline void WebResource::setUrlPath(const std::string &urlPath) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.path = urlPath;
}

inline const std::string &WebResource::getUrlPath() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.path;
}

inline void WebResource::clearUrlPath() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.path.clear();
}

#endif
