%module Resources

%{
#undef New // protocol buffers + SWIG interaction
#define SWIG_FILE_WITH_INIT // for Python
#include "Resources.h"
%}

Resource *ConstructResource(resource_t type, string *serial = NULL);
WebResource *Resource2WebResource(Resource *resource);
