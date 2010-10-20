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

bool parseLabel(std::string *data, std::string *value);
bool parseString(std::string *data, std::string *value, char separator);
bool parseInt(std::string *data, int *value);
bool parseLong(std::string *data, long *value);
bool parseIp4(std::string *data, ip4_addr_t *value);
bool parseIp6(std::string *data, ip6_addr_t *value);

#endif
