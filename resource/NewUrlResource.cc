#include <config.h>

#include "Resource.h"
#include "ResourceAttrInfoT.h"
#include "NewUrlResource.h"
#include "NewUrlResource.pb.h"

using namespace std;

#ifndef WRAPPER

log4cxx::LoggerPtr NewUrlResource::logger(log4cxx::Logger::getLogger("resources.NewUrlResource"));
NewUrlResourceInfo NewUrlResource::resourceInfo;

NewUrlResourceInfo::NewUrlResourceInfo() {
	SetTypeId(99);
	SetTypeString("NewUrlResource");
	SetTypeStringTerse("NUR");
	SetObjectName("NewUrlResource");

	vector<ResourceAttrInfo*> *l = new vector<ResourceAttrInfo*>();
	ResourceAttrInfoT<NewUrlResource> *ai;

	ai = new ResourceAttrInfoT<NewUrlResource>(GetTypeId());
	ai->InitInt32("id", &NewUrlResource::GetId, &NewUrlResource::SetId);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<NewUrlResource>(GetTypeId());
	ai->InitInt32("status", &NewUrlResource::GetStatus, &NewUrlResource::SetStatus);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<NewUrlResource>(GetTypeId());
	ai->InitUInt64("siteMD5", &NewUrlResource::GetSiteMD5, &NewUrlResource::SetSiteMD5);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<NewUrlResource>(GetTypeId());
	ai->InitUInt64("PathMD5", &NewUrlResource::GetPathMD5, &NewUrlResource::SetPathMD5);
	l->push_back(ai);

	ai = new ResourceAttrInfoT<NewUrlResource>(GetTypeId());
	ai->InitString("url", &NewUrlResource::GetUrl, &NewUrlResource::SetUrl);
	l->push_back(ai);

	SetAttrInfoList(l);
}

string NewUrlResource::ToString(Object::LogLevel logLevel) {
	string s;
	char buf[1024];
	snprintf(buf, sizeof(buf), "[%s %d %d] MD5: %.16llx %.16llx, url: %s\n", resourceInfo.GetTypeStringTerse(), GetId(), GetStatus(), GetSiteMD5(), GetPathMD5(), GetUrl().c_str());
	s = buf;
	return s;
}

#else

extern "C" Resource* hector_resource_create() {
	return (Resource*)new NewUrlResource();
}

#endif
