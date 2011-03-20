#ifndef _WEB_SITE_PATH_H_
#define _WEB_SITE_PATH_H_

#include <config.h>

#include <math.h>
#include <functional>
#include <string>
#include <log4cxx/logger.h>
#include "WebSiteResource.pb.h"

class WebSitePath {
public:
	enum PathStatus {
		NONE = 0,
		OK = 1,			// periodically updated
		NEW_LINK = 2,		// not yet downloaded
		REDIRECT = 3,		// permanent redirection
		ERROR = 4,		// error
		DISABLED = 5,		// n consecutive errors
	};

	WebSitePath(): pathStatus(NONE), lastPathStatusUpdate(0), size(0), cksum(0), lastModified(0), modificationHistory(0) {};
	~WebSitePath() {};

	// high-level API
	int NextRefresh();

	// low-level API
	void SetPathStatus(PathStatus pathStatus);
	PathStatus GetPathStatus() const;
	void SetLastPathStatusUpdate(uint32_t time);
	uint32_t GetLastPathStatusUpdate() const;
	void SetErrorCount(int count);
	int GetErrorCount() const;
	void SetRefreshing(bool refreshing);
	bool GetRefreshing() const;
	void SetSize(uint32_t);
	uint32_t GetSize() const;
	void SetCksum(uint32_t);
	uint32_t GetCksum() const;
	void SetLastModified(uint32_t);
	uint32_t GetLastModified() const;
	void SetModificationHistory(uint32_t history);
	uint32_t GetModificationHistory() const;

private:
	uint32_t pathStatus;		// status(2B) + inRefresh(1B) + error count(1B)
	uint32_t lastPathStatusUpdate;	// when status was updated
	uint32_t size;			// size, to see whether page was changed or not
	uint32_t cksum;			// checksum, to see whether page was changed or not
	uint32_t lastModified;		// when page was last changed
	uint32_t modificationHistory;	// 4x1B period of modification (2^n minutes)
};

inline void WebSitePath::SetPathStatus(PathStatus pathStatus) {
	this->pathStatus = (this->pathStatus & 0xFFFF0000) | pathStatus;
}

inline WebSitePath::PathStatus WebSitePath::GetPathStatus() const {
	return (PathStatus)(pathStatus & 0x0000FFFF);
}

inline void WebSitePath::SetLastPathStatusUpdate(uint32_t time) {
	lastPathStatusUpdate = time;
}

inline uint32_t WebSitePath::GetLastPathStatusUpdate() const {
	return lastPathStatusUpdate;
}

inline void WebSitePath::SetErrorCount(int count) {
	pathStatus = (pathStatus & 0x0000FFFF) | ((count & 0xFF) << 24);
}

inline int WebSitePath::GetErrorCount() const {
	return pathStatus >> 24;
}

inline void WebSitePath::SetRefreshing(bool refreshing) {
	if (refreshing)
		pathStatus |= 0x00010000;
	else
		pathStatus &= 0xFF00FFFF;
}

inline bool WebSitePath::GetRefreshing() const {
	return (pathStatus & 0x00FF0000);
}

inline void WebSitePath::SetSize(uint32_t size) {
	this->size = size;
}

inline uint32_t WebSitePath::GetSize() const {
	return size;
}

inline void WebSitePath::SetCksum(uint32_t cksum) {
	this->cksum = cksum;
}

inline uint32_t WebSitePath::GetCksum() const {
	return cksum;
}

inline void WebSitePath::SetLastModified(uint32_t time) {
	lastModified = time;
}

inline uint32_t WebSitePath::GetLastModified() const {
	return lastModified;
}

inline void WebSitePath::SetModificationHistory(uint32_t history) {
	this->modificationHistory = history;
}

inline uint32_t WebSitePath::GetModificationHistory() const {
	return modificationHistory;
}

#endif
