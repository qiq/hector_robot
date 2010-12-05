#ifndef _WEB_SITE_PATH_H_
#define _WEB_SITE_PATH_H_

#include <config.h>

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

	WebSitePath(): pathStatus(NONE), lastPathStatusUpdate(0), cksum(0), lastModified(0), modifiedHistory(0) {};
	~WebSitePath() {};

	// high-level API
	void Refresh(uint32_t cksum);
	int NextRefresh();

	// low-level API
	void setPathStatus(PathStatus pathStatus);
	PathStatus getPathStatus() const;
	void setLastPathStatusUpdate(uint32_t time);
	uint32_t getLastPathStatusUpdate() const;
	void setErrorCount(int count);
	int getErrorCount() const;
	void setRefreshing(bool refreshing);
	bool getRefreshing() const;
	void setCksum(uint32_t);
	uint32_t getCksum() const;
	void setLastModified(uint32_t);
	uint32_t getLastModified() const;
	void setModifiedHistory(uint32_t history);
	uint32_t getModifiedHistory() const;

private:
	uint32_t pathStatus;		// status(2B) + inRefresh(1B) + error count(1B)
	uint32_t lastPathStatusUpdate;	// when status was updated
	uint32_t cksum;			// checksum, to see whether page was changed or not
	uint32_t lastModified;		// when page was last changed
	uint32_t modifiedHistory;	// 4x1B period of modification (2^n minutes)
};

inline void WebSitePath::setPathStatus(PathStatus pathStatus) {
	this->pathStatus = (this->pathStatus & 0xFFFF0000) | pathStatus;
}

inline WebSitePath::PathStatus WebSitePath::getPathStatus() const {
	return (PathStatus)(pathStatus & 0x0000FFFF);
}

inline void WebSitePath::setLastPathStatusUpdate(uint32_t time) {
	lastPathStatusUpdate = time;
}

inline uint32_t WebSitePath::getLastPathStatusUpdate() const {
	return lastPathStatusUpdate;
}

inline void WebSitePath::setErrorCount(int count) {
	pathStatus = (pathStatus & 0x0000FFFF) | ((count & 0xFF) << 24);
}

inline int WebSitePath::getErrorCount() const {
	return pathStatus >> 24;
}

inline void WebSitePath::setRefreshing(bool refreshing) {
	if (refreshing)
		pathStatus |= 0x00010000;
	else
		pathStatus &= 0x00FF0000;
}

inline bool WebSitePath::getRefreshing() const {
	return (pathStatus & 0x00FF0000);
}

inline void WebSitePath::setCksum(uint32_t cksum) {
	this->cksum = cksum;
}

inline uint32_t WebSitePath::getCksum() const {
	return cksum;
}

inline void WebSitePath::setLastModified(uint32_t time) {
	lastModified = (uint32_t)time;
	// TODO: set modifiedHistory
}

inline uint32_t WebSitePath::getLastModified() const {
	return lastModified;
}

inline void WebSitePath::setModifiedHistory(uint32_t history) {
	this->modifiedHistory = history;
}

inline uint32_t WebSitePath::getModifiedHistory() const {
	return modifiedHistory;
}

#endif
