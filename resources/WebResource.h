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
#include "Resource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "WebResource.pb.h"

class ResourceAttrInfo;

class WebResourceInfo : public ResourceInfo {
public:
	WebResourceInfo();
};

class WebResource : public Resource {
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
	Resource *Clone();
	void Clear();
	// save and restore resource
	bool Serialize(ResourceOutputStream &output);
	bool Deserialize(ResourceInputStream &input);
	// used by queues in case there is limit on queue size
	int GetSize();
	// get info about this resource
	ResourceInfo *GetResourceInfo();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string ToString(Object::LogLevel = Object::INFO);

	// WebResource-specific
	void SetUrl(const std::string &url);
	const std::string GetUrl();
	void ClearUrl();
	void ComposeUrl();		// construct URL from Url parts
	void SetIpAddr(IpAddr &addr);
	IpAddr GetIpAddr();
	void ClearIpAddr();
	void SetHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values);
	std::vector<std::string> *GetHeaderNames();
	std::vector<std::string> *GetHeaderValues();
	void SetHeaderValue(const std::string &name, const std::string &value);
	const std::string GetHeaderValue(const std::string &name);
	int GetHeaderCount();
	void ClearHeaderField(const std::string &name);
	void ClearHeader();
	void SetRedirectCount(int count);
	int GetRedirectCount();
	void ClearRedirectCount();
	void SetContent(const std::string &content);
	const std::string GetContent();
	std::string *GetContentMutable();
	void ClearContent();
	void SetScheduled(long time);
	long GetScheduled();
	void ClearScheduled();

	// Url parts
        void SetUrlScheme(int urlScheme);
        int GetUrlScheme();
	void ClearUrlScheme();
        void SetUrlUsername(const std::string &urlUsername);
        const std::string GetUrlUsername();
	void ClearUrlUsername();
        void SetUrlPassword(const std::string &urlPassword);
        const std::string GetUrlPassword();
	void ClearUrlPassword();
        void SetUrlHost(const std::string &urlHost);
        const std::string GetUrlHost();
	void ClearUrlHost();
        void SetUrlPort(int port);
        int GetUrlPort();
	void ClearUrlPort();
        void SetUrlPath(const std::string &urlPath);
        const std::string GetUrlPath();
	void ClearUrlPath();

	static bool IsInstance(Resource *resource);

protected:
	// saved properties
	hector::resources::WebResource r;

	std::tr1::unordered_map<std::string, std::string> headers;
	unsigned int header_map_ready:1;
	unsigned int header_map_dirty:1;
	void LoadHeaders();
	void SaveHeaders();

	IpAddr addr;
	void LoadIpAddr();
	void SaveIpAddr();

	ParsedUrl url;
	unsigned int parsed_url_ready:1;
	unsigned int parsed_url_dirty:1;	
	void LoadParsedUrl();
	void SaveParsedUrl();

	static WebResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

inline bool WebResource::Serialize(ResourceOutputStream &output) {
	// Prepare Headers, ParsedUrl, IpAddr
	if (header_map_dirty)
		SaveHeaders();
	if (parsed_url_dirty)
		SaveParsedUrl();
	SaveIpAddr();

	output.WriteVarint32(r.ByteSize());
	r.SerializeWithCachedSizes(output.GetCodedOutputStream());
	return true;
}

inline bool WebResource::Deserialize(ResourceInputStream &input) {
	header_map_ready = 0;
	header_map_dirty = 0;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;

	uint32_t size;
	if (!input.ReadVarint32(&size))
                return false;
	google::protobuf::io::CodedInputStream::Limit l = input.PushLimit(size);
	bool result = r.ParseFromCodedStream(input.GetCodedInputStream());
	input.PopLimit(l);

	LoadIpAddr();
	return result;
}

inline int WebResource::GetSize() {
	return r.content().length();
}

inline ResourceInfo *WebResource::GetResourceInfo() {
	return &WebResource::resourceInfo;
}

inline void WebResource::SetUrl(const std::string &url) {
	parsed_url_ready = 0;
	// ignore anchor part
	size_t offset = url.find('#');
	if (offset != std::string::npos)
		r.set_url(url.substr(0, offset));
	else
		r.set_url(url);
}

inline const std::string WebResource::GetUrl() {
	if (parsed_url_dirty)
		SaveParsedUrl();
	return r.url();
}

inline void WebResource::ClearUrl() {
	parsed_url_ready = 0;
	r.clear_url();
}

// construct URL from Url parts
inline void WebResource::ComposeUrl() {
	if (parsed_url_dirty)
		SaveParsedUrl();
}

inline void WebResource::SetIpAddr(IpAddr &addr) {
	this->addr = addr;
}

inline IpAddr WebResource::GetIpAddr() {
	return addr;
}

inline void WebResource::ClearIpAddr() {
	addr.SetEmpty();
}

inline void WebResource::SetRedirectCount(int count) {
	r.set_redirect_count(count);
}

inline int WebResource::GetRedirectCount() {
	return r.redirect_count();
}

inline void WebResource::ClearRedirectCount() {
	r.clear_redirect_count();
}

inline void WebResource::SetContent(const std::string &content) {
	r.set_content(content);
}

inline const std::string WebResource::GetContent() {
	return r.content();
}

inline std::string *WebResource::GetContentMutable() {
	return r.mutable_content();
}

inline void WebResource::ClearContent() {
	r.clear_content();
}

inline void WebResource::SetScheduled(long time) {
	r.set_scheduled(time);
}

inline long WebResource::GetScheduled() {
	return (long)r.scheduled();
}

inline void WebResource::ClearScheduled() {
	r.clear_scheduled();
}

inline void WebResource::SetUrlScheme(int urlScheme) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.scheme = (Scheme)urlScheme;
}

inline int WebResource::GetUrlScheme() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return (int)url.scheme;
}

inline void WebResource::ClearUrlScheme() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.scheme = NONE;
}

inline void WebResource::SetUrlUsername(const std::string &urlUsername) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.username = urlUsername;
}

inline const std::string WebResource::GetUrlUsername() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.username;
}

inline void WebResource::ClearUrlUsername() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.username.clear();
}

inline void WebResource::SetUrlPassword(const std::string &urlPassword) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.password = urlPassword;
}

inline const std::string WebResource::GetUrlPassword() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.password;
}

inline void WebResource::ClearUrlPassword() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.password.clear();
}

inline void WebResource::SetUrlHost(const std::string &urlHost) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.host = urlHost;
}

inline const std::string WebResource::GetUrlHost() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.host;
}

inline void WebResource::ClearUrlHost() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.host.clear();
}

inline void WebResource::SetUrlPort(int urlPort) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.port = urlPort;
}

inline int WebResource::GetUrlPort() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.port;
}

inline void WebResource::ClearUrlPort() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.port = 0;
}

inline void WebResource::SetUrlPath(const std::string &urlPath) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	parsed_url_dirty = 1;
	url.path = urlPath;
}

inline const std::string WebResource::GetUrlPath() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return url.path;
}

inline void WebResource::ClearUrlPath() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	url.path.clear();
}

inline bool WebResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
