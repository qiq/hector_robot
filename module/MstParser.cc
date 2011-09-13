/**
 */
#include <config.h>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include "robot_common.h"
#include "MstParser.h"
#include "TextResource.h"

using namespace std;

MstParser::MstParser(ObjectRegistry *objects, const char *id, int threadIndex): Module(objects, id, threadIndex) {
	items = 0;
	parserDir = NULL;
	model = NULL;
	decodeType = strdup("non-proj");
	order = 2;

	props = new ObjectProperties<MstParser>(this);
	props->Add("items", &MstParser::GetItems);
	props->Add("parserDir", &MstParser::GetParserDir, &MstParser::SetParserDir, true);
	props->Add("model", &MstParser::GetModel, &MstParser::SetModel, true);
	props->Add("decodeType", &MstParser::GetDecodeType, &MstParser::SetDecodeType, true);
	props->Add("order", &MstParser::GetOrder, &MstParser::SetOrder, true);

	pid = 0;
	fdin = -1;
	fdout = -1;
	fderr = -1;
}

MstParser::~MstParser() {
	if (pid) {
		string r, w;
		ReadWrite(r, w, false);
		kill(-pid, 9);
	}
	if (fdin != -1)
		close(fdin);
	if (fdout != -1)
		close(fdout);
	if (fderr != -1)
		close(fderr);
	delete props;
}

char *MstParser::GetItems(const char *name) {
	return int2str(items);
}

char *MstParser::GetParserDir(const char *name) {
	return strdup(parserDir);
}

void MstParser::SetParserDir(const char *name, const char *value) {
	free(parserDir);
	parserDir = strdup(value);
}

char *MstParser::GetModel(const char *name) {
	return strdup(model);
}

void MstParser::SetModel(const char *name, const char *value) {
	free(model);
	model = strdup(value);
}

char *MstParser::GetDecodeType(const char *name) {
	return strdup(decodeType);
}

void MstParser::SetDecodeType(const char *name, const char *value) {
	free(decodeType);
	decodeType = strdup(value);
}

char *MstParser::GetOrder(const char *name) {
	return int2str(order);
}

void MstParser::SetOrder(const char *name, const char *value) {
	order = str2int(value);
}

bool MstParser::Init(vector<pair<string, string> > *params) {
	// second stage?
	if (!params)
		return true;

	if (!props->InitProperties(params))
		return false;

	if (!parserDir) {
		LOG_ERROR(this, "parserDir argument not specified, cannot run parser.");
		return false;
	}

	if (!model) {
		LOG_ERROR(this, "model argument not specified, cannot run parser.");
		return false;
	}

	int pipein[2];
	int pipeout[2];
	int pipeerr[2];

	if (pipe(pipein) < 0) {
		LOG_ERROR(this, "Cannot open pipe: " << strerror(errno));
		return false;
	}
	if (pipe(pipeout) < 0) {
		LOG_ERROR(this, "Cannot open pipe: " << strerror(errno));
		return false;
	}
	if (pipe(pipeerr) < 0) {
		LOG_ERROR(this, "Cannot open pipe: " << strerror(errno));
		return false;
	}

	pid = fork();
	if (pid < 0) {
		LOG_ERROR(this, "Cannot fork: " << strerror(errno));
		return false;
	}
	if (pid == 0) {
		// child
		dup2(pipein[0], STDIN_FILENO);
		close(pipein[0]);
		close(pipein[1]);
		dup2(pipeout[1], STDOUT_FILENO);
		close(pipeout[0]);
		close(pipeout[1]);
		dup2(pipeerr[1], STDERR_FILENO);
		close(pipeerr[0]);
		close(pipeerr[1]);

		char command[1024*3];
		snprintf(command, sizeof(command), "java -classpath %s:%s/lib/trove.jar -Xmx8000m mstparser.DependencyParser 'model-name:%s' 'decode-type:%s' order:%d test test-file:/dev/stdin output-file:/dev/stdout format:MST", parserDir, parserDir, model, decodeType, order);
		//setsid();
		const char *args[] = { "/bin/sh", "-c", command, NULL };
		if (execvp("/bin/sh", (char *const *)args) == -1) {
			LOG_ERROR(this, "Cannot exec '" << command << "': " << strerror(errno));
		}
		exit(255);
	}
	// parent
	close(pipein[0]);
	close(pipeout[1]);
	close(pipeerr[1]);
	fdin = pipein[1];
	fdout = pipeout[0];
	fderr = pipeerr[0];

	return true;
}

bool MstParser::ReadWrite(string &writeBuffer, string &readBuffer, bool waitForRead) {
	bool writeDone = writeBuffer.length() == 0;
	bool readDone = !waitForRead;
	bool readStderrDone = false;
	int readBytes = 0;
	int writtenBytes = 0;
	bool timeout = false;
	bool failure = false;

	// ignore SIGPIPE
	struct sigaction old;
	struct sigaction nosigpipe;
	nosigpipe.sa_handler = SIG_IGN;
	sigemptyset(&nosigpipe.sa_mask);
	nosigpipe.sa_flags = 0;
	sigaction(SIGPIPE, &nosigpipe, &old);

	while (!failure && (!writeDone || !readDone || !timeout)) {
		fd_set read_fd;
		fd_set write_fd;
		FD_ZERO(&read_fd);
		FD_ZERO(&write_fd);
		int fdmax = 0;
		if (!writeDone) {
			FD_SET(fdin, &write_fd);
			if (fdin > fdmax)
				fdmax = fdin;
		}
		if (!readDone) {
			FD_SET(fdout, &read_fd);
			if (fdout > fdmax)
				fdmax = fdout;
		}
		if (!readStderrDone) {
			FD_SET(fderr, &read_fd);
			if (fderr > fdmax)
				fdmax = fderr;
		}
		struct timeval selectTimeout = { 0, 0 };
		int ready = select(fdmax+1, readDone && readStderrDone ? NULL : &read_fd, writeDone ? NULL : &write_fd, NULL, readDone && writeDone ? &selectTimeout : NULL);
		if (ready < 0) {
			LOG_ERROR(this, "Error select: " << strerror(errno));
			sigaction(SIGPIPE, &old, NULL);
			return false;
		}
		// timeout
		timeout = ready == 0 ? true : false;
		if (timeout)
			continue;
		// ready to write to stdin
		if (!writeDone && FD_ISSET(fdin, &write_fd)) {
			int w = write(fdin, writeBuffer.data() + writtenBytes, writeBuffer.size() - writtenBytes);
			if (w < 0) {
				LOG_INFO(this, "Error writing: " << strerror(errno));
				failure = true;
				writeDone = true;
			} else if (w > 0) {
				writtenBytes += w;
				if (writtenBytes == (int)writeBuffer.size())
					writeDone = true;
			}
		}
		// ready to read stdout
		if (!readDone && FD_ISSET(fdout, &read_fd)) {
			char buffer[1024];
			int r = read(fdout, buffer, sizeof(buffer));
			if (r < 0) {
				LOG_INFO(this, "Error reading: " << strerror(errno));
				failure = true;
				readDone = true;
			} else if (r > 0) {
				readBytes += r;
				readBuffer.append(buffer, r);
				// find out whether we have read enough (5) lines
				int nl = 0;
				size_t off = 0;
				while ((off = readBuffer.find_first_of('\n', off)) != string::npos) {
					nl++;
					off++;
				}
				if (nl >= 5)
					readDone = true;
			} else {
				failure = true;
				readDone = true;
			}
		}
		// ready to read stderr
		if (!readStderrDone && FD_ISSET(fderr, &read_fd)) {
			char buffer[1024];
			int r = read(fderr, buffer, sizeof(buffer));
			if (r < 0) {
				LOG_INFO(this, strerror(errno));
			} else if (r > 0) {
				stderrBuffer.append(buffer, r);
				// only print complete lines
				size_t nl = stderrBuffer.find_last_of('\n');
				if (nl != string::npos) {
					string msg(stderrBuffer, 0, nl);
					LOG_ERROR(this, msg);
					stderrBuffer.erase(0, nl+1);
				} else if (stderrBuffer.length() > 1000) {
					// line too long: print it anyway
					LOG_ERROR(this, stderrBuffer);
					stderrBuffer.clear();
				}
			} else {
				readStderrDone = true;
			}
		}
	}
	sigaction(SIGPIPE, &old, NULL);
	return !failure;
}

// first two positions or first and fifth
string &MstParser::GetShortTag(string &tag) {
	const char *data = tag.data();
	char fifth = data[4];
	tag.erase(2);
	if (fifth != '-')
		tag[1] = fifth;
	return tag;
}

bool MstParser::ParseSentence(vector<string> &form, vector<string> &tag, vector<int> &head, vector<string> &depRel) {
	head.clear();
	depRel.clear();
	string a;
	string b;
	for (int i = 0; i < (int)form.size(); i++) {
		if (i > 0) {
			a.append("\t");
			b.append("\t");
		}
		a.append("ExD");
		b.append("0");
	}
	string data = join('\t', form) + "\n" + join('\t', tag) + "\n" + a + "\n" + b + "\n\n";
	string result;
	bool error = false;
	if (ReadWrite(data, result, true)) {
		// parse result: forms, tags, afuns, parents, blank line
		vector<string> lines;
		split(lines, '\n', result);
		if (lines.size() == 5) {
			vector<string> headStr;
			split(headStr, '\t', lines[3]);
			for (int i = 0; i < (int)headStr.size(); i++)
				head.push_back(atoi(headStr[i].c_str()));
			split(depRel, '\t', lines[2]);
		} else {
			error = true;
		}
	} else {
		error = true;
	}
	if (error) {
		for (int i = 0; i < (int)form.size(); i++) {
			head.push_back(0);
			depRel.push_back("ExD");
		}
	}
	return error == false;
}

Resource *MstParser::ProcessSimpleSync(Resource *resource) {
	if (!TextResource::IsInstance(resource))
		return resource;
	TextResource *tr = static_cast<TextResource*>(resource);

	int nWords = tr->GetFormCount();
	int nTags = tr->GetPosTagCount();

	// for every sentence: create input data, write/read, parse the data
	vector<string> form;
	vector<string> tag;
	vector<int> head;
	vector<string> depRel;
	int offset = 0;
	for (int i = 0; i < nWords; i++) {
		if (tr->GetFlags(i) & TextResource::TOKEN_SENTENCE_START && form.size() > 0) {
			if (!ParseSentence(form, tag, head, depRel))
				LOG_ERROR(this, "Cannot parse sentence: " << join(' ', form));
			for (int j = 0; j < (int)head.size(); j++) {
				tr->SetHead(offset, head[j]);
				tr->SetDepRel(offset, depRel[j]);
				offset++;
			}
			form.clear();
			tag.clear();
		}
		form.push_back(tr->GetForm(i));
		if (i < nTags) {
			string t = tr->GetPosTag(i);
			tag.push_back(GetShortTag(t));
		} else {
			tag.push_back("X@-------------");
		}
	}
	if (form.size() > 0) {
		if (!ParseSentence(form, tag, head, depRel))
			LOG_ERROR(this, "Cannot parse sentence" << join(' ', form));
		for (int j = 0; j < (int)head.size(); j++) {
			tr->SetHead(offset, head[j]);
			tr->SetDepRel(offset, depRel[j]);
			offset++;
		}
	}

	return resource;
}

// factory functions

extern "C" Module* hector_module_create(ObjectRegistry *objects, const char *id, int threadIndex) {
	return new MstParser(objects, id, threadIndex);
}
