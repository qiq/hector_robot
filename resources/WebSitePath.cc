#include <config.h>

#include "WebSitePath.h"

using namespace std;

// get time of next refresh in seconds
int WebSitePath::NextRefresh() {
	int a = modifiedHistory >> 24;
	int b = (modifiedHistory >> 16) & 0xFF;
	int c = (modifiedHistory >> 8) & 0xFF;
	int d = modifiedHistory & 0xFF;
	int next = (a + b*2 + c*3 + d*4)/10;
	return floor(exp(next*log(1.5)));
}
