%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "UrlResource.h"
%}

class UrlResourceInfo : public ResourceInfo {
public:
        UrlResourceInfo();
};

class UrlResource : public Resource {
public:
        UrlResource();
        UrlResource(const UrlResource &wr);
        ~UrlResource();
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
        void SetUrl(const std::string &url);
        const std::string GetUrl();
        void ClearUrl();

        static bool IsInstance(Resource *resource);
};
