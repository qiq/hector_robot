%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "IndexResource.h"
%}

class IndexResourceInfo : public ResourceInfo {
public:
        IndexResourceInfo();
};

class IndexResource : public Resource {
public:
        IndexResource();
        IndexResource(const IndexResource &wr);
        ~IndexResource();
        Resource *Clone();
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input);
        int GetSize();
        ResourceInfo *GetResourceInfo();
        std::string ToString(Object::LogLevel = Object::INFO);

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
};
