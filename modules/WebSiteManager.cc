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
#include "WebSiteManager.h"
#include "TestResource.h"

using namespace std;

// sleep TIME_TICK useconds waiting for socket changes
#define DEFAULT_TIME_TICK 100*1000

CallDns::CallDns(ProcessingEngine *engine, int maxRequests) : CallProcessingEngine(engine, maxRequests) {
}

Resource *CallDns::PrepareResource(Resource *src) {
	WebSiteResource *wsr = static_cast<WebSiteResource*>(src);
	WebResource *wr;
	if (unused.size() > 0) {
		wr = unused.back();
		unused.pop_back();
	} else {
		wr = new WebResource();
	}
	wr->setUrlHost(wsr->getUrlHost());
	wr->setAttachedResource(src);
	return wr;
}

Resource *CallDns::FinishResource(Resource *tmp) {
	WebResource *wr = static_cast<WebResource*>(tmp);
	WebSiteResource *wsr = static_cast<WebSiteResource*>(tmp->getAttachedResource());
	wr->clearAttachedResource();
	unused.push_back(wr);

	wsr->setIp4Addr(wr->getIp4Addr());
	wsr->setIp6Addr(wr->getIp6Addr());
	wsr->setIpAddrExpire(wr->getIpAddrExpire());
	return wsr;
}

CallRobots::CallRobots(ProcessingEngine *engine, int maxRequests) : CallProcessingEngine(engine, maxRequests) {
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
	wr->setUrlPath("robots.txt");
	wr->setIp4Addr(wsr->getIp4Addr());
	wr->setIp6Addr(wsr->getIp6Addr());
	wr->setAttachedResource(wsr);
	return wr;
}

Resource *CallRobots::FinishResource(Resource *tmp) {
	WebResource *wr = static_cast<WebResource*>(tmp);
	WebSiteResource *wsr = static_cast<WebSiteResource*>(tmp->getAttachedResource());
	wr->clearAttachedResource();
	unused.push_back(wr);

	wsr->setIp4Addr(wr->getIp4Addr());
	wsr->setIp6Addr(wr->getIp6Addr());
	wsr->setIpAddrExpire(wr->getIpAddrExpire());
	return wsr;
}

WebSiteManager::WebSiteManager(ObjectRegistry *objects, const char *id, int threadIndex) : Module(objects, id, threadIndex) {
	items = 0;
	maxRequests = 1000;
	timeTick = DEFAULT_TIME_TICK;
	dnsEngine = NULL;
	robotsEngine = NULL;

	values = new ObjectValues<WebSiteManager>(this);
	values->addGetter("items", &WebSiteManager::getItems);
	values->addGetter("timeTick", &WebSiteManager::getTimeTick);
	values->addSetter("timeTick", &WebSiteManager::setTimeTick);
	values->addGetter("maxRequests", &WebSiteManager::getMaxRequests);
	values->addSetter("maxRequests", &WebSiteManager::setMaxRequests);
	values->addGetter("dnsEngine", &WebSiteManager::getDnsEngine);
	values->addGetter("robotsEngine", &WebSiteManager::getRobotsEngine);

	pool = new MemoryPool<WebSiteResource>(10*1024);
}

WebSiteManager::~WebSiteManager() {
	delete values;
	delete pool;
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

char *WebSiteManager::getRobotsEngine(const char *name) {
	return robotsEngine ? strdup(robotsEngine) : NULL;
}

char *WebSiteManager::getSave(const char *name) {
	return strdup("");
}

// actually save all wsr records
void WebSiteManager::setSave(const char *name, const char *value) {
	if (!SaveWebSiteResources(value))
		LOG_ERROR("Cannot save WebSiteManager data");
}

char *WebSiteManager::getLoad(const char *name) {
	return strdup("");
}

// actually load all wsr records
void WebSiteManager::setLoad(const char *name, const char *value) {
	if (!LoadWebSiteResources(value))
		LOG_ERROR("Cannot load WebSiteManager data");
}

bool WebSiteManager::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;

	if (!dnsEngine || strlen(dnsEngine) == 0) {
		LOG_ERROR("dnsEngine not defined");
		return false;
	}
	ProcessingEngine *engine = dynamic_cast<ProcessingEngine*>(objects->getObject(dnsEngine));
	if (!engine) {
		LOG_ERROR("Invalid dnsEngine parameter" << dnsEngine);
	return false;
	}
	callDns = new CallDns(engine, maxRequests);

	if (!robotsEngine || strlen(robotsEngine) == 0) {
		LOG_ERROR("robotsEngine not defined");
		return false;
	}
	engine = dynamic_cast<ProcessingEngine*>(objects->getObject(robotsEngine));
	if (!engine) {
		LOG_ERROR("Invalid robotsEngine parameter" << robotsEngine);
	return false;
	}
	callRobots = new CallRobots(engine, maxRequests);

	return true;
}

// try to get wsr, if not present, create it
WebSiteResource *WebSiteManager::getWebSiteResource(WebResource *wr) {
	WebSiteResource key;
	key.setUrlScheme(wr->getUrlScheme());
	key.setUrlHost(wr->getUrlHost());
	key.setUrlPort(wr->getUrlPort());
	tr1::unordered_map<WebSiteResource*, WebSiteResource*>::iterator iter = sites.find(&key);
	if (iter != sites.end())
		return iter->second;
	// create, if not found
	WebSiteResource *wsr = pool->alloc();
	wsr->setUrlScheme(wr->getUrlScheme());
	wsr->setUrlHost(wr->getUrlHost());
	wsr->setUrlPort(wr->getUrlPort());
	sites[wsr] = wsr;
	return wsr;
}

void WebSiteManager::StartProcessing(WebResource *wr, WebSiteResource *wsr, bool robotsOnly) {
	// resource is not yet being processed
	tr1::unordered_map<WebSiteResource*, WebResource*>::iterator iter = processingResources.find(wsr);
	if (iter == processingResources.end()) {
		wr->setAttachedResource(wsr);
		processingResources[wsr] = wr;
		if (!robotsOnly)
			callDnsInput.push(wsr);
		else
			callRobotsInput.push(wsr);
	} else {
		// chain waiting web-resources
		wr->setAttachedResource(iter->second);
		processingResources[wsr] = wr;
	}
}

void WebSiteManager::FinishProcessing(WebSiteResource *wsr, queue<Resource*> *outputResources) {
	tr1::unordered_map<WebSiteResource*, WebResource*>::iterator iter = processingResources.find(wsr);
	assert(iter != processingResources.end());
	WebResource *wr = static_cast<WebResource*>(iter->second);
	Resource *next = wr->getAttachedResource();
	wr->setAttachedResource(wsr);
	outputResources->push(wr);
	while (next != static_cast<Resource*>(iter->first)) {
		WebResource *wr = static_cast<WebResource*>(next);
		next = wr->getAttachedResource();
		wr->setAttachedResource(wsr);
		outputResources->push(wr);
	}
}

bool WebSiteManager::LoadWebSiteResources(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		LOG_ERROR("Cannot open file " << filename << ": " << strerror(errno));
		return false;
	}
	google::protobuf::io::FileInputStream *stream = new google::protobuf::io::FileInputStream(fd);

	bool result = true;
	char buffer[5];
	while (1) {
		int r = ReadBytes(fd, buffer, 5);
		if (r < 0) {
			LOG_ERROR("Error reading from file: " << strerror(errno));
			result = false;
			break;
		}
		if (r == 0)	// finished
			break;
		if (r != 5) {
			LOG_ERROR("Error reading from file: " << strerror(errno));
			result = false;
			break;
		}
		WebSiteResource *wsr = pool->alloc();
		if (!wsr->Deserialize(stream, *(uint32_t*)buffer)) {
			result = false;
			break;
		}
		// TODO: set resource id, so that it is unique?
		sites[wsr] = wsr;
	}
	if (stream)
		stream->Close();
	return result;
}

bool WebSiteManager::SaveWebSiteResources(const char *filename) {
	int fd = open(filename, O_WRONLY);
	if (fd < 0) {
		LOG_ERROR("Cannot open file " << filename << ": " << strerror(errno));
		return false;
	}
	google::protobuf::io::FileOutputStream *stream = new google::protobuf::io::FileOutputStream(fd);

	bool result = true;
	for (tr1::unordered_map<WebSiteResource*, WebSiteResource*>::iterator iter = sites.begin(); iter != sites.end(); ++iter) {
		char buffer[5];
		*(uint32_t*)buffer = (uint32_t)iter->second->getSerializedSize();
		*(uint8_t*)(buffer+4) = WebSiteResource::typeId;
		int r = WriteBytes(fd, buffer, 5);
		if (r < 0) {
			LOG_ERROR("Error writing to file: " << strerror(errno));
			result = false;
			break;
		}
		if (r == 0)	// finished
			break;
		if (r != 5) {
			LOG_ERROR("Error writing to file: " << strerror(errno));
			result = false;
			break;
		}
		iter->second->SerializeWithCachedSizes(stream);
	}
	if (stream)
		stream->Close();
	return result;
}

int WebSiteManager::ProcessMulti(queue<Resource*> *inputResources, queue<Resource*> *outputResources) {
	int currentTime = time(NULL);
	ObjectLockRead();
	int max = maxRequests;
	int tick = timeTick/2;
	ObjectUnlock();
	int resources = callDnsInput.size() + callDns->ProcessingResources() + callDnsOutput.size() + callRobotsInput.size() + callRobots->ProcessingResources() + callRobotsOutput.size();
	while (inputResources->size() > 0 && resources < maxRequests) {
		if (inputResources->front()->getTypeId() == WebResource::typeId) {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			// get domain info
			WebSiteResource *wsr = getWebSiteResource(wr);
			if (wsr->getIpAddrExpire() < currentTime || wsr->getRobotsExpire() < currentTime) {
				StartProcessing(wr, wsr, wsr->getIpAddrExpire() >= currentTime);
			} else {
				// no problem with the wsr, just attach it to wr
				wr->setAttachedResource(wsr);
				outputResources->push(wr);
			}
		} else {
			outputResources->push(inputResources->front());
		}
		inputResources->pop();
	}

	int dnsN = callDns->Process(&callDnsInput, &callDnsOutput, tick);
	dnsN -= callDnsInput.size();
	while (callDnsOutput.size() > 0) {
		WebSiteResource *wsr = static_cast<WebSiteResource*>(callDnsOutput.front());
		callDnsOutput.pop();
		if (wsr->getRobotsExpire() < currentTime)
			callRobotsInput.push(wsr);
		else
			FinishProcessing(wsr, outputResources);
	}

        int robotsN = callRobots->Process(&callRobotsInput, &callRobotsOutput, tick);
	robotsN -= callRobotsInput.size();
	while (callRobotsOutput.size() > 0) {
		WebResource *wr = static_cast<WebResource*>(callRobotsOutput.front());
		callDnsOutput.pop();
		WebSiteResource *wsr = static_cast<WebSiteResource*>(wr->getAttachedResource());
		FinishProcessing(wsr, outputResources);
	}

	int min = dnsN < robotsN ? dnsN : robotsN;
	return min;
}

int WebSiteManager::ProcessingResources() {
	return callDnsInput.size() + callDns->ProcessingResources() + callDnsOutput.size() + callRobotsInput.size() + callRobots->ProcessingResources() + callRobotsOutput.size();
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
