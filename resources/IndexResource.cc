#include <config.h>

#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "IndexResource.h"
#include "IndexResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr IndexResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.IndexResource"));
IndexResourceInfo IndexResource::resourceInfo;

IndexResourceInfo::IndexResourceInfo() {
	SetTypeId(12);
	SetTypeString("IndexResource");
	SetTypeStringTerse("IR");
	SetObjectName("IndexResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<IndexResource> *ai;

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitInt32("id", &IndexResource::GetId, &IndexResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitInt32("status", &IndexResource::GetStatus, &IndexResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitUInt64("siteMD5", &IndexResource::GetSiteMD5, &IndexResource::SetSiteMD5);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitUInt64("PathMD5", &IndexResource::GetPathMD5, &IndexResource::SetPathMD5);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitUInt32("lastModified", &IndexResource::GetLastModified, &IndexResource::SetLastModified);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitUInt32("modificationHistory", &IndexResource::GetModificationHistory, &IndexResource::SetModificationHistory);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<IndexResource>(GetTypeId());
	ai->InitUInt32("indexStatus", &IndexResource::GetIndexStatus, &IndexResource::SetIndexStatus);
	l->push_back(ai);

	SetAttrInfoList(l);
}

string IndexResource::ToString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[IR %d %d] MD5: %.16llx %.16llx, modified: %u, history: [", GetId(), GetStatus(), GetSiteMD5(), GetPathMD5(), GetLastModified());
	s = buf;
	uint32_t lm = GetLastModified();
	for (int i = 0; i < 4; i++) {
		snprintf(buf, sizeof(buf), " %u", lm >> (i*8) & 0xFF);
		s += buf;
	}
	uint32_t is = GetIndexStatus();
	snprintf(buf, sizeof(buf), "], errors: %d, status: %d\n", (is >> 24) & 0xFF, is & 0x00FFFFFF);
	s += buf;
	return s;
}

#else

extern "C" Resource* hector_resource_create() {
	return (Resource*)new IndexResource();
}

#endif
