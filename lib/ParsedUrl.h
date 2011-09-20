/**
  ParsedUrl.h
*/

#ifndef _LIB_PARSEDURL_H_
#define _LIB_PARSEDURL_H_

#include <config.h>

#include "MD5.h"

class ParsedUrl {
public:
	enum Scheme {
		NONE = 0,
		HTTP = 1,
		HTTPS = 2
	};

	ParsedUrl();
	ParsedUrl(const std::string &url);
	~ParsedUrl() {};

	void SetUrl(const std::string &url);
	const std::string GetUrl();
	void ClearUrl();
	void SetSiteMD5(uint64_t md5);
	uint64_t GetSiteMD5();
	void SetPathMD5(uint64_t md5);
	uint64_t GetPathMD5();
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

private:
	std::string url;
	uint64_t site_md5;
	uint64_t path_md5;

	Scheme scheme;
	std::string username;
	std::string password;
	std::string host;
	int port;
	std::string path;

	unsigned int parsed_url_ready:1;
	unsigned int parsed_url_dirty:1;
	unsigned int site_md5_ready:1;
	unsigned int path_md5_ready:1;

	void LoadParsedUrl();
	void SaveParsedUrl();
};

inline ParsedUrl::ParsedUrl() {
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
	site_md5_ready = 0;
	path_md5_ready = 0;
}

inline ParsedUrl::ParsedUrl(const std::string &url) {
	SetUrl(url);
}

inline void ParsedUrl::SetUrl(const std::string &url) {
	// ignore anchor part
	size_t offset = url.find('#');
	if (offset != std::string::npos)
		this->url = url.substr(0, offset);
	else
		this->url = url;
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
	site_md5_ready = 0;
	path_md5_ready = 0;
}

inline const std::string ParsedUrl::GetUrl() {
	if (parsed_url_dirty)
		SaveParsedUrl();
	return url;
}

inline void ParsedUrl::ClearUrl() {
	url.clear();
	parsed_url_ready = 0;
	parsed_url_dirty = 0;
	site_md5_ready = 0;
	path_md5_ready = 0;
}

inline uint64_t ParsedUrl::GetSiteMD5() {
	if (!site_md5_ready) {
		char buf[1024];
		snprintf(buf, sizeof(buf), "%d %s %d", scheme, host.c_str(), port);
		uint64_t result[2];
	        MD5::HashBuffer((char*)&result, buf, strlen(buf));
		site_md5 = result[0];
	}
	return site_md5;
}

inline void ParsedUrl::SetSiteMD5(uint64_t md5) {
	site_md5 = md5;
	site_md5_ready = 1;
}

inline uint64_t ParsedUrl::GetPathMD5() {
	if (!path_md5_ready) {
		uint64_t result[2];
		MD5::HashBuffer((char*)&result, path.data(), path.length());
		path_md5 = result[0];
	}
	return path_md5;
}

inline void ParsedUrl::SetPathMD5(uint64_t md5) {
	path_md5 = md5;
	path_md5_ready = 1;
}

inline void ParsedUrl::SetUrlScheme(int urlScheme) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	scheme = (Scheme)urlScheme;
	parsed_url_dirty = 1;
}

inline int ParsedUrl::GetUrlScheme() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return (int)scheme;
}

inline void ParsedUrl::ClearUrlScheme() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	scheme = NONE;
}

inline void ParsedUrl::SetUrlUsername(const std::string &urlUsername) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	username = urlUsername;
	parsed_url_dirty = 1;
}

inline const std::string ParsedUrl::GetUrlUsername() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return username;
}

inline void ParsedUrl::ClearUrlUsername() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	username.clear();
	parsed_url_dirty = 1;
}

inline void ParsedUrl::SetUrlPassword(const std::string &urlPassword) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	password = urlPassword;
	parsed_url_dirty = 1;
}

inline const std::string ParsedUrl::GetUrlPassword() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return password;
}

inline void ParsedUrl::ClearUrlPassword() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	password.clear();
	parsed_url_dirty = 1;
}

inline void ParsedUrl::SetUrlHost(const std::string &urlHost) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	host = urlHost;
	parsed_url_dirty = 1;
}

inline const std::string ParsedUrl::GetUrlHost() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return host;
}

inline void ParsedUrl::ClearUrlHost() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	host.clear();
	parsed_url_dirty = 1;
}

inline void ParsedUrl::SetUrlPort(int urlPort) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	port = urlPort;
	parsed_url_dirty = 1;
}

inline int ParsedUrl::GetUrlPort() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return port;
}

inline void ParsedUrl::ClearUrlPort() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	port = 0;
	parsed_url_dirty = 1;
}

inline void ParsedUrl::SetUrlPath(const std::string &urlPath) {
	if (!parsed_url_ready)
		LoadParsedUrl();
	path = urlPath;
	parsed_url_dirty = 1;
}

inline const std::string ParsedUrl::GetUrlPath() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	return path;
}

inline void ParsedUrl::ClearUrlPath() {
	if (!parsed_url_ready)
		LoadParsedUrl();
	path.clear();
	parsed_url_dirty = 1;
}

#endif
