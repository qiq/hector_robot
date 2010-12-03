#include <config.h>

#include "WebSitePath.h"

using namespace std;

// test whether path is ready to be fetched
bool WebSitePath::ReadyToFetch() {
	if (getPathStatus() == DISABLED)
		return false;
	return true;
}

void WebSitePath::UpdateError() {
}

bool WebSitePath::UpdateRedirect() {
}

bool WebSitePath::UpdateOK() {
}

// get time of next refresh in seconds
inline int WebSitePath::NextRefresh() {
}

