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
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input);
        int GetTypeId();
        const char *GetTypeString(bool terse = false);
        const char *GetObjectName();
        int GetSize();
        std::string ToString(Object::LogLevel = Object::INFO);

        void SetUrl(const std::string &url);
        const std::string GetUrl();
        void ClearUrl();
        void ComposeUrl();
        void SetIpAddr(IpAddr &addr);
        IpAddr GetIpAddr();
        void ClearIpAddr();
        void SetHeaderFields(const std::vector<std::string> &header_names, const std::vector<std::string> &header_values);
        std::vector<std::string> *GetHeaderNames();
        std::vector<std::string> *GetHeaderValues();
        void SetHeaderValue(const std::string &name, const std::string &value);
        const std::string GetHeaderValue(const std::string &name);
        int GetRedirectCount();
        void ClearHeaderField(const std::string &name);
        void ClearHeader();
        void SetRedirectCount(int count);
        void ClearRedirectCount();
        void SetContent(const std::string &content);
        const std::string GetContent();
        void ClearContent();
        void SetScheduled(long time);
        long GetScheduled();
        void ClearScheduled();

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
WebResource *ResourceToWebResource(Resource *r) {
        return r && WebResource::IsInstance(r) ? static_cast<WebResource*>(r) : NULL;
}
%}

