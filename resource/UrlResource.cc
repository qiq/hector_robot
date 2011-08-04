#include <config.h>

#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "UrlResource.h"
#include "UrlResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr UrlResource::logger(log4cxx::Logger::getLogger("resources.UrlResource"));
UrlResourceInfo UrlResource::resourceInfo;

UrlResourceInfo::UrlResourceInfo() {
	SetTypeId(12);
	SetTypeString("UrlResource");
	SetTypeStringTerse("UR");
	SetObjectName("UrlResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<UrlResource> *ai;

	ai = new ResourceAttrInfoT<UrlResource>(GetTypeId());
	ai->InitInt32("id", &UrlResource::GetId, &UrlResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<UrlResource>(GetTypeId());
	ai->InitInt32("status", &UrlResource::GetStatus, &UrlResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<UrlResource>(GetTypeId());
	ai->InitUInt64("siteMD5", &UrlResource::GetSiteMD5, &UrlResource::SetSiteMD5);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<UrlResource>(GetTypeId());
	ai->InitUInt64("PathMD5", &UrlResource::GetPathMD5, &UrlResource::SetPathMD5);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<UrlResource>(GetTypeId());
	ai->InitString("url", &UrlResource::GetUrl, &UrlResource::SetUrl);
	l->push_back(ai);

	SetAttrInfoList(l);
}

string UrlResource::ToString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[%s %d %d] MD5: %.16llx %.16llx, url: %s\n", resourceInfo.GetTypeStringTerse(), GetId(), GetStatus(), (unsigned long long)GetSiteMD5(), (unsigned long long)GetPathMD5(), GetUrl().c_str());
	s = buf;
	return s;
}

#else

extern "C" Resource* hector_resource_create() {
	return (Resource*)new UrlResource();
}

#endif
