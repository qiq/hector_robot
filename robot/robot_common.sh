base=..
export LD_LIBRARY_PATH=$base/resources:$base/modules:$base/servers:$base/perl/.libs:$LD_LIBRARY_PATH
export PERL5LIB=$base/perl:$base/modules/perl:$PERL5LIB

. hector_common.sh
