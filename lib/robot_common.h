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

typedef struct {
	uint32_t addr;
} ip4_addr_t;

typedef struct {
	uint8_t addr[16];
} ip6_addr_t;

void skipWs(std::string *data);
std::string parseLabel(std::string *data);
std::string parseString(std::string *data, bool separator);
int parseInt(std::string *data);
ip4_addr_t parseIp4(std::string *data);
ip6_addr_t parseIp6(std::string *data);

#endif
