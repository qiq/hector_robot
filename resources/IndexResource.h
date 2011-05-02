/**
 * info about a URL (excluding URL) :-)
 */

#ifndef _INDEX_RESOURCE_H_
#define _INDEX_RESOURCE_H_

#include <config.h>

#include <vector>
#include <string>
#include <tr1/unordered_map>
#include <log4cxx/logger.h>
#include "common.h"
#include "Resource.h"
#include "ResourceInputStream.h"
#include "ResourceOutputStream.h"
#include "IndexResource.pb.h"

class ResourceAttrInfo;

class IndexResourceInfo : public ResourceInfo {
public:
	IndexResourceInfo();
};

class IndexResource : public Resource {
public:
	IndexResource();
	IndexResource(const IndexResource &wr);
	~IndexResource();
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

	// IndexResource-specific
	void SetSiteMD5(uint64_t md5);
	uint64_t GetSiteMD5();
	void ClearSiteMD5();
	void SetPathMD5(uint64_t md5);
	uint64_t GetPathMD5();
	void ClearPathMD5();
	void SetLastModified(uint32_t lastModified);
	uint32_t GetLastModified();
	void ClearLastModified();
	void SetModificationHistory(uint32_t history);
	uint32_t GetModificationHistory();
	void ClearModificationHistory();
	void SetIndexStatus(uint32_t indexStatus);
	uint32_t GetIndexStatus();
	void ClearIndexStatus();

	static bool IsInstance(Resource *resource);

protected:
	// saved properties
	hector::resources::IndexResource r;

	static IndexResourceInfo resourceInfo;
	static log4cxx::LoggerPtr logger;
};

inline IndexResource::IndexResource() {
}

inline IndexResource::IndexResource(const IndexResource &wr) : Resource(wr), r(wr.r) {
}

inline IndexResource::~IndexResource() {
}

inline Resource *IndexResource::Clone() {
	return new IndexResource(*this);
}

inline void IndexResource::Clear() {
	Resource::Clear();
	r.Clear();
}

inline bool IndexResource::Serialize(ResourceOutputStream &output) {
	output.WriteVarint32(r.ByteSize());
	r.SerializeWithCachedSizes(output.GetCodedOutputStream());
	return true;
}

inline bool IndexResource::Deserialize(ResourceInputStream &input) {
	uint32_t size;
	if (!input.ReadVarint32(&size))
                return false;
	google::protobuf::io::CodedInputStream::Limit l = input.PushLimit(size);
	bool result = r.ParseFromCodedStream(input.GetCodedInputStream());
	input.PopLimit(l);
	return result;
}

inline int IndexResource::GetSize() {
	return 32;
}

inline ResourceInfo *IndexResource::GetResourceInfo() {
	return &IndexResource::resourceInfo;
}

inline void IndexResource::SetSiteMD5(uint64_t md5) {
	r.set_site_md5(md5);
}

inline uint64_t IndexResource::GetSiteMD5() {
	return r.site_md5();
}

inline void IndexResource::ClearSiteMD5() {
	r.clear_site_md5();
}

inline void IndexResource::SetPathMD5(uint64_t md5) {
	r.set_path_md5(md5);
}

inline uint64_t IndexResource::GetPathMD5() {
	return r.path_md5();
}

inline void IndexResource::ClearPathMD5() {
	r.clear_path_md5();
}

inline void IndexResource::SetLastModified(uint32_t lastModified) {
	r.set_last_modified(lastModified);
}

inline uint32_t IndexResource::GetLastModified() {
	return r.last_modified();
}

inline void IndexResource::ClearLastModified() {
	r.clear_last_modified();
}

inline void IndexResource::SetModificationHistory(uint32_t history) {
	r.set_modification_history(history);
}

inline uint32_t IndexResource::GetModificationHistory() {
	return r.modification_history();
}

inline void IndexResource::ClearModificationHistory() {
	r.clear_modification_history();
}

inline void IndexResource::SetIndexStatus(uint32_t indexStatus) {
	r.set_index_status(indexStatus);
}

inline uint32_t IndexResource::GetIndexStatus() {
	return r.index_status();
}

inline void IndexResource::ClearIndexStatus() {
	r.clear_index_status();
}

inline bool IndexResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
