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

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = {
		'_object' => $object,
		'_id' => $id,
		'_threadIndex' => $threadIndex,
		'items' => 0,
		'prefixRE' => "data\/[^\/]+/",
		'_prefixes' => {},
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
