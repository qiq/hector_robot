/**
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include <ev.h>
#include <limits>
#include "googleurl/src/gurl.h"
#include "UrlExtractor.h"
#include "WebResource.h"

using namespace std;

UrlExtractor::UrlExtractor(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	newUrlStatus = 2;
	imageLinks = false;

	values = new ObjectValues<UrlExtractor>(this);
	values->addGetter("items", &UrlExtractor::getItems);
	values->addGetter("newUrlStatus", &UrlExtractor::getNewUrlStatus);
	values->addSetter("newUrlStatus", &UrlExtractor::setNewUrlStatus);
	values->addGetter("imageLinks", &UrlExtractor::getImageLinks);
	values->addSetter("imageLinks", &UrlExtractor::setImageLinks);

	scanner_create(&state, &scanner);
}

UrlExtractor::~UrlExtractor() {
	scanner_destroy(scanner);

	delete values;
}

char *UrlExtractor::getItems(const char *name) {
	return int2str(items);
}

char *UrlExtractor::getNewUrlStatus(const char *name) {
	return int2str(newUrlStatus);
}

void UrlExtractor::setNewUrlStatus(const char *name, const char *value) {
	newUrlStatus = str2int(value);
}

char *UrlExtractor::getImageLinks(const char *name) {
	return bool2str(imageLinks);
}

void UrlExtractor::setImageLinks(const char *name, const char *value) {
	imageLinks = str2bool(value);
}

bool UrlExtractor::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;

	return true;
}

int UrlExtractor::ProcessMulti(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources) {
	ObjectLockRead();
	bool images = imageLinks;
	ObjectUnlock();
	while (inputResources->size() > 0) {
		if (inputResources->front()->getTypeId() != WebResource::typeId) {
			outputResources->push(inputResources->front());
		} else {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			outputResources->push(wr);
			GURL *base = new GURL(wr->getUrl());
			string *content = wr->getContentMutable();
			scanner_set_buffer(content->data(), content->size(), &state, scanner);
			char *text;
			token_type tok;
			while ((tok = scanner_scan(&text, scanner)) != TOK_EOF) {
				switch (tok) {
				case TOK_BASE: {
						delete base;
						// do not process anchors
						char *s = strchr(text, '#');
						if (s)
							*s = '\0';
						base = new GURL(text);
					}
					break;
				case TOK_IMG_URL:
					if (!images)
						break;
				case TOK_URL:
				case TOK_REDIRECT: {
						// do not process anchors
						char *s = strchr(text, '#');
						if (s)
							*s = '\0';
						GURL resolved = base->Resolve(text);
						if (resolved.is_valid()) {
							const string &url = resolved.spec();
							if (url.substr(0, 5) == "http:" || url.substr(0, 6) == "https:")
								urls.insert(url);
						}
					}
					break;
				default:
					LOG_ERROR(this, "Invalid scanner state: %d\n" << tok);
					break;
				}
			}
			delete base;
		}
		inputResources->pop();
	}

	ObjectLockRead();
	int newStatus = newUrlStatus;
	ObjectUnlock();
	for (tr1::unordered_set<string>::iterator iter = urls.begin(); iter != urls.end(); ++iter) {
		WebResource *tmp = new WebResource();
		tmp->setUrl(*iter);
		tmp->setStatus(newStatus);
		outputResources->push(tmp);
	}
	urls.clear();

	if (expectingResources)
		*expectingResources = 1000;
	return 0;
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new UrlExtractor(objects, id, threadIndex);
}
