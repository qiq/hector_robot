%newobject WebResource::Serialize();
%newobject WebResource::toString();

%{
#include "WebResource.h"
%}

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
        bool Deserialize(std::string *s);
        int getSerializedSize();
        bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
        bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
        int getSize();
        char *toString(Object::LogLevel = Object::INFO);

        void setUrl(const char *url);
        const char *getUrl();
        void clearUrl();
        void setTime(long time);
        long getTime();
        void clearTime();
        void setMimeType(const char *mimeType);
        const char *getMimeType();
        void clearMimeType();
        void setContent(const char *content);
        const char *getContent();
        void clearContent();
        void setHeaderFields(std::vector<std::string> *header_names, std::vector<std::string> *header_values);
        std::vector<std::string> *getHeaderNames();
        void setHeaderValue(const char *name, const char *value);
        const char *getHeaderValue(const char *name);
        void clearHeaderFields();
        void setExtractedUrls(std::vector<std::string> *extracted_urls);
        std::vector<std::string> *getExtractedUrls();
        void clearExtracedUrls();
        void setIp4Addr(ip4_addr_t addr);
        ip4_addr_t getIp4Addr();
        void clearIp4Addr();
        void setIp6Addr(ip6_addr_t addr);
        ip6_addr_t getIp6Addr();
        void clearIp6Addr();
        void setIpAddrExpire(long time);
        long getIpAddrExpire();
        void clearIpAddrExpire();

        void setUrlScheme(const char *urlScheme);
        const char *getUrlScheme();
        void clearUrlScheme();
        void setUrlUsername(const char *urlUsername);
        const char *getUrlUsername();
        void clearUrlUsername();
        void setUrlPassword(const char *urlPassword);
        const char *getUrlPassword();
        void clearUrlPassword();
        void setUrlHost(const char *urlHost);
        const char *getUrlHost();
        void clearUrlHost();
        void setUrlPort(int port);
        int getUrlPort();
        void clearUrlPort();
        void setUrlPath(const char *urlPath);
        const char *getUrlPath();
        void clearUrlPath();
        void setUrlQuery(const char *urlQuery);
        const char *getUrlQuery();
        void clearUrlQuery();
};
