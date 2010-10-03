/**
 * Common functions, macros, etc
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include "robot_common.h"

using namespace std;

void skipWs(string *data) {
	size_t offset = data->find_first_not_of(" \t\n\r");
	if (offset != string::npos)
		data->erase(0, offset);
	else
		data->clear();
}

bool parseLabel(string *data, string *value) {
	skipWs(data);
	size_t offset = 0;
	while (offset < data->length()) {
		int c = data->at(offset);
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
			break;
		offset++;
	}
	if (offset != string::npos) {
		if (offset == 0)
			return false;
		(*value) = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		(*value) = (*data);
		data->clear();
	}
	return true;
}

bool parseString(string *data, string *value, char separator) {
	skipWs(data);
	if (data->length() == 0)
		return false;
	int i = 0;
	if (data->at(0) == separator)
		i++;
	bool finished = false;
	bool escape = false;
	while (!finished && i < data->length()) {
		char c = data->at(i);
		if (c == '\\') {
			if (!escape) {
				escape = true;
			} else {
				value->append(1, '\\');
				escape = false;
			}
		} else if (c == separator) {
			if (!escape)
				finished = true;
			else
				value->append(1, separator);
		} else {
			value->append(1, data->at(i));
		}
		i++;
	}
	data->erase(0, i);
	return true;
}

bool parseInt(string *data, int *value) {
	skipWs(data);
	size_t offset = data->find_first_not_of("0123456789");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return false;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	*value = atoi(s.c_str());
	return true;
}

bool parseLong(string *data, int *value) {
	skipWs(data);
	size_t offset = data->find_first_not_of("0123456789");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return false;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	*value = atol(s.c_str());
	return true;
}

bool parseIp4(string *data, ip4_addr_t *value) {
	skipWs(data);
	size_t offset = data->find_first_not_of("0123456789.");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return false;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	bool result = inet_pton(AF_INET, s.c_str(), &value->addr) == 1;
	value->addr = ntohl(value->addr);
	return result;
}

bool parseIp6(string *data, ip6_addr_t *value) {
	skipWs(data);
	size_t offset = data->find_first_not_of("0123456789abcdefABCDEF:");
	string s;
	if (offset != string::npos) {
		if (offset == 0)
			return false;
		s = data->substr(0, offset);
		data->erase(0, offset);
	} else {
		s = (*data);
		data->clear();
	}
	bool result = inet_pton(AF_INET6, s.c_str(), &value->addr) == 1;
	for (int i = 0; i < 8; i++) {
		uint8_t tmp = value->addr[i];
		value->addr[i] = value->addr[15-i];
		value->addr[15-i] = tmp;
	}
	return result;
}
