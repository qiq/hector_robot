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
	values->AddGetter("items", &WebResourceGenerator::GetItems);
	values->AddGetter("maxItems", &WebResourceGenerator::GetMaxItems);
	values->AddSetter("maxItems", &WebResourceGenerator::SetMaxItems, true);
	values->AddGetter("idPrefix", &WebResourceGenerator::GetIdPrefix);
	values->AddSetter("idPrefix", &WebResourceGenerator::SetIdPrefix);
}

WebResourceGenerator::~WebResourceGenerator() {
	free(idPrefix);

	delete values;
}

char *WebResourceGenerator::GetItems(const char *name) {
	return int2str(items);
}

char *WebResourceGenerator::GetMaxItems(const char *name) {
	return int2str(maxItems);
}

void WebResourceGenerator::SetMaxItems(const char *name, const char *value) {
	maxItems = str2int(value);
}

char *WebResourceGenerator::GetIdPrefix(const char *name) {
	return idPrefix ? strdup(idPrefix) : NULL;
}

void WebResourceGenerator::SetIdPrefix(const char *name, const char *value) {
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
	typeId = Resource::GetRegistry()->NameToId("WebResource");
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
	WebResource *wr = static_cast<WebResource*>(Resource::GetRegistry()->AcquireResource(typeId));
	wr->SetId(GetThreadIndex()*10000+items);
	char s[1024];
	snprintf(s, sizeof(s), "http://test.org/%s%d-%d", idPrefix ? idPrefix : "", GetThreadIndex(), items);
	wr->SetUrl(s);
	items++;
	LOG_INFO_R(this, wr, "Created (" << wr->GetUrl() << ")");
	return wr;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new WebResourceGenerator(objects, id, threadIndex);
}
