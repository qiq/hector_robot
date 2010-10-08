/**
 *
 */
#include <config.h>

#include <assert.h>
#include "googleurl/src/gurl.h"
#include "common.h"
#include "Resources.h"
#include "ParseUrl.h"
#include "WebResource.h"

ParseUrl::ParseUrl(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	values = new ObjectValues<ParseUrl>(this);

	values->addGetter("items", &ParseUrl::getItems);
}

ParseUrl::~ParseUrl() {
	delete values;
}

char *ParseUrl::getItems(const char *name) {
	return int2str(items);
}

bool ParseUrl::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;
	typeId = Resources::Name2Id("WebResource");
	if (typeId < 0) {
		LOG_ERROR("Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *ParseUrl::Process(Resource *resource) {
	WebResource *wr = dynamic_cast<WebResource*>(resource);
	if (wr) {
		GURL *gurl = new GURL(wr->getUrl());
		wr->setUrlScheme(gurl->scheme().c_str());
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
	}
	return resource;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new ParseUrl(objects, id, threadIndex);
}
