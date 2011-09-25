# Unaccents.pm, simple, perl
# Remove all accents from words.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# 
# Status:
# not changed

package Unaccent;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use HectorRobotCommon;
use utf8;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;

	bless($self, $class);
	return $self;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	if ($nWords > 0) {
		for (my $i = 0; $i < $nWords; $i++) {
			my $form = $tr->GetForm($i);
			utf8::decode($form);
			$tr->SetForm($i, &HectorRobotCommon::unaccent($form));
		}
	} else {
		my $text = $tr->GetText();
		utf8::decode($text);
		$tr->SetText(&HectorRobotCommon::unaccent($text));
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
