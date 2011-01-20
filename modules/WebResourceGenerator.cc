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
	values->AddGetter("items", &WebResourceGenerator::getItems);
	values->AddGetter("maxItems", &WebResourceGenerator::getMaxItems);
	values->AddSetter("maxItems", &WebResourceGenerator::setMaxItems, true);
	values->AddGetter("idPrefix", &WebResourceGenerator::getIdPrefix);
	values->AddSetter("idPrefix", &WebResourceGenerator::setIdPrefix);
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
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;
	if (maxItems)
		LOG_INFO(this, "Going to produce " << maxItems << " WebResources.");
	typeId = Resource::registry.NameToId("WebResource");
	if (typeId < 0) {
		LOG_ERROR(this, "Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *WebResourceGenerator::ProcessInputSync(bool sleep) {
	if (maxItems && items >= maxItems)
		return NULL;
	// we can use just new WebResource(), we use Resources::AcquireResource() for demo purpose
	WebResource *wr = static_cast<WebResource*>(Resource::registry.AcquireResource(typeId));
	wr->setId(getThreadIndex()*10000+items);
	char s[1024];
	snprintf(s, sizeof(s), "http://test.org/%s%d-%d", idPrefix ? idPrefix : "", getThreadIndex(), items);
	wr->setUrl(s);
	items++;
	LOG_INFO_R(this, wr, "Created (" << wr->getUrl() << ")");
	return wr;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new WebResourceGenerator(objects, id, threadIndex);
}
