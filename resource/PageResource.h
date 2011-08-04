/**
 * Class representing resource (mainly HTML pages) while processing.
 * It uses Google Protocol Buffers to de/serialize.
 */

#ifndef _PAGE_RESOURCE_H_
#define _PAGE_RESOURCE_H_

#include <config.h>

#include <vector>
#include <string>
#include <tr1/unordered_map>
#include <log4cxx/logger.h>
#include "common.h"
#include "IpAddr.h"
#include "ParsedUrl.h"
#include "Resource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "PageResource.pb.h"

class ResourceAttrInfo;

class PageResourceInfo : public ResourceInfo {
public:
	PageResourceInfo();
};

class PageResource : public Resource {
public:
	PageResource();
	PageResource(const PageResource &wr);
	~PageResource();
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

	// PageResource-specific
	void SetUrl(const std::string &url);
	const std::string GetUrl();
	void ClearUrl();
	void SetSiteMD5(uint64_t md5);
	uint64_t GetSiteMD5();
	void ClearSiteMD5();
	void SetPathMD5(uint64_t md5);
	uint64_t GetPathMD5();
	void ClearPathMD5();
	void ComputeMD5();
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
	// memory-only properties
	int redirect_count;
	// saved properties
	hector::resources::PageResource r;

	// helper: headers map, so that we can lookup headers fast
	std::tr1::unordered_map<std::string, std::string> headers;
	unsigned int header_map_ready:1;
	unsigned int header_map_dirty:1;
	void LoadHeaders();
	void SaveHeaders();

	// helper: parsed IP address
	IpAddr addr;
	void LoadIpAddr();
	void SaveIpAddr();

	ParsedUrl url;

	static PageResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

inline PageResource::PageResource() : redirect_count(0) {
	header_map_ready = 0;
	header_map_dirty = 0;
}

inline PageResource::PageResource(const PageResource &wr) : Resource(wr), redirect_count(wr.redirect_count), r(wr.r), headers(wr.headers) {
	header_map_ready = 0;
	header_map_dirty = 0;
}

inline PageResource::~PageResource() {
}

inline Resource *PageResource::Clone() {
	return new PageResource(*this);
}

inline void PageResource::Clear() {
	Resource::Clear();
	redirect_count = 0;
	r.Clear();
	addr.SetEmpty();
	header_map_ready = 0;
	header_map_dirty = 0;
	headers.clear();
	url.ClearUrl();
}

inline bool PageResource::Serialize(ResourceOutputStream &output) {
	// Prepare Headers, ParsedUrl, IpAddr
	if (header_map_dirty)
		SaveHeaders();
	SaveIpAddr();
	r.set_url(url.GetUrl());
	r.set_site_md5(url.GetSiteMD5());
	r.set_path_md5(url.GetPathMD5());

	output.SerializeMessage(r);
	return true;
}

inline bool PageResource::Deserialize(ResourceInputStream &input, bool headerOnly) {
	if (headerOnly)
		return true;
	header_map_ready = 0;
	header_map_dirty = 0;

	bool result = input.ParseMessage(r);

	LoadIpAddr();
	url.SetUrl(r.url());
	url.SetSiteMD5(r.site_md5());
	url.SetPathMD5(r.path_md5());
	return result;
}

inline bool PageResource::Skip(ResourceInputStream &input) {
	return input.ParseMessage(r, 0, true);
}

inline int PageResource::GetSize() {
	return r.content().length();
}

inline ResourceInfo *PageResource::GetResourceInfo() {
	return &PageResource::resourceInfo;
}

inline void PageResource::SetUrl(const std::string &url) {
	this->url.SetUrl(url);
}

inline const std::string PageResource::GetUrl() {
	return url.GetUrl();
}

inline void PageResource::ClearUrl() {
	url.ClearUrl();
}

inline void PageResource::SetSiteMD5(uint64_t md5) {
	url.SetSiteMD5(md5);
}

inline uint64_t PageResource::GetSiteMD5() {
	return url.GetSiteMD5();
}

inline void PageResource::ClearSiteMD5() {
	url.SetSiteMD5(0);
}

inline void PageResource::SetPathMD5(uint64_t md5) {
	url.SetPathMD5(md5);
}

inline uint64_t PageResource::GetPathMD5() {
	return url.GetPathMD5();
}

inline void PageResource::ClearPathMD5() {
	url.SetPathMD5(0);
}

inline void PageResource::SetIpAddr(IpAddr &addr) {
	this->addr = addr;
}

inline IpAddr PageResource::GetIpAddr() {
	return addr;
}

inline void PageResource::ClearIpAddr() {
	addr.SetEmpty();
}

inline void PageResource::SetRedirectCount(int count) {
	redirect_count = count;
}

inline int PageResource::GetRedirectCount() {
	return redirect_count;
}

inline void PageResource::ClearRedirectCount() {
	redirect_count = 0;
}

inline void PageResource::SetContent(const std::string &content) {
	r.set_content(content);
}

inline const std::string PageResource::GetContent() {
	return r.content();
}

inline std::string *PageResource::GetContentMutable() {
	return r.mutable_content();
}

inline void PageResource::ClearContent() {
	r.clear_content();
}

inline void PageResource::SetUrlScheme(int urlScheme) {
	url.SetUrlScheme(urlScheme);
}

inline int PageResource::GetUrlScheme() {
	return url.GetUrlScheme();
}

inline void PageResource::ClearUrlScheme() {
	url.ClearUrlScheme();
}

inline void PageResource::SetUrlUsername(const std::string &urlUsername) {
	url.SetUrlUsername(urlUsername);
}

inline const std::string PageResource::GetUrlUsername() {
	return url.GetUrlUsername();
}

inline void PageResource::ClearUrlUsername() {
	url.ClearUrlUsername();
}

inline void PageResource::SetUrlPassword(const std::string &urlPassword) {
	url.SetUrlPassword(urlPassword);
}

inline const std::string PageResource::GetUrlPassword() {
	return url.GetUrlPassword();
}

inline void PageResource::ClearUrlPassword() {
	url.ClearUrlPassword();
}

inline void PageResource::SetUrlHost(const std::string &urlHost) {
	url.SetUrlHost(urlHost);
}

inline const std::string PageResource::GetUrlHost() {
	return url.GetUrlHost();
}

inline void PageResource::ClearUrlHost() {
	url.ClearUrlHost();
}

inline void PageResource::SetUrlPort(int urlPort) {
	url.SetUrlPort(urlPort);
}

inline int PageResource::GetUrlPort() {
	return url.GetUrlPort();
}

inline void PageResource::ClearUrlPort() {
	url.ClearUrlPort();
}

inline void PageResource::SetUrlPath(const std::string &urlPath) {
	url.SetUrlPath(urlPath);
}

inline const std::string PageResource::GetUrlPath() {
	return url.GetUrlPath();
}

inline void PageResource::ClearUrlPath() {
	url.ClearUrlPath();
}

inline bool PageResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
