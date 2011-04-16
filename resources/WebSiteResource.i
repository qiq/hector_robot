%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "WebSiteResource.h"
%}

%include "Scheme.i"

class WebSiteResourceInfo : public ResourceInfo {
public:
        WebSiteResourceInfo();
};

class WebSiteResource : public Resource {
public:
        WebSiteResource();
        WebSiteResource(const WebSiteResource &wsr);
        ~WebSiteResource();
        Resource *Clone();
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input);
        int GetId();
        void SetId(int id);
        int GetStatus();
        void SetStatus(int status);
        Resource *GetAttachedResource();
        void SetAttachedResource(Resource *attachedResource);
        void ClearAttachedResource();
        int GetSize();
        ResourceInfo *GetResourceInfo();
        std::string ToString(Object::LogLevel = Object::INFO);

        // WebSiteResource-specific
        // preferred way: locks WSR and sets everything at once
        void SetUrl(int urlScheme, const std::string &urlHost, int urlPort);
        void GetUrl(int &urlScheme, std::string &urlHost, int &urlPort);
        void SetIpAddrExpire(IpAddr &addr, uint32_t time);
        void GetIpAddrExpire(IpAddr &addr, uint32_t &time);
        void SetRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, uint32_t time);
        void GetRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, uint32_t &time);
        int PathReadyToFetch(const char *path, uint32_t currentTime, uint32_t lastScheduled);
        bool PathNewLinkReady(const char *path, uint32_t currentTime);
        bool PathUpdateError(const char *path, uint32_t currentTime, int maxCount);
        bool PathUpdateRedirect(const char *path, uint32_t currentTime, bool redirectPermanent);
        bool PathUpdateOK(const char *path, uint32_t currentTime, uint32_t size, uint32_t cksum);
        int PathNextRefresh(const char *path);

        // change on-item methods
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
};

%inline %{
WebSiteResource *ResourceToWebSiteResource(Resource *r) {
        return r && WebSiteResource::IsInstance(r) ? static_cast<WebSiteResource*>(r) : NULL;
}

void DeleteVectorOfString(std::vector<std::string> *v) {
        delete v;
}

%}
