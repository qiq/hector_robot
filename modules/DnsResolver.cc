/**
 * DnsResolver module.
 */
#include <config.h>

#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include "DnsResolver.h"
#include "ProcessingEngine.h"
#include "TestResource.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

DnsResolver::DnsResolver(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	repeat = 3;
	timeout = 10;
	maxRequests = 1000;
	timeTick = DEFAULT_TIME_TICK;

	values = new ObjectValues<DnsResolver>(this);
	values->addGetter("items", &DnsResolver::getItems);
	values->addGetter("repeat", &DnsResolver::getRepeat);
	values->addSetter("repeat", &DnsResolver::setRepeat, true);
	values->addGetter("timeout", &DnsResolver::getTimeout);
	values->addSetter("timeout", &DnsResolver::setTimeout);
	values->addGetter("maxRequests", &DnsResolver::getMaxRequests);
	values->addSetter("maxRequests", &DnsResolver::setMaxRequests, true);
	values->addGetter("timeTick", &DnsResolver::getTimeTick);
	values->addSetter("timeTick", &DnsResolver::setTimeTick);
}

DnsResolver::~DnsResolver() {
	delete values;
}

char *DnsResolver::getItems(const char *name) {
	return int2str(items);
}

char *DnsResolver::getRepeat(const char *name) {
	return int2str(repeat);
}

void DnsResolver::setRepeat(const char *name, const char *value) {
	repeat = str2int(value);
}

char *DnsResolver::getTimeout(const char *name) {
	return int2str(timeout);
}

void DnsResolver::setTimeout(const char *name, const char *value) {
	timeout = str2int(value);
}

char *DnsResolver::getMaxRequests(const char *name) {
	return int2str(maxRequests);
}

void DnsResolver::setMaxRequests(const char *name, const char *value) {
	maxRequests = str2int(value);
}

char *DnsResolver::getTimeTick(const char *name) {
	return int2str(timeTick);
}

void DnsResolver::setTimeTick(const char *name, const char *value) {
	timeTick = str2long(value);
}

// called by libunbound when resource address is resolved
void CompletedCallback(void *data, int err, struct ub_result *result) {
	DnsResourceInfo *ri = (DnsResourceInfo*)data;
	if (err == 0) {
		if (result->havedata) {
			ip4_addr_t addr;
			addr.addr = ntohl(((struct in_addr*)result->data[0])->s_addr);
			ri->current->setIp4Addr(addr);
			ri->current->setStatus(0);
		} else {
			if (result->nxdomain) {
				LOG4CXX_DEBUG(ri->logger, "NXDOMAIN: " << ri->current->getUrlHost());
				ri->current->setIp4Addr(ip4_addr_empty);
				ri->current->setStatus(1);
			} else {
				LOG4CXX_ERROR(ri->logger, "TODO: process timeout and other errors " << result->rcode);
				// FIXME: process timeout somehow
				ri->current->setIp4Addr(ip4_addr_empty);
				ri->current->setStatus(1);
			}
		}
		ub_resolve_free(result);
	} else {
		LOG4CXX_DEBUG(ri->logger, "Resolve error: " << ub_strerror(err));
        }
	ri->parent->FinishResolution(ri);
}

void DnsResolver::StartResolution(WebResource *wr) {
	DnsResourceInfo *ri = unused.back();
	unused.pop_back();
	ri->current = wr;
	ri->retryCount = 0;
	int result = ub_resolve_async(ctx, (char*)wr->getUrlHost().c_str(), 1, 1, (void*)ri, CompletedCallback, &ri->id);
	if (result != 0) {
		LOG_ERROR("Cannot start asynchronous DNS lookup: " << result);
		return;
	}
	running[ri->id] = ri;
}

void DnsResolver::FinishResolution(DnsResourceInfo *ri) {
	outputResources->push(ri->current);
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	running.erase(ri->id);
	unused.push_back(ri);
}

bool DnsResolver::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;

	if (maxRequests <= 0) {
		LOG_ERROR("Invalid maxRequests value: " << maxRequests);
		return false;
	}

	ctx = ub_ctx_create();
	if (!ctx) {
		LOG_ERROR("Could not create unbound context");
		return false;
	}

	fd = ub_fd(ctx);
	if (fd == -1) {
		LOG_ERROR("Could not get unbound ready descriptor");
		return false;
	}

	for (int i = 0; i < maxRequests; i++) {
		DnsResourceInfo *ri = new DnsResourceInfo();
		ri->parent = this;
		ri->logger = logger;
		unused.push_back(ri);
	}
	return true;
}

int DnsResolver::ProcessMulti(queue<Resource*> *inputResources, queue<Resource*> *outputResources) {
	this->outputResources = outputResources;
	// get input resources and start resolution for them
	while (inputResources->size() > 0 && running.size() < maxRequests) {
		if (inputResources->front()->getTypeId() != WebResource::typeId) {
			outputResources->push(inputResources->front());
		} else {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			StartResolution(wr);
		}
		inputResources->pop();
	}

	if (running.size() == 0)
		return maxRequests;

	struct timeval startTime;
	gettimeofday(&startTime, NULL);
	ObjectLockRead();
	int timeoutFull = timeTick;
	ObjectUnlock();
	int timeout = timeoutFull;
	struct timeval currentTime;
	while (timeout > 0) {
		// wait for results
	        struct timeval tv;
	        tv.tv_sec = timeout / 1000000;
	        tv.tv_usec = timeout % 1000000;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		int retval = select(1, &rfds, NULL, NULL, &tv);
		if (retval < 0) {
	                LOG_ERROR("Error in select() = " << errno);
	                return maxRequests-running.size();
	        } else if (FD_ISSET(fd, &rfds)) {
			// process finished resources
			int retval = ub_process(ctx);
			if (retval != 0) {
				LOG_ERROR("Resolve error: " << ub_strerror(retval));
				return maxRequests-running.size();
			}
		}
		gettimeofday(&currentTime, NULL);
		timeout = timeoutFull * 1000000 - (currentTime.tv_sec - startTime.tv_sec) * 1000000 + (currentTime.tv_usec - startTime.tv_usec);
	}

	// finished resources are already appended to the outputResources queue
	return maxRequests-running.size();
}

int DnsResolver::ProcessingResources() {
	return running.size();
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new DnsResolver(objects, id, threadIndex);
}
