/**
 * WebSiteManager: takes care of all WebSiteResource items. It is able to call
 * two other processing engines: DNS resolver and robots.txt fetch + parse in
 * order to fill WebSiteResource.
 */

#ifndef _MODULES_WEB_SITE_MANAGER_H_
#define _MODULES_WEB_SITE_MANAGER_H_

#include <config.h>

#include <tr1/unordered_map>
#include <unbound.h>
#include "CallProcessingEngine.h"
#include "common.h"
#include "MemoryPool.h"
#include "Module.h"
#include "ObjectValues.h"
#include "WebResource.h"
#include "WebSiteResource.h"

class CallDns : public CallProcessingEngine {
public:
	CallDns(ProcessingEngine *engine, int maxRequests);
	~CallDns() {};

protected:	
	Resource *PrepareResource(Resource *src);
	Resource *FinishResource(Resource *tmp);

private:
	// WebResource cache
	std::vector<WebResource*> unused;
};

class CallRobots : public CallProcessingEngine {
public:
	CallRobots(ProcessingEngine *engine, int maxRequests);
	~CallRobots() {};

protected:	
	Resource *PrepareResource(Resource *src);
	Resource *FinishResource(Resource *tmp);

private:
	// WebResource cache
	std::vector<WebResource*> unused;
};

class WebSiteManager : public Module {
public:
	WebSiteManager(ObjectRegistry *objects, const char *id, int threadIndex);
	~WebSiteManager();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type getType();
	int ProcessMulti(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources);
	int ProcessingResources();
	bool SaveCheckpointSync(const char *path);
	bool RestoreCheckpointSync(const char *path);

private:
	// properties
	int items;		// ObjectLock
	int maxRequests;	// ObjectLock, number of concurrent requests
	int timeTick;		// ObjectLock
	char *dnsEngine;	// read-only
	char *robotsEngine;	// read-only

	ObjectValues<WebSiteManager> *values;
	char *getItems(const char *name);
	char *getMaxRequests(const char *name);
	void setMaxRequests(const char *name, const char *value);
	char *getTimeTick(const char *name);
	void setTimeTick(const char *name, const char *value);
	char *getDnsEngine(const char *name);
	char *getRobotsEngine(const char *name);
	char *getSave(const char *name);
	void setSave(const char *name, const char *value);
	char *getLoad(const char *name);
	void setLoad(const char *name, const char *value);

	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();

	MemoryPool<WebSiteResource> *pool;
	std::tr1::unordered_map<WebSiteResource*, WebSiteResource*, WebSiteResource_hash> sites;

	CallDns *callDns;
	std::queue<Resource*> callDnsInput;
	std::queue<Resource*> callDnsOutput;

	CallRobots *callRobots;
	std::queue<Resource*> callRobotsInput;
	std::queue<Resource*> callRobotsOutput;

	// we do not process the same WSR more than once at the same time, so
	// we keep info about what WebSiteResources we are working on
	std::tr1::unordered_map<WebSiteResource*, WebResource*> processingResources;

	WebSiteResource *getWebSiteResource(WebResource *wr);
	void StartProcessing(WebResource *wr, WebSiteResource *wsr, bool robotsOnly);
	void FinishProcessing(WebSiteResource *wsr, queue<Resource*> *outputResources);
	bool LoadWebSiteResources(const char *filename);
	bool SaveWebSiteResources(const char *filename);
};

inline Module::Type WebSiteManager::getType() {
	return MULTI;
}

inline char *WebSiteManager::getValueSync(const char *name) {
	return values->getValueSync(name);
}

inline bool WebSiteManager::setValueSync(const char *name, const char *value) {
	return values->setValueSync(name, value);
}

inline bool WebSiteManager::isInitOnly(const char *name) {
	return values->isInitOnly(name);
}

inline std::vector<std::string> *WebSiteManager::listNamesSync() {
	return values->listNamesSync();
}

#endif
