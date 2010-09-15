
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));

WebResource::WebResource() {
}

ProtobufResource *WebResource::Clone() {
	return new WebResource(*this);
}

int WebResource::getSize() {
	return 1; //FIXME
}

char *WebResource::toString() {
	char buf[1024];
	snprintf(buf, sizeof(buf), "WebResource [%d, %d]: %s", this->getId(), this->getStatus(), this->getURL());
	return strdup(buf);
}

