# to be included in bash
base=$BASH_SOURCE
base=`dirname "$base"`
base=`readlink -f "$base"`'/..'
base=`readlink -f "$base"`

. $base/setenv.sh

export "LD_LIBRARY_PATH=$base/module/.libs:$base/resource/.libs:$base/server/.libs:$base/perl/.libs:$base/python/.libs:$LD_LIBRARY_PATH"
export "PERL5LIB=$base/perl:$base/module/perl:$base/resource/perl:$PERL5LIB"
export "PYTHONPATH=$base/python:$base/module/python:$base/resource/python:$PYTHONPATH"
