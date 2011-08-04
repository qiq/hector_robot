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
	bool Deserialize(ResourceInputStream &input, bool headerOnly);
	bool Skip(ResourceInputStream &input);
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
	struct IndexResourceData {
		uint64_t site_md5;
		uint64_t path_md5;
		uint32_t last_modified;
		uint32_t modification_history;
		uint32_t index_status;
		uint32_t unused;
	};
	// saved properties
	IndexResourceData r;

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
	memset(&r, sizeof(r), 1);
}

inline bool IndexResource::Serialize(ResourceOutputStream &output) {
	output.WriteRaw((char*)&r, sizeof(r));
	return true;
}

inline bool IndexResource::Deserialize(ResourceInputStream &input, bool headerOnly) {
	if (headerOnly)
		return true;
	return input.ReadRaw((char*)&r, sizeof(r));
}

inline bool IndexResource::Skip(ResourceInputStream &input) {
	return input.Skip(sizeof(r));
}

inline int IndexResource::GetSize() {
	return sizeof(IndexResourceData);
}

inline ResourceInfo *IndexResource::GetResourceInfo() {
	return &IndexResource::resourceInfo;
}

inline void IndexResource::SetSiteMD5(uint64_t md5) {
	r.site_md5 = md5;
}

inline uint64_t IndexResource::GetSiteMD5() {
	return r.site_md5;
}

inline void IndexResource::ClearSiteMD5() {
	r.site_md5 = 0;
}

inline void IndexResource::SetPathMD5(uint64_t md5) {
	r.path_md5 = md5;
}

inline uint64_t IndexResource::GetPathMD5() {
	return r.path_md5;
}

inline void IndexResource::ClearPathMD5() {
	r.path_md5 = 0;
}

inline void IndexResource::SetLastModified(uint32_t lastModified) {
	r.last_modified = lastModified;
}

inline uint32_t IndexResource::GetLastModified() {
	return r.last_modified;
}

inline void IndexResource::ClearLastModified() {
	r.last_modified = 0;
}

inline void IndexResource::SetModificationHistory(uint32_t history) {
	r.modification_history = history;
}

inline uint32_t IndexResource::GetModificationHistory() {
	return r.modification_history;
}

inline void IndexResource::ClearModificationHistory() {
	r.modification_history = 0;
}

inline void IndexResource::SetIndexStatus(uint32_t indexStatus) {
	r.index_status = indexStatus;
}

inline uint32_t IndexResource::GetIndexStatus() {
	return r.index_status;
}

inline void IndexResource::ClearIndexStatus() {
	r.index_status = 0;
}

inline bool IndexResource::IsInstance(Resource *resource) {
	return resource->GetTypeId() == resourceInfo.GetTypeId();
}

#endif
