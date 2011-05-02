/**
Fetcher.la, multi, native
Fetch an object (html page) using HTTP, uses CURL in non-blocking mode and libev.
Should not drop resources, because resource path may be locked. Sets X-Status
value with HTTP Status-Line.

Dependencies: libcurl, libev

Parameters:
items			r/o	Total items processed
minServerRelax		r/w	Time between access to the same IP address
timeout			r/w	Download timeout
from			init	From: header field
userAgent		init	User-Agent: header field
maxRequests		init	Number of concurrent requests
maxContentLength	r/w	Maximum length of the content to download
timeTick		r/w	Max time to spend in ProcessMulti()
allowedContentTypes	r/w	List of mime-types that are allowed to be downloaded

Status:
0	OK
1	(maybe) temporary error (DNS error, connection refused, not found and )
2	permanent error (e.g. not allowed content type, invalid size, ...)
*/

#ifndef _MODULES_FETCH_H_
#define _MODULES_FETCH_H_

#include <config.h>

#include <queue>
#include <string>
#include <ev.h>
#include <curl/curl.h>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "PageResource.h"

typedef struct CurlResourceInfo_ {
	int index;		// index in the resourceInfo table
	PageResource *current;	// currently processed Resource
	std::string *content;	// content to be filled (pointer to current PageResource)
	long contentLength;	// content size of current object (copy of Content-Length, if known)
	long maxContentLength;	// copy from Fetcher object
	bool contentIsText;	// content is text (text/html or text/plain)?
	std::deque<PageResource*> waiting;	// waiting resource in the current hash bucket
	CURL *easy;		// CURL easy handle (to be reused)
	curl_socket_t socketfd;	// socket used by CURL
	struct curl_slist *headers; // headers to be set
	struct ev_io event;	// io event associated with the socket
	bool evSet;		// whether event was set or not
	uint32_t time;		// request start time (for running request) or
				// next request allowed time (for waiting resources)
	struct CurlInfo_ *info; // parent
} CurlResourceInfo;


struct CurlResourceInfo_compare {
	bool operator()(CurlResourceInfo const*a, CurlResourceInfo const *b) const {
		return a->time > b->time;
	}
};

typedef struct CurlInfo_ {
	class Fetcher *parent;

	struct ev_loop *loop;
	struct ev_timer curlTimer;
	struct ev_timer tickTimer;

	CURLM *multi;
	int resources;		// number of resources currently in queue/processed
	int waiting;		// number of resources waiting
	int stillRunning;	// running requests -- just communicating using curl
	uint32_t currentTime;	// current time

	CurlResourceInfo *resourceInfo;
	std::vector<CurlResourceInfo*> waitingHeap;

        log4cxx::LoggerPtr logger;
} CurlInfo;


class Fetcher : public Module {
public:
	Fetcher(ObjectRegistry *objects, const char *id, int threadIndex);
	~Fetcher();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	int ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources);

	CurlInfo curlInfo;
	void QueueResource(PageResource *pr);
	void StartQueuedResourcesFetch();
	void StartResourceFetch(PageResource *pr, int index);
	void FinishResourceFetch(CurlResourceInfo *ri, int result);
	bool CheckContentType(std::string *contentType);

private:
	int items;		// ObjectLock, items processed
	int minServerRelax;	// ObjectLock, time between access to the same IP address
	int timeout;		// ObjectLock, download timeout
	char *from;		// initOnly, From: header field
	char *userAgent;	// initOnly, User-Agent: header field
	int maxRequests;	// initOnly, number of concurrent requests
	long maxContentLength;	// ObjectLock, maximum length of the content to download
	int timeTick;		// ObjectLock, max time to spend in ProcessMulti()
	std::vector<std::string> allowedContentTypes;	// ObjectLock, list of mime-types that are allowed to be downloaded

	char *GetItems(const char *name);
	char *GetMinServerRelax(const char *name);
	void SetMinServerRelax(const char *name, const char *value);
	char *GetTimeout(const char *name);
	void SetTimeout(const char *name, const char *value);
	char *GetFrom(const char *name);
	void SetFrom(const char *name, const char *value);
	char *GetUserAgent(const char *name);
	void SetUserAgent(const char *name, const char *value);
	char *GetMaxRequests(const char *name);
	void SetMaxRequests(const char *name, const char *value);
	char *GetMaxContentLength(const char *name);
	void SetMaxContentLength(const char *name, const char *value);
	char *GetTimeTick(const char *name);
	void SetTimeTick(const char *name, const char *value);
	char *GetAllowedContentTypes(const char *name);
	void SetAllowedContentTypes(const char *name, const char *value);

	ObjectProperties<Fetcher> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::queue<Resource*> *outputResources;
};

inline Module::Type Fetcher::GetType() {
	return MULTI;
}

inline char *Fetcher::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool Fetcher::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *Fetcher::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
