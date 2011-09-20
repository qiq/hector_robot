/**
 *
 */
#include <config.h>

#include <assert.h>
#include "common.h"
#include "GeneratePageResource.h"
#include "PageResource.h"

using namespace std;

GeneratePageResource::GeneratePageResource(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxItems = 0;
	idPrefix = NULL;

	props = new ObjectProperties<GeneratePageResource>(this);
	props->Add("items", &GeneratePageResource::GetItems);
	props->Add("maxItems", &GeneratePageResource::GetMaxItems, &GeneratePageResource::SetMaxItems, true);
	props->Add("idPrefix", &GeneratePageResource::GetIdPrefix, &GeneratePageResource::SetIdPrefix);
}

GeneratePageResource::~GeneratePageResource() {
	free(idPrefix);

	delete props;
}

char *GeneratePageResource::GetItems(const char *name) {
	return int2str(items);
}

char *GeneratePageResource::GetMaxItems(const char *name) {
	return int2str(maxItems);
}

void GeneratePageResource::SetMaxItems(const char *name, const char *value) {
	maxItems = str2int(value);
}

char *GeneratePageResource::GetIdPrefix(const char *name) {
	return idPrefix ? strdup(idPrefix) : NULL;
}

void GeneratePageResource::SetIdPrefix(const char *name, const char *value) {
	free(idPrefix);
	idPrefix = strdup(value);
}

bool GeneratePageResource::Init(vector<pair<string, string> > *params) {
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

Resource *GeneratePageResource::ProcessInputSync(bool sleep) {
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

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new GeneratePageResource(objects, id, threadIndex);
}
