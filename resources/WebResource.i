%include "std_string.i"

%newobject WebResource::Serialize();
%newobject WebResource::toString();

%{
#include "WebResource.h"
%}

%include "Scheme.i"

class WebResource : public Resource {
public:
        WebResource();
        ~WebResource();
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

        void setUrl(const std::string &url);
        const std::string &getUrl();
        void clearUrl();
        void setTime(long time);
        long getTime();
        void clearTime();
        void setMimeType(const std::string &mimeType);
        const std::string &getMimeType();
        void clearMimeType();
        void setContent(const std::string &content);
        const std::string &getContent();
        void clearContent();
        void setHeaderFields(std::vector<std::string> *header_names, std::vector<std::string> *header_values);
        std::vector<std::string> *getHeaderNames();
        void setHeaderValue(const std::string &name, const std::string &value);
        const std::string &getHeaderValue(const std::string &name);
        void clearHeaderFields();
        void setExtractedUrls(std::vector<std::string> *extracted_urls);
        std::vector<std::string> *getExtractedUrls();
        void clearExtractedUrls();
        void setIp4Addr(ip4_addr_t addr);
        ip4_addr_t getIp4Addr();
        void clearIp4Addr();
        void setIp6Addr(ip6_addr_t addr);
        ip6_addr_t getIp6Addr();
        void clearIp6Addr();
        void setIpAddrExpire(long time);
        long getIpAddrExpire();
        void clearIpAddrExpire();

        void setUrlScheme(int urlScheme);
        int getUrlScheme();
        void clearUrlScheme();
        void setUrlUsername(const std::string &urlUsername);
        const std::string &getUrlUsername();
        void clearUrlUsername();
        void setUrlPassword(const std::string &urlPassword);
        const std::string &getUrlPassword();
        void clearUrlPassword();
        void setUrlHost(const std::string &urlHost);
        const std::string &getUrlHost();
        void clearUrlHost();
        void setUrlPort(int port);
        int getUrlPort();
        void clearUrlPort();
        void setUrlPath(const std::string &urlPath);
        const std::string &getUrlPath();
        void clearUrlPath();
        void setUrlQuery(const std::string &urlQuery);
        const std::string &getUrlQuery();
        void clearUrlQuery();
};
