%include "std_string.i"

%newobject WebSiteResource::Serialize();
%newobject WebSiteResource::toString();

%{
#include "WebSiteResource.h"
%}

%include "Scheme.i"

class WebSiteResource : public Resource {
public:
        WebSiteResource();
        ~WebSiteResource();
        Resource *Clone();
        int getTypeId();
        const char *getTypeStr();
        const char *getModuleStr();
        int getId();
        void setId(int id);
        int getStatus();
        void setStatus(int status);
        std::string *Serialize();
        bool Deserialize(const char *data, int size);
        int getSerializedSize();
        bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
        bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
        int getSize();
        char *toString(Object::LogLevel = Object::INFO);

        void setHostname(const std::string &url);
        const std::string &getHostname();
        void clearHostname();
        void setIp4Addr(ip4_addr_t addr);
        ip4_addr_t getIp4Addr();
        void clearIp4Addr();
        void setIp6Addr(ip6_addr_t addr);
        ip6_addr_t getIp6Addr();
        void clearIp6Addr();
        void setIpAddrExpire(long time);
        long getIpAddrExpire();
        void clearIpAddrExpire();

        void setAllowUrls(std::vector<std::string> *allow_urls);
        std::vector<std::string> *getAllowUrls();
        void clearAllowUrls();
        void setDisallowUrls(std::vector<std::string> *disallow_urls);
        std::vector<std::string> *getDisallowUrls();
        void clearDisallowUrls();
        void setRobotsExpire(long time);
        long getRobotsExpire();
        void clearRobotsExpire();
};
