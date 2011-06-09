#include <config.h>

#include "robot_common.h"
#include "googleurl/src/gurl.h"
#include "ParsedUrl.h"

using namespace std;

void ParsedUrl::LoadParsedUrl() {
	// parse URL
	GURL *gurl = new GURL(url);
	if (gurl->SchemeIs("http")) {
		scheme = HTTP;
	} else if (gurl->SchemeIs("https")) {
		scheme = HTTPS;
	} else {
		scheme = NONE;
	}
	username = gurl->username();
	password = gurl->password();
	host = gurl->host();
	port = gurl->EffectiveIntPort();
	string p = gurl->path();
	string q = gurl->query();
	if (!q.empty()) {
		p += "?";
		p += q;
	}
	path = p;
	delete gurl;

	parsed_url_ready = 1;
}

void ParsedUrl::SaveParsedUrl() {
	// construct url
	std::string s;
	int defaultPort;
	switch (scheme) {
	case HTTP:
		s = "http";
		defaultPort = 80;
		break;
	case HTTPS:
		s = "https";
		defaultPort = 443;
		break;
	case NONE:
	default:
		break;
	}
	s += "://";
	if (username != "") {
		s += username;
		s += ":";
		s += password;
		s += "@";
	}
	s += host;
	if (port != defaultPort) {
		char buffer[21];
		snprintf(buffer, sizeof(buffer), ":%d", port);
		s += buffer;
	}
	s += path;
	url = s;

	parsed_url_dirty = 0;
}
