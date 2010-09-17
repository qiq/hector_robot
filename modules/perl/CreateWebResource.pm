# items: number of resources created
# urlList: list of URL (text or CDATA section)
# urlFile: file with list of URL

package CreateWebResource;

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
		'urlList' => undef,
		'urlFile' => undef,
		'_url' => [],
	};
	bless($self, $class);
	return $self;
}

sub DESTROY {
}

sub createUrlList {
	my ($self, $s) = @_;
	return [] if (not defined $s);
	my @a;
	foreach my $url (split(/[\n\r]+/, $s)) {
		$url =~ s/^\s+//;
		$url =~ s/\s+$//;
		next if ($url eq '');
		push(@a, $url);
	}
	return \@a;
}

sub Init {
	my ($self, $params) = @_;
	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}
	my @url;
	if (defined $self->{'urlList'}) {
		push(@url, @{$self->createUrlList($self->{'urlList'})});
	}
	if (defined $self->{'urlFile'}) {
		if (defined open(my $fh, '<'.$self->{'urlFile'})) {
			my @lines = <$fh>;
			close($fh);
			push(@url, @{$self->createUrlList(join("\n", @lines))});
		} else {
			$self->{'_object'}->log_error("Cannot open file: ".$self->{'urlFile'});
			return 0;
		}
	}
	$self->{'_url'} = \@url;
	return 1;
}

sub getType {
	my ($self) = @_;
	return $Hector::Module::INPUT;
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

sub Process() {
	my ($self, $resource) = @_;

	if (defined $resource) {
		$self->{'_object'}->log_error("Resource is already defined.");
		return undef;
	}
	if (@{$self->{'_url'}} == 0) {
		$self->{'_object'}->log_info("Finished, total WebResources created: ".$self->{'items'});
		return undef;
	}
	$resource = HectorRobot::WebResource->new();
	$resource->setId($self->{'_threadIndex'}*10000+$self->{'items'});
	$resource->setURL(shift(@{$self->{'_url'}}));
	$self->{'items'}++;
	return $resource;
}

sub ProcessMulti() {
	my ($self, $inputResources, $outputResources) = @_;

	$self->{'_object'}->log_error("ProcessMulti() is not implemented");

	return 0;
}

1;
