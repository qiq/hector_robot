/**
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include "googleurl/src/gurl.h"
#include "UrlExtractor.h"
#include "WebResource.h"

using namespace std;

UrlExtractor::UrlExtractor(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	newUrlStatus = 2;
	imageLinks = false;
	allowedSchemes = "http";
	allowedSchemesSet.insert("http");

	values = new ObjectValues<UrlExtractor>(this);
	values->AddGetter("items", &UrlExtractor::GetItems);
	values->AddGetter("newUrlStatus", &UrlExtractor::GetNewUrlStatus);
	values->AddSetter("newUrlStatus", &UrlExtractor::SetNewUrlStatus);
	values->AddGetter("imageLinks", &UrlExtractor::GetImageLinks);
	values->AddSetter("imageLinks", &UrlExtractor::SetImageLinks);
	values->AddGetter("allowedSchemes", &UrlExtractor::GetAllowedSchemes);
	values->AddSetter("allowedSchemes", &UrlExtractor::SetAllowedSchemes);

	scanner_create(&state, &scanner);

	webResourceTypeId = Resource::GetRegistry()->NameToId("WebResource");
}

UrlExtractor::~UrlExtractor() {
	scanner_destroy(scanner);

	delete values;
}

char *UrlExtractor::GetItems(const char *name) {
	return int2str(items);
}

char *UrlExtractor::GetNewUrlStatus(const char *name) {
	return int2str(newUrlStatus);
}

void UrlExtractor::SetNewUrlStatus(const char *name, const char *value) {
	newUrlStatus = str2int(value);
}

char *UrlExtractor::GetImageLinks(const char *name) {
	return bool2str(imageLinks);
}

void UrlExtractor::SetImageLinks(const char *name, const char *value) {
	imageLinks = str2bool(value);
}

char *UrlExtractor::GetAllowedSchemes(const char *name) {
	return strdup(allowedSchemes.c_str());
}

void UrlExtractor::SetAllowedSchemes(const char *name, const char *value) {
	allowedSchemes = value;
	allowedSchemesSet.clear();
        char *s = strdup(value);
        bool space = true;
        char *start = NULL;
        while (*s) {
                if (isspace(*s)) {
                        if (!space) {
                                *s = '\0';
                		allowedSchemesSet.insert(start);
                                space = true;
                        }
                } else {
                        if (space) {
                                space = false;
                                start = s;
                        }
                }
                s++;
        }
        if (!space)
                allowedSchemesSet.insert(start);
        free(s);
}

bool UrlExtractor::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;

	return true;
}

int UrlExtractor::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources) {
	while (inputResources->size() > 0) {
		if (!WebResource::IsInstance(inputResources->front())) {
			outputResources->push(inputResources->front());
		} else {
			WebResource *wr = static_cast<WebResource*>(inputResources->front());
			outputResources->push(wr);
			GURL *base = new GURL(wr->GetUrl());
			string *content = wr->GetContentMutable();
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
					if (!imageLinks)
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
							size_t colon = url.find_first_of(':');
							assert(colon != string::npos);
							string scheme = url.substr(0, colon);
							if (allowedSchemesSet.find(scheme) != allowedSchemesSet.end())
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

	for (tr1::unordered_set<string>::iterator iter = urls.begin(); iter != urls.end(); ++iter) {
		WebResource *tmp = static_cast<WebResource*>(Resource::GetRegistry()->AcquireResource(webResourceTypeId));
		tmp->SetUrl(*iter);
		tmp->SetStatus(newUrlStatus);
		outputResources->push(tmp);
	}
	urls.clear();
	items++;

	if (expectingResources)
		*expectingResources = 1000;
	return 0;
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new UrlExtractor(objects, id, threadIndex);
}
