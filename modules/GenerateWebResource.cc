/**
 *
 */
#include <config.h>

#include <assert.h>
#include "common.h"
#include "Resources.h"
#include "GenerateWebResource.h"
#include "WebResource.h"

GenerateWebResource::GenerateWebResource(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	maxItems = 0;
	idPrefix = NULL;
	values = new ObjectValues<GenerateWebResource>(this);

	values->addGetter("items", &GenerateWebResource::getItems);
	values->addGetter("maxItems", &GenerateWebResource::getMaxItems);
	values->addSetter("maxItems", &GenerateWebResource::setMaxItems);
	values->addGetter("idPrefix", &GenerateWebResource::getIdPrefix);
	values->addSetter("idPrefix", &GenerateWebResource::setIdPrefix);
}

GenerateWebResource::~GenerateWebResource() {
	free(idPrefix);

	delete values;
}

char *GenerateWebResource::getItems(const char *name) {
	return int2str(items);
}

char *GenerateWebResource::getMaxItems(const char *name) {
	return int2str(maxItems);
}

void GenerateWebResource::setMaxItems(const char *name, const char *value) {
	maxItems = str2int(value);
}

char *GenerateWebResource::getIdPrefix(const char *name) {
	return idPrefix ? strdup(idPrefix) : NULL;
}

void GenerateWebResource::setIdPrefix(const char *name, const char *value) {
	free(idPrefix);
	idPrefix = strdup(value);
}

bool GenerateWebResource::Init(vector<pair<string, string> > *params) {
	if (!values->InitValues(params))
		return false;
	if (maxItems)
		LOG_INFO(logger, "Going to produce " << maxItems << " WebResources.");
	typeId = Resources::Name2Id("WebResource");
	if (typeId < 0) {
		LOG_ERROR(logger, "Cannot load WebResource library");
		return false;
	}
	return true;
}

Resource *GenerateWebResource::Process(Resource *resource) {
	ObjectLockRead();
	if (maxItems && items >= maxItems) {
		ObjectUnlock();
		return NULL;
	}
	ObjectUnlock();
	assert(resource == NULL);
	// we can use just new WebResource(), we use Resources::CreateResource() for demo purpose
	WebResource *wr = dynamic_cast<WebResource*>(Resources::CreateResource(typeId));
	wr->setId(getThreadIndex()*10000+items);
	char s[1024];
	snprintf(s, sizeof(s), "http://test.org/%s%d-%d", idPrefix ? idPrefix : "", getThreadIndex(), items++);
	wr->setUrl(s);
	LOG_INFO(logger, "Created WebResource (" << wr->getUrl() << ")");
	return wr;
}

// the class factories

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return (Module*)new GenerateWebResource(objects, id, threadIndex);
}
