#include <config.h>

#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "IndexResource.h"
#include "IndexResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr IndexResource::logger(log4cxx::Logger::getLogger("resources.IndexResource"));
IndexResourceInfo IndexResource::resourceInfo;

IndexResourceInfo::IndexResourceInfo() {
	SetTypeId(14);
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
	snprintf(buf, sizeof(buf), "[%s %d %d] MD5: %.16llx %.16llx, modified: %u, history: [", resourceInfo.GetTypeStringTerse(), GetId(), GetStatus(), (unsigned long long)GetSiteMD5(), (unsigned long long)GetPathMD5(), GetLastModified());
	s = buf;
	uint32_t mh = GetModificationHistory();
	for (int i = 0; i < 4; i++) {
		snprintf(buf, sizeof(buf), "%s%u", i > 0 ? " " : "", mh >> (i*8) & 0xFF);
		s += buf;
	}
	uint32_t is = GetIndexStatus();
	snprintf(buf, sizeof(buf), "], errors: %d, status: %d", (is >> 24) & 0xFF, is & 0x00FFFFFF);
	s += buf;
	return s;
}

#else

extern "C" Resource* hector_resource_create() {
	return (Resource*)new IndexResource();
}

#endif
