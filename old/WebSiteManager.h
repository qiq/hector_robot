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
#include "ObjectProperties.h"
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

	int webResourceTypeId;	// WebResource typeId
};

class WebSiteManager : public Module {
public:
	WebSiteManager(ObjectRegistry *objects, const char *id, int threadIndex);
	~WebSiteManager();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	int ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources);
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

	char *GetItems(const char *name);
	char *GetMaxRequests(const char *name);
	void SetMaxRequests(const char *name, const char *value);
	char *GetTimeTick(const char *name);
	void SetTimeTick(const char *name, const char *value);
	char *GetDnsEngine(const char *name);
	void SetDnsEngine(const char *name, const char *value);
	char *GetRobotsEngine(const char *name);
	void SetRobotsEngine(const char *name, const char *value);
	char *GetLoad(const char *name);
	void SetLoad(const char *name, const char *value);
	char *GetSave(const char *name);
	void SetSave(const char *name, const char *value);
	char *GetRobotsMaxRedirects(const char *name);
	void SetRobotsMaxRedirects(const char *name, const char *value);
	char *GetRobotsNegativeTTL(const char *name);
	void SetRobotsNegativeTTL(const char *name, const char *value);

	ObjectProperties<WebSiteManager> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

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

	WebSiteResource *GetWebSiteResource(WebResource *wr);
	void CopyRobotsInfo(WebSiteResource *src, WebSiteResource *dst);
	void StartProcessing(Resource *r, WebSiteResource *wsr, bool robotsOnly);
	bool IsRedirectCycle(WebSiteResource *current, WebSiteResource *wsr);
	void FinishProcessing(WebSiteResource *wsr, queue<Resource*> *outputResources);
	bool LoadWebSiteResources(const char *filename);
	bool SaveWebSiteResources(const char *filename);
};

inline Module::Type WebSiteManager::GetType() {
	return MULTI;
}

inline char *WebSiteManager::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool WebSiteManager::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *WebSiteManager::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
