/**
 * Common functions, macros, etc
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include "robot_common.h"

using namespace std;

void skipWs(string *data) {
	size_t offset = data->find_first_not_of(" \t");
	if (offset != string::npos)
		data->erase(0, offset);
}

string parseLabel(string *data) {
	skipWs(data);
	size_t offset = 0;
	while (offset < data->length()) {
		int c = data->at(offset);
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '1') || c == '_'))
			break;
	}
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return NULL;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	return s;
}

string parseString(string *data, char separator) {
	skipWs(data);
	if (data->length() == 0)
		return "";
	int i = 0;
	if (data->at(0) == separator)
		i++;
	string result;
	bool finished = false;
	bool escape = false;
	while (!finished && i < data->length()) {
		char c = data->at(i);
		if (c == '\\') {
			if (!escape) {
				escape = true;
			} else {
				result.append(1, '\\');
				escape = false;
			}
		} else if (c == separator) {
			if (!escape)
				finished = true;
			else
				result.append(1, separator);
		} else {
			result.append(1, data->at(i));
		}
		i++;
	}
	data->erase(0, i);
	return result;
}

int parseInt(string *data) {
	skipWs(data);
	size_t offset = data->find_first_not_of("0123456789");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return -1;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	return atoi(s.c_str());
}

ip4_addr_t parseIp4(string *data) {
	ip4_addr_t addr;
	memset((void*)&addr, 0, sizeof(ip4_addr_t));
	size_t offset = data->find_first_not_of("0123456789.");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return addr;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	if (inet_pton(AF_INET, s.c_str(), &addr.addr) != 1)
		memset((void*)&addr, 0, sizeof(ip4_addr_t));
	return addr;
}

ip6_addr_t parseIp6(string *data) {
	ip6_addr_t addr;
	memset((void*)&addr, 0, sizeof(ip6_addr_t));
	size_t offset = data->find_first_not_of("0123456789.:");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return addr;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	if (inet_pton(AF_INET6, s.c_str(), &addr.addr) != 1)
		memset((void*)&addr, 0, sizeof(ip6_addr_t));
	return addr;
}
