# DetectNoAccents.pm, simple, perl
# Mark words (using TOKEN_UNRECOGNIZED) that are unaccented and do not occur in
# common text.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# unaccentedWordsFile	r/o	n/a	File containing unaccented words, one word per line.
# 
# Status:
# not changed

package DetectNoAccents;

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
	$self->{'unaccentedWordsFile'} = "";

	$self->{'_words'} = {};

	bless($self, $class);
	return $self;
}

sub Init {
	my ($self, $params) = @_;

	# second stage?
	return 1 if (not defined $params);

	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}

	if ($self->{'unaccentedWordsFile'} eq "") {
		$self->LOG_ERROR("unaccentedWordsFile not defined");
		return 0;
	}
	my $fd;
	if (!open($fd, "<".$self->{'unaccentedWordsFile'})) {
		$self->LOG_ERROR("Cannot open file: ".$self->{'unaccentedWordsFile'}.": $!";
		return 0;
	}
	while (<$fd>) {
		chomp;
		$self->{'_words'}->{$_} = 1;
	}
	close($fd);

	return 1;
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetWordCount();
	for (my $i = 0; $i < $nWords; $i++) {
		my $word = $tr->GetForm($i);
		if (exists $self->{'_words'}->{$word})
			$tr->SetFlags($i, $tr->GetFlags($i) | $HectorRobot::TOKEN_UNRECOGNIZED);
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
