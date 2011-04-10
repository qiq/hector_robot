/**
 * Class representing queue of resources (mainly HTML pages) while processing.
 * It uses Google Protocol Buffers to de/serialize.
 */

#ifndef _WEB_RESOURCE_WRAPPER_H_
#define _WEB_RESOURCE_WRAPPER_H_

#include <config.h>

#include "Resource.h"

extern "C" Resource* create();

#endif
