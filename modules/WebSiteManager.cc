/**
 * WebSiteManager module.
 */
#include <config.h>

#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "TestResource.h"
#include "WebSiteManager.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

CallDns::CallDns(int maxRequests) : CallProcessingEngine(maxRequests, true) {
}

Resource *CallDns::PrepareResource(Resource *src) {
	assert(WebSiteResource::IsInstance(src));
	WebSiteResource *wsr = static_cast<WebSiteResource*>(src);
	LOG4CXX_TRACE(logger, wsr->ToStringShort() << " " << "DNS start: " << wsr->GetUrlHost());
	return src;
}

Resource *CallDns::FinishResource(Resource *tmp) {
	assert(WebSiteResource::IsInstance(tmp));
	WebSiteResource *wsr = static_cast<WebSiteResource*>(tmp);
	LOG4CXX_TRACE(logger, wsr->ToStringShort() << " " << "DNS finish: " << wsr->GetUrlHost());
	return tmp;
}

CallRobots::CallRobots(int maxRequests) : CallProcessingEngine(maxRequests, true) {
	webResourceTypeId = Resource::GetRegistry()->NameToId("WebResource");
}

Resource *CallRobots::PrepareResource(Resource *src) {
	WebSiteResource *wsr = static_cast<WebSiteResource*>(src);
	WebResource *wr;
	if (unused.size() > 0) {
		wr = unused.back();
		unused.pop_back();
	} else {
		wr = static_cast<WebResource*>(Resource::GetRegistry()->AcquireResource(webResourceTypeId));

	}
	wr->SetUrlScheme(wsr->GetUrlScheme());
	wr->SetUrlHost(wsr->GetUrlHost());
	wr->SetUrlPort(wsr->GetUrlPort());
	wr->SetUrlPath("/robots.txt");
	IpAddr ip = wsr->GetIpAddr();
	wr->SetIpAddr(ip);
	wr->SetAttachedResource(wsr);
	LOG4CXX_TRACE(logger, wsr->ToStringShort() << " " << "Robots start (" << wr->ToStringShort() << "): " << wsr->GetUrlHost());
	return wr;
}

Resource *CallRobots::FinishResource(Resource *tmp) {
	WebResource *wr = static_cast<WebResource*>(tmp);
	WebSiteResource *wsr = static_cast<WebSiteResource*>(tmp->GetAttachedResource());
	wr->ClearAttachedResource();
	int status = wr->GetStatus();
	// redirect: set allow url to the redirected value
	if (status == 2) {
		vector<string> v;
		v.push_back(wr->GetUrl());
		wsr->ClearAllowUrls();
		wsr->SetAllowUrls(v);
	}
	unused.push_back(wr);
	wsr->SetStatus(status);
	LOG4CXX_TRACE(logger, wsr->ToStringShort() << " " << "Robots finish (" << tmp->ToStringShort() << "): " << wsr->GetUrlHost() << " (" << status << ")");
	return wsr;
}

WebSiteManager::WebSiteManager(ObjectRegistry *objects, const char *id, int threadIndex) : Module(objects, id, threadIndex) {
	items = 0;
	maxRequests = 1000;
	timeTick = DEFAULT_TIME_TICK;
	dnsEngine = NULL;
	robotsEngine = NULL;
	robotsMaxRedirects = 5;
	robotsNegativeTTL = 86400;

	values = new ObjectValues<WebSiteManager>(this);
	values->AddGetter("items", &WebSiteManager::GetItems);
	values->AddGetter("maxRequests", &WebSiteManager::GetMaxRequests);
	values->AddSetter("maxRequests", &WebSiteManager::SetMaxRequests, true);
	values->AddGetter("timeTick", &WebSiteManager::GetTimeTick);
	values->AddSetter("timeTick", &WebSiteManager::SetTimeTick);
	values->AddGetter("dnsEngine", &WebSiteManager::GetDnsEngine);
	values->AddSetter("dnsEngine", &WebSiteManager::SetDnsEngine);
	values->AddGetter("robotsEngine", &WebSiteManager::GetRobotsEngine);
	values->AddSetter("robotsEngine", &WebSiteManager::SetRobotsEngine);
	values->AddGetter("load", &WebSiteManager::GetLoad);
	values->AddSetter("load", &WebSiteManager::SetLoad);
	values->AddGetter("save", &WebSiteManager::GetSave);
	values->AddSetter("save", &WebSiteManager::SetSave);
	values->AddGetter("robotsMaxRedirects", &WebSiteManager::GetRobotsMaxRedirects);
	values->AddSetter("robotsMaxRedirects", &WebSiteManager::SetRobotsMaxRedirects);
	values->AddGetter("robotsNegativeTTL", &WebSiteManager::GetRobotsNegativeTTL);
	values->AddSetter("robotsNegativeTTL", &WebSiteManager::SetRobotsNegativeTTL);

	pool = new MemoryPool<WebSiteResource, true>(10*1024);

	waitingResourcesCount = 0;
}

WebSiteManager::~WebSiteManager() {
	delete values;
	delete pool;
	free(dnsEngine);
	free(robotsEngine);
	delete callDns;
	delete callRobots;
}

char *WebSiteManager::GetItems(const char *name) {
	return int2str(items);
}

char *WebSiteManager::GetMaxRequests(const char *name) {
	return int2str(maxRequests);
}

void WebSiteManager::SetMaxRequests(const char *name, const char *value) {
	maxRequests = str2int(value);
}

char *WebSiteManager::GetTimeTick(const char *name) {
	return int2str(timeTick);
}

void WebSiteManager::SetTimeTick(const char *name, const char *value) {
	timeTick = str2int(value);
}

char *WebSiteManager::GetDnsEngine(const char *name) {
	return dnsEngine ? strdup(dnsEngine) : NULL;
}

void WebSiteManager::SetDnsEngine(const char *name, const char *value) {
	free(dnsEngine);
	dnsEngine = strdup(value);
}

char *WebSiteManager::GetRobotsEngine(const char *name) {
	return robotsEngine ? strdup(robotsEngine) : NULL;
}

void WebSiteManager::SetRobotsEngine(const char *name, const char *value) {
	free(robotsEngine);
	robotsEngine = strdup(value);
}

char *WebSiteManager::GetLoad(const char *name) {
	return strdup("");
}

// actually load all wsr records
void WebSiteManager::SetLoad(const char *name, const char *value) {
	if (!LoadWebSiteResources(value))
		LOG_ERROR(this, "Cannot load WebSiteManager data");
}

char *WebSiteManager::GetSave(const char *name) {
	return strdup("");
}

// actually save all wsr records
void WebSiteManager::SetSave(const char *name, const char *value) {
	if (!SaveWebSiteResources(value))
		LOG_ERROR(this, "Cannot save WebSiteManager data");
}

char *WebSiteManager::GetRobotsMaxRedirects(const char *name) {
	return int2str(robotsMaxRedirects);
}

void WebSiteManager::SetRobotsMaxRedirects(const char *name, const char *value) {
	robotsMaxRedirects = str2int(value);
}

char *WebSiteManager::GetRobotsNegativeTTL(const char *name) {
	return int2str(robotsNegativeTTL);
}

void WebSiteManager::SetRobotsNegativeTTL(const char *name, const char *value) {
	robotsNegativeTTL = str2int(value);
}

bool WebSiteManager::Init(vector<pair<string, string> > *params) {
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

	if (!values->InitValues(params))
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

	return true;
}

// try to get wsr, if not present, create it
WebSiteResource *WebSiteManager::GetWebSiteResource(WebResource *wr) {
	key.SetUrlScheme(wr->GetUrlScheme());
	key.SetUrlHost(wr->GetUrlHost());
	key.SetUrlPort(wr->GetUrlPort());
	tr1::unordered_map<WebSiteResource*, WebSiteResource*, WebSiteResource_hash, WebSiteResource_equal>::iterator iter = sites.find(&key);
	if (iter != sites.end())
		return iter->second;
	// create, if not found
	WebSiteResource *wsr = pool->Alloc();
	wsr->SetUrl(wr->GetUrlScheme(), wr->GetUrlHost(), wr->GetUrlPort());
	sites[wsr] = wsr;
	return wsr;
}

void WebSiteManager::CopyRobotsInfo(WebSiteResource *src, WebSiteResource *dst) {
	vector<string> allow;
	vector<string> disallow;
	long time;
	src->GetRobots(allow, disallow, time);
	dst->SetRobots(allow, disallow, time);
}

void WebSiteManager::StartProcessing(Resource *r, WebSiteResource *wsr, bool robotsOnly) {
	// wsr is not yet being processed
	tr1::unordered_map<WebSiteResource*, vector<Resource*> *>::iterator iter = waitingResources.find(wsr);
	if (iter == waitingResources.end()) {
		LOG_TRACE_R(this, r, "start (" << wsr->ToStringShort() << ")");
		vector<Resource*> *v = new vector<Resource*>();
		v->push_back(r);
		waitingResources[wsr] = v;
		if (WebResource::IsInstance(r))
			waitingResourcesCount++;
		if (!robotsOnly)
			callDnsInput.push(wsr);
		else
			callRobotsInput.push(wsr);
	} else {
		LOG_TRACE_R(this, r, "waiting (" << wsr->ToStringShort() << ")");
		// append to the waiting resources list
		iter->second->push_back(r);
		if (WebResource::IsInstance(r))
			waitingResourcesCount++;
	}
}

bool WebSiteManager::IsRedirectCycle(WebSiteResource *current, WebSiteResource *wsr) {
	tr1::unordered_map<WebSiteResource*, vector<Resource*>* >::iterator iter = waitingResources.find(current);
	if (iter == waitingResources.end())
		return false;
	vector<Resource*> *v = iter->second;
	for (vector<Resource*>::iterator iter = v->begin(); iter != v->end(); ++iter) {
		if (WebSiteResource::IsInstance(*iter)) {
			WebSiteResource *prev = static_cast<WebSiteResource*>(*iter);
			if (prev == wsr || IsRedirectCycle(prev, wsr))
				return true;
		}
	}
	return false;
}

void WebSiteManager::FinishProcessing(WebSiteResource *wsr, queue<Resource*> *outputResources) {
	tr1::unordered_map<WebSiteResource*, vector<Resource*>* >::iterator iter = waitingResources.find(wsr);
	assert(iter != waitingResources.end());
	if (wsr->GetStatus() == 2) {
		// robots.txt was redirected, redirect target is allowed_urls[0]
		int redirects = wsr->GetRobotsRedirectCount();
		vector<string> *v = wsr->GetAllowUrls();
		assert(v->size() == 1);
		string url = v->front();
		delete v;
		if (redirects < robotsMaxRedirects) {
			WebResource wr;
			wr.SetUrl(url);
			WebSiteResource *next = GetWebSiteResource(&wr);
			// redirect to self? report error and process resources
			if (IsRedirectCycle(wsr, next)) {
				LOG_DEBUG_R(this, wsr, "Redirect to self: " << url);
				vector<string> allow;
				vector<string> disallow;
				wsr->SetRobots(allow, disallow, robotsNegativeTTL);
			} else {
				if (next->GetIpAddrExpire() < (long)currentTime || next->GetRobotsExpire() < (long)currentTime) {
					// WSR not up-to-date: recursively resolve WSR
					LOG_TRACE_R(this, wsr, "Recursively resolve WSR");
					next->SetRobotsRedirectCount(redirects+1);
					StartProcessing(wsr, next, next->GetIpAddrExpire() >= (long)currentTime);
					return;
				}
				// WSR is ready, just copy info
				CopyRobotsInfo(next, wsr);
				wsr->SetStatus(next->GetStatus());
			}
		} else {
			// error: too many redirects, make resources
			LOG_DEBUG_R(this, wsr, "Too many redirects: " << url);
			vector<string> allow;
			vector<string> disallow;
			wsr->SetRobots(allow, disallow, robotsNegativeTTL);
		}
	}
	// WSR is now refreshed
	vector<Resource*> *v = iter->second;
	for (vector<Resource*>::iterator iter = v->begin(); iter != v->end(); ++iter) {
		Resource *r = *iter;
		if (WebResource::IsInstance(r)) {
			// WebResource, just put it into the output queue
			LOG_TRACE_R(this, r, "finish WR (" << wsr->ToStringShort() << ")");
			r->SetAttachedResource(wsr);
			r->SetStatus(0);
			outputResources->push(r);
			waitingResourcesCount--;
			items++;
		} else {
			LOG_TRACE_R(this, r, "finish WSR (recurse)");
			// WebSiteResource: resolve robots.txt redirection
			WebSiteResource *prev = static_cast<WebSiteResource*>(r);
			// copy robots info from current resource to previous (in the redirection chain)
			CopyRobotsInfo(wsr, prev);
			prev->SetStatus(wsr->GetStatus());
			// recursively finish processing of resources
			FinishProcessing(prev, outputResources);
		}
	}
	delete v;
	waitingResources.erase(wsr);
}

bool WebSiteManager::LoadWebSiteResources(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		LOG_ERROR(this, "Cannot open file " << filename << ": " << strerror(errno));
		return false;
	}
	google::protobuf::io::FileInputStream *file = new google::protobuf::io::FileInputStream(fd);
	google::protobuf::io::CodedInputStream *stream = new google::protobuf::io::CodedInputStream(file);

	bool result = true;
	while (1) {
		WebSiteResource *wsr = pool->Alloc();
		char buffer[5];
		result = stream->ReadRaw(buffer, 5);
		if (!result) {
			// end-of-file
			result = true;
			break;
		}
		uint32_t size = *(uint32_t*)buffer;
		uint8_t typeId = *(uint8_t*)(buffer+4);
		if (typeId != wsr->GetTypeId()) {
			LOG_ERROR(this, "Invalid resource type: " << typeId);
			break;
		}
		google::protobuf::io::CodedInputStream::Limit l = stream->PushLimit(size);
		result = wsr->Deserialize(stream);
		stream->PopLimit(l);
		if (!result) {
			LOG_ERROR(this, "Error reading resource");
			break;
		}
		// so that no path is locked
		wsr->ClearPathsRefreshing();
		sites[wsr] = wsr;
	}
	delete stream;
	delete file;
	close(fd);
	return result;
}

bool WebSiteManager::SaveWebSiteResources(const char *filename) {
	int fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd < 0) {
		LOG_ERROR(this, "Cannot open file " << filename << ": " << strerror(errno));
		return false;
	}
	google::protobuf::io::FileOutputStream *file = new google::protobuf::io::FileOutputStream(fd);
	google::protobuf::io::CodedOutputStream *stream = new google::protobuf::io::CodedOutputStream(file);

	bool result = true;
	for (tr1::unordered_map<WebSiteResource*, WebSiteResource*, WebSiteResource_hash, WebSiteResource_equal>::iterator iter = sites.begin(); iter != sites.end(); ++iter) {
		if (!Resource::Serialize(iter->second, stream)) {
			LOG_ERROR_R(this, iter->second, "Cannot serialize resource");
			result = false;
			break;
		}
	}
	delete stream;
	delete file;
	close(fd);
	return result;
}

int WebSiteManager::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources) {
	currentTime = time(NULL);
	LOG_TRACE(this, "waitingResourcesCount: " << waitingResourcesCount << ", maxRequests: " << maxRequests);
	while (inputResources->size() > 0 && waitingResourcesCount < maxRequests) {
		if (WebResource::IsInstance(inputResources->front())) {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			// get domain info
			WebSiteResource *wsr = GetWebSiteResource(wr);
			// status == 2 -> do not refresh IP address or robots.txt
			if (wr->GetStatus() == 2) {
				wr->SetAttachedResource(wsr);
				outputResources->push(wr);
			} else {
				LOG_TRACE_R(this, wr, "Checking WR: " << wr->GetUrl());
				if (wsr->GetIpAddrExpire() < (long)currentTime || wsr->GetRobotsExpire() < (long)currentTime) {
					StartProcessing(wr, wsr, wsr->GetIpAddrExpire() >= (long)currentTime);
				} else {
					LOG_TRACE_R(this, wr, "NOP: " << wr->GetUrl());
					// no problem with the WSR, just attach it to WR
					wr->SetAttachedResource(wsr);
					wr->SetStatus(0);
					outputResources->push(wr);
				}
			}
		} else {
			outputResources->push(inputResources->front());
		}
		inputResources->pop();
	}

	int dnsN;
	(void)callDns->Process(&callDnsInput, &callDnsOutput, &dnsN, timeTick/2);
	dnsN -= callDnsInput.size();
	while (callDnsOutput.size() > 0) {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(callDnsOutput.front());
		callDnsOutput.pop();
		IpAddr ip = wsr->GetIpAddr();
		if (!ip.IsEmpty() && wsr->GetRobotsExpire() < (long)currentTime)
			callRobotsInput.push(wsr);
		else
			FinishProcessing(wsr, outputResources);
	}

	int robotsN;
        (void)callRobots->Process(&callRobotsInput, &callRobotsOutput, &robotsN, timeTick/2);
	robotsN -= callRobotsInput.size();
	while (callRobotsOutput.size() > 0) {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(callRobotsOutput.front());
		callRobotsOutput.pop();
		FinishProcessing(wsr, outputResources);
	}

	int min = dnsN < robotsN ? dnsN : robotsN;
	if (expectingResources)
		*expectingResources = min;
	return waitingResourcesCount;
}

bool WebSiteManager::SaveCheckpointSync(const char *path) {
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s.%s", path, GetId());
	return SaveWebSiteResources(buffer);
}

bool WebSiteManager::RestoreCheckpointSync(const char *path) {
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s.%s", path, GetId());
	return LoadWebSiteResources(buffer);
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new WebSiteManager(objects, id, threadIndex);
}
