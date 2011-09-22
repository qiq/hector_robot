# DetectNoAccentsTrain.pm, simple, perl
# Extract top N words that do not occur un-accented (save un-accented variant of words).
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# maxWords		r/o	n/a	Max number of words to save in a file.
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
use HectorRobotCommon;

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'maxWords'} = 100000;
	$self->{'unaccentedWordsFile'} = "";

	$self->{'_fd'} = undef;
	$self->{'_freq'} = {};

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
	if (!open($self->{'_fd'}, ">".$self->{'unaccentedWordsFile'})) {
		$self->LOG_ERROR("Cannot open file: ".$self->{'unaccentedWordsFile'}.": $!";
		return 0;
	}

	return 1;
}

sub DESTROY {
	my @a = sort { $self->{'_freq'}->{$b} <=> $self->{'_freq'}->{$a} } (keys %{$self->{'_freq'}});
	my @result;
	foreach my $a (@a) {
		my $da = &deaccent($a);
		next if ($da eq $a);
		next if (defined $freq{$da} and $self->{'_freq'}->{$da} > $self->{'_freq'}->{$a}*0.1);
		push(@result, $da);
	}
	foreach my $r (splice(@result, 0, $self->{'maxWords'})) {
		print $self->{'_fd'} "$r\n";
	}
	
	close($self->{'_fd'});
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetWordCount();
	for (my $i = 0; $i < $nWords; $i++) {
		my $word = $tr->GetForm($i);
		$self->{'_freq'}->{$word}++;
	}
	
	$self->{'items'}++;
	return $resource;
}

1;
