/**
 * Fetcher object (html page) using HTTP.
 * Uses CURL in non-blocking mode and libev.
 */

#ifndef _MODULES_FETCH_H_
#define _MODULES_FETCH_H_

#include <config.h>

#include <queue>
#include <string>
#include <tr1/unordered_map>
#include <ev.h>
#include <curl/curl.h>
#include "common.h"
#include "Module.h"
#include "ObjectValues.h"
#include "WebResource.h"

typedef struct CurlResourceInfo_ {
	int index;		// index in the resourceInfo table
	WebResource *current;	// currently processed Resource
	std::string *content;	// content to be filled (pointer to current WebResource)
	long contentLength;	// content size of current object (copy of Content-Length, if known)
	long maxContentLength;	// copy from Fetcher object
	bool contentIsText;	// content is text (text/html or text/plain)?
	std::deque<WebResource*> waiting;	// waiting resource in the current hash bucket
	CURL *easy;		// CURL easy handle (to be reused)
	curl_socket_t socketfd;	// socket used by CURL
	struct curl_slist *headers; // headers to be set
	struct ev_io event;	// io event associated with the socket
	bool evSet;		// whether event was set or not
	uint32_t time;		// last request completion time
	struct CurlInfo_ *info; // parent

	bool operator<(const struct CurlResourceInfo_ &ri) {
		return time > ri.time;
	}
} CurlResourceInfo;

typedef struct CurlInfo_ {
	class Fetcher *parent;

	struct ev_loop *loop;
	struct ev_timer curlTimer;
	struct ev_timer tickTimer;

	CURLM *multi;
	int resources;		// number of resources currently in queue/processed
	int stillRunning;	// running requests -- just communicating using curl
	time_t currentTime;	// current time

	CurlResourceInfo *resourceInfo;
	std::vector<CurlResourceInfo*> resourceInfoHeap;

        log4cxx::LoggerPtr logger;
} CurlInfo;


class Fetcher : public Module {
public:
	Fetcher(ObjectRegistry *objects, const char *id, int threadIndex);
	~Fetcher();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	int ProcessMulti(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources);
	int ProcessingResources();

	CurlInfo curlInfo;
	void QueueResource(WebResource *wr);
	void StartQueuedResourcesFetcher();
	void StartResourceFetcher(WebResource *wr, int index);
	void FinishResourceFetcher(CurlResourceInfo *ri, int result);

private:
	int items;		// ObjectLock, items processed
	int minServerRelax;	// ObjectLock, time between access to the same IP address
	int timeout;		// ObjectLock, download timeout
	char *from;		// initOnly, From: header field
	char *userAgent;	// initOnly, User-Agent: header field
	int maxRequests;	// initOnly, number of concurrent requests
	long maxContentLength;	// ObjectLock, maximum length of the content to download
	int timeTick;		// ObjectLock, max time to spend in ProcessMulti()

	ObjectValues<Fetcher> *values;

	std::queue<Resource*> *outputResources;

	char *getItems(const char *name);
	char *getMinServerRelax(const char *name);
	void setMinServerRelax(const char *name, const char *value);
	char *getTimeout(const char *name);
	void setTimeout(const char *name, const char *value);
	char *getFrom(const char *name);
	void setFrom(const char *name, const char *value);
	char *getUserAgent(const char *name);
	void setUserAgent(const char *name, const char *value);
	char *getMaxRequests(const char *name);
	void setMaxRequests(const char *name, const char *value);
	char *getMaxContentLength(const char *name);
	void setMaxContentLength(const char *name, const char *value);
	char *getTimeTick(const char *name);
	void setTimeTick(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
};

inline Module::Type Fetcher::getType() {
	return MULTI;
}

inline char *Fetcher::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool Fetcher::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool Fetcher::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *Fetcher::listNamesSync() {
	return values->listNamesSync();
}

#endif
