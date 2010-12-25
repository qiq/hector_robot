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

CallDns::CallDns(int maxRequests) : CallProcessingEngine(maxRequests) {
}

Resource *CallDns::PrepareResource(Resource *src) {
LOG4CXX_DEBUG(logger, "Starting dns resolution: " << src->getId());
	return src;
}

Resource *CallDns::FinishResource(Resource *tmp) {
LOG4CXX_DEBUG(logger, "Finishing dns resolution: " << tmp->getId());
	return tmp;
}

CallRobots::CallRobots(int maxRequests) : CallProcessingEngine(maxRequests) {
}

Resource *CallRobots::PrepareResource(Resource *src) {
	WebSiteResource *wsr = static_cast<WebSiteResource*>(src);
	WebResource *wr;
	if (unused.size() > 0) {
		wr = unused.back();
		unused.pop_back();
	} else {
		wr = new WebResource();
	}
	wr->setUrlScheme(wsr->getUrlScheme());
	wr->setUrlHost(wsr->getUrlHost());
	wr->setUrlPort(wsr->getUrlPort());
	wr->setUrlPath("/robots.txt");
	IpAddr ip = wsr->getIpAddr();
	wr->setIpAddr(ip);
	wr->setAttachedResource(wsr);
LOG4CXX_DEBUG(logger, "Starting robots resolution: " << wsr->getUrlHost());
	return wr;
}

Resource *CallRobots::FinishResource(Resource *tmp) {
	WebResource *wr = static_cast<WebResource*>(tmp);
	WebSiteResource *wsr = static_cast<WebSiteResource*>(tmp->getAttachedResource());
	wr->clearAttachedResource();
	int status = wr->getStatus();
	// redirect: set allow url to the redirected value
	if (status == 2) {
		vector<string> v;
		v.push_back(wr->getUrl());
		wsr->clearAllowUrls();
		wsr->setAllowUrls(v);
	}
	unused.push_back(wr);
	wsr->setStatus(status);
LOG4CXX_DEBUG(logger, "Finishing robots resolution: " << wsr->getUrlHost() << " (" << status << ")");
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
	values->addGetter("items", &WebSiteManager::getItems);
	values->addGetter("maxRequests", &WebSiteManager::getMaxRequests);
	values->addSetter("maxRequests", &WebSiteManager::setMaxRequests, true);
	values->addGetter("timeTick", &WebSiteManager::getTimeTick);
	values->addSetter("timeTick", &WebSiteManager::setTimeTick);
	values->addGetter("dnsEngine", &WebSiteManager::getDnsEngine);
	values->addSetter("dnsEngine", &WebSiteManager::setDnsEngine);
	values->addGetter("robotsEngine", &WebSiteManager::getRobotsEngine);
	values->addSetter("robotsEngine", &WebSiteManager::setRobotsEngine);
	values->addGetter("load", &WebSiteManager::getLoad);
	values->addSetter("load", &WebSiteManager::setLoad);
	values->addGetter("save", &WebSiteManager::getSave);
	values->addSetter("save", &WebSiteManager::setSave);
	values->addGetter("robotsMaxRedirects", &WebSiteManager::getRobotsMaxRedirects);
	values->addSetter("robotsMaxRedirects", &WebSiteManager::setRobotsMaxRedirects);
	values->addGetter("robotsNegativeTTL", &WebSiteManager::getRobotsNegativeTTL);
	values->addSetter("robotsNegativeTTL", &WebSiteManager::setRobotsNegativeTTL);

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

char *WebSiteManager::getItems(const char *name) {
	return int2str(items);
}

char *WebSiteManager::getMaxRequests(const char *name) {
	return int2str(maxRequests);
}

void WebSiteManager::setMaxRequests(const char *name, const char *value) {
	maxRequests = str2int(value);
}

char *WebSiteManager::getTimeTick(const char *name) {
	return int2str(timeTick);
}

void WebSiteManager::setTimeTick(const char *name, const char *value) {
	timeTick = str2int(value);
}

char *WebSiteManager::getDnsEngine(const char *name) {
	return dnsEngine ? strdup(dnsEngine) : NULL;
}

void WebSiteManager::setDnsEngine(const char *name, const char *value) {
	free(dnsEngine);
	dnsEngine = strdup(value);
}

char *WebSiteManager::getRobotsEngine(const char *name) {
	return robotsEngine ? strdup(robotsEngine) : NULL;
}

void WebSiteManager::setRobotsEngine(const char *name, const char *value) {
	free(robotsEngine);
	robotsEngine = strdup(value);
}

char *WebSiteManager::getLoad(const char *name) {
	return strdup("");
}

// actually load all wsr records
void WebSiteManager::setLoad(const char *name, const char *value) {
	if (!LoadWebSiteResources(value))
		LOG_ERROR(this, "Cannot load WebSiteManager data");
}

char *WebSiteManager::getSave(const char *name) {
	return strdup("");
}

// actually save all wsr records
void WebSiteManager::setSave(const char *name, const char *value) {
	if (!SaveWebSiteResources(value))
		LOG_ERROR(this, "Cannot save WebSiteManager data");
}

char *WebSiteManager::getRobotsMaxRedirects(const char *name) {
	return int2str(robotsMaxRedirects);
}

void WebSiteManager::setRobotsMaxRedirects(const char *name, const char *value) {
	robotsMaxRedirects = str2int(value);
}

char *WebSiteManager::getRobotsNegativeTTL(const char *name) {
	return int2str(robotsNegativeTTL);
}

void WebSiteManager::setRobotsNegativeTTL(const char *name, const char *value) {
	robotsNegativeTTL = str2int(value);
}

bool WebSiteManager::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params) {
		ProcessingEngine *engine = dynamic_cast<ProcessingEngine*>(objects->getObject(dnsEngine));
		if (!engine) {
			LOG_ERROR(this, "Invalid dnsEngine parameter: " << dnsEngine);
			return false;
		}
		callDns->setProcessingEngine(engine);

		engine = dynamic_cast<ProcessingEngine*>(objects->getObject(robotsEngine));
		if (!engine) {
			LOG_ERROR(this, "Invalid robotsEngine parameter: " << robotsEngine);
			return false;
		}
		callRobots->setProcessingEngine(engine);
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
WebSiteResource *WebSiteManager::getWebSiteResource(WebResource *wr) {
	key.setUrlScheme(wr->getUrlScheme());
	key.setUrlHost(wr->getUrlHost());
	key.setUrlPort(wr->getUrlPort());
	tr1::unordered_map<WebSiteResource*, WebSiteResource*, WebSiteResource_hash, WebSiteResource_equal>::iterator iter = sites.find(&key);
	if (iter != sites.end())
		return iter->second;
	// create, if not found
	WebSiteResource *wsr = pool->Alloc();
	wsr->setUrl(wr->getUrlScheme(), wr->getUrlHost(), wr->getUrlPort());
	sites[wsr] = wsr;
	return wsr;
}

void WebSiteManager::CopyRobotsInfo(WebSiteResource *src, WebSiteResource *dst) {
	vector<string> allow;
	vector<string> disallow;
	long time;
	src->getRobots(allow, disallow, time);
	dst->setRobots(allow, disallow, time);
}

void WebSiteManager::StartProcessing(Resource *r, WebSiteResource *wsr, bool robotsOnly) {
	// wsr is not yet being processed
	tr1::unordered_map<WebSiteResource*, vector<Resource*> *>::iterator iter = waitingResources.find(wsr);
	if (iter == waitingResources.end()) {
LOG_DEBUG_R(this, r, "start processing (really)");
		//r->setAttachedResource(wsr);
		vector<Resource*> *v = new vector<Resource*>();
		v->push_back(r);
		waitingResources[wsr] = v;
		if (r->getTypeId() == WebResource::typeId)
			waitingResourcesCount++;
		if (!robotsOnly)
			callDnsInput.push(wsr);
		else
			callRobotsInput.push(wsr);
	} else {
LOG_DEBUG_R(this, r, "start processing (wait)");
		// append to the waiting resources list
		iter->second->push_back(r);
		if (r->getTypeId() == WebResource::typeId)
			waitingResourcesCount++;
	}
}

void WebSiteManager::FinishProcessing(WebSiteResource *wsr, queue<Resource*> *outputResources) {
LOG_DEBUG_R(this, wsr, "finished wsr");
	tr1::unordered_map<WebSiteResource*, vector<Resource*>* >::iterator iter = waitingResources.find(wsr);
	assert(iter != waitingResources.end());
	if (wsr->getStatus() == 2) {
		// robots.txt was redirected, redirect target is allowed_urls[0]
		int redirects = wsr->getRobotsRedirectCount();
		vector<string> *v = wsr->getAllowUrls();
		assert(v->size() == 1);
		string url = v->front();
		delete v;
		ObjectLockRead();
		int maxRedirects = robotsMaxRedirects;
		ObjectUnlock();
		if (redirects < maxRedirects) {
			WebResource wr;
			wr.setUrl(url);
			WebSiteResource *next = getWebSiteResource(&wr);
			if (next->getIpAddrExpire() < (long)currentTime || next->getRobotsExpire() < (long)currentTime) {
LOG_DEBUG_R(this, wsr, "recurse");
				// WSR not up-to-date: recursively resolve WSR
				next->setRobotsRedirectCount(redirects+1);
				StartProcessing(wsr, next, next->getIpAddrExpire() >= (long)currentTime);
				return;
			}
			// WSR is ready, just copy info
			CopyRobotsInfo(next, wsr);
			wsr->setStatus(next->getStatus());
		} else {
			// error: too many redirects, make resources
			LOG_DEBUG_R(this, wsr, "Too many redirects: " << url);
			vector<string> allow;
			vector<string> disallow;
			ObjectLockRead();
			int ttl = robotsNegativeTTL;
			ObjectUnlock();
			wsr->setRobots(allow, disallow, ttl);
		}
	}
	// WSR is now refreshed
	vector<Resource*> *v = iter->second;
	for (vector<Resource*>::iterator iter = v->begin(); iter != v->end(); ++iter) {
		Resource *r = *iter;
		if (r->getTypeId() == WebResource::typeId) {
			// WebResource, just put it into the output queue
LOG_DEBUG_R(this, r, "finished WR");
			r->setAttachedResource(wsr);
			r->setStatus(0);
			outputResources->push(r);
			waitingResourcesCount--;
			ObjectLockWrite();
			items++;
			ObjectUnlock();
		} else {
LOG_DEBUG_R(this, r, "finished WSR (recurse)");
			// WebSiteResource: resolve robots.txt redirection
			WebSiteResource *prev = static_cast<WebSiteResource*>(r);
			// copy robots info from current resource to previous (in the redirection chain)
			CopyRobotsInfo(wsr, prev);
			prev->setStatus(wsr->getStatus());
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
		if (typeId != WebSiteResource::typeId) {
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

int WebSiteManager::ProcessMulti(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources) {
	currentTime = time(NULL);
	ObjectLockRead();
	int tick = timeTick/2;
	ObjectUnlock();
	while (inputResources->size() > 0 && waitingResourcesCount < maxRequests) {
		if (inputResources->front()->getTypeId() == WebResource::typeId) {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			// get domain info
			WebSiteResource *wsr = getWebSiteResource(wr);
			// status == 2 -> do not refresh IP address or robots.txt
			if (wr->getStatus() == 2) {
				wr->setAttachedResource(wsr);
				outputResources->push(wr);
			} else {
LOG_DEBUG_R(this, wr, "checking wr: " << wr->getUrl());
				if (wsr->getIpAddrExpire() < (long)currentTime || wsr->getRobotsExpire() < (long)currentTime) {
					StartProcessing(wr, wsr, wsr->getIpAddrExpire() >= (long)currentTime);
				} else {
LOG_DEBUG_R(this, wr, "checking wr: no-op" << wr->getUrl());
					// no problem with the WSR, just attach it to WR
					wr->setAttachedResource(wsr);
					wr->setStatus(0);
					outputResources->push(wr);
				}
			}
		} else {
			outputResources->push(inputResources->front());
		}
		inputResources->pop();
	}

	int dnsN;
	(void)callDns->Process(&callDnsInput, &callDnsOutput, &dnsN, tick);
	dnsN -= callDnsInput.size();
	while (callDnsOutput.size() > 0) {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(callDnsOutput.front());
		callDnsOutput.pop();
		IpAddr ip = wsr->getIpAddr();
		if (!ip.isEmpty() && wsr->getRobotsExpire() < (long)currentTime)
			callRobotsInput.push(wsr);
		else
			FinishProcessing(wsr, outputResources);
	}

	int robotsN;
        (void)callRobots->Process(&callRobotsInput, &callRobotsOutput, &robotsN, tick);
	robotsN -= callRobotsInput.size();
	while (callRobotsOutput.size() > 0) {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(callRobotsOutput.front());
		callRobotsOutput.pop();
		FinishProcessing(wsr, outputResources);
	}

	int min = dnsN < robotsN ? dnsN : robotsN;
	if (expectingResources)
		*expectingResources = min;
LOG_DEBUG(this, "min: " << min << ", waitingResourcesCount: " << waitingResourcesCount);
	return waitingResourcesCount;
}

bool WebSiteManager::SaveCheckpointSync(const char *path) {
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s.%s", path, getId());
	return SaveWebSiteResources(buffer);
}

bool WebSiteManager::RestoreCheckpointSync(const char *path) {
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s.%s", path, getId());
	return LoadWebSiteResources(buffer);
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new WebSiteManager(objects, id, threadIndex);
}
