/**
 * DnsResolver: translate DNS name to IP address
 * Supports WebResource and WebSiteResource
 */

#ifndef _MODULES_RESOLVE_DNS_H_
#define _MODULES_RESOLVE_DNS_H_

#include <config.h>

#include <tr1/unordered_map>
extern "C" {
#include <unbound.h>
}
#include "common.h"
#include "Module.h"
#include "ObjectValues.h"
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
	Module::Type getType();
	int ProcessMulti(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources);
	int ProcessingResources();

	void StartResolution(Resource *reesource);
	void FinishResolution(DnsResourceInfo *ri, int status, uint32_t ip4, uint32_t ipAddrExpire);
private:
	int items;		// ObjectLock, items processed
	int maxRequests;	// initOnly, number of concurrent requests
	int timeTick;		// ObjectLock, max time to spend in ProcessMulti()
	char *forwardServer;	// initOnly, DNS server to use
	int forwardPort;	// initOnly, port number of the DNS server
	int negativeTTL;	// ObjectLock, number of seconds to keep info about DNS failure/NXdomain

	ObjectValues<DnsResolver> *values;

	struct ub_ctx* ctx;	// unbound context
	int fd;			// file descriptor we are waiting for read
	std::vector<DnsResourceInfo*> unused;
	std::tr1::unordered_map<int, DnsResourceInfo*> running;
	std::queue<Resource*> *outputResources;
	uint32_t currentTime;	// time of ProcessMulti() call

	char *getItems(const char *name);
	char *getMaxRequests(const char *name);
	void setMaxRequests(const char *name, const char *value);
	char *getTimeTick(const char *name);
	void setTimeTick(const char *name, const char *value);
	char *getForwardServer(const char *name);
	void setForwardServer(const char *name, const char *value);
	char *getForwardPort(const char *name);
	void setForwardPort(const char *name, const char *value);
	char *getNegativeTTL(const char *name);
	void setNegativeTTL(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();
};

inline Module::Type DnsResolver::getType() {
	return MULTI;
}

inline char *DnsResolver::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool DnsResolver::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool DnsResolver::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *DnsResolver::listNamesSync() {
	return values->listNamesSync();
}

#endif
