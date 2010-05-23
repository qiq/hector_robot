
#include "WebResourceWrapper.h"
#include "WebResource.h"

using namespace std;

// the class factories

extern "C" Resource* create() {
	return (Resource*)new WebResource();
}
