/**
 * DnsResolver module.
 */
#include <config.h>

#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
extern "C" {
#include <ldns/wire2host.h>
}
#include "DnsResolver.h"
#include "ProcessingEngine.h"
#include "WebResource.h"
#include "WebSiteResource.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

DnsResolver::DnsResolver(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxRequests = 1000;
	timeTick = DEFAULT_TIME_TICK;
	forwardServer = NULL;
	forwardPort = 0;

	values = new ObjectValues<DnsResolver>(this);
	values->addGetter("items", &DnsResolver::getItems);
	values->addGetter("maxRequests", &DnsResolver::getMaxRequests);
	values->addSetter("maxRequests", &DnsResolver::setMaxRequests, true);
	values->addGetter("timeTick", &DnsResolver::getTimeTick);
	values->addSetter("timeTick", &DnsResolver::setTimeTick);
	values->addGetter("forwardServer", &DnsResolver::getForwardServer);
	values->addSetter("forwardServer", &DnsResolver::setForwardServer);
	values->addGetter("forwardPort", &DnsResolver::getForwardPort);
	values->addSetter("forwardPort", &DnsResolver::setForwardPort);
	values->addGetter("negativeTTL", &DnsResolver::getNegativeTTL);
	values->addSetter("negativeTTL", &DnsResolver::setNegativeTTL);
}

DnsResolver::~DnsResolver() {
	assert(running.size() == 0);
	delete values;
	ub_ctx_delete(ctx);
	for (vector<DnsResourceInfo*>::iterator iter = unused.begin(); iter != unused.end(); ++iter)
		delete *iter;
}

char *DnsResolver::getItems(const char *name) {
	return int2str(items);
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

char *DnsResolver::getForwardServer(const char *name) {
	return forwardServer ? strdup(forwardServer) : NULL;
}

void DnsResolver::setForwardServer(const char *name, const char *value) {
	free(forwardServer);
	forwardServer = strdup(value);
}

char *DnsResolver::getForwardPort(const char *name) {
	return int2str(forwardPort);
}

void DnsResolver::setForwardPort(const char *name, const char *value) {
	forwardPort = str2long(value);
}

char *DnsResolver::getNegativeTTL(const char *name) {
	return int2str(negativeTTL);
}

void DnsResolver::setNegativeTTL(const char *name, const char *value) {
	negativeTTL = str2int(value);
}

// called by libunbound when resource address is resolved
void CompletedCallback(void *data, int error, struct ub_result *result) {
	DnsResourceInfo *ri = (DnsResourceInfo*)data;
	int status = 1;
	uint32_t ip4 = 0;
	uint32_t ipAddrExpire = 0;
	if (error == 0) {
		if (result->havedata && result->len[0] == 4) {
			ip4 = ((struct in_addr*)result->data[0])->s_addr;
			// we want TTL, so that we have to parse the packet again (ugh!)
			ldns_pkt *pkt;
			if (ldns_wire2pkt(&pkt, (const uint8_t *)result->answer_packet, result->answer_len) == LDNS_STATUS_OK) {
				ldns_rr_list *answer = pkt->_answer;
				if (answer->_rr_count >= 1) {
					struct timeval currentTime;
					gettimeofday(&currentTime, NULL);
					status = 0;
					ipAddrExpire = currentTime.tv_sec + answer->_rrs[0]->_ttl;
				} else {
					LOG_INFO_R(ri->parent, ri->current, "Error parsing packet data.");
				}
				ldns_pkt_free(pkt);
			} else {
				LOG_INFO_R(ri->parent, ri->current, "Error parsing packet data.");
			}
		} else {
			const char *host;
			if (ri->current->getTypeId() == WebResource::typeId) {
				WebResource *wr = static_cast<WebResource*>(ri->current);
				host = wr->getUrlHost().c_str();
			} else {
				WebSiteResource *wsr = static_cast<WebSiteResource*>(ri->current);
				host = wsr->getUrlHost().c_str();
			}
			if (result->rcode == 0) {
				LOG_DEBUG_R(ri->parent, ri->current, "No IP address " << host);
			} else if (result->nxdomain) {
				LOG_DEBUG_R(ri->parent, ri->current, "NXDOMAIN " << host);
			} else {
				LOG_INFO_R(ri->parent, ri->current, "Query failed " << host << ": " << ub_strerror(result->rcode));
			}
		}
		ub_resolve_free(result);
	} else {
		LOG_INFO_R(ri->parent, ri->current, "Resolve error (" << ub_strerror(error) << ")");
	}

	ri->parent->FinishResolution(ri, status, ip4, ipAddrExpire);
}

void DnsResolver::StartResolution(Resource *resource) {
	DnsResourceInfo *ri = unused.back();
	unused.pop_back();
	ri->current = resource;
	const char *host = NULL;
	if (resource->getTypeId() == WebResource::typeId) {
		WebResource *wr = static_cast<WebResource*>(resource);
		host = wr->getUrlHost().c_str();
	} else {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(resource);
		host = wsr->getUrlHost().c_str();
	}
	int result = ub_resolve_async(ctx, (char*)host, 1, 1, (void*)ri, &CompletedCallback, &ri->id);
	if (result != 0) {
		LOG_ERROR(this, "Cannot start asynchronous DNS lookup: " << result);
		return;
	}
	running[ri->id] = ri;
}

void DnsResolver::FinishResolution(DnsResourceInfo *ri, int status, uint32_t ip4, uint32_t ipAddrExpire) {
	if (status == 1) {
		ip4 = 0;
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		ObjectLockRead();
		ipAddrExpire = currentTime.tv_sec + negativeTTL;
		ObjectUnlock();
	}
	ri->current->setStatus(status);
	IpAddr ip;
	ip.setIp4Addr(ip4);
	if (ri->current->getTypeId() == WebResource::typeId) {
		WebResource *wr = static_cast<WebResource*>(ri->current);
		wr->setIpAddr(ip);
		// WebResource has no ipAddrExpire
	} else {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(ri->current);
		wsr->setIpAddrExpire(ip, ipAddrExpire);
	}
	outputResources->push(ri->current);
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	running.erase(ri->id);
	unused.push_back(ri);
}

bool DnsResolver::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;

	if (maxRequests <= 0) {
		LOG_ERROR(this, "Invalid maxRequests value: " << maxRequests);
		return false;
	}

	ctx = ub_ctx_create();
	if (!ctx) {
		LOG_ERROR(this, "Could not create unbound context");
		return false;
	}

	fd = ub_fd(ctx);
	if (fd == -1) {
		LOG_ERROR(this, "Could not get unbound ready descriptor");
		return false;
	}

	if (forwardServer) {
		int retval;
		if (forwardPort) {
			char buffer[1024];
			snprintf(buffer, sizeof(buffer), "%s@%d", forwardServer, forwardPort);
			retval = ub_ctx_set_fwd(ctx, buffer);
		} else {
			retval = ub_ctx_set_fwd(ctx, forwardServer);
		}
		if (retval < 0) {
			LOG_ERROR(this, "Could not set forwarder DNS server (" << forwardServer << "): " <<  ub_strerror(retval));
			return false;
		}
	}

	for (int i = 0; i < maxRequests; i++) {
		DnsResourceInfo *ri = new DnsResourceInfo();
		ri->parent = this;
		unused.push_back(ri);
	}
	return true;
}

int DnsResolver::ProcessMulti(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources) {
	this->outputResources = outputResources;
	// get input resources and start resolution for them
	while (inputResources->size() > 0 && (int)running.size() < maxRequests) {
		Resource *r = inputResources->front();
		if (r->getTypeId() != WebSiteResource::typeId && r->getTypeId() != WebResource::typeId) {
			outputResources->push(inputResources->front());
		} else {
			StartResolution(r);
		}
		inputResources->pop();
	}

	if (running.size() == 0) {
		if (expectingResources)
			*expectingResources = maxRequests;
		return 0;
	}

	struct timeval startTime;
	gettimeofday(&startTime, NULL);
	ObjectLockRead();
	int timeoutFull = timeTick;
	ObjectUnlock();
	int timeout = timeoutFull;
	struct timeval currentTime;
	while (running.size() > 0 && timeout > 0) {
		// wait for results
	        struct timeval tv;
	        tv.tv_sec = timeout / 1000000;
	        tv.tv_usec = timeout % 1000000;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		int retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if (retval < 0) {
	                LOG_ERROR(this, "Error in select() = " << errno);
			if (expectingResources)
				*expectingResources = maxRequests-running.size();
			return running.size();
	        } else if (FD_ISSET(fd, &rfds)) {
			// process finished resources
			int retval = ub_process(ctx);
			if (retval != 0) {
				LOG_ERROR(this, "Resolve error: " << ub_strerror(retval));
				if (expectingResources)
					*expectingResources = maxRequests-running.size();
				return running.size();
			}
		}
		gettimeofday(&currentTime, NULL);
		timeout = timeoutFull - (currentTime.tv_sec - startTime.tv_sec) * 1000000 + (currentTime.tv_usec - startTime.tv_usec);
	}

	// finished resources are already appended to the outputResources queue
	if (expectingResources)
		*expectingResources = maxRequests-running.size();
	return running.size();
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new DnsResolver(objects, id, threadIndex);
}
