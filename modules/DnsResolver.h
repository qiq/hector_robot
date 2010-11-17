/**
 * DnsResolver: translate DNS name to IP address
 */

#ifndef _MODULES_RESOLVE_DNS_H_
#define _MODULES_RESOLVE_DNS_H_

#include <config.h>

#include <tr1/unordered_map>
#include <unbound.h>
#include "common.h"
#include "Module.h"
#include "ObjectValues.h"
#include "WebResource.h"

class DnsResolver;

typedef struct DnsResourceInfo_ {
	int id;			// unbound request id
	WebResource *current;	// currently processed Resource
	int retryCount;		// how many we tried
	DnsResolver *parent;	// parent
        log4cxx::LoggerPtr logger;
} DnsResourceInfo;


class DnsResolver : public Module {
public:
	DnsResolver(ObjectRegistry *objects, const char *id, int threadIndex);
	~DnsResolver();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	int ProcessMulti(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources);
	int ProcessingResources();

	void StartResolution(WebResource *wr);
	void FinishResolution(DnsResourceInfo *ri);
private:
	int items;		// ObjectLock, items processed
	int repeat;		// ObjectLock, repeat request if timeout occurs
	int timeout;		// ObjectLock, resolution timeout
	int maxRequests;	// initOnly, number of concurrent requests
	int timeTick;		// ObjectLock, max time to spend in ProcessMulti()

	ObjectValues<DnsResolver> *values;

	struct ub_ctx* ctx;	// unbound context
	int fd;			// file descriptor we are waiting for read
	std::vector<DnsResourceInfo*> unused;
	std::tr1::unordered_map<int, DnsResourceInfo*> running;
	std::queue<Resource*> *outputResources;

	char *getItems(const char *name);
	char *getRepeat(const char *name);
	void setRepeat(const char *name, const char *value);
	char *getTimeout(const char *name);
	void setTimeout(const char *name, const char *value);
	char *getMaxRequests(const char *name);
	void setMaxRequests(const char *name, const char *value);
	char *getTimeTick(const char *name);
	void setTimeTick(const char *name, const char *value);

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
