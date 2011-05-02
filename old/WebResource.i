%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "WebResource.h"
%}

%include "Scheme.i"

class WebResourceInfo : public ResourceInfo {
public:
        WebResourceInfo();
};

class WebResource : public Resource {
public:
        WebResource();
        ~WebResource();
        Resource *Clone();
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input);
        int GetSize();
        ResourceInfo *GetResourceInfo();
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
        void SetScheduled(uint32_t time);
        uint32_t GetScheduled();
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

