/**
 * ResolveDns module.
 */
#include <config.h>

#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
extern "C" {
#include <ldns/wire2host.h>
}
#include "ResolveDns.h"
#include "ProcessingEngine.h"
#include "PageResource.h"
#include "SiteResource.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

ResolveDns::ResolveDns(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxRequests = 1000;
	timeTick = DEFAULT_TIME_TICK;
	forwardServer = NULL;
	forwardPort = 0;
	negativeTTL = 86400;

	props = new ObjectProperties<ResolveDns>(this);
	props->Add("items", &ResolveDns::GetItems);
	props->Add("maxRequests", &ResolveDns::GetMaxRequests, &ResolveDns::SetMaxRequests, true);
	props->Add("timeTick", &ResolveDns::GetTimeTick, &ResolveDns::SetTimeTick);
	props->Add("forwardServer", &ResolveDns::GetForwardServer, &ResolveDns::SetForwardServer);
	props->Add("forwardPort", &ResolveDns::GetForwardPort, &ResolveDns::SetForwardPort);
	props->Add("negativeTTL", &ResolveDns::GetNegativeTTL, &ResolveDns::SetNegativeTTL);
}

ResolveDns::~ResolveDns() {
	assert(running.size() == 0);
	free(forwardServer);
	delete props;
	ub_ctx_delete(ctx);
	for (vector<DnsResourceInfo*>::iterator iter = unused.begin(); iter != unused.end(); ++iter)
		delete *iter;
}

char *ResolveDns::GetItems(const char *name) {
	return int2str(items);
}

char *ResolveDns::GetMaxRequests(const char *name) {
	return int2str(maxRequests);
}

void ResolveDns::SetMaxRequests(const char *name, const char *value) {
	maxRequests = str2int(value);
}

char *ResolveDns::GetTimeTick(const char *name) {
	return int2str(timeTick);
}

void ResolveDns::SetTimeTick(const char *name, const char *value) {
	timeTick = str2long(value);
}

char *ResolveDns::GetForwardServer(const char *name) {
	return forwardServer ? strdup(forwardServer) : NULL;
}

void ResolveDns::SetForwardServer(const char *name, const char *value) {
	free(forwardServer);
	forwardServer = strdup(value);
}

char *ResolveDns::GetForwardPort(const char *name) {
	return int2str(forwardPort);
}

void ResolveDns::SetForwardPort(const char *name, const char *value) {
	forwardPort = str2long(value);
}

char *ResolveDns::GetNegativeTTL(const char *name) {
	return int2str(negativeTTL);
}

void ResolveDns::SetNegativeTTL(const char *name, const char *value) {
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
			if (PageResource::IsInstance(ri->current)) {
				PageResource *pr = static_cast<PageResource*>(ri->current);
				host = pr->GetUrlHost().c_str();
			} else {
				SiteResource *sr = static_cast<SiteResource*>(ri->current);
				host = sr->GetUrlHost().c_str();
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

void ResolveDns::StartResolution(Resource *resource) {
	const char *host = NULL;
	if (PageResource::IsInstance(resource)) {
		PageResource *pr = static_cast<PageResource*>(resource);
		host = pr->GetUrlHost().c_str();
	} else if (SiteResource::IsInstance(resource)) {
		SiteResource *sr = static_cast<SiteResource*>(resource);
		host = sr->GetUrlHost().c_str();
	} else {
		LOG_ERROR_R(this, resource, "Unknown resource type: " << resource->GetTypeString());
		return;
	}

	if (host[0] >= '0' && host[0] <= '9') {
		IpAddr addr;
		if (addr.ParseIp4Addr(host)) {
			UpdateResource(resource, 0, &addr, numeric_limits<int>::max());
			return;
		}
	} else if (host[0] == '[') {
		IpAddr addr;
		if (addr.ParseIp6Addr(host+1)) {
			UpdateResource(resource, 0, &addr, numeric_limits<int>::max());
			return;
		}
	}

	DnsResourceInfo *ri = unused.back();
	unused.pop_back();
	ri->current = resource;
	int result = ub_resolve_async(ctx, (char*)host, 1, 1, (void*)ri, &CompletedCallback, &ri->id);
	if (result != 0) {
		LOG_ERROR_R(this, resource, "Cannot start asynchronous DNS lookup: " << result);
		unused.push_back(ri);
		return;
	}
	running[ri->id] = ri;
}

void ResolveDns::FinishResolution(DnsResourceInfo *ri, int status, uint32_t ip4, uint32_t ipAddrExpire) {
	if (status == 1) {
		ip4 = 0;
		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		ipAddrExpire = currentTime.tv_sec + negativeTTL;
	}
	IpAddr addr;
	addr.SetIp4Addr(ip4);
	UpdateResource(ri->current, status, &addr, ipAddrExpire);
	running.erase(ri->id);
	unused.push_back(ri);
}

void ResolveDns::UpdateResource(Resource *resource, int status, IpAddr *addr, uint32_t ipAddrExpire) {
	resource->SetStatus(status);
	const char *host = NULL;
	if (PageResource::IsInstance(resource)) {
		PageResource *pr = static_cast<PageResource*>(resource);
		pr->SetIpAddr(*addr);
		// PageResource has no ipAddrExpire
		host = pr->GetUrlHost().c_str();
	} else {
		SiteResource *sr = static_cast<SiteResource*>(resource);
		sr->SetIpAddr(*addr);
		sr->SetIpAddrExpire(ipAddrExpire);
		host = sr->GetUrlHost().c_str();
	}
	LOG_DEBUG_R(this, resource, "DNS " << host << ": " << addr->ToString());
	outputResources->push(resource);
	items++;
}

bool ResolveDns::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
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

	// do threaded async resolution
	if (ub_ctx_async(ctx, 1) < 0) {
		LOG_ERROR(this, "Could not set threaded async resolution");
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

bool ResolveDns::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources, int *processingResources) {
	this->outputResources = outputResources;
	// get input resources and start resolution for them
	while (inputResources->size() > 0 && (int)running.size() < maxRequests) {
		Resource *r = inputResources->front();
		if (!SiteResource::IsInstance(r) && !PageResource::IsInstance(r)) {
			outputResources->push(inputResources->front());
		} else {
			StartResolution(r);
		}
		inputResources->pop();
	}

	if (running.size() == 0) {
		if (expectingResources)
			*expectingResources = maxRequests;
		if (processingResources)
			*processingResources = 0;
		return false;
	}

	struct timeval startTime;
	gettimeofday(&startTime, NULL);
	int timeout = timeTick;
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
			if (processingResources)
				*processingResources = running.size();
			return running.size() > 0;
	        } else if (FD_ISSET(fd, &rfds)) {
			// process finished resources
			int retval = ub_process(ctx);
			if (retval != 0) {
				LOG_ERROR(this, "Resolve error: " << ub_strerror(retval));
				if (expectingResources)
					*expectingResources = maxRequests-running.size();
				if (processingResources)
					*processingResources = running.size();
				return running.size() > 0;
			}
		}
		gettimeofday(&currentTime, NULL);
		timeout = timeTick - (currentTime.tv_sec - startTime.tv_sec) * 1000000 + (currentTime.tv_usec - startTime.tv_usec);
	}

	// finished resources are already appended to the outputResources queue
	if (expectingResources)
		*expectingResources = maxRequests-running.size();
	if (processingResources)
		*processingResources = running.size();
	return running.size() > 0;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new ResolveDns(objects, id, threadIndex);
}
