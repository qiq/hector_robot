/**
 */
#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Scheduler.h"
#include "WebResource.h"
#include "WebSiteResource.h"

using namespace std;

Scheduler::Scheduler(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	outputDir = NULL;

	props = new ObjectProperties<Scheduler>(this);
	props->Add("items", &Scheduler::GetItems);
	props->Add("outputDir", &Scheduler::GetOutputDir, &Scheduler::SetOutputDir);

	currentTime = 0;
	webResourceTypeId = Resource::GetRegistry()->NameToId("WebResource");
}

Scheduler::~Scheduler() {
	CloseFiles();
	delete props;
}

char *Scheduler::GetItems(const char *name) {
	return int2str(items);
}

char *Scheduler::GetOutputDir(const char *name) {
	return strdup(outputDir);
}

void Scheduler::SetOutputDir(const char *name, const char *value) {
	free(outputDir);
	if (value && strlen(value) > 0 && value[strlen(value)-1] != '/') {
		outputDir = (char*)malloc(strlen(value)+2);
		if (outputDir) {
			strcpy(outputDir, value);
			strcat(outputDir, "/");
		}
	} else {
		outputDir = strdup(value);
	}
	CloseFiles();
}

void Scheduler::CloseFiles() {
	// close all files
	for (tr1::unordered_map<int, OpenFile*>::iterator iter = openFiles.begin(); iter != openFiles.end(); ++iter) {
		delete iter->second->stream;
		close(iter->second->fd);
		delete iter->second;
	}
	openFiles.clear();
}

bool Scheduler::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	if (!outputDir || strlen(outputDir) < 1) {
		LOG_ERROR(this, "outputDir argument not defined");
		return false;
	}

	return true;
}

Resource *Scheduler::ProcessSimpleSync(Resource *resource) {
	if (!WebResource::IsInstance(resource))
		return resource;
	WebResource *wr = static_cast<WebResource*>(resource);
	Resource *r = wr->GetAttachedResource();
	if (!WebSiteResource::IsInstance(r))
		return wr;
	WebSiteResource *wsr = static_cast<WebSiteResource*>(r);

	// next update should be in 'next' seconds
	long n = wsr->PathNextRefresh(wr->GetUrlPath().c_str());
	if (n < 0)
		return resource;

	uint32_t t = time(NULL);
	uint32_t now = t/1000;
	if (now > currentTime) {
		CloseFiles();
		currentTime = now;
	}
	int next = n/1000;
	if (next == 0)
		next = 1;

	// is file already open?
	tr1::unordered_map<int, OpenFile*>::iterator iter = openFiles.find(now+next);
	OpenFile *of;
	if (iter == openFiles.end()) {
		// construct filename and open file
		char filename[1024];
		snprintf(filename, sizeof(filename), "%s%d", outputDir, now+next);
		int fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd < 0) {
			LOG_ERROR(this, "Cannot open file " << filename << ": " << strerror(errno));
			return wr;
		}
		of = new OpenFile;
		of->fd = fd;
		of->stream = new ResourceOutputStream(of->fd);
		openFiles[now+next] = of;
	} else {
		of = iter->second;
	}

	WebResource *other = static_cast<WebResource*>(Resource::GetRegistry()->AcquireResource(webResourceTypeId));
	other->SetUrl(wr->GetUrl());
	other->SetScheduled(t);
	if (!Resource::Serialize(other, *of->stream, false)) {
		LOG_ERROR_R(this, resource, "Error serializing resource");
		return resource;
	}
	Resource::GetRegistry()->ReleaseResource(other);
	items++;
	LOG_DEBUG_R(this, wr, "Scheduling " << wr->GetUrl() << " " << next << " (" << now+next << ")");

	return resource;
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Scheduler(objects, id, threadIndex);
}
