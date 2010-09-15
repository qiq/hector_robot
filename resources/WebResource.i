%newobject TestProtobufResource::Serialize();
%newobject TestProtobufResource::toString();

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
        int getId();
        void setId(int id);
        int getStatus();
        void setStatus(int status);
        string *Serialize();
        bool Deserialize(string *s);
        int getSerializedSize();
        bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
        bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
        int getSize();
        char *toString();

        void setURL(const char *url);
        const char *getURL();

};
