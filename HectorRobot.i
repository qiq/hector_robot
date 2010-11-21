%module HectorRobot

%{
#undef New // protocol buffers + SWIG interaction
#undef die // Perl
#define SWIG_FILE_WITH_INIT // for Python

class TestResource;             // for Perl SWIG (otherwise does not compile
class TestProtobufResource;
%}

%import Hector.i
%include WebResource.i
