# WebResourceCreator.pm, input, perl
# Create WRs according to the list of url got in the config or in the file.
# 
# Dependencies: none
# 
# Parameters:
# items			r/o	Total items processed
# urlList		r/w	List of URLs (text or CDATA section)
# urlFile		r/w	File with list of URLs
# 
# Status:
# 0 (default of WR)

package WebResourceCreator;

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
		'_finished' => 0,
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

sub loadFile {
	my ($self, $file) = @_;
	if (defined open(my $fh, '<'.$file)) {
		my @lines = <$fh>;
		close($fh);
		push(@{$self->{'_url'}}, @{$self->createUrlList(join("\n", @lines))});
		$self->{'urlFile'} = $file;
		$self->{'_finished'} = 0;
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
	my @url;
	if (defined $self->{'urlList'}) {
		push(@{$self->{'_url'}}, @{$self->createUrlList($self->{'urlList'})});
		$self->{'_finished'} = 0;
	}
	if (defined $self->{'urlFile'}) {
		return 0 if (not $self->loadFile($self->{'urlFile'}));
	}
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
		if ($name eq 'urlList') {
			push(@{$self->{'_url'}}, @{$self->createUrlList($value)});
			$self->{'_finished'} = 0;
		} elsif ($name eq 'urlFile') {
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

sub ProcessInput() {
	my ($self, $resource) = @_;

	if (defined $resource) {
		$self->{'_object'}->log_error($resource->toStringShort()." Resource is already defined.");
		return undef;
	}
	if (@{$self->{'_url'}} == 0) {
		if (not $self->{'_finished'}) {
			$self->{'_object'}->log_info("Finished, total WebResources created: ".$self->{'items'});
			$self->{'_finished'} = 1;
		}
		return undef;
	}
	$resource = HectorRobot::WebResource->new();
	$resource->setId($self->{'_threadIndex'}*10000+$self->{'items'});
	my $url = shift(@{$self->{'_url'}});
	$url = "http://".$url if ($url !~ /^[[:alpha:]]+:\/\//);
	$resource->setUrl($url);
	$self->{'items'}++;
	return $resource;
}

1;
