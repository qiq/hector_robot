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
        bool Deserialize(const char *data, int size);
        int getSerializedSize();
        bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
        bool SerializeWithCachedSizes(google::protobuf::io::ZeroCopyOutputStream *output);
        bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
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
        void setUrlScheme(int urlScheme);
        int getUrlScheme();
        void clearUrlScheme();
        void setUrlHost(const std::string &urlHost);
        const std::string &getUrlHost();
        void clearUrlHost();
        void setUrlPort(int urlPort);
        int getUrlPort();
        void clearUrlPort();
        void setIp4Addr(ip4_addr_t addr);
        ip4_addr_t getIp4Addr();
        void clearIp4Addr();
        void setIp6Addr(ip6_addr_t addr);
        ip6_addr_t getIp6Addr();
        void clearIp6Addr();
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
};

%inline %{
WebSiteResource *ResourceToWebSiteResource(Resource *r) {
        return dynamic_cast<WebSiteResource*>(r);
}
%}
