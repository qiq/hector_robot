%include "std_string.i"

%{
#include "robot_common.h"
%}

bool parseLabel(std::string *data, std::string *value);
bool parseString(std::string *data, std::string *value, char separator);
bool parseInt(std::string *data, int *value);
bool parseLong(std::string *data, long *value);
bool parseIp4(std::string *data, IpAddr *value);
bool parseIp6(std::string *data, IpAddr *value);

long CountCksum(const char *data, int size);

std::string AbsolutizeUrl(const std::string &baseUrl, const std::string &url);
