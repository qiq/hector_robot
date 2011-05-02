/**
 *
 */
#include <config.h>

#include <assert.h>
#include "common.h"
#include "PageResourceGenerator.h"
#include "PageResource.h"

using namespace std;

PageResourceGenerator::PageResourceGenerator(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxItems = 0;
	idPrefix = NULL;

	props = new ObjectProperties<PageResourceGenerator>(this);
	props->Add("items", &PageResourceGenerator::GetItems);
	props->Add("maxItems", &PageResourceGenerator::GetMaxItems, &PageResourceGenerator::SetMaxItems, true);
	props->Add("idPrefix", &PageResourceGenerator::GetIdPrefix, &PageResourceGenerator::SetIdPrefix);
}

PageResourceGenerator::~PageResourceGenerator() {
	free(idPrefix);

	delete props;
}

char *PageResourceGenerator::GetItems(const char *name) {
	return int2str(items);
}

char *PageResourceGenerator::GetMaxItems(const char *name) {
	return int2str(maxItems);
}

void PageResourceGenerator::SetMaxItems(const char *name, const char *value) {
	maxItems = str2int(value);
}

char *PageResourceGenerator::GetIdPrefix(const char *name) {
	return idPrefix ? strdup(idPrefix) : NULL;
}

void PageResourceGenerator::SetIdPrefix(const char *name, const char *value) {
	free(idPrefix);
	idPrefix = strdup(value);
}

bool PageResourceGenerator::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;
	if (maxItems)
		LOG_INFO(this, "Going to produce " << maxItems << " PageResources.");
	typeId = Resource::GetRegistry()->NameToId("PageResource");
	if (typeId < 0) {
		LOG_ERROR(this, "Cannot load PageResource library");
		return false;
	}
	return true;
}

Resource *PageResourceGenerator::ProcessInputSync(bool sleep) {
	if (maxItems && items >= maxItems)
		return NULL;
	// we can use just new PageResource(), we use Resources::AcquireResource() for demo purpose
	PageResource *pr = static_cast<PageResource*>(Resource::GetRegistry()->AcquireResource(typeId));
	pr->SetId(GetThreadIndex()*10000+items);
	char s[1024];
	snprintf(s, sizeof(s), "http://test.org/%s%d-%d", idPrefix ? idPrefix : "", GetThreadIndex(), items);
	pr->SetUrl(s);
	items++;
	LOG_INFO_R(this, pr, "Created (" << pr->GetUrl() << ")");
	return pr;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new PageResourceGenerator(objects, id, threadIndex);
}
