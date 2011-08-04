%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "SiteResource.h"
%}

class SiteResourceInfo : public ResourceInfo {
public:
        SiteResourceInfo();
};

class SiteResource : public SharedResource {
public:
        SiteResource();
        SiteResource(const SiteResource &wr);
        ~SiteResource();
        Resource *Clone();
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input, bool headerOnly);
        bool Skip(ResourceInputStream &input);
        int GetSize();
        ResourceInfo *GetResourceInfo();
        std::string ToString(Object::LogLevel = Object::INFO);

        void SetSiteMD5(uint64_t md5);
        uint64_t GetSiteMD5();
        void ClearSiteMD5();
        void SetUrlScheme(int urlScheme);
        int GetUrlScheme();
        void ClearUrlScheme();
        void SetUrlHost(const std::string &urlHost);
        const std::string GetUrlHost();
        void ClearUrlHost();
        void SetUrlPort(int urlPort);
        int GetUrlPort();
        void ClearUrlPort();
        void SetIpAddr(IpAddr &addr);
        IpAddr GetIpAddr();
        void ClearIpAddr();
        void SetIpAddrExpire(uint32_t time);
        uint32_t GetIpAddrExpire();
        void ClearIpAddrExpire();
        void SetAllowUrls(const std::vector<std::string> &allow_urls);
        void SetAllowUrl(int index, const std::string &url);
        std::vector<std::string> *GetAllowUrls();
        const std::string GetAllowUrl(int index);
        int CountAllowUrls();
        void ClearAllowUrls();
        void SetDisallowUrls(const std::vector<std::string> &disallow_urls);
        void SetDisallowUrl(int index, const std::string &url);
        std::vector<std::string> *GetDisallowUrls();
        const std::string GetDisallowUrl(int index);
        int CountDisallowUrls();
        void ClearDisallowUrls();
        void SetRobotsExpire(uint32_t time);
        uint32_t GetRobotsExpire();
        void ClearRobotsExpire();
        void SetRobotsRedirectCount(int redirects);
        int GetRobotsRedirectCount();
        void ClearRobotsRedirectCount();

        void SetRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, uint32_t time);
        void GetRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, uint32_t &time);

        static bool IsInstance(Resource *resource);
};

%inline %{
SiteResource *ResourceToSiteResource(Resource *r) {
        return r && SiteResource::IsInstance(r) ? static_cast<SiteResource*>(r) : NULL;
}
%}
