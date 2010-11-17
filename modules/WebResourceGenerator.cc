/**
 *
 */
#include <config.h>

#include <assert.h>
#include "common.h"
#include "WebResourceGenerator.h"
#include "WebResource.h"

using namespace std;

WebResourceGenerator::WebResourceGenerator(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxItems = 0;
	idPrefix = NULL;

	values = new ObjectValues<WebResourceGenerator>(this);
	values->addGetter("items", &WebResourceGenerator::getItems);
	values->addGetter("maxItems", &WebResourceGenerator::getMaxItems);
	values->addSetter("maxItems", &WebResourceGenerator::setMaxItems, true);
	values->addGetter("idPrefix", &WebResourceGenerator::getIdPrefix);
	values->addSetter("idPrefix", &WebResourceGenerator::setIdPrefix);
}

WebResourceGenerator::~WebResourceGenerator() {
	free(idPrefix);

	delete values;
}

char *WebResourceGenerator::getItems(const char *name) {
	return int2str(items);
}

char *WebResourceGenerator::getMaxItems(const char *name) {
	return int2str(maxItems);
}

void WebResourceGenerator::setMaxItems(const char *name, const char *value) {
	maxItems = str2int(value);
}

char *WebResourceGenerator::getIdPrefix(const char *name) {
	return idPrefix ? strdup(idPrefix) : NULL;
}

void WebResourceGenerator::setIdPrefix(const char *name, const char *value) {
	free(idPrefix);
	idPrefix = strdup(value);
}

bool WebResourceGenerator::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;
	if (maxItems)
		LOG_INFO("Going to produce " << maxItems << " WebResources.");
	typeId = Resource::NameToId("WebResource");
	if (typeId < 0) {
		LOG_ERROR("Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *WebResourceGenerator::ProcessInput(bool sleep) {
	ObjectLockRead();
	int it = items;
	ObjectUnlock();
	if (maxItems && it >= maxItems)
		return NULL;
	// we can use just new WebResource(), we use Resources::CreateResource() for demo purpose
	WebResource *wr = static_cast<WebResource*>(Resource::CreateResource(typeId));
	wr->setId(getThreadIndex()*10000+items);
	char s[1024];
	snprintf(s, sizeof(s), "http://test.org/%s%d-%d", idPrefix ? idPrefix : "", getThreadIndex(), items);
	wr->setUrl(s);
	ObjectLockWrite();
	items++;
	ObjectUnlock();
	LOG_INFO("Created WebResource (" << wr->getUrl() << ")");
	return wr;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new WebResourceGenerator(objects, id, threadIndex);
}
