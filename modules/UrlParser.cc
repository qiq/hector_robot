/**
 *
 */
#include <config.h>

#include <assert.h>
#include "googleurl/src/gurl.h"
#include "common.h"
#include "UrlParser.h"
#include "WebResource.h"

using namespace std;

UrlParser::UrlParser(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;

	values = new ObjectValues<UrlParser>(this);
	values->addGetter("items", &UrlParser::getItems);
}

UrlParser::~UrlParser() {
	delete values;
}

char *UrlParser::getItems(const char *name) {
	return int2str(items);
}

bool UrlParser::Init(vector<pair<string, string> > *params) {
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

Resource *UrlParser::ProcessSimple(Resource *resource) {
	if (resource->getTypeId() != WebResource::typeId)
		return resource;
	WebResource *wr = static_cast<WebResource*>(resource);
	GURL *gurl = new GURL(wr->getUrl());
	if (gurl->SchemeIs("http"))
		wr->setUrlScheme(HTTP);
	else if (gurl->SchemeIs("https"))
		wr->setUrlScheme(HTTPS);
	else
		wr->setUrlScheme(NONE);
	wr->setUrlUsername(gurl->username());
	wr->setUrlPassword(gurl->password());
	wr->setUrlHost(gurl->host());
	wr->setUrlPort(gurl->EffectiveIntPort());
	string p = gurl->path();
	string q = gurl->query();
	if (!q.empty()) {
		p += "?";
		p += q;
	}
	wr->setUrlPath(p);
	delete gurl;
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	return resource;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new UrlParser(objects, id, threadIndex);
}
