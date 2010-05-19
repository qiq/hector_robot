
#include "WebResource.h"
#include "WebResource.pb.h"

using namespace std;

log4cxx::LoggerPtr WebResource::logger(log4cxx::Logger::getLogger("lib.processing_engine.WebResource"));

WebResource::WebResource() {
	// check library vs header file versions
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

ProtobufResource *WebResource::Clone() {
	return new WebResource(*this);
}

int WebResource::getSize() {
	return 1; //FIXME
}
