/**
 *
 */
#include <config.h>

#include <assert.h>
#include "googleurl/src/gurl.h"
#include "common.h"
#include "ComposeUrl.h"
#include "WebResource.h"

using namespace std;

ComposeUrl::ComposeUrl(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	clear = 0;

	values = new ObjectValues<ComposeUrl>(this);
	values->addGetter("items", &ComposeUrl::getItems);
	values->addGetter("clear", &ComposeUrl::getClear);
	values->addSetter("clear", &ComposeUrl::setClear);
}

ComposeUrl::~ComposeUrl() {
	delete values;
}

char *ComposeUrl::getItems(const char *name) {
	return int2str(items);
}

char *ComposeUrl::getClear(const char *name) {
	return bool2str(clear);
}

void ComposeUrl::setClear(const char *name, const char *value) {
	clear = str2bool(value);
}

bool ComposeUrl::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;
	typeId = Resource::NameToId("WebResource");
	if (typeId < 0) {
		LOG_ERROR("Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *ComposeUrl::ProcessSimple(Resource *resource) {
	if (resource->getTypeId() != WebResource::typeId)
		return resource;
	WebResource *wr = static_cast<WebResource*>(resource);
	if (wr->getUrlScheme() == "")
		return resource;
	string s = wr->getUrlScheme();
	s += "://";
	if (wr->getUrlUsername() != "") {
		s += wr->getUrlUsername();
		s += ":";
		s += wr->getUrlPassword();
		s += "@";
	}
	s += wr->getUrlHost();
	if (wr->getUrlPort() != 80) {
		s += ":";
		s += wr->getUrlPort();
	}
	s += wr->getUrlPath();
	if (wr->getUrlQuery() != "") {
		s += "?";
		s += wr->getUrlQuery();
	}
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
		wr->clearUrlQuery();
	}
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	return resource;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new ComposeUrl(objects, id, threadIndex);
}
