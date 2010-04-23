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
#include "Resource.h"
#include "WebResource.pb.h"

using namespace std;

class WebResource : public Resource {
protected:
public:
	WebResource();
	~WebResource() {};
	// create copy of a resource
	Resource *Clone();
	// type id of a resource (to be used by Resources::CreateResource(typeid))
	int getType();
	// id should be unique across all resources
	int getId();
	void setId(int id);
	// status may be tested in Processor to select target queue
	int getStatus();
	void setStatus(int status);
	// save and restore resource
	string *serialize();
	bool deserialize(string *s);
	// used by queues in case there is limit on queue size
	int getSize();

	void setURL(const char *url);
	const char *getURL();

protected:
	hector::resources::WebResource r;

	static log4cxx::LoggerPtr logger;
};

inline int WebResource::getType() {
	return 1;
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

inline void WebResource::setURL(const char *url) {
	r.set_url(url);
}

inline const char *WebResource::getURL() {
	return r.url().c_str();
}

#endif
