/**
 *
 */
#include <config.h>

#include <assert.h>
#include "googleurl/src/gurl.h"
#include "common.h"
#include "UrlComposer.h"
#include "WebResource.h"

using namespace std;

UrlComposer::UrlComposer(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	clear = 0;

	values = new ObjectValues<UrlComposer>(this);
	values->addGetter("items", &UrlComposer::getItems);
	values->addGetter("clear", &UrlComposer::getClear);
	values->addSetter("clear", &UrlComposer::setClear);
}

UrlComposer::~UrlComposer() {
	delete values;
}

char *UrlComposer::getItems(const char *name) {
	return int2str(items);
}

char *UrlComposer::getClear(const char *name) {
	return bool2str(clear);
}

void UrlComposer::setClear(const char *name, const char *value) {
	clear = str2bool(value);
}

bool UrlComposer::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;
	typeId = Resource::NameToId("WebResource");
	if (typeId < 0) {
		LOG_ERROR(this, "Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *UrlComposer::ProcessSimple(Resource *resource) {
	if (resource->getTypeId() != WebResource::typeId)
		return resource;
	WebResource *wr = static_cast<WebResource*>(resource);
	string s;
	int defaultPort;
	switch (wr->getUrlScheme()) {
	case HTTP:
		s = "http";
		defaultPort = 80;
		break;
	case HTTPS:
		s = "https";
		defaultPort = 443;
		break;
	case NONE:
	default:
		return resource;
	}
	s += "://";
	if (wr->getUrlUsername() != "") {
		s += wr->getUrlUsername();
		s += ":";
		s += wr->getUrlPassword();
		s += "@";
	}
	s += wr->getUrlHost();
	if (wr->getUrlPort() != defaultPort) {
		char buffer[20];
		snprintf(buffer, sizeof(buffer), ":%d", wr->getUrlPort());
		s += buffer;
	}
	s += wr->getUrlPath();
	wr->setUrl(s.c_str());

	ObjectLockRead();
	bool clear = this->clear;
	ObjectUnlock();
	if (clear) {
		wr->clearUrlScheme();
		wr->clearUrlUsername();
		wr->clearUrlPassword();
		wr->clearUrlHost();
		wr->clearUrlPort();
		wr->clearUrlPath();
	}
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	return resource;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new UrlComposer(objects, id, threadIndex);
}
