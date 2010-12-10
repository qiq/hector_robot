/**
 * Download module.
 * We allow only maxRequests to be processed at a time: every IP address is
 * hashed into a bucket [0, maxRequests-1], only one active resource is
 * allowed for a bucket.
 *
 * So far we do not implement auth, cookies, ssl, or robots.txt checking in any way.
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include <ev.h>
#include "Fetcher.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

Fetcher::Fetcher(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	minServerRelax = 60;
	timeout = 10;
	from = NULL;
	userAgent = strdup("Mozilla/5.0 (compatible; hector_robot/Fetcher 1.0; +http://host/)");
	maxRequests = 5;
	maxContentLength = 1024*1024;
	timeTick = DEFAULT_TIME_TICK;

	values = new ObjectValues<Fetcher>(this);
	values->addGetter("items", &Fetcher::getItems);
	values->addGetter("minServerRelax", &Fetcher::getMinServerRelax);
	values->addSetter("minServerRelax", &Fetcher::setMinServerRelax);
	values->addGetter("timeout", &Fetcher::getTimeout);
	values->addSetter("timeout", &Fetcher::setTimeout);
	values->addGetter("from", &Fetcher::getFrom);
	values->addSetter("from", &Fetcher::setFrom, true);
	values->addGetter("userAgent", &Fetcher::getUserAgent);
	values->addSetter("userAgent", &Fetcher::setUserAgent, true);
	values->addGetter("maxRequests", &Fetcher::getMaxRequests);
	values->addSetter("maxRequests", &Fetcher::setMaxRequests, true);
	values->addGetter("maxContentLength", &Fetcher::getMaxContentLength);
	values->addSetter("maxContentLength", &Fetcher::setMaxContentLength);
	values->addGetter("timeTick", &Fetcher::getTimeTick);
	values->addSetter("timeTick", &Fetcher::setTimeTick);

	curlInfo.logger = this->logger;
}

Fetcher::~Fetcher() {
	for (int i = 0; i < maxRequests; i++) {
		CurlResourceInfo *ri = &curlInfo.resourceInfo[i];
		delete ri->current;
		for (deque<WebResource*>::iterator iter = ri->waiting.begin(); iter != ri->waiting.end(); ++iter)
			delete *iter;
		curl_easy_cleanup(ri->easy);
		if (ri->headers)
			curl_slist_free_all(ri->headers);
	}
	curl_multi_cleanup(curlInfo.multi);
	ev_loop_destroy(curlInfo.loop);
	free(userAgent);
	free(from);

	delete values;
}

char *Fetcher::getItems(const char *name) {
	return int2str(items);
}

char *Fetcher::getMinServerRelax(const char *name) {
	return int2str(minServerRelax);
}

void Fetcher::setMinServerRelax(const char *name, const char *value) {
	minServerRelax = str2int(value);
}

char *Fetcher::getTimeout(const char *name) {
	return int2str(timeout);
}

void Fetcher::setTimeout(const char *name, const char *value) {
	timeout = str2int(value);
}

char *Fetcher::getFrom(const char *name) {
	return from ? strdup(from) : NULL;
}

void Fetcher::setFrom(const char *name, const char *value) {
	free(from);
	from = strdup(value);
}

char *Fetcher::getUserAgent(const char *name) {
	return userAgent ? strdup(userAgent) : NULL;
}

void Fetcher::setUserAgent(const char *name, const char *value) {
	free(userAgent);
	userAgent = strdup(value);
}

char *Fetcher::getMaxRequests(const char *name) {
	return int2str(maxRequests);
}

void Fetcher::setMaxRequests(const char *name, const char *value) {
	maxRequests = str2int(value);
}

char *Fetcher::getMaxContentLength(const char *name) {
	return int2str(maxContentLength);
}

void Fetcher::setMaxContentLength(const char *name, const char *value) {
	maxContentLength = str2long(value);
}

char *Fetcher::getTimeTick(const char *name) {
	return int2str(timeTick);
}

void Fetcher::setTimeTick(const char *name, const char *value) {
	timeTick = str2long(value);
}

void CheckCompleted(CurlInfo *ci);

// called by libev when the timeTick timeout expires
void TimeTickCallback(EV_P_ struct ev_timer *t, int revents) {
	CurlInfo *ci = (CurlInfo*)t->data;

	ev_unloop(ci->loop, EVUNLOOP_ALL);
}

// called by libev when something happens with some Curl socket
void SocketCallback(EV_P_ struct ev_io *w, int revents) {
	CurlInfo *ci = (CurlInfo*) w->data;

	int what = (revents & EV_READ ? CURL_POLL_IN : 0) | (revents & EV_WRITE ? CURL_POLL_OUT : 0);
	CURLMcode rc = curl_multi_socket_action(ci->multi, w->fd, what, &ci->stillRunning);
	if (rc != CURLM_OK) {
		LOG4CXX_ERROR(ci->logger, "SocketCallback: Curl error: " << rc);
		return;
	}
	CheckCompleted(ci);
	if (ci->stillRunning <= 0)
		ev_timer_stop(ci->loop, &ci->curlTimer);
}

// called by libev when the Curl timeout expires
void TimerCallback(EV_P_ struct ev_timer *t, int revents) {
	CurlInfo *ci = (CurlInfo *)t->data;

	CURLMcode rc = curl_multi_socket_action(ci->multi, CURL_SOCKET_TIMEOUT, 0, &ci->stillRunning);
	if (rc != CURLM_OK) {
		LOG4CXX_ERROR(ci->logger, "TimerCallback: Curl error: " << rc);
		return;
	}
	CheckCompleted(ci);
}

// check for completed transfers, and remove their easy handles */
void CheckCompleted(CurlInfo *ci) {
	int msgs_left;
	CURLMsg *msg;
	while ((msg = curl_multi_info_read(ci->multi, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			CURL *easy = msg->easy_handle;
			int result = (int)msg->data.result;
			CurlResourceInfo *ri;
			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &ri);
			ci->parent->FinishResourceFetch(ri, result);
			curl_multi_remove_handle(ci->multi, easy);
		}
	}
}

// change event processing of the socket
void WatchSocket(CurlResourceInfo *ri, curl_socket_t s, CURL *easy, int what, CurlInfo *ci) {
	what = (what & CURL_POLL_IN ? EV_READ : 0) | (what & CURL_POLL_OUT ? EV_WRITE : 0);
	if (ri->evSet)
		ev_io_stop(ci->loop, &ri->event);
	ev_io_init(&ri->event, SocketCallback, s, what);
	ri->evSet = true;
	ri->event.data = ci;
	ev_io_start(ci->loop, &ri->event);
}

// called when socket state changes (CURLMOPT_SOCKETFUNCTION)
int MultiSocketCallback(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp) {
	CurlInfo *ci = (CurlInfo*)userp;
	CurlResourceInfo *ri = (CurlResourceInfo*)socketp;

	if (what == CURL_POLL_REMOVE) {
    		if (ri->evSet)
			ev_io_stop(ci->loop, &ri->event);
	} else {
		if (!ri) {
			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &ri);
			WatchSocket(ri, s, easy, what, ci);
			curl_multi_assign(ci->multi, s, ri);
    		} else {
			WatchSocket(ri, s, easy, what, ci);
		}
	}
	return 0;
}

// update the event timer after curl_multi library calls (CURLMOPT_TIMERFUNCTION)
int MultiTimerCallback(CURLM *multi, long timeout_ms, CurlInfo *ci) {
	ev_timer_stop(ci->loop, &ci->curlTimer);
	if (timeout_ms > 0) {
		double t = timeout_ms / 1000;
		ev_timer_init(&ci->curlTimer, TimerCallback, t, 0.);
		ev_timer_start(ci->loop, &ci->curlTimer);
	} else {
		TimerCallback(ci->loop, &ci->curlTimer, 0);
	}
	return 0;
}

// called when Curl has data for us (CURLOPT_WRITEFUNCTION)
size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data) {
	size_t realsize = size * nmemb;
	CurlResourceInfo *ri = (CurlResourceInfo*)data;

	if (ri->content->length() == 0) {
		if (ri->contentIsText) {
			if (ri->contentLength > 0)
				ri->content->reserve(ri->contentLength < ri->maxContentLength ? ri->contentLength : ri->maxContentLength);
		} else {
			// non-text object which is too large: we are not interested in
			if (ri->contentLength > ri->maxContentLength)
				return 0;
		}
	}
	// text object is too large? Trim!
	if (ri->content->length() + realsize > ri->maxContentLength)
		realsize -= (ri->content->length() + realsize - ri->maxContentLength);

	// really append data
	ri->content->append((char*)ptr, realsize);
	return realsize;
}

// called web Curl has some headers (CURLOPT_HEADERFUNCTION)
size_t HeaderCallback(void *ptr, size_t size, size_t nmemb, void *data) {
	size_t realsize = size * nmemb;
	CurlResourceInfo *ri = (CurlResourceInfo*)data;

	// end of headers
	if (realsize == 0)
		return realsize;

	string s((char *)ptr, realsize);
	chomp(&s);
	// end of headers
	if (s.length() == 0)
		return realsize;
	size_t pos = s.find_first_of(":");
	if (pos == string::npos) {
		// status line?
		if (!s.compare(0, 4, "HTTP")) {
			ri->current->setHeaderValue("X-Status", s.c_str());
		} else {
			LOG_DEBUG_R(ri->info->parent, ri->current, "Invalid header field: " << s);
		}
	} else {
		// real header
		string name = s.substr(0, pos);
		int i = 1;
		while (s.at(pos+i) == ' ')
			i++;
		s.erase(0, pos+i);
		if (name != "X-Status")		// we do not want malicious server to overwrite the status :)
			ri->current->setHeaderValue(name.c_str(), s.c_str());
		if (name == "Content-Size") {
			ri->contentLength = atol(s.c_str());
		} else if (name == "Content-Type") {
			if (!s.compare(0, 9, "text/html") || !s.compare(0, 10, "text/plain"))
				ri->contentIsText = true;
		}
	}
	return realsize;
}

void Fetcher::QueueResource(WebResource *wr) {
	// count hash of the IP address
	IpAddr &ip = wr->getIpAddr();
	uint32_t ip_sum;
	if (ip.isIp4Addr()) {
		ip_sum = ip.getIp4Addr();
	} else {
		uint64_t ip6 = ip.getIp6Addr(true);
		ip_sum = ip6 & 0xFFFFFFFF + (ip6 >> 32) & 0xFFFFFFFF;
		ip6 = ip.getIp6Addr(false);
		ip_sum += ip6 & 0xFFFFFFFF + (ip6 >> 32) & 0xFFFFFFFF;
	}
	ObjectLockRead();
	int hash = ip_sum % maxRequests;
	int wait = minServerRelax;
	ObjectUnlock();
	CurlResourceInfo *ri = &curlInfo.resourceInfo[hash];

	// busy: just append to the bucket
	curlInfo.resources++;
	if (ri->current || curlInfo.currentTime < ri->time + wait) {
		ri->waiting.push_back(wr);
		if (!ri->current) {	// just waiting for timeout (not currently being processed)
			curlInfo.resourceInfoHeap.push_back(ri);
			push_heap(curlInfo.resourceInfoHeap.begin(), curlInfo.resourceInfoHeap.end());
		}
		return;
	}
	StartResourceFetch(wr, hash);
}

// check resources in the heap and start fetch of resources that waited long enough
void Fetcher::StartQueuedResourcesFetch() {
	ObjectLockRead();
	int wait = minServerRelax;
	ObjectUnlock();
	while (curlInfo.resourceInfoHeap.size() > 0 && curlInfo.currentTime >= curlInfo.resourceInfoHeap[0]->time+wait) {
		CurlResourceInfo *ri = curlInfo.resourceInfoHeap[0];
		pop_heap(curlInfo.resourceInfoHeap.begin(), curlInfo.resourceInfoHeap.end());
		curlInfo.resourceInfoHeap.pop_back();
		WebResource *wr = ri->waiting.front();
		int index = ri->index;
		ri->waiting.pop_front();
		StartResourceFetch(wr, index);
	}
}

// really start download
void Fetcher::StartResourceFetch(WebResource *wr, int index) {
	// set URL
	const string &url = wr->getUrl();
	if (url.empty()) {
		LOG_ERROR_R(this, wr, "No url found");
		outputResources->push(wr);
		return;
	}
	wr->clearHeaderFields();
	CurlResourceInfo *ri = &curlInfo.resourceInfo[index];
	ri->current = wr;
	ri->content = wr->getContentMutable();
	ri->content->clear();
	curl_easy_setopt(ri->easy, CURLOPT_URL, url.c_str());

	// set IP4/6 address
	IpAddr &ip = wr->getIpAddr();
	struct sockaddr_storage addr;
	if (ip.isIp4Addr()) {
		addr.ss_family = AF_INET;
		((struct sockaddr_in*)&addr)->sin_addr.s_addr = ip.getIp4Addr();
	} else {
		addr.ss_family = AF_INET6;
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6*)&addr;
		*((uint64_t*)addr6->sin6_addr.s6_addr) = ip.getIp6Addr(true);
		*(((uint64_t*)addr6->sin6_addr.s6_addr)+1) = ip.getIp6Addr(false);
	}
	curl_easy_setopt(ri->easy, CURLOPT_DNS_IP_ADDR, &addr);
	ri->contentLength = 0;
	ObjectLockRead();
	ri->maxContentLength = maxContentLength;
	ObjectUnlock();
	ri->contentIsText = false;

	// start!
	CURLMcode rc = curl_multi_add_handle(curlInfo.multi, ri->easy);
	if (rc != CURLM_OK)
		LOG_ERROR(this, "Error adding easy handle to multi: " << rc);
}

// save resource to the outputQueue, process errors, etc.
void Fetcher::FinishResourceFetch(CurlResourceInfo *ri, int result) {
	ri->current->setStatus(result);
	outputResources->push(ri->current);
	ObjectLockWrite();
	++items;
	ObjectUnlock();

	// update heap info
	ri->current = NULL;
	ri->content = NULL; // to be safe :)
	ri->time = curlInfo.currentTime;
	curlInfo.resources--;

	// add resource to the heap again
	if (ri->waiting.size() > 0) {
		curlInfo.resourceInfoHeap.push_back(ri);
		push_heap(curlInfo.resourceInfoHeap.begin(), curlInfo.resourceInfoHeap.end());
	}
}

bool Fetcher::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;

	if (maxRequests <= 0) {
		LOG_ERROR(this, "Invalid maxRequests value: " << maxRequests);
		return false;
	}

	curlInfo.parent = this;
	curlInfo.loop = ev_loop_new(0);
	ev_timer_init(&curlInfo.curlTimer, TimerCallback, 0., 0.);
	curlInfo.curlTimer.data = &curlInfo;
	curlInfo.multi = curl_multi_init();
	curl_multi_setopt(curlInfo.multi, CURLMOPT_SOCKETFUNCTION, MultiSocketCallback);
	curl_multi_setopt(curlInfo.multi, CURLMOPT_SOCKETDATA, &curlInfo);
	curl_multi_setopt(curlInfo.multi, CURLMOPT_TIMERFUNCTION, MultiTimerCallback);
	curl_multi_setopt(curlInfo.multi, CURLMOPT_TIMERDATA, &curlInfo);
	curlInfo.resources = 0;
	curlInfo.stillRunning = 0;

	curlInfo.resourceInfoHeap.reserve(maxRequests);
	curlInfo.resourceInfo = new CurlResourceInfo[maxRequests];
	for (int i = 0; i < maxRequests; i++) {
		CurlResourceInfo *ri = &curlInfo.resourceInfo[i];
		ri->index = i;
		ri->current = NULL;
		ri->easy = curl_easy_init();
		if (!ri->easy) {
			LOG_ERROR(this, "Cannot initialize easy Curl handle");
			return false;
		}
		curl_easy_setopt(ri->easy, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(ri->easy, CURLOPT_WRITEDATA, ri);
		curl_easy_setopt(ri->easy, CURLOPT_HEADERFUNCTION, HeaderCallback);
		curl_easy_setopt(ri->easy, CURLOPT_HEADERDATA, ri);
		curl_easy_setopt(ri->easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTP|CURLPROTO_HTTPS);
		curl_easy_setopt(ri->easy, CURLOPT_FOLLOWLOCATION, 0);
		ri->headers = NULL;
		if (from && strcmp(from, "")) {
			char buffer[1024];
			snprintf(buffer, sizeof(buffer), "From: %s", from);
			ri->headers = curl_slist_append(ri->headers, "From:");
			curl_easy_setopt(ri->easy, CURLOPT_HTTPHEADER, ri->headers);
		}
		if (userAgent && strcmp(userAgent, ""))
			curl_easy_setopt(ri->easy, CURLOPT_USERAGENT, userAgent);
		ObjectLockRead();
		long t = timeout;
		ObjectUnlock();
		curl_easy_setopt(ri->easy, CURLOPT_PRIVATE, ri);
		curl_easy_setopt(ri->easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(ri->easy, CURLOPT_LOW_SPEED_TIME, t);
		curl_easy_setopt(ri->easy, CURLOPT_LOW_SPEED_LIMIT, 100L);

		ri->socketfd = -1;
		ri->evSet = false;
		ri->time = 0;
		ri->info = &curlInfo;

		ri->contentLength = 0;
		ri->contentIsText = false;
	}
	return true;
}

int Fetcher::ProcessMulti(queue<Resource*> *inputResources, queue<Resource*> *outputResources) {
	curlInfo.currentTime = time(NULL);
	this->outputResources = outputResources;

	// start queued resources
	StartQueuedResourcesFetch();

	// queue/start input resources
	while (inputResources->size() > 0 && curlInfo.resources < maxRequests) {
		if (inputResources->front()->getTypeId() != WebResource::typeId) {
			outputResources->push(inputResources->front());
		} else {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			IpAddr &ip = wr->getIpAddr();
			if (!ip.isEmpty()) {
				QueueResource(wr);
			} else {
				wr->setStatus(1);
				outputResources->push(wr);
			}
		}
		inputResources->pop();
	}

	if (curlInfo.resources == 0)
		return maxRequests;

	// set timer for the timerTick
	ObjectLockRead();
	int tick = timeTick;
	ObjectUnlock();
	ev_timer_init(&curlInfo.tickTimer, TimeTickCallback, tick/1000000, 0.);
	ev_timer_start(curlInfo.loop, &curlInfo.tickTimer);
	curlInfo.tickTimer.data = &curlInfo;

	// run loop: wait for socket actions or timeout
	ev_loop(curlInfo.loop, 0);

	// finished resources are already appended to the outputResources queue
	return maxRequests-curlInfo.resources;
}

int Fetcher::ProcessingResources() {
	return curlInfo.resources;
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Fetcher(objects, id, threadIndex);
}
