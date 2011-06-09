/**
 * SiteManager module.
 */
#include <config.h>

#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "MarkerResource.h"
#include "SiteManager.h"
#include "ResourceInputStreamBinary.h"
#include "ResourceInputStreamText.h"
#include "ResourceOutputStreamBinary.h"
#include "ResourceOutputStreamText.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

CallDns::CallDns(int maxRequests) : CallProcessingEngine(maxRequests, true) {
}

Resource *CallDns::PrepareResource(Resource *src) {
	assert(SiteResource::IsInstance(src));
	SiteResource *sr = static_cast<SiteResource*>(src);
	LOG4CXX_TRACE(logger, sr->ToStringShort() << " " << "DNS start: " << sr->GetUrlHost());
	return src;
}

Resource *CallDns::FinishResource(Resource *tmp) {
	assert(SiteResource::IsInstance(tmp));
	SiteResource *sr = static_cast<SiteResource*>(tmp);
	LOG4CXX_TRACE(logger, sr->ToStringShort() << " " << "DNS finish: " << sr->GetUrlHost());
	return tmp;
}

CallRobots::CallRobots(int maxRequests) : CallProcessingEngine(maxRequests, true) {
	pageResourceTypeId = Resource::GetRegistry()->NameToId("PageResource");
}

Resource *CallRobots::PrepareResource(Resource *src) {
	SiteResource *sr = static_cast<SiteResource*>(src);
	PageResource *pr = static_cast<PageResource*>(Resource::GetRegistry()->AcquireResource(pageResourceTypeId));
	pr->SetUrlScheme(sr->GetUrlScheme());
	pr->SetUrlHost(sr->GetUrlHost());
	pr->SetUrlPort(sr->GetUrlPort());
	pr->SetUrlPath("/robots.txt");
	IpAddr ip = sr->GetIpAddr();
	pr->SetIpAddr(ip);
	pr->SetAttachedResource(sr);
	LOG4CXX_TRACE(logger, sr->ToStringShort() << " " << "Robots start (" << pr->ToStringShort() << "): " << sr->GetUrlHost());
	return pr;
}

Resource *CallRobots::FinishResource(Resource *tmp) {
	PageResource *pr = static_cast<PageResource*>(tmp);
	SiteResource *sr = static_cast<SiteResource*>(tmp->GetAttachedResource());
	pr->ClearAttachedResource();
	int status = pr->GetStatus();
	// redirect: set first allow url to the redirected value
	if (status == 2) {
		vector<string> v;
		v.push_back(pr->GetUrl());
		sr->ClearAllowUrls();
		sr->SetAllowUrls(v);
	}
	Resource::GetRegistry()->ReleaseResource(pr);
	sr->SetStatus(status);
	LOG4CXX_TRACE(logger, sr->ToStringShort() << " " << "Robots finish (" << tmp->ToStringShort() << "): " << sr->GetUrlHost() << " (" << status << ")");
	return sr;
}

SiteManager::SiteManager(ObjectRegistry *objects, const char *id, int threadIndex) : Module(objects, id, threadIndex) {
	items = 0;
	maxRequests = 1000;
	timeTick = DEFAULT_TIME_TICK;
	dnsEngine = NULL;
	robotsEngine = NULL;
	inputSiteResourceFilename = NULL;
	inputSiteResourceText = false;
	outputSiteResourceFilename = NULL;
	outputSiteResourceText = false;
	robotsMaxRedirects = 5;
	robotsNegativeTTL = 86400;
	siteResourcesRead = 0;

	props = new ObjectProperties<SiteManager>(this);
	props->Add("items", &SiteManager::GetItems);
	props->Add("maxRequests", &SiteManager::GetMaxRequests, &SiteManager::SetMaxRequests, true);
	props->Add("timeTick", &SiteManager::GetTimeTick, &SiteManager::SetTimeTick);
	props->Add("dnsEngine", &SiteManager::GetDnsEngine, &SiteManager::SetDnsEngine);
	props->Add("robotsEngine", &SiteManager::GetRobotsEngine, &SiteManager::SetRobotsEngine);
	props->Add("inputSiteResourceFilename", &SiteManager::GetInputSiteResourceFilename, &SiteManager::SetInputSiteResourceFilename, true);
	props->Add("inputSiteResourceText", &SiteManager::GetInputSiteResourceText, &SiteManager::SetInputSiteResourceText, true);
	props->Add("outputSiteResourceFilename", &SiteManager::GetOutputSiteResourceFilename, &SiteManager::SetOutputSiteResourceFilename, true);
	props->Add("outputSiteResourceText", &SiteManager::GetOutputSiteResourceText, &SiteManager::SetOutputSiteResourceText, true);
	props->Add("robotsMaxRedirects", &SiteManager::GetRobotsMaxRedirects, &SiteManager::SetRobotsMaxRedirects);
	props->Add("robotsNegativeTTL", &SiteManager::GetRobotsNegativeTTL, &SiteManager::SetRobotsNegativeTTL);
	props->Add("siteResources", &SiteManager::GetSiteResources);

	markerRead = false;
	siteResourcesRead = false;
	siteResourcesWritten = false;
	ready = NULL;

	runningRequests = 0;
	waitingResourcesCount = 0;
	ifd = -1;
	ifs = NULL;
	istream = NULL;
	ofd = -1;
	ofs = NULL;
	ostream = NULL;

	siteResourceTypeId = -1;
}

SiteManager::~SiteManager() {
	delete props;
	free(dnsEngine);
	free(robotsEngine);
	delete callDns;
	delete callRobots;

	for (tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.begin(); iter != resources.end(); ++iter)
		delete iter->second;

	delete istream;
	if (ifd >= 0) {
		flock(ifd, LOCK_UN);
		close(ifd);
	}
	delete ifs;

	delete ostream;
	if (ofd >= 0) {
		flock(ofd, LOCK_UN);
		close(ofd);
	}
	delete ofs;
}

char *SiteManager::GetItems(const char *name) {
	return int2str(items);
}

char *SiteManager::GetMaxRequests(const char *name) {
	return int2str(maxRequests);
}

void SiteManager::SetMaxRequests(const char *name, const char *value) {
	maxRequests = str2int(value);
}

char *SiteManager::GetTimeTick(const char *name) {
	return int2str(timeTick);
}

void SiteManager::SetTimeTick(const char *name, const char *value) {
	timeTick = str2int(value);
}

char *SiteManager::GetDnsEngine(const char *name) {
	return dnsEngine ? strdup(dnsEngine) : NULL;
}

void SiteManager::SetDnsEngine(const char *name, const char *value) {
	free(dnsEngine);
	dnsEngine = strdup(value);
}

char *SiteManager::GetRobotsEngine(const char *name) {
	return robotsEngine ? strdup(robotsEngine) : NULL;
}

void SiteManager::SetRobotsEngine(const char *name, const char *value) {
	free(robotsEngine);
	robotsEngine = strdup(value);
}

char *SiteManager::GetInputSiteResourceFilename(const char *name) {
	return strdup(inputSiteResourceFilename);
}

void SiteManager::SetInputSiteResourceFilename(const char *name, const char *value) {
	free(inputSiteResourceFilename);
	inputSiteResourceFilename = strdup(value);
}

char *SiteManager::GetInputSiteResourceText(const char *name) {
	return bool2str(inputSiteResourceText);
}

void SiteManager::SetInputSiteResourceText(const char *name, const char *value) {
	inputSiteResourceText = str2bool(value);
}

char *SiteManager::GetOutputSiteResourceFilename(const char *name) {
	return strdup(outputSiteResourceFilename);
}

void SiteManager::SetOutputSiteResourceFilename(const char *name, const char *value) {
	free(outputSiteResourceFilename);
	outputSiteResourceFilename = strdup(value);
}

char *SiteManager::GetOutputSiteResourceText(const char *name) {
	return bool2str(outputSiteResourceText);
}

void SiteManager::SetOutputSiteResourceText(const char *name, const char *value) {
	outputSiteResourceText = str2bool(value);
}

char *SiteManager::GetRobotsMaxRedirects(const char *name) {
	return int2str(robotsMaxRedirects);
}

void SiteManager::SetRobotsMaxRedirects(const char *name, const char *value) {
	robotsMaxRedirects = str2int(value);
}

char *SiteManager::GetRobotsNegativeTTL(const char *name) {
	return int2str(robotsNegativeTTL);
}

void SiteManager::SetRobotsNegativeTTL(const char *name, const char *value) {
	robotsNegativeTTL = str2int(value);
}

char *SiteManager::GetSiteResources(const char *name) {
	return int2str(siteResources);
}

bool SiteManager::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params) {
		ProcessingEngine *engine = dynamic_cast<ProcessingEngine*>(objects->GetObject(dnsEngine));
		if (!engine) {
			LOG_ERROR(this, "Invalid dnsEngine parameter: " << dnsEngine);
			return false;
		}
		callDns->SetProcessingEngine(engine);

		engine = dynamic_cast<ProcessingEngine*>(objects->GetObject(robotsEngine));
		if (!engine) {
			LOG_ERROR(this, "Invalid robotsEngine parameter: " << robotsEngine);
			return false;
		}
		callRobots->SetProcessingEngine(engine);
		return true;
	}

	if (!props->InitProperties(params))
		return false;

	if (!dnsEngine || strlen(dnsEngine) == 0) {
		LOG_ERROR(this, "dnsEngine not defined");
		return false;
	}
	callDns = new CallDns(maxRequests);

	if (!robotsEngine || strlen(robotsEngine) == 0) {
		LOG_ERROR(this, "robotsEngine not defined");
		return false;
	}
	callRobots = new CallRobots(maxRequests);

	// input SiteResource file
	if (!inputSiteResourceFilename || !strcmp(inputSiteResourceFilename, "")) {
		LOG_ERROR(this, "No inputSiteResourcesFilename specified");
		return false;
	}
	ifd = open(inputSiteResourceFilename, O_RDONLY);
	if (ifd < 0) {
		LOG_ERROR(this, "Cannot open file " << inputSiteResourceFilename << ": " << strerror(errno));
		return false;
	}
	if (flock(ifd, LOCK_SH) < 0) {
		LOG_ERROR(this, "Cannot lock file " << inputSiteResourceFilename << ": " << strerror(errno));
		return false;
	}
	istream = new ResourceInputStreamBinary(ifd);
	if (!inputSiteResourceText) {
		istream = new ResourceInputStreamBinary(ifd);
	} else {
		ifs = new ifstream();
		ifs->open(inputSiteResourceFilename, ifstream::in|ifstream::binary);
		if (ifs->fail()) {
			LOG_ERROR(this, "Cannot open input file " << inputSiteResourceFilename);
			return false;
		}
		istream = new ResourceInputStreamText(ifs);
	}

	// output SiteResource file
	if (!outputSiteResourceFilename || !strcmp(outputSiteResourceFilename, "")) {
		LOG_ERROR(this, "No outputSiteResourcesFilename specified");
		return false;
	}
	ofd = open(outputSiteResourceFilename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (ofd < 0) {
		LOG_ERROR(this, "Cannot open file " << outputSiteResourceFilename << ": " << strerror(errno));
		return false;
	}
	if (flock(ofd, LOCK_EX) < 0) {
		LOG_ERROR(this, "Cannot lock file " << outputSiteResourceFilename << ": " << strerror(errno));
		return false;
	}
	if (!outputSiteResourceText) {
		ostream = new ResourceOutputStreamBinary(ofd);
	} else {
		ofs = new ofstream();
		ofs->open(outputSiteResourceFilename, ofstream::out|ifstream::binary);
		if (ofs->fail()) {
			LOG_ERROR(this, "Cannot open output file " << outputSiteResourceFilename);
			return false;
		}
		ostream = new ResourceOutputStreamText(ofs);
	}

	siteResourceTypeId = Resource::GetRegistry()->NameToId("SiteResource");
	assert(siteResourceTypeId > 0);

	return true;
}

void SiteManager::MarkSiteResourceReady(SiteResources *srs) {
	assert(!srs->GetReadyNext() && !srs->GetReadyPrev());
	if (ready)
		ready->SetReadyPrev(srs);
	srs->SetReadyNext(ready);
	ready = srs;
}

void SiteManager::UnmarkSiteResourceReady(SiteResources *srs) {
	SiteResources *next = srs->GetReadyNext();
	SiteResources *prev = srs->GetReadyPrev();
	if (prev)
		prev->SetReadyNext(next);
	else
		ready = next;
	if (next)
		next->SetReadyPrev(prev);
	srs->SetReadyPrev(NULL);
	srs->SetReadyNext(NULL);
}

void SiteManager::CopyRobotsInfo(SiteResource *src, SiteResource *dst) {
	vector<string> allow;
	vector<string> disallow;
	uint32_t time;
	src->GetRobots(allow, disallow, time);
	dst->SetRobots(allow, disallow, time);
}

void SiteManager::StartProcessing(SiteResources *srs, bool robotsOnly) {
	assert(srs->GetSiteResource() != NULL);
	// SR already being dealt with?
	if (srs->IsProcessing())
		return;
	// SR is not yet being processed
	srs->SetProcessing(true);
	if (!robotsOnly)
		callDnsInput.push(srs->GetSiteResource());
	else
		callRobotsInput.push(srs->GetSiteResource());
}

bool SiteManager::IsRedirectCycle(SiteResource *current, SiteResource *sr) {
	tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.find(current->GetSiteMD5());
	if (iter == resources.end())
		return false;
	vector<SiteResource*> *srs = iter->second->GetWaitingSites();
	for (vector<SiteResource*>::iterator iter = srs->begin(); iter != srs->end(); ++iter) {
		if (SiteResource::IsInstance(*iter)) {
			SiteResource *prev = static_cast<SiteResource*>(*iter);
			if (prev == sr || IsRedirectCycle(prev, sr))
				return true;
		}
	}
	return false;
}

void SiteManager::FinishProcessing(SiteResource *sr) {
	tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.find(sr->GetSiteMD5());
	assert(iter != resources.end());
	SiteResources *srs = iter->second;
	srs->SetProcessing(false);
	if (sr->GetStatus() == 2) {
		// robots.txt was redirected, redirect target is allowed_urls[0]
		int redirects = sr->GetRobotsRedirectCount();
		vector<string> *v = sr->GetAllowUrls();
		assert(v->size() == 1);
		string url = v->front();
		delete v;
		if (redirects < robotsMaxRedirects) {
			PageResource pr;
			pr.SetUrl(url);
			tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.find(pr.GetSiteMD5());
			SiteResource *next;
			if (iter == resources.end()) {
				// we don't know target robots.txt SiteResource, so we create one,
				// it will be used exclusively for robots.txt
				SiteResources *srs = new SiteResources(true);
				next = static_cast<SiteResource*>(Resource::GetRegistry()->AcquireResource(siteResourceTypeId));
				next->SetSiteMD5(pr.GetSiteMD5());
				next->SetUrlScheme(pr.GetUrlScheme());
				next->SetUrlHost(pr.GetUrlHost());
				next->SetUrlPort(pr.GetUrlPort());
				srs->SetSiteResource(next);
				resources[pr.GetSiteMD5()] = srs;
			} else {
				next = srs->GetSiteResource();
			}
			// redirect to self? report error and process resources
			if (IsRedirectCycle(sr, next)) {
				LOG_DEBUG_R(this, sr, "Redirect to self: " << url);
				vector<string> allow;
				vector<string> disallow;
				sr->SetRobots(allow, disallow, robotsNegativeTTL);
			} else {
				if (next->GetIpAddrExpire() < currentTime || next->GetRobotsExpire() < currentTime) {
					// SR not up-to-date: recursively resolve SR
					LOG_TRACE_R(this, sr, "Recursively resolve SR");
					next->SetRobotsRedirectCount(redirects+1);
					tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.find(pr.GetSiteMD5());
					assert(iter != resources.end());
					srs->AppendWaitingSite(sr);
					StartProcessing(srs, next->GetIpAddrExpire() >= currentTime);
					return;
				}
				// SR is ready, just copy info
				CopyRobotsInfo(next, sr);
				sr->SetStatus(next->GetStatus());
			}
		} else {
			// error: too many redirects, make resources
			LOG_DEBUG_R(this, sr, "Too many redirects: " << url);
			vector<string> allow;
			vector<string> disallow;
			sr->SetRobots(allow, disallow, robotsNegativeTTL);
		}
	}

	// Hurray! SR is refreshed now
	// make all PageResources ready for the output
	if (srs->GetPageResources()->size() > 0)
		MarkSiteResourceReady(srs);

	// recursively finish redirected SR
	std::vector<SiteResource*> *waiting = srs->GetWaitingSites();
	for (vector<SiteResource*>::iterator iter = waiting->begin(); iter != waiting->end(); ++iter) {
		LOG_TRACE_R(this, sr, "finish SR (recurse)");
		// SiteResource: resolve robots.txt redirection
		SiteResource *prev = static_cast<SiteResource*>(*iter);
		// copy robots info from current resource to previous (in the redirection chain)
		CopyRobotsInfo(sr, prev);
		prev->SetStatus(sr->GetStatus());
		// recursively finish processing of resources
		FinishProcessing(prev);
	}
}

bool SiteManager::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources, int *processingResources) {
	// we always accept resources from the input, either to queue them for
	// further processing or to put them to the output queue
	if (expectingResources)
		*expectingResources = 1000;

	currentTime = time(NULL);
	LOG_TRACE(this, "waitingResourcesCount: " << waitingResourcesCount);
	while (inputResources->size() > 0 && waitingResourcesCount < maxRequests) {
		Resource *r = inputResources->front();
		inputResources->pop();
		if (PageResource::IsInstance(r)) {
			PageResource *pr = static_cast<PageResource*>(r);
			// ignore PageResources we have already seen
			uint64_t site_md5 = pr->GetSiteMD5();
			uint64_t path_md5 = pr->GetPathMD5();
			SitePathMD5 md5(site_md5, path_md5);
			if (seen.find(md5) != seen.end()) {
				LOG_DEBUG_R(this, pr, "Ignoring duplicated URL: " << pr->GetUrl());
				r->SetFlag(Resource::DELETED);
				outputResources->push(r);
				continue;
			}
			seen.insert(md5);

			std::tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.find(site_md5);
			if (!markerRead) {
				// not all PageResources read, just append the current one to the list
				SiteResources *srs;
				if (iter != resources.end()) {
					srs = iter->second;
				} else {
					// we did not see this site yet
					srs = new SiteResources();
					resources[site_md5] = srs;
				}
				srs->AppendPageResource(pr);
				waitingResourcesCount++;
				continue;
			}

			// markerRead, so this must be a redirect
			if (iter == resources.end()) {
				// we don't know anything about the site
				LOG_DEBUG_R(this, pr, "Unknown redirect site: " << pr->GetUrl());
				r->SetFlag(Resource::DELETED);
				outputResources->push(r);
				continue;
			}
			SiteResources *srs = iter->second;

			// redirect and we know the site, but it is not loaded yet
			SiteResource *sr = srs->GetSiteResource();
			if (!sr) {
				srs->AppendPageResource(pr);
				waitingResourcesCount++;
				continue;
			}

			// redirect, we know the site (it was loaded)
			LOG_TRACE_R(this, pr, "Checking PR: " << pr->GetUrl());
			if (sr->GetIpAddrExpire() < currentTime || sr->GetRobotsExpire() < currentTime) {
 				// site is not ready
				srs->AppendPageResource(pr);
				waitingResourcesCount++;
				StartProcessing(srs, sr->GetIpAddrExpire() >= currentTime);
				continue;
			}
			// no problem with the SR, just attach it to WR
			LOG_TRACE_R(this, pr, "NOP: " << pr->GetUrl());
			pr->SetStatus(0);
			srs->AppendPageResource(pr);
			waitingResourcesCount++;
			// making resources ready
			MarkSiteResourceReady(srs);
		} else if (MarkerResource::IsInstance(r)) {
			LOG_ERROR_R(this, r, "Input resources read");
			markerRead = true;
			siteResourcesRead = false;
			r->SetFlag(Resource::DELETED);
			outputResources->push(r);
		} else {
			LOG_ERROR_R(this, r, "Invalid resource type: " << r->GetTypeString());
			r->SetFlag(Resource::DELETED);
			outputResources->push(r);
		}
	}

	if (!markerRead) {
		if (processingResources)
			*processingResources = waitingResourcesCount;
		return false;
	}

	// marker was read, so now we process SiteResources
	if (!siteResourcesRead) {
		int resourcesProcessed = 0;
		SiteResource *sr = static_cast<SiteResource*>(Resource::GetRegistry()->AcquireResource(siteResourceTypeId));
		while (runningRequests < maxRequests) {
			// read one site resource from the input stream
			// first, we read header-only
			if (!sr->Deserialize(*istream, true)) {
				LOG_INFO(this, "Site resources read: " << siteResourcesRead);
				// end of input file
				siteResourcesRead = true;
				// get SiteResources that we do not have SiteResource for (they will be created)
				for (tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.begin(); iter != resources.end(); ++iter) {
					if (!iter->second->GetSiteResource())
						newSiteResources.push_back(iter->second);
				}
				// we are done reading
				break;
			}
			std::tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.find(sr->GetSiteMD5());
			if (iter == resources.end()) {
				// we are not interested in this resource
				sr->Skip(*istream);
				continue;
			}
			if (!sr->Deserialize(*istream, false)) {
				LOG_ERROR(this, "Cannot deserialize resource");
				break;
			}
			SiteResources *srs = iter->second;
			if (srs->GetSiteResource())
				continue;	// the same SiteResource already read
			srs->SetSiteResource(sr);
			siteResourcesRead++;

			// resolve resource, if necessary
			if (sr->GetIpAddrExpire() < currentTime || sr->GetRobotsExpire() < currentTime) {
	 			// site is not ready
				StartProcessing(srs, sr->GetIpAddrExpire() >= currentTime);
				sr = static_cast<SiteResource*>(Resource::GetRegistry()->AcquireResource(siteResourceTypeId));
				continue;
			}
			sr = static_cast<SiteResource*>(Resource::GetRegistry()->AcquireResource(siteResourceTypeId));

			// or just add it to the output queue
			if (srs->GetPageResources()->size() > 0)
				MarkSiteResourceReady(srs);

			// check timeout every 100 SiteResources read
			if (++resourcesProcessed % 100 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
				break;
		}
		Resource::GetRegistry()->ReleaseResource(sr);
	}

	// if there are SiteResources that we did not read from the file, we create them
	if (newSiteResources.size() > 0) {
		int resourcesProcessed = 0;
		while (newSiteResources.size() > 0 && runningRequests < maxRequests) {
			SiteResources *srs = newSiteResources.back();
			newSiteResources.pop_back();
			vector<PageResource*> *pages = srs->GetPageResources();
			assert(pages->size() > 0);
			PageResource *pr = pages->back();
			SiteResource *sr = static_cast<SiteResource*>(Resource::GetRegistry()->AcquireResource(siteResourceTypeId));
			sr->SetSiteMD5(pr->GetSiteMD5());
			sr->SetUrlScheme(pr->GetUrlScheme());
			sr->SetUrlHost(pr->GetUrlHost());
			sr->SetUrlPort(pr->GetUrlPort());
			srs->SetSiteResource(sr);

			// resolve resource, if necessary
			if (sr->GetIpAddrExpire() < currentTime || sr->GetRobotsExpire() < currentTime) {
	 			// site is not ready
				StartProcessing(srs, sr->GetIpAddrExpire() >= currentTime);
				continue;
			}

			// or just add it to the output queue
			if (srs->GetPageResources()->size() > 0)
				MarkSiteResourceReady(srs);

			// check timeout every 10 SiteResources found
			if (++resourcesProcessed % 10 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
				break;
		}
	}

	// process DNS/robots inputs/outputs
	int dnsN;
	(void)callDns->Process(&callDnsInput, &callDnsOutput, &dnsN, timeTick/2);
	dnsN -= callDnsInput.size();
	while (callDnsOutput.size() > 0) {
		SiteResource *sr = static_cast<SiteResource*>(callDnsOutput.front());
		callDnsOutput.pop();
		IpAddr ip = sr->GetIpAddr();
		if (!ip.IsEmpty() && sr->GetRobotsExpire() < currentTime)
			callRobotsInput.push(sr);
		else
			FinishProcessing(sr);
	}

	int robotsN;
        (void)callRobots->Process(&callRobotsInput, &callRobotsOutput, &robotsN, timeTick/2);
	robotsN -= callRobotsInput.size();
	while (callRobotsOutput.size() > 0) {
		SiteResource *sr = static_cast<SiteResource*>(callRobotsOutput.front());
		callRobotsOutput.pop();
		FinishProcessing(sr);
	}

	// process ready resources, put one from every "bucket" to the outputQueue
	SiteResources *srs = ready;
	while (srs) {
		vector<PageResource*> *resources = srs->GetPageResources();
		PageResource *pr = resources->back();
		resources->pop_back();
		pr->SetAttachedResource(srs->GetSiteResource());
		outputResources->push(pr);
		waitingResourcesCount--;
		if (resources->size() == 0) {
			SiteResources *next = srs->GetReadyNext();
			UnmarkSiteResourceReady(srs);
			srs = next;
		} else {
			srs = ready->GetReadyNext();
		}
	}

	// we processed all resources (up to those redirected to the same site)
	if (siteResourcesRead && waitingResourcesCount == 0) {
		// prepare resources to be written
		for (tr1::unordered_map<uint64_t, SiteResources*>::iterator iter = resources.begin(); iter != resources.end(); ++iter) {
			writeResources.push_back(iter->second->GetSiteResource());
		}
	}

	// write site resources
	if (!siteResourcesWritten && writeResources.size() > 0) {
		int resourcesProcessed = 0;
		while (writeResources.size() > 0) {
			// write one site resource to the output stream
			SiteResource *sr = writeResources.back();
			writeResources.pop_back();
			uint64_t md5 = sr->GetSiteMD5();
			ostream->WriteLittleEndian64(md5);
			if (!sr->Serialize(*ostream)) {
				LOG_ERROR_R(this, sr, "Cannot serialize");
				break;
			}
			if (++resourcesProcessed % 10 == 0 && ((int)(time(NULL)-currentTime) > timeTick))
				break;
		}
	}

	if (processingResources)
		*processingResources = waitingResourcesCount;
	return waitingResourcesCount > 0 || writeResources.size() > 0;
}

bool SiteManager::SaveCheckpointSync(const char *path) {
	// TODO
	return true;
}

bool SiteManager::RestoreCheckpointSync(const char *path) {
	// TODO
	return true;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new SiteManager(objects, id, threadIndex);
}
