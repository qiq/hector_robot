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

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'items' => 0,
		'unaccentedWordsFile' => "",
		'_words' => {},
	};
	bless($self, $class);
	return $self;
}

sub DESTROY {
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
		$self->{'_object'}->log_error("unaccentedWordsFile not defined");
		return 0;
	}
	my $fd;
	if (!open($fd, "<".$self->{'unaccentedWordsFile'})) {
		$self->{'_object'}->log_error("Cannot open file: ".$self->{'unaccentedWordsFile'}.": $!";
		return 0;
	}
	while (<$fd>) {
		chomp;
		$self->{'_words'}->{$_} = 1;
	}
	close($fd);

	return 1;
}

sub GetType {
	my ($self) = @_;
	return $Hector::Module::SIMPLE;
}

sub GetProperty {
	my ($self, $name) = @_;
	if (exists $self->{$name}) {
		return $self->{$name};
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return undef;
	}
}

sub SetProperty {
	my ($self, $name, $value) = @_;
	if (exists $self->{$name}) {
		$self->{$name} = $value;
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return 0;
	}
	return 1;
}

sub ListProperties {
	my ($self) = @_;
	return [ grep { $_ !~ /^_/ } keys %{$self} ];
}

sub SaveCheckpoint {
	my ($self, $path, $id) = @_;
	$self->{'_object'}->log_info("SaveCheckpoint($path, $id)");
}

sub RestoreCheckpoint {
	my ($self, $path, $id) = @_;
	$self->{'_object'}->log_info("RestoreCheckpoint($path, $id)");
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = 0;

	for (my $i = 0; $i < $nWords; $i++) {
		my $word = $tr->GetForm($i);
		if (exists $self->{'_words'}->{$word})
			$tr->SetFlags($i, $tr->GetFlags($i) | $HectorRobot::TOKEN_UNRECOGNIZED);
	}

	
	$self->{'items'}++;
	return $resource;
}

sub Start() {
	my ($self) = @_;
}

sub Stop() {
	my ($self) = @_;
}

sub Pause() {
	my ($self) = @_;
}

sub Resume() {
	my ($self) = @_;
}

1;
