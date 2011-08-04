%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "PageResource.h"
%}

%include "Scheme.i"

class PageResourceInfo : public ResourceInfo {
public:
        PageResourceInfo();
};

class PageResource : public Resource {
public:
        PageResource();
        ~PageResource();
        Resource *Clone();
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input, bool headerOnly);
        bool Skip(ResourceInputStream &input);
        int GetSize();
        ResourceInfo *GetResourceInfo();
        std::string ToString(Object::LogLevel = Object::INFO);

        void SetUrl(const std::string &url);
        const std::string GetUrl();
        void ClearUrl();
        void SetSiteMD5(uint64_t md5);
        uint64_t GetSiteMD5();
        void ClearSiteMD5();
        void SetPathMD5(uint64_t md5);
        uint64_t GetPathMD5();
        void ClearPathMD5();
        void ComputeMD5();
        void SetIpAddr(IpAddr &addr);
        IpAddr GetIpAddr();
        void ClearIpAddr();
        void SetHeaderFields(const std::vector<std::string> &names, const std::vector<std::string> &values);
        std::vector<std::string> *GetHeaderNames();
        std::vector<std::string> *GetHeaderValues();
        void SetHeaderValue(const std::string &name, const std::string &value);
        const std::string GetHeaderValue(const std::string &name);
        int GetHeaderCount();
        void ClearHeaderField(const std::string &name);
        void ClearHeader();
        void SetRedirectCount(int count);
        int GetRedirectCount();
        void ClearRedirectCount();
        void SetContent(const std::string &content);
        const std::string GetContent();
        std::string *GetContentMutable();
        void ClearContent();

        void SetUrlScheme(int urlScheme);
        int GetUrlScheme();
        void ClearUrlScheme();
        void SetUrlUsername(const std::string &urlUsername);
        const std::string GetUrlUsername();
        void ClearUrlUsername();
        void SetUrlPassword(const std::string &urlPassword);
        const std::string GetUrlPassword();
        void ClearUrlPassword();
        void SetUrlHost(const std::string &urlHost);
        const std::string GetUrlHost();
        void ClearUrlHost();
        void SetUrlPort(int port);
        int GetUrlPort();
        void ClearUrlPort();
        void SetUrlPath(const std::string &urlPath);
        const std::string GetUrlPath();
        void ClearUrlPath();
};

%inline %{
PageResource *ResourceToPageResource(Resource *r) {
        return r && PageResource::IsInstance(r) ? static_cast<PageResource*>(r) : NULL;
}
%}

