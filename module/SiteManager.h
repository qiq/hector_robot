/**
SiteManager.la, multi, native
Read PageResources from the input until Marker is read (means end-of-input). It
uses Multi() interface, always accept resources from the input. When Marker is
read, SiteResources are read from the file. They are refreshed on-demand and
then PageResources are passed to the output with appropriate SiteResource
attached. One PageResource from every SiteResource is passed to the output
every time tick.

Dependencies: none

Parameters:
items			r/o	Total items processed
timeTick		r/w	Max time to spend in ProcessMulti()
dnsEngine		init	ProcessingEngine to call for DNS resolution
robotsEngine		init	ProcessingEngine to call for robots.txt refresh
robotsMaxRedirects	r/w	Max redirects (default is 5)
robotsNegativeTTL	r/w	Time to keep negative robots.txt result (too many redirects

*/

#ifndef _MODULES_SITE_MANAGER_H_
#define _MODULES_SITE_MANAGER_H_

#include <config.h>

#include <tr1/unordered_map>
extern "C" {
#include <unbound.h>
}
#include <fstream>
#include "CallProcessingEngine.h"
#include "common.h"
#include "MemoryPool.h"
#include "Module.h"
#include "ObjectProperties.h"
#include "PageResource.h"
#include "SitePathMD5.h"
#include "SiteResource.h"

class CallDns : public CallProcessingEngine {
public:
	CallDns(int maxRequests);
	~CallDns() {};

protected:	
	Resource *PrepareResource(Resource *src);
	Resource *FinishResource(Resource *tmp);

private:
	// PageResource cache
	std::vector<PageResource*> unused;
};

class CallRobots : public CallProcessingEngine {
public:
	CallRobots(int maxRequests);
	~CallRobots() {};

protected:	
	Resource *PrepareResource(Resource *src);
	Resource *FinishResource(Resource *tmp);

private:
	int pageResourceTypeId;	// PageResource typeId
};

class SiteResources {
public:
	SiteResources(bool robotsOnly = false);
	~SiteResources();
	bool IsProcessing();
	void SetProcessing(bool processing);
	void SetSiteResource(SiteResource *site);
	SiteResource *GetSiteResource();
	std::vector<PageResource*> *GetPageResources();
	void AppendPageResource(PageResource *page);
	std::vector<SiteResource*> *GetWaitingSites();
	void AppendWaitingSite(SiteResource *sr);
	void SetReadyPrev(SiteResources *srs);
	SiteResources *GetReadyPrev();
	void SetReadyNext(SiteResources *srs);
	SiteResources *GetReadyNext();
private:
	bool processing;
	bool robotsOnly;
	SiteResource *site;
	// pages that are to be downloaded from the site
	std::vector<PageResource*> *pages;
	// site resources whose robots.txt are redirected to the current one
	std::vector<SiteResource*> *waitingSites;

	// double-linked ready list
	SiteResources *readyPrev;
	SiteResources *readyNext;
};

SiteResources::SiteResources(bool robotsOnly) {
	processing = false;
	this->robotsOnly = robotsOnly;
	site = NULL;
	pages = new std::vector<PageResource*>();
	waitingSites = new std::vector<SiteResource*>();
	readyPrev = NULL;
	readyNext = NULL;
}

SiteResources::~SiteResources() {
	Resource::GetRegistry()->ReleaseResource(site);
	for (std::vector<PageResource*>::iterator iter = pages->begin(); iter != pages->end(); ++iter)
		Resource::GetRegistry()->ReleaseResource(*iter);
	delete pages;
	for (std::vector<SiteResource*>::iterator iter = waitingSites->begin(); iter != waitingSites->end(); ++iter)
		Resource::GetRegistry()->ReleaseResource(*iter);
}

bool SiteResources::IsProcessing() {
	return processing;
}

void SiteResources::SetProcessing(bool processing) {
	this->processing = processing;
}

void SiteResources::SetSiteResource(SiteResource *site) {
	delete this->site;
	this->site = site;
}

SiteResource *SiteResources::GetSiteResource() {
	return site;
}

std::vector<PageResource*> *SiteResources::GetPageResources() {
	return pages;
}

void SiteResources::AppendPageResource(PageResource *page) {
	pages->push_back(page);
}

std::vector<SiteResource*> *SiteResources::GetWaitingSites() {
	return waitingSites;
}

void SiteResources::SetReadyPrev(SiteResources *srs) {
	readyPrev = srs;
}

SiteResources *SiteResources::GetReadyPrev() {
	return readyPrev;
}

void SiteResources::SetReadyNext(SiteResources *srs) {
	readyNext = srs;
}

SiteResources *SiteResources::GetReadyNext() {
	return readyNext;
}

class SiteManager : public Module {
public:
	SiteManager(ObjectRegistry *objects, const char *id, int threadIndex);
	~SiteManager();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	bool ProcessMultiSync(std::queue<Resource*> *inputResources, std::queue<Resource*> *outputResources, int *expectingResources, int *processingResources);
	bool SaveCheckpointSync(const char *path);
	bool RestoreCheckpointSync(const char *path);

private:
	// properties
	int items;
	int maxRequests;
	int timeTick;
	char *dnsEngine;
	char *robotsEngine;
	int robotsMaxRedirects;
	int robotsNegativeTTL;
	int siteResources;

	char *GetItems(const char *name);
	char *GetMaxRequests(const char *name);
	void SetMaxRequests(const char *name, const char *value);
	char *GetTimeTick(const char *name);
	void SetTimeTick(const char *name, const char *value);
	char *GetDnsEngine(const char *name);
	void SetDnsEngine(const char *name, const char *value);
	char *GetRobotsEngine(const char *name);
	void SetRobotsEngine(const char *name, const char *value);
	char *GetRobotsMaxRedirects(const char *name);
	void SetRobotsMaxRedirects(const char *name, const char *value);
	char *GetRobotsNegativeTTL(const char *name);
	void SetRobotsNegativeTTL(const char *name, const char *value);
	char *GetSiteResources(const char *name);
	void SetSiteResources(const char *name, const char *value);

	ObjectProperties<SiteManager> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	// 0: reading PageResources
	// 1: all PageResources read, reading SiteResources (and copying them to the output, if not used)
	// 2: all SiteResources read, processing PageResources
	// 3: all PageResources processed, writing changed SiteResources
	int status;

	// entire site resources file read
	bool siteResourcesRead;

	// all site resources were written to the file
	bool siteResourcesWritten;
	std::vector<SiteResource*> writeResources;

	// resources that we have already downloaded
	std::tr1::unordered_set<SitePathMD5, SitePathMD5_hash, SitePathMD5_equal> seen;

	// prage resources prepared to be dealt with
	std::tr1::unordered_map<uint64_t, SiteResources*> resources;

	// finished PageResources waiting to be pushed into the output queue
	// It is a doble-linked list
	SiteResources *ready;

	// we did not read SiteResource for these -- we create them 
	std::vector<SiteResources*> newSiteResources;

	// time when ProcessMulti started
	uint32_t currentTime;

	// DNS processing chain
	CallDns *callDns;
	std::queue<Resource*> callDnsInput;
	std::queue<Resource*> callDnsOutput;

	// robots processing chain
	CallRobots *callRobots;
	std::queue<Resource*> callRobotsInput;
	std::queue<Resource*> callRobotsOutput;

	// resources being processed (for DNS and robots.txt)
	int runningRequests;

	// items waiting to be finished
	int waitingResourcesCount;

	// cached type of SiteResource
	int siteResourceTypeId;

	void CopyRobotsInfo(SiteResource *src, SiteResource *dst);
	void StartProcessing(SiteResources *srs, bool robotsOnly);
	bool IsRedirectCycle(SiteResource *current, SiteResource *sr);
	void FinishProcessing(SiteResource *sr);
	void MarkSiteResourceReady(SiteResources *srs);
	void UnmarkSiteResourceReady(SiteResources *srs);
};

inline Module::Type SiteManager::GetType() {
	return MULTI;
}

inline char *SiteManager::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool SiteManager::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *SiteManager::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
