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
        char *toString();

        void setUrl(const char *url);
        const char *getUrl();
        void setTime(unsigned int time);
        unsigned int getTime();
        void setMimeType(const char *mimeType);
        const char *getMimeType();
        void setContent(const char *content);
        const char *getContent();
        void setHeaderFields(std::vector<std::string> *header_names, std::vector<std::string> *header_values);
        std::vector<std::string> *getHeaderNames();
        void setHeaderValue(const char *name, const char *value);
        const char *getHeaderValue(const char *name);
        void setExtractedUrls(std::vector<std::string> *extracted_urls);
        std::vector<std::string> *getExtractedUrls();

        void setUrlScheme(const char *urlScheme);
        const char *getUrlScheme();
        void setUrlHost(const char *urlHost);
        const char *getUrlHost();
        void setUrlPath(const char *urlPath);
        const char *getUrlPath();
};
