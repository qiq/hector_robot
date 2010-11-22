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
		LOG_ERROR("Cannot load WebResource library");
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
		wr->setUrlScheme(SCHEME_HTTP);
	else if (gurl->SchemeIs("https"))
		wr->setUrlScheme(SCHEME_HTTPS);
	else
		wr->setUrlScheme(SCHEME_NONE);
	wr->setUrlUsername(gurl->username().c_str());
	wr->setUrlPassword(gurl->password().c_str());
	wr->setUrlHost(gurl->host().c_str());
	wr->setUrlPort(gurl->EffectiveIntPort());
	wr->setUrlPath(gurl->path().c_str());
	wr->setUrlQuery(gurl->query().c_str());
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
