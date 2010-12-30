# Filter.pm, simple, perl
# Run a Perl script to filter resources, it may use any resource's methods.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# filter		r/w	Filter as specified in the XML file (text or CDATA section)
# filterFile		r/w	Filter in the file (replaces filter)
# 
# Status:
# 0 or as set in the filter

package Filter;

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
		'filter' => undef,
		'filterFile' => undef,
	};
	bless($self, $class);
	return $self;
}

sub DESTROY {
}

sub loadFile {
	my ($self, $file) = @_;
	if (defined open(my $fh, '<'.$file)) {
		my @lines = <$fh>;
		close($fh);
		$self->{'filter'} = join("\n", @lines);
		$self->{'filterFile'} = $file;
		return 1;
	} else {
		$self->{'_object'}->log_error("Cannot open file: ".$file);
		return 0;
	}
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
	if (defined $self->{'filterFile'}) {
		return 0 if (not $self->loadFile($self->{'filterFile'}));
	}
	return 1;
}

sub getType {
	my ($self) = @_;
	return $Hector::Module::SIMPLE;
}

sub getValueSync {
	my ($self, $name) = @_;
	if (exists $self->{$name}) {
		return $self->{$name};
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return undef;
	}
}

sub setValueSync {
	my ($self, $name, $value) = @_;
	if (exists $self->{$name}) {
		$self->{$name} = $value;
		if ($name eq 'filterFile') {
			return 0 if (not $self->loadFile($value));
		}
	} else {
		$self->{'_object'}->log_error("Invalid value name: $name");
		return 0;
	}
	return 1;
}

sub listNamesSync {
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

$Filter::resource = undef;
$Filter::object = undef;

sub log_trace() {
	my ($msg) = @_;
	$Filter::object->log_trace($Filter::resource->toStringShort." ".$msg);
}

sub log_debug() {
	my ($msg) = @_;
	$Filter::object->log_debug($Filter::resource->toStringShort." ".$msg);
}

sub log_info() {
	my ($msg) = @_;
	$Filter::object->log_info($Filter::resource->toStringShort." ".$msg);
}

sub log_error() {
	my ($msg) = @_;
	$Filter::object->log_error($Filter::resource->toStringShort." ".$msg);
}

sub log_fatal() {
	my ($msg) = @_;
	$Filter::object->log_fatal($Filter::resource->toStringShort." ".$msg);
}

sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if (not defined $self->{'filter'});

	$Filter::resource = $resource;
	$Filter::object = $self->{'_object'};

	eval $self->{'filter'};
	&log_error($@) if ($@);

	$self->{'items'}++;
	return $resource;
}

1;
