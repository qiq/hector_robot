/**
WebSiteManager.la, multi, native
Takes care of all WebSiteResource items: keeps them in memory, loads and saves
them. It may call two other processing engines: DNS resolver and robots.txt
fetcher + parser. These two are used to fill WebSiteResource.

Dependencies: none

Parameters:
items			r/o	Total items processed
maxRequests		init	Number of concurrent requests
timeTick		r/w	Max time to spend in ProcessMulti()
dnsEngine		init	ProcessingEngine to call for DNS resolution
robotsEngine		init	ProcessingEngine to call for robots.txt refresh
load			r/w	Save WebSiteResources to the file
save			r/w	Load WebSiteResources from the file
robotsMaxRedirects	r/w	Max redirects (default is 5)
robotsNegativeTTL	r/w	Time to keep negative robots.txt result (too many redirects

Status:
- input:
0 (?):	Plain input WR, WSR should be attached, DNS and robots should be
	filled correctly in WSR. Also used for outside incoming new-link WRs
	(TODO).
2:	Attach WSR, but do not perform DNS and robots resolution. Used for
	new-link WRs.
- output:
0:	WSR attached, DNS and robots filled
1:	Error in DNS or robots resolution, currently not used
2:	WSR attached, no DNS and robots.txt resolution performed
//3:	Output new-link to the supervisor (TODO)
*/

#ifndef _MODULES_WEB_SITE_MANAGER_H_
#define _MODULES_WEB_SITE_MANAGER_H_

#include <config.h>

#include <tr1/unordered_map>
extern "C" {
#include <unbound.h>
}
#include "CallProcessingEngine.h"
#include "common.h"
#include "MemoryPool.h"
#include "Module.h"
#include "ObjectValues.h"
#include "WebResource.h"
#include "WebSiteResource.h"

class CallDns : public CallProcessingEngine {
public:
	CallDns(int maxRequests);
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
	CallRobots(int maxRequests);
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
	int ProcessMulti(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources);
	bool SaveCheckpointSync(const char *path);
	bool RestoreCheckpointSync(const char *path);

private:
	// properties
	int items;		// ObjectLock, ro
	int maxRequests;	// initOnly, number of concurrent requests
	int timeTick;		// ObjectLock
	char *dnsEngine;	// read-only
	char *robotsEngine;	// read-only
	int robotsMaxRedirects;	// ObjectLock
	int robotsNegativeTTL;	// ObjectLock

	char *getItems(const char *name);
	char *getMaxRequests(const char *name);
	void setMaxRequests(const char *name, const char *value);
	char *getTimeTick(const char *name);
	void setTimeTick(const char *name, const char *value);
	char *getDnsEngine(const char *name);
	void setDnsEngine(const char *name, const char *value);
	char *getRobotsEngine(const char *name);
	void setRobotsEngine(const char *name, const char *value);
	char *getLoad(const char *name);
	void setLoad(const char *name, const char *value);
	char *getSave(const char *name);
	void setSave(const char *name, const char *value);
	char *getRobotsMaxRedirects(const char *name);
	void setRobotsMaxRedirects(const char *name, const char *value);
	char *getRobotsNegativeTTL(const char *name);
	void setRobotsNegativeTTL(const char *name, const char *value);

	ObjectValues<WebSiteManager> *values;
	char *getValueSync(const char *name);
	bool setValueSync(const char *name, const char *value);
	bool isInitOnly(const char *name);
	std::vector<std::string> *listNamesSync();

	uint32_t currentTime;
	MemoryPool<WebSiteResource, true> *pool;
	std::tr1::unordered_map<WebSiteResource*, WebSiteResource*, WebSiteResource_hash, WebSiteResource_equal> sites;
	WebSiteResource key;

	CallDns *callDns;
	std::queue<Resource*> callDnsInput;
	std::queue<Resource*> callDnsOutput;

	CallRobots *callRobots;
	std::queue<Resource*> callRobotsInput;
	std::queue<Resource*> callRobotsOutput;

	// we do not process the same WSR more than once at the same time, so
	// we keep info about what WebSiteResources we are working on. There
	// may be WebResources (common case) and WebSiteResources
	// (redirect-sources) waiting.
	std::tr1::unordered_map<WebSiteResource*, std::vector<Resource*>* > waitingResources;
	int waitingResourcesCount;

	WebSiteResource *getWebSiteResource(WebResource *wr);
	void CopyRobotsInfo(WebSiteResource *src, WebSiteResource *dst);
	void StartProcessing(Resource *r, WebSiteResource *wsr, bool robotsOnly);
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
