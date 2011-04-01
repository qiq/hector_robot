%include "std_string.i"

%{
#include "robot_common.h"
%}

bool ParseLabel(std::string &data, std::string &value);
bool ParseString(std::string &data, std::string &value, char separator);
bool ParseInt(std::string &data, int &value);
bool ParseLong(std::string &data, long &value);
bool ParseIp4(std::string &data, IpAddr &value);
bool ParseIp6(std::string &data, IpAddr &value);

long CountCksum(const char *data, int size);

std::string AbsolutizeUrl(const std::string &baseUrl, const std::string &url);
