# CreateTextResource.pm, input, perl
# Create TextResources
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# maxItems		r/o	Max resources to create
# 
# Status:
# 0 (default of TR)

package CreateTextResource;

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
		'_finished' => 0,
		'items' => 0,
		'maxItems' => 100,
		'mark' => 0,
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
	return $Hector::Module::INPUT;
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

sub ProcessInput() {
	my ($self, $resource) = @_;

	if (defined $resource) {
		$self->{'_object'}->log_error($resource->ToStringShort()." Resource is already defined.");
		return undef;
	}
	if ($self->{'items'} >= $self->{'maxItems'}) {
		if (not $self->{'_finished'}) {
			$self->{'_object'}->log_info("Finished, total TextResources created: ".$self->{'items'});
			$self->{'_finished'} = 1;
			if ($self->{'mark'}) {
				return &Hector::Resource::GetRegistry()->AcquireResource("MarkerResource");
			}
		}
		return undef;
	}
	$resource = &Hector::Resource::GetRegistry()->AcquireResource("TextResource");
	if (not defined $resource) {
		$self->{'_object'}->log_error("Cannot create resource type: TextResource");
		return undef;
	}
	$resource = HectorRobot::ResourceToTextResource($resource);
	$resource->SetId($self->{'_threadIndex'}*10000+$self->{'items'});
	$resource->SetText("Working example příliš");
	$resource->SetFlags(0, $HectorRobot::TOKEN_SENTENCE_START);
	$resource->SetForm(0, "Working");
	$resource->SetLemma(0, "work");
	$resource->SetPosTag(0, "V");
	$resource->SetHead(0, "1");
	$resource->SetDepRel(0, "test");
	for (my $i = 1; $i < 2; $i++) {
		$resource->SetFlags($i, $HectorRobot::TOKEN_NONE);
		$resource->SetForm($i, "example");
		$resource->SetLemma($i, "example");
		$resource->SetPosTag($i, "N");
		$resource->SetHead($i, "1");
		$resource->SetDepRel($i, "nounization");
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
