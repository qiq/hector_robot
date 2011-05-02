/**
 * MD5/2 of the site and path plus URL itself
 */

#ifndef _URL_RESOURCE_H_
#define _URL_RESOURCE_H_

#include <config.h>

#include <vector>
#include <string>
#include <log4cxx/logger.h>
#include "common.h"
#include "Resource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "UrlResource.pb.h"

class ResourceAttrInfo;

class UrlResourceInfo : public ResourceInfo {
public:
	UrlResourceInfo();
};

class UrlResource : public Resource {
public:
	UrlResource();
	UrlResource(const UrlResource &wr);
	~UrlResource();
	// create copy of a resource
	Resource *Clone();
	void Clear();
	// save and restore resource
	bool Serialize(ResourceOutputStream &output);
	bool Deserialize(ResourceInputStream &input);
	// used by queues in case there is limit on queue size
	int GetSize();
	// get info about this resource
	ResourceInfo *GetResourceInfo();
	// return string representation of the resource (e.g. for debugging purposes)
	std::string ToString(Object::LogLevel = Object::INFO);

	// UrlResource-specific
	void SetSiteMD5(uint64_t md5);
	uint64_t GetSiteMD5();
	void ClearSiteMD5();
	void SetPathMD5(uint64_t md5);
	uint64_t GetPathMD5();
	void ClearPathMD5();
	void SetUrl(const std::string &url);
	const std::string GetUrl();
	void ClearUrl();

	static bool IsInstance(Resource *resource);

protected:
	// saved properties
	hector::resources::UrlResource r;

	static UrlResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

inline UrlResource::UrlResource() {
}

inline UrlResource::UrlResource(const UrlResource &wr) : Resource(wr), r(wr.r) {
}

inline UrlResource::~UrlResource() {
}

inline Resource *UrlResource::Clone() {
	return new UrlResource(*this);
}

inline void UrlResource::Clear() {
	Resource::Clear();
	r.Clear();
}

inline bool UrlResource::Serialize(ResourceOutputStream &output) {
	output.WriteVarint32(r.ByteSize());
	r.SerializeWithCachedSizes(output.GetCodedOutputStream());
	return true;
}

inline bool UrlResource::Deserialize(ResourceInputStream &input) {
	uint32_t size;
	if (!input.ReadVarint32(&size))
                return false;
	google::protobuf::io::CodedInputStream::Limit l = input.PushLimit(size);
	bool result = r.ParseFromCodedStream(input.GetCodedInputStream());
	input.PopLimit(l);
	return result;
}

inline int UrlResource::GetSize() {
	return 32;
}

inline ResourceInfo *UrlResource::GetResourceInfo() {
	return &UrlResource::resourceInfo;
}

inline void UrlResource::SetSiteMD5(uint64_t md5) {
	r.set_site_md5(md5);
}

inline uint64_t UrlResource::GetSiteMD5() {
	return r.site_md5();
}

inline void UrlResource::ClearSiteMD5() {
	r.clear_site_md5();
}

inline void UrlResource::SetPathMD5(uint64_t md5) {
	r.set_path_md5(md5);
}

inline uint64_t UrlResource::GetPathMD5() {
	return r.path_md5();
}

inline void UrlResource::ClearPathMD5() {
	r.clear_path_md5();
}

inline void UrlResource::SetUrl(const std::string &url) {
	r.set_url(url);
}

inline const std::string UrlResource::GetUrl() {
	return r.url();
}

inline void UrlResource::ClearUrl() {
	r.clear_url();
}

inline bool UrlResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
