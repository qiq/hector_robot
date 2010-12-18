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

	values = new ObjectValues<Scheduler>(this);
	values->addGetter("items", &Scheduler::getItems);
	values->addGetter("outputDir", &Scheduler::getOutputDir);
	values->addSetter("outputDir", &Scheduler::setOutputDir);

	currentTime = 0;
}

Scheduler::~Scheduler() {
	CloseFiles();
	delete values;
}

char *Scheduler::getItems(const char *name) {
	return int2str(items);
}

char *Scheduler::getOutputDir(const char *name) {
	return strdup(outputDir);
}

void Scheduler::setOutputDir(const char *name, const char *value) {
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
		delete iter->second->file;
		close(iter->second->fd);
		delete iter->second;
	}
	openFiles.clear();
}

bool Scheduler::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!values->InitValues(params))
		return false;

	if (!outputDir || strlen(outputDir) < 1) {
		LOG_ERROR(this, "outputDir argument not defined");
		return false;
	}

	return true;
}

Resource *Scheduler::ProcessSimple(Resource *resource) {
	if (resource->getTypeId() != WebResource::typeId)
		return resource;
	WebResource *wr = static_cast<WebResource*>(resource);
	Resource *r = wr->getAttachedResource();
	if (r->getTypeId() != WebSiteResource::typeId)
		return wr;
	WebSiteResource *wsr = static_cast<WebSiteResource*>(r);

	uint32_t t = time(NULL);
	uint32_t now = t/1000;
	if (now > currentTime) {
		CloseFiles();
		currentTime = now;
	}
	// next update should be in 'next' seconds
	int next = wsr->PathNextModification(wr->getUrlPath().c_str())/1000;

	// is file already open?
	tr1::unordered_map<int, OpenFile*>::iterator iter = openFiles.find(now+next);
	OpenFile *of;
	if (iter == openFiles.end()) {
		// construct filename and open file
		char filename[1024];
		ObjectLockRead();
		snprintf(filename, sizeof(filename), "%s%d", outputDir, now+next);
		ObjectUnlock();
		int fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd < 0) {
			LOG_ERROR(this, "Cannot open file " << filename << ": " << strerror(errno));
			return wr;
		}
		of = new OpenFile;
		of->fd = fd;
		of->file = new google::protobuf::io::FileOutputStream(of->fd);
		of->stream = new google::protobuf::io::CodedOutputStream(of->file);
		openFiles[now+next] = of;
	} else {
		of = iter->second;
	}

	WebResource *other = new WebResource();
	other->setUrl(wr->getUrl());
	other->setScheduled(t);
	if (!Resource::Serialize(other, of->stream)) {
		LOG_ERROR_R(this, resource, "Error serializing resource");
		return resource;
	}
	delete other;

	return resource;
}

// factory functions

extern "C" Module* create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new Scheduler(objects, id, threadIndex);
}
