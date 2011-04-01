/**
 * Common functions, macros, etc
 */

#ifndef _LIB_ROBOT_COMMON_H_
#define _LIB_ROBOT_COMMON_H_

#include <config.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <log4cxx/logger.h>
#include "common.h"
#include "IpAddr.h"
#include "SuperFastHash.h"

bool ParseLabel(std::string &data, std::string &value);
bool ParseString(std::string &data, std::string &value, char separator);
bool ParseInt(std::string &data, int &value);
bool ParseLong(std::string &data, long &value);
bool ParseIp4(std::string &data, IpAddr &value);
bool ParseIp6(std::string &data, IpAddr &value);

// convert integer to string (at least 11 bytes)
char* itoa(int value, char* str);

// count fast hash
inline long CountCksum(const char *data, int size) {
	return (long)SuperFastHash(data, size);
}

std::string AbsolutizeUrl(const std::string &baseUrl, const std::string &url);

#endif
