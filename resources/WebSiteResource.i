%include "std_string.i"
%include "std_vector.i"
%newobject WebSiteResource::Serialize();

%{
#include "WebSiteResource.h"
%}

%include "Scheme.i"

class WebSiteResource : public ProtobufResource {
public:
        WebSiteResource();
        WebSiteResource(const WebSiteResource &wsr);
        ~WebSiteResource();
        Resource *Clone();
        std::string *Serialize();
        int GetSerializedSize();
        bool SerializeWithCachedSize(google::protobuf::io::CodedOutputStream *output);
        bool Deserialize(const char *data, int size);
        bool Deserialize(google::protobuf::io::CodedInputStream *input);
        int getTypeId();
        const char *getTypeStr();
        const char *getTypeStrShort();
        const char *getModuleStr();
        int getId();
        void setId(int id);
        int getStatus();
        void setStatus(int status);
        int getSize();
        std::string toString(Object::LogLevel = Object::INFO);

        // WebSiteResource-specific
        // preferred way: locks WSR and sets everything at once
        void setUrl(int urlScheme, const std::string &urlHost, int urlPort);
        void getUrl(int &urlScheme, std::string &urlHost, int &urlPort);
        void setIpAddrExpire(IpAddr &addr, long time);
        void getIpAddrExpire(IpAddr &addr, long &time);
        void setRobots(const std::vector<std::string> &allow_urls, const std::vector<std::string> &disallow_urls, long time);
        void getRobots(std::vector<std::string> &allow_urls, std::vector<std::string> &disallow_urls, long &time);
        int PathReadyToFetch(const char *path, long currentTime, long lastScheduled);
        bool PathNewLinkReady(const char *path, long currentTime);
        bool PathUpdateError(const char *path, long currentTime, int maxCount);
        bool PathUpdateRedirect(const char *path, long currentTime, bool redirectPermanent);
        bool PathUpdateOK(const char *path, long currentTime, long size, long cksum);
        long PathNextRefresh(const char *path);

        // change on-item methods
        void setUrlScheme(int urlScheme);
        int getUrlScheme();
        void clearUrlScheme();
        void setUrlHost(const std::string &urlHost);
        const std::string &getUrlHost();
        void clearUrlHost();
        void setUrlPort(int urlPort);
        int getUrlPort();
        void clearUrlPort();
        void setIpAddr(IpAddr &addr);
        IpAddr getIpAddr();
        void clearIpAddr();
        void setIpAddrExpire(long time);
        long getIpAddrExpire();
        void clearIpAddrExpire();
        void setAllowUrls(const std::vector<std::string> &allow_urls);
        std::vector<std::string> *getAllowUrls();
        void clearAllowUrls();
        void setDisallowUrls(const std::vector<std::string> &disallow_urls);
        std::vector<std::string> *getDisallowUrls();
        void clearDisallowUrls();
        void setRobotsExpire(long time);
        long getRobotsExpire();
        void clearRobotsExpire();
        void setRobotsRedirectCount(int redirects);
        int getRobotsRedirectCount();
        void clearRobotsRedirectCount();
};

%inline %{
WebSiteResource *ResourceToWebSiteResource(Resource *r) {
        return r->getTypeId() == WebSiteResource::typeId ? static_cast<WebSiteResource*>(r) : NULL;
}

void DeleteVectorOfString(std::vector<std::string> *v) {
        delete v;
}

%}
