%include "std_string.i"
%include "std_vector.i"
%newobject WebResource::Serialize();

%{
#include "WebResource.h"
%}

%include "Scheme.i"

class WebResource : public Resource {
public:
        WebResource();
        ~WebResource();
        Resource *Clone();
        std::string *Serialize();
        bool Deserialize(const char *data, int size);
        int getSerializedSize();
        bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
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

        void setUrl(const std::string &url);
        const std::string &getUrl();
        void clearUrl();
        void ComposeUrl();
        void setIpAddr(IpAddr &addr);
        IpAddr &getIpAddr();
        void clearIpAddr();
        void setHeaderFields(const std::vector<std::string> &header_names, const std::vector<std::string> &header_values);
        std::vector<std::string> *getHeaderNames();
        void setHeaderValue(const std::string &name, const std::string &value);
        const std::string &getHeaderValue(const std::string &name);
        void clearHeaderFields();
        void setRedirectCount(int count);
        int getRedirectCount();
        void clearRedirectCount();
        void setContent(const std::string &content);
        const std::string &getContent();
        void clearContent();
        void setLastScheduled(long time);
        long getLastScheduled();
        void clearLastScheduled();

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
};

%inline %{
WebResource *ResourceToWebResource(Resource *r) {
        return dynamic_cast<WebResource*>(r);
}
%}

