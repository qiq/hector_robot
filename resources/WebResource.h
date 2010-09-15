/**
 * Class representing queue of resources (mainly HTML pages) while processing.
 * It uses Google Protocol Buffers to de/serialize.
 */

#ifndef _WEB_RESOURCE_H_
#define _WEB_RESOURCE_H_

#include <config.h>

#include <vector>
#include <string>
#include <log4cxx/logger.h>
#include "ProtobufResource.h"
#include "WebResource.pb.h"

class WebResource : public ProtobufResource {
public:
	WebResource();
	~WebResource() {};
	// create copy of a resource
	ProtobufResource *Clone();
	// type id of a resource (to be used by Resources::CreateResource(typeid))
	int getTypeId();
	// type string of a resource
	const char *getTypeStr();
	// id should be unique across all resources
	int getId();
	void setId(int id);
	// status may be tested in Processor to select target queue
	int getStatus();
	void setStatus(int status);
	// save and restore resource
	std::string *Serialize();
	bool Deserialize(std::string *s);
	int getSerializedSize();
	bool Serialize(google::protobuf::io::ZeroCopyOutputStream *output);
	bool Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size);
	// used by queues in case there is limit on queue size
	int getSize();
	// return string representation of the resource (e.g. for debugging purposes)
	char *toString();

	void setURL(const char *url);
	const char *getURL();

	static const int typeId = 10;

protected:
	hector::resources::WebResource r;

	static log4cxx::LoggerPtr logger;
};

inline int WebResource::getTypeId() {
	return typeId;
}

inline const char *WebResource::getTypeStr() {
	return "WebResource";
}

inline int WebResource::getId() {
	return r.id();
}

inline void WebResource::setId(int id) {
	r.set_id(id);
}

inline int WebResource::getStatus() {
	return r.status();
}

inline void WebResource::setStatus(int status) {
	r.set_status(status);
}

inline std::string *WebResource::Serialize() {
	return MessageSerialize(&r);
}

inline bool WebResource::Deserialize(std::string *s) {
	return MessageDeserialize(&r, s);
}

inline int WebResource::getSerializedSize() {
	return MessageGetSerializedSize(&r);
}

inline bool WebResource::Serialize(google::protobuf::io::ZeroCopyOutputStream *output) {
	return MessageGetSerializedSize(&r);
}

inline bool WebResource::Deserialize(google::protobuf::io::ZeroCopyInputStream *input, int size) {
	return MessageDeserialize(&r, input, size);
}

inline void WebResource::setURL(const char *url) {
	r.set_url(url);
}

inline const char *WebResource::getURL() {
	return r.url().c_str();
}

#endif
