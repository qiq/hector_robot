# CreatePageResource.pm, input, perl
# Create PRs according to the list of url got in the config or in the file.
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

package CreatePageResource;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('INPUT', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'urlList'} = undef;
	$self->{'urlFile'} = undef;
	$self->{'mark'} = 0;

	$self->{'_url'} = [];
	$self->{'_finished'} = 0;

	bless($self, $class);
	return $self;
}

sub CreateUrlList {
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

sub LoadFile {
	my ($self, $file) = @_;
	if (defined open(my $fh, '<'.$file)) {
		my @lines = <$fh>;
		close($fh);
		push(@{$self->{'_url'}}, @{$self->CreateUrlList(join("\n", @lines))});
		$self->{'urlFile'} = $file;
		$self->{'_finished'} = 0;
		return 1;
	} else {
		$self->LOG_ERROR("Cannot open file: ".$file);
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
		push(@{$self->{'_url'}}, @{$self->CreateUrlList($self->{'urlList'})});
		$self->{'_finished'} = 0;
	}
	if (defined $self->{'urlFile'}) {
		return 0 if (not $self->LoadFile($self->{'urlFile'}));
	}

	return 1;
}

sub SetProperty {
	my ($self, $name, $value) = @_;
	if (exists $self->{$name}) {
		$self->{$name} = $value;
		if ($name eq 'urlList') {
			push(@{$self->{'_url'}}, @{$self->CreateUrlList($value)});
			$self->{'_finished'} = 0;
		} elsif ($name eq 'urlFile') {
			return 0 if (not $self->LoadFile($value));
		}
	} else {
		$self->LOG_ERROR("Invalid value name: $name");
		return 0;
	}
	return 1;
}

sub ProcessInput() {
	my ($self, $resource) = @_;

	if (defined $resource) {
		$self->LOG_ERROR($resource, "Resource is already defined.");
		return undef;
	}
	if (@{$self->{'_url'}} == 0) {
		if (not $self->{'_finished'}) {
			$self->LOG_INFO("Finished, total PageResources created: ".$self->{'items'});
			$self->{'_finished'} = 1;
			if ($self->{'mark'}) {
				return &Hector::Resource::GetRegistry()->AcquireResource("MarkerResource");
			}
		}
		return undef;
	}
	$resource = &Hector::Resource::GetRegistry()->AcquireResource("PageResource");
	if (not defined $resource) {
		$self->LOG_ERROR("Cannot create resource type: PageResource");
		return undef;
	}
	$resource = HectorRobot::ResourceToPageResource($resource);
	$resource->SetId($self->{'_threadIndex'}*10000+$self->{'items'});
	my $url = shift(@{$self->{'_url'}});
	$url = "http://".$url if ($url !~ /^[[:alpha:]]+:\/\//);
	$resource->SetUrl($url);
	$self->{'items'}++;
	return $resource;
}

1;
