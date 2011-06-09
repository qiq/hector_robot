/**
DnsResolver.la, multi, native
Translate DNS name to IP address, supports PageResource and WebSiteResource.

Dependencies: libunbound

Parameters:
items		r/o	Total items processed
maxRequests	init	Number of concurrent requests
timeTick	r/w	Max time to spend in ProcessMulti()
forwardServer	init	DNS server to use
forwardPort	init	Port number of the DNS server
negativeTTL	r/w	Number of seconds to keep info about DNS failure/NXdomain
*/

#ifndef _MODULES_RESOLVE_DNS_H_
#define _MODULES_RESOLVE_DNS_H_

#include <config.h>

#include <tr1/unordered_map>
extern "C" {
#include <unbound.h>
}
#include "common.h"
#include "IpAddr.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "Resource.h"

class DnsResolver;

typedef struct DnsResourceInfo_ {
	int id;			// unbound request id
	Resource *current;	// currently processed Resource
	DnsResolver *parent;	// parent
} DnsResourceInfo;


class DnsResolver : public Module {
public:
	DnsResolver(ObjectRegistry *objects, const char *id, int threadIndex);
	~DnsResolver();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	bool ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources, int *processingResources);

	void StartResolution(Resource *reesource);
	void FinishResolution(DnsResourceInfo *ri, int status, uint32_t ip4, uint32_t ipAddrExpire);
private:
	int items;		// ObjectLock, items processed
	int maxRequests;	// initOnly, number of concurrent requests
	int timeTick;		// ObjectLock, max time to spend in ProcessMulti()
	char *forwardServer;	// initOnly, DNS server to use
	int forwardPort;	// initOnly, port number of the DNS server
	int negativeTTL;	// ObjectLock, number of seconds to keep info about DNS failure/NXdomain

	char *GetItems(const char *name);
	char *GetMaxRequests(const char *name);
	void SetMaxRequests(const char *name, const char *value);
	char *GetTimeTick(const char *name);
	void SetTimeTick(const char *name, const char *value);
	char *GetForwardServer(const char *name);
	void SetForwardServer(const char *name, const char *value);
	char *GetForwardPort(const char *name);
	void SetForwardPort(const char *name, const char *value);
	char *GetNegativeTTL(const char *name);
	void SetNegativeTTL(const char *name, const char *value);

	ObjectProperties<DnsResolver> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	struct ub_ctx* ctx;	// unbound context
	int fd;			// file descriptor we are waiting for read
	std::vector<DnsResourceInfo*> unused;
	std::tr1::unordered_map<int, DnsResourceInfo*> running;
	std::queue<Resource*> *outputResources;
	uint32_t currentTime;	// time of ProcessMulti() call

	void UpdateResource(Resource *resource, int status, IpAddr *addr, uint32_t ipAddrExpire);
};

inline Module::Type DnsResolver::GetType() {
	return MULTI;
}

inline char *DnsResolver::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool DnsResolver::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *DnsResolver::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
