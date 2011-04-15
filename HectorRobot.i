%module HectorRobot

%{
#undef New // protocol buffers + SWIG interaction
#undef die // Perl
#define SWIG_FILE_WITH_INIT // for Python

class TestResource;             // for Perl SWIG (otherwise does not compile
class TestResourceInfo;
class TestProtobufResource;
class TestProtobufResourceInfo;
class MarkerResource;
class MarkerResourceInfo;
class PerlResource;
%}

%include std_string.i
%include std_vector.i
%template(StringVector_hectorrobot) std::vector<std::string>;

%import Hector.i
%include WebResource.i
%include WebSiteResource.i
%include robot_common.i
