/**
 */
#include <config.h>

#include <string.h>
#include <sys/socket.h>
#include "googleurl/src/gurl.h"
#include "robot_common.h"
#include "UrlExtractor.h"
#include "PageResource.h"
#include "UrlResource.h"

using namespace std;

UrlExtractor::UrlExtractor(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	newUrlStatus = 0;
	imageLinks = false;
	allowedSchemes = "http";
	allowedSchemesSet.insert("http");

	props = new ObjectProperties<UrlExtractor>(this);
	props->Add("items", &UrlExtractor::GetItems);
	props->Add("newUrlStatus", &UrlExtractor::GetNewUrlStatus, &UrlExtractor::SetNewUrlStatus);
	props->Add("imageLinks", &UrlExtractor::GetImageLinks, &UrlExtractor::SetImageLinks);
	props->Add("allowedSchemes", &UrlExtractor::GetAllowedSchemes, &UrlExtractor::SetAllowedSchemes);

	scanner_create(&state, &scanner);

	urlResourceTypeId = Resource::GetRegistry()->NameToId("UrlResource");
}

UrlExtractor::~UrlExtractor() {
	scanner_destroy(scanner);

	delete props;
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

	if (!props->InitProperties(params))
		return false;

	return true;
}

void UrlExtractor::AppendUrl(const char *url, GURL *base, queue<Resource*> *outputResources) {
	bool do_free = false;
	const char *s = strchr(url, '#');
	if (s) {
		url = strndup(url, s-url);
		do_free = true;
	}
	GURL resolved = base->Resolve(url);
	if (resolved.is_valid()) {
		const string &url = resolved.spec();
		size_t colon = url.find_first_of(':');
		assert(colon != string::npos);
		string scheme = url.substr(0, colon);
		if (allowedSchemesSet.find(scheme) != allowedSchemesSet.end()) {
			// check whether we have already seen this URL
			ParsedUrl u(url);
			SitePathMD5 md5(u.GetSiteMD5(), u.GetPathMD5());
			if (seen.find(md5) == seen.end()) {
				// not seen
				UrlResource *ur = static_cast<UrlResource*>(Resource::GetRegistry()->AcquireResource(urlResourceTypeId));
				ur->SetUrl(url);
				ur->SetSiteMD5(u.GetSiteMD5());
				ur->SetPathMD5(u.GetPathMD5());
				outputResources->push(ur);
				seen.insert(md5);
			}
		}
	}
	if (do_free)
		free((void*)url);
}

bool UrlExtractor::ProcessMultiSync(queue<Resource*> *inputResources, queue<Resource*> *outputResources, int *expectingResources, int *processingResources) {
	while (inputResources->size() > 0) {
		if (!PageResource::IsInstance(inputResources->front())) {
			outputResources->push(inputResources->front());
		} else {
			PageResource *pr = static_cast<PageResource*>(inputResources->front());
			outputResources->push(pr);
			GURL *base = new GURL(pr->GetUrl());

			// redirection
			string status = pr->GetHeaderValue("X-Status");
			size_t offset = status.find(" ");
			if (offset != string::npos && status.at(offset+1) == '3') {
				string location = pr->GetHeaderValue("Location");
				if (!location.empty())
					AppendUrl(location.c_str(), base, outputResources);
			}

			// parse HTML-only pages
			string contentType = pr->GetHeaderValue("Content-Type");
			if (!contentType.compare(0, strlen("text/html"), "text/html")
				|| !contentType.compare(0, strlen("application/xhtml+xml"), "application/xhtml+xml")) {
				string *content = pr->GetContentMutable();
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
					case TOK_REDIRECT:
						AppendUrl(text, base, outputResources);
						break;
					default:
						LOG_ERROR(this, "Invalid scanner state: %d\n" << tok);
						break;
					}
				}
			}
			delete base;
			items++;
		}
		inputResources->pop();
	}

	if (expectingResources)
		*expectingResources = 1000;
	if (processingResources)
		*processingResources = 0;
	return false;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new UrlExtractor(objects, id, threadIndex);
}
