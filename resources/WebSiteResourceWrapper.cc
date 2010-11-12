
#include "WebSiteResourceWrapper.h"
#include "WebSiteResource.h"

using namespace std;

// the class factories

extern "C" Resource* create() {
	return (Resource*)new WebSiteResource();
}
