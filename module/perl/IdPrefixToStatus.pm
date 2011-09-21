# IdPrefixToStatus.pm, simple, perl
# Aids to split WebCorpus TextResources into files after deduplication.
# Sets status according to the TextId prefix.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# prefixRE			Regular expression to be used to distinguish
#				prefixes. Default: ^data\/[^\/]+/
# 
# Status:
# 0	set according to the prefix (1+)

package IdPrefixToStatus;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'prefixRE'} = "data\/[^\/]+/";

	$self->{'_prefixes'} = {};

	bless($self, $class);
	return $self;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);
	my $id = $tr->GetTextId();
	my $prefix = $self->{'prefixRE'};
	if ($id =~ s/(${prefix}).*/$1/) {
		if (not exists $self->{'_prefixes'}->{$id}) {
			$self->{'_prefixes'}->{$id} = 1+scalar(keys %{$self->{'_prefixes'}});
		}

		$tr->SetStatus($self->{'_prefixes'}->{$id});
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
